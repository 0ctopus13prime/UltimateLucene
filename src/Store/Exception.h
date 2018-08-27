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
#ifndef SRC_STORE_EXCEPTION_H_
#define SRC_STORE_EXCEPTION_H_

#include <stdexcept>
#include <string>

namespace lucene {
namespace core {
namespace store {

class AlreadyClosedException: std::runtime_error {
 public:
  AlreadyClosedException()
    : std::runtime_error("AlreadyClosedException") {
  }

  explicit AlreadyClosedException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit AlreadyClosedException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_STORE_EXCEPTION_H_
