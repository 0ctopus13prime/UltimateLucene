#include <sys/stat.h>
#include <sys/mman.h>
#include <Util/Exception.h>
#include <Util/File.h>
#include <Store/DataInput.h>
#include <Store/Directory.h>
#include <Store/Exception.h>
#include <Store/Lock.h>

using lucene::core::store::AlreadyClosedException;
using lucene::core::store::IndexInput;
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
using lucene::core::util::IOException;

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
  try {
    std::unique_ptr<IndexInput> is = from.OpenInput(src, context);
    std::unique_ptr<IndexOutput> os = CreateOutput(dest, context);
    os->CopyBytes(*is, is->Length());
  } catch(...) {
    IOUtils::DeleteFilesIgnoringExceptions(*this, {});
  }
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

const std::string& FSDirectory::GetDirectory() {
  EnsureOpen();
  return directory;
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
  const char* addr = static_cast<const char*>(mmap(NULL,
                                                   sb.st_size,
                                                   PROT_READ,
                                                   MAP_PRIVATE,
                                                   fd,
                                                   0));
  if (addr == MAP_FAILED) {
    throw IOException("Failed to map " + abs_path);
  }

  close(fd);

  std::string resource_desc("MMapIndexInput(path=\"");
  resource_desc += abs_path;
  resource_desc += '\"';
  return std::make_unique<ByteBufferIndexInput>(resource_desc, addr, sb.st_size);
}
