#include <sys/stat.h>
#include <sys/mman.h>
#include <Index/File.h>
#include <Util/Exception.h>
#include <Util/File.h>
#include <Store/DataInput.h>
#include <Store/Directory.h>
#include <Store/Exception.h>
#include <Store/Lock.h>
#include <algorithm>

using lucene::core::index::IndexFileNames;
using lucene::core::store::AlreadyClosedException;
using lucene::core::store::IndexInput;
using lucene::core::store::ChecksumIndexInput;
using lucene::core::store::IndexOutput;
using lucene::core::store::Lock;
using lucene::core::store::LockFactory;
using lucene::core::store::Directory;
using lucene::core::store::IOUtils;
using lucene::core::store::BaseDirectory;
using lucene::core::store::FSDirectory;
using lucene::core::store::MMapDirectory;
using lucene::core::store::FSLockFactory;
using lucene::core::store::ByteBufferIndexInput;
using lucene::core::util::FileUtil;
using lucene::core::util::IOException;
using lucene::core::util::NoSuchFileException;

/**
 *  IOUtils
 */
void
IOUtils::DeleteFilesIgnoringExceptions(Directory& dir,
                                       const std::vector<std::string>& files) {
  for (const std::string& file_name : files) {
    try {
      dir.DeleteFile(file_name);
    } catch(...) {
      // Nothing
    }
  }
}

/**
 *  Directory
 */
void Directory::CopyFrom(Directory& from,
                         const std::string& src,
                         const std::string& dest,
                         const IOContext& context) {
  // If we copy from another file to another file
  // then we can use native approach like `sendfile`?
  // It is much more faster and efficient

  try {
    std::unique_ptr<IndexInput> is(from.OpenInput(src, context));
    std::unique_ptr<IndexOutput> os(CreateOutput(dest, context));
    os->CopyBytes(*(is.get()), is->Length());
  } catch(...) {
    IOUtils::DeleteFilesIgnoringExceptions(*this, {});
  }
}

std::unique_ptr<ChecksumIndexInput>
Directory::OpenChecksumInput(const std::string& name,
                             const IOContext& context) {
  return
  std::make_unique<BufferedChecksumIndexInput>(OpenInput(name, context));
}

/**
 *  BaseDirectory
 */
BaseDirectory::BaseDirectory(const std::shared_ptr<LockFactory>& lock_factory)
  : Directory(),
    is_open(true),
    lock_factory(lock_factory) {
}
  
void BaseDirectory::EnsureOpen() {
  if (!is_open) {
    throw AlreadyClosedException("This Directory is closed");      
  }
}

std::unique_ptr<Lock> BaseDirectory::ObtainLock(const std::string& name) {
  return lock_factory->ObtainLock(*this, name);
}

/**
 *  FSDirectory
 */
FSDirectory::FSDirectory(const std::string path,
            const std::shared_ptr<LockFactory>& lock_factory)
  : BaseDirectory(lock_factory),
    directory(),
    pending_deletes(),
    ops_since_last_delete(),
    next_temp_file_counter() {
  if (!lucene::core::util::FileUtil::IsDirectory(path)) {
    lucene::core::util::FileUtil::CreateDirectory(path); 
  }

  directory = lucene::core::util::FileUtil::ToRealPath(path);
}

void FSDirectory::EnsureCanRead(const std::string& name) {
  if (pending_deletes.find(name) != pending_deletes.end()) {
    throw lucene::core::util::NoSuchFileException(
          std::string("File \"") +
          name +
          "\" is pending delete and cannot be opened for read");
  }
}

void FSDirectory::MaybeDeletePendingFiles() {
  if (!pending_deletes.empty()) {
    DeletePendingFiles();
  }
}

void FSDirectory::PrivateDeleteFile(const std::string& name,
                                  const bool is_pending_delete) {
  const std::string resolved = FileUtil::ToRealPath(directory + '/' + name);

  try {
    FileUtil::Delete(resolved);
    pending_deletes.erase(name);
  } catch (lucene::core::util::NoSuchFileException&) {
    pending_deletes.erase(name);
  } catch (lucene::core::util::IOException&) {
    pending_deletes.insert(name);
  }
}

const std::string& FSDirectory::GetDirectory() {
  EnsureOpen();
  return directory;
}

std::vector<std::string>
FSDirectory::ListAllWithSkipNames(const std::string& dir,
                                  const std::set<std::string>& skip_names) {
  std::vector<std::string> all_files =
  lucene::core::util::FileUtil::ListFiles(dir);

  std::vector<std::string> ret;
  ret.reserve(all_files.size());

  for (const std::string& name : all_files) {
    if (skip_names.find(name) == skip_names.end()) {
      ret.push_back(name); 
    }
  }

  std::sort(ret.begin(), ret.end());
  return ret;
}

