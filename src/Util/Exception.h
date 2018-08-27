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
#ifndef SRC_UTIL_EXCEPTION_H_
#define SRC_UTIL_EXCEPTION_H_

#include <stdexcept>
#include <string>

namespace lucene {
namespace core {
namespace util {

class InvalidStateException: public std::runtime_error {
 public:
  InvalidStateException()
    : std::runtime_error("InvalidStateException") {
  }

  explicit InvalidStateException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit InvalidStateException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class UnsupportedOperationException: public std::runtime_error {
 public:
  UnsupportedOperationException()
    : std::runtime_error("UnsupportedOperationException") {
  }

  explicit UnsupportedOperationException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit UnsupportedOperationException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class IOException: public std::runtime_error {
 public:
  IOException()
    : std::runtime_error("IOException") {
  }

  explicit IOException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit IOException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class IllegalArgumentException: public std::runtime_error {
 public:
  IllegalArgumentException()
    : std::runtime_error("IllegalArgumentException") {
  }

  explicit IllegalArgumentException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit IllegalArgumentException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class EOFException: public std::runtime_error {
 public:
  EOFException()
    : std::runtime_error("EOFException") {
  }

  explicit EOFException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit EOFException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

class NoSuchFileException: public std::runtime_error {
 public:
  NoSuchFileException()
    : std::runtime_error("NoSuchFileException") {
  }

  explicit NoSuchFileException(const std::string& err_msg)
    : std::runtime_error(err_msg) {
  }

  explicit NoSuchFileException(const char* err_msg)
    : std::runtime_error(err_msg) {
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_EXCEPTION_H_
