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

namespace lucene {
namespace core {
namespace store {

class Lock {
 public:
  virtual void Close();
  virtual void EnsureValid();
};

}  // store
}  // core
}  // lucene

#endif  // SRC_STORE_LOCK_H_
