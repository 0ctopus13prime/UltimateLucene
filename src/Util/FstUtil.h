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

#ifndef SRC_UTIL_FSTUTIL_H_
#define SRC_UTIL_FSTUTIL_H_

#include <Util/Ref.h>
#include <Util/Fst.h>

namespace lucene {
namespace core {
namespace util {

class FSTUtil {
 private:
  FSTUtil() = default;

 public:
  template <typename T>
  static T Get(FST<T>& fst, BytesRef& input) {
    return T();
  }
};

}  // util
}  // core
}  // lucene

#endif  // SRC_UTIL_FSTUTIL_H_
