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

namespace lucene {
namespace core {
namespace util {

class InvalidStateException: std::runtime_error {
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

class UnsupportedOperationException: std::runtime_error {
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

class IOException: std::runtime_error {
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

class IllegalArgumentException: std::runtime_error {
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

class EOFException: std::runtime_error {
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

}  // store
}  // core
}  // lucene

#endif  // SRC_STORE_EXCEPTION_H_
