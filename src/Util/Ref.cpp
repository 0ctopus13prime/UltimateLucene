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

#include <assert.h>
#include <Util/ArrayUtil.h>
#include <Util/Ref.h>
#include <Util/Unicode.h>
#include <cstring>
#include <memory>
#include <utility>
#include <stdexcept>
#include <string>

using lucene::core::util::BytesRef;
using lucene::core::util::BytesRefBuilder;
using lucene::core::util::IntsRefBuilder;
using lucene::core::util::UnicodeUtil;

/*
 * BytesRef
 */

char* BytesRef::DEFAULT_BYTES = new char[1];

uint32_t BytesRef::CompareTo(const BytesRef& other) const {
  if (IsValid() && other.IsValid()) {
    if (bytes == other.bytes
          && offset == other.offset
          && length == other.length) {
      return 0;
    }

    uint32_t my_len = length - offset;
    uint32_t his_len = other.length - other.offset;
    uint32_t len = (my_len < his_len ? my_len : his_len);
    char* my_bytes_ptr = bytes.get();
    char* his_bytes_ptr = other.bytes.get();

    for (uint32_t i = 0 ; i < len ; ++i) {
      char mine = my_bytes_ptr[i];
      char his = his_bytes_ptr[i];
      char diff = (mine - his);
      if (diff != 0) {
        return diff;
      }
    }

    // One is a prefix of another, or, they are equal.
    return (my_len - his_len);
  }

  throw IllegalStateException(
                "This BytesRef is not valid or"
                "other BytesRef is not valid during CompareTo");
}

/**
 *  IntsRefBuilder 
 */

void IntsRefBuilder::CopyUTF8Bytes(const BytesRef& bytes) {
  Grow(bytes.length);
  ref.length = UnicodeUtil::UTF8toUTF32(bytes, ref.ints);
}
