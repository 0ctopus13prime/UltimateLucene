#include <limits.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <Util/Exception.h>
#include <Store/Lock.h>

using lucene::core::store::FSLockFactory;
using lucene::core::store::NativeFSLockFactory;
using lucene::core::store::Lock;
using lucene::core::util::UnsupportedOperationException;

/**
 *  FSDirectory
 */
std::unique_ptr<Lock>
FSLockFactory::ObtainLock(Directory& directory, const std::string& lock_name) {
  if (FSDirectory* ptr = dynamic_cast<FSDirectory*>(&directory)) {
    return ObtainFSLock(*ptr, lock_name);
  } else {
    throw UnsupportedOperationException(
    "Classes that inherited from FSLockFactory "
    "can only be used with FSDirectory");
  }
}

/**
 *  NativeFSLockFactory
 */

const std::shared_ptr<NativeFSLockFactory>
NativeFSLockFactory::INSTANCE(new NativeFSLockFactory());

const std::set<std::string> LOCK_HELD();

std::unique_ptr<Lock>
NativeFSLockFactory::ObtainFSLock(FSDirectory& dir,
                                  const std::string& lock_name) {
  const std::string& path = dir.GetDirectory();
  // Make the directory
  mkdir(path.c_str(),
        S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

  const std::string lock_file = path + "/" + lock_name;
  {
    // Make the lock file
    const int lock_file_fd = open(lock_file.c_str(),
                             O_CREAT | O_CLOEXEC | O_EXCL,
                             // 755
                             S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (lock_file_fd != -1) {
      close(lock_file_fd);
    }
  }

  char path_buf[PATH_MAX + 1];
  const std::string abs_lock_file(realpath(lock_file.c_str(), path_buf));

  const int lock_file_fd =
  open(abs_lock_file.c_str(), O_CLOEXEC | O_RDONLY);
  const int flock_result = flock(lock_file_fd, LOCK_EX | LOCK_NB);
  if (flock_result == -1) {
    close(lock_file_fd);
    throw lucene::core::util::IOException(
    std::string("Lock held by another program: ") + abs_lock_file);
  }

  struct stat sb;
  stat(abs_lock_file.c_str(), &sb);

  return std::make_unique<NativeFSLock>(lock_file_fd,
                                        abs_lock_file,
                                        sb.st_ctime);
}


/**
 *  NativeFSLock
 */
NativeFSLockFactory::NativeFSLock::~NativeFSLock() {
  Close();  
}

NativeFSLockFactory::NativeFSLock::NativeFSLock(const int fd,
                                          const std::string& abs_lock_file,
                                          const time_t ctime)
  : fd(fd),
    abs_lock_file(abs_lock_file),
    ctime(ctime),
    closed(false) {
}

void NativeFSLockFactory::NativeFSLock::Close() {
  std::lock_guard<std::mutex> guard(close_mutex);
  if (closed) {
    return;
  }

  flock(fd, LOCK_UN);
  close(fd);

  closed = true;
  NativeFSLockFactory::ClearLockHeld(abs_lock_file);
}
