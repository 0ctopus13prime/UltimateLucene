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

#ifndef SRC_INDEX_EXCEPTION_H_
#define SRC_INDEX_EXCEPTION_H_

#include <stdexcept>
#include <string>

namespace lucene {
namespace core {
namespace index {

class CorruptIndexException: public std::runtime_error {
 public:
  CorruptIndexException()
    : std::runtime_error("CorruptIndexException") {
  }

  explicit CorruptIndexException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit CorruptIndexException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class IndexFormatTooOldException: public std::runtime_error {
 public:
  IndexFormatTooOldException()
    : std::runtime_error("IndexFormatTooOldException") {
  }

  explicit IndexFormatTooOldException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit IndexFormatTooOldException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class IndexFormatTooNewException: public std::runtime_error {
 public:
  IndexFormatTooNewException()
    : std::runtime_error("IndexFormatTooNewException") {
  }

  explicit IndexFormatTooNewException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit IndexFormatTooNewException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

}  // namespace index
}  // namespace core
}  // namespace lucene

#endif  // SRC_INDEX_EXCEPTION_H_
