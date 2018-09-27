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

#ifndef SRC_UTIL_UNICODE_H_
#define SRC_UTIL_UNICODE_H_

#include <limits>
#include <Util/Ref.h>
#include <Util/Exception.h>

namespace lucene {
namespace core {
namespace util {

class UnicodeUtil {
 private:
  const static int32_t V;
  const static int32_t UTF8_CODE_LENGTH[];

 private:
  UnicodeUtil() = default;

 public:
  static int32_t UTF8toUTF32(const BytesRef& utf8, int32_t*& ints) {
    uint32_t utf32_count = 0;    
    uint32_t utf8_upto = utf8.Offset(); 
    const char* bytes = utf8.Bytes();
    uint32_t utf8_limit = (utf8.Offset() + utf8.Length());

    while (utf8_upto < utf8_limit) {
      const int32_t num_bytes = UTF8_CODE_LENGTH[bytes[utf8_upto] & 0xFF];
      int32_t v = 0;
      
      switch (num_bytes) {
        case 1:
          ints[utf32_count++] = bytes[utf8_upto++];
          continue;
        case 2:
          v = bytes[utf8_upto++] & 31;
          break;
        case 3:
          v = bytes[utf8_upto++] & 15;
          break;
        case 4:
          v = bytes[utf8_upto++] & 7;
          break;
        default:
          throw IllegalArgumentException("Invalid UTF8");
      }

      int32_t limit = utf8_upto + num_bytes - 1;
      while (utf8_upto < limit) {
        v = (v << 6 | (bytes[utf8_upto++] & 63));
      }

      ints[utf32_count++] = v;
    }

    return utf32_count;
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // 
