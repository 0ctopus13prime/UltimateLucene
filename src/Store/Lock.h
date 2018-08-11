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
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace store {

class Directory;
class FSDirectory;

class Lock {
 public:
  Lock() = default;
  virtual ~Lock() = default;
  virtual void Close() = 0;
  virtual void EnsureValid() = 0;
};

class LockFactory {
 public:
  LockFactory() = default;
  virtual ~LockFactory() = default;
  virtual std::unique_ptr<Lock>
  ObtainLock(Directory& dir, const std::string& lock_name) = 0;
};

class FSLockFactory: public LockFactory {
 public:
  static std::unique_ptr<FSLockFactory> MakeDefault() {
    return std::unique_ptr<FSLockFactory>();
  }

 protected:
  virtual std::unique_ptr<Lock>
  ObtainFSLock(FSDirectory& dir, const std::string& lock_name) = 0;

 public:
  FSLockFactory() = default;

  ~FSLockFactory() = default;

  std::unique_ptr<Lock>
  ObtainLock(Directory& directory, const std::string& lock_name) {
    return std::unique_ptr<Lock>();
  }
};


}  // store
}  // core
}  // lucene

#endif  // SRC_STORE_LOCK_H_
