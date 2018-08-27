/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SRC_STORE_LOCK_H_
#define SRC_STORE_LOCK_H_

#include <Store/Directory.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace lucene {
namespace core {
namespace store {

class FSLockFactory: public LockFactory {
 public:
  static std::shared_ptr<FSLockFactory> GetDefault() {
    // TODO(0ctopus13prime): Fix this
    return std::shared_ptr<FSLockFactory>();
  }

 protected:
  virtual std::unique_ptr<Lock>
  ObtainFSLock(FSDirectory& dir, const std::string& lock_name) = 0;

 public:
  FSLockFactory() = default;

  ~FSLockFactory() = default;

  std::unique_ptr<Lock> ObtainLock(Directory& directory,
                                   const std::string& lock_name);
};

class NativeFSLockFactory: public FSLockFactory {
 private:
  static void ClearLockHeld(const std::string& path) {
    // TODO(0ctopus13prime): Fix this
  }

 private:
  class NativeFSLock: public Lock {
   private:
    int fd;
    std::string abs_lock_file;
    time_t ctime;
    std::atomic_bool closed;
    std::mutex close_mutex;

   public:
    NativeFSLock(const int fd,
                 const std::string& abs_lock_file,
                 const time_t ctime);

    NativeFSLock(const NativeFSLock& other) = delete;

    NativeFSLock(NativeFSLock&& other) = delete;

    NativeFSLock& operator=(const NativeFSLock& other) = delete;

    NativeFSLock& operator=(NativeFSLock&& other) = delete;

    ~NativeFSLock();

    void EnsureValid() {
      // TODO(0ctopus13prime): Fix this
    }

    void Close();
  };

 public:
  static const std::shared_ptr<NativeFSLockFactory> INSTANCE;

 private:
  // TODO(0ctopus13prime): Make this thread safe
  static const std::set<std::string> LOCK_HELD;

 private:
  NativeFSLockFactory() = default;

 protected:
  std::unique_ptr<Lock> ObtainFSLock(FSDirectory& dir,
                                     const std::string& lock_name);
};

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_STORE_LOCK_H_