void FSDirectory::Fsync(const std::string& name) {
  FileUtil::Fsync(name);
}

std::vector<std::string> FSDirectory::ListAll() {
  std::vector<std::string> ret =
  FileUtil::ListFiles(directory);
  std::sort(ret.begin(), ret.end());
  return ret;
}

uint64_t FSDirectory::FileLength(const std::string& name) {
  return FileUtil::Size(directory + '/' + name);
}

std::unique_ptr<IndexOutput>
FSDirectory::CreateOutput(const std::string& name, const IOContext& context) {
  EnsureOpen();
  pending_deletes.erase(name);
  return std::make_unique<FileIndexOutput>(
         std::string("FileIndexOutput(path=\"") + directory + '/' + name,
         name,
         directory + '/' + name);
}

std::unique_ptr<IndexOutput> 
FSDirectory::CreateTempOutput(const std::string& prefix,
                              const std::string& suffix,
                              const IOContext& context) {
  EnsureOpen();
  MaybeDeletePendingFiles();
  std::string path = directory + '/';
  const uint32_t path_prefix_len = path.length();
  std::string name;

  do {
    path.resize(path_prefix_len);
    name = IndexFileNames::SegmentFileName(
           prefix,
           suffix + '_' + std::to_string(next_temp_file_counter++),
           "tmp");
    if (pending_deletes.find(name) != pending_deletes.end()) {
      continue;
    }

    path += name;    
  } while(FileUtil::Exists(path));

  return std::make_unique<FileIndexOutput>(
         std::string("FileIndexOutput(path=\"") + path,
         name,
         path);
}

void FSDirectory::Sync(const std::vector<std::string>& names) {
  EnsureOpen();
  for (const std::string& name : names) {
    Fsync(directory + '/' + name);
  }

  MaybeDeletePendingFiles();
}

void FSDirectory::Rename(const std::string& source, const std::string& dest) {
  EnsureOpen();
  if (pending_deletes.find(source) != pending_deletes.end()) {
    throw NoSuchFileException(std::string("File \"") +
                              source +
                              "\" is pending delete and cannot be moved");
  }

  pending_deletes.erase(dest);
  FileUtil::Move(directory + '/' + source,
                 directory + '/' + dest);
  MaybeDeletePendingFiles();
}

void FSDirectory::SyncMetaData() {
  EnsureOpen();
  FileUtil::Fsync(directory);
  MaybeDeletePendingFiles();
}

void FSDirectory::Close() {
  // TODO(0ctopus13prime): Not synchronized. Make it thread safe
  is_open = false;
  DeletePendingFiles();
}

void FSDirectory::DeleteFile(const std::string& name) {
  if (pending_deletes.find(name) != pending_deletes.end()) {
    throw NoSuchFileException(std::string("File \"") +
                              name +
                              "\" is already pending delete");
  }
  PrivateDeleteFile(name, false);
  MaybeDeletePendingFiles();
}

bool FSDirectory::CheckPendingDeletions() {
  DeletePendingFiles();
  return !pending_deletes.empty();
}

// TODO(0ctopus13prime): Not synchronized. Make it thread safe
void FSDirectory::DeletePendingFiles() {
  if (!pending_deletes.empty()) {
    for (const std::string& name : pending_deletes) {
      PrivateDeleteFile(name, true); 
    }
  }
}

/**
 *  MMapDirectory
 */

MMapDirectory::MMapDirectory(const std::string& path)
  : MMapDirectory(path, FSLockFactory::GetDefault()) {
}

MMapDirectory::MMapDirectory(const std::string& path,
              const std::shared_ptr<LockFactory>& lock_factory)
  : FSDirectory(path, lock_factory) {
}

std::unique_ptr<IndexInput> MMapDirectory::OpenInput(const std::string& name,
                                                     const IOContext& context) {
  EnsureOpen();
  EnsureCanRead(name);
  const std::string abs_path(
  lucene::core::util::FileUtil::ToRealPath(directory + "/" + name));

  const int fd = open(abs_path.c_str(), O_RDONLY);
  struct stat sb;
  if (fstat(fd, &sb) == -1 ) {
    throw IOException("Failed to open " + abs_path);
  }
  char* addr = static_cast<char*>(mmap(NULL,
                                       sb.st_size,
                                       PROT_READ,
                                       MAP_PRIVATE,
                                       fd,
                                       0));
  if (addr == MAP_FAILED) {
    throw IOException("Failed to map " + abs_path);
  }

  close(fd);
  if (preload) {
    madvise(static_cast<void*>(addr), sb.st_size, MADV_WILLNEED);
  }

  std::string resource_desc("MMapIndexInput(path=\"");
  resource_desc += abs_path;
  resource_desc += '\"';
  return std::make_unique<ByteBufferIndexInput>(resource_desc,
                                                addr,
                                                sb.st_size);
}
