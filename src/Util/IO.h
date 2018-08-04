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

#ifndef SRC_UTIL_IO_H_
#define SRC_UTIL_IO_H_

#include <Store/Directory.h>
#include <initializer_list>
#include <string>

namespace lucene {
namespace core {
namespace util {

class IOUtils {
 private:
  IOUtils() { }

 public:
  static void
  DeleteFilesIgnoringExceptions(const lucene::core::store::Directory& dir,
                                std::vector<std::string>& files) {
    for (const std::string& file_name : files) {
      try {
        dir.DeleteFile(file_name);
      } catch(...) {
        // Nothing
      }
    }
  }
};

}  // util
}  // core
}  // lucene

#endif  // SRC_UTIL_IO_H_
