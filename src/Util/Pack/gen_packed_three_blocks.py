#!/usr/bin/python

# Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


HEADER_FILE = "PackedThreeBlocks.h"
LICENSE = """
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
""".lstrip()

HEAD = """
// This file has been automatically generated, DO NOT EDIT

#ifndef SRC_UTIL_PACK_PACKEDTHREEBLOCKS_H_
#define SRC_UTIL_PACK_PACKEDTHREEBLOCKS_H_

#include <Store/DataInput.h>
#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <algorithm>
#include <memory>

namespace lucene {
namespace core {
namespace util {
"""

TAIL = """
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKEDTHREEBLOCKS_H_
"""

TYPES = {8: "char", 16: "uint16_t"}
METHODS = {8: "Int8", 16: "Int16"}
MASKS = {8: " & 0xFFL", 16: " & 0xFFFFL", 32: " & 0xFFFFFFFFL", 64: ""}
CASTS = {8: "static_cast<char> ", 16: "static_cast<uint16_t> ", 32: "static_cast<uint32_t> ", 64: ""}

f = None

def l(line):
  global f
  f.write(line)

def ln(line):
  global f
  l(line + "\n")

def start():
  global f
  f = open(HEADER_FILE, "w")
  l(LICENSE)
  l(HEAD)

def end():
  global f
  l(TAIL)
  f.close()

def gen():
  for bpv in TYPES.keys():
    type
    l("\nclass Packed%dThreeBlocks : public PackedInts::MutableImpl {" % bpv)
    ln(
"""
 public:
  static const uint32_t MAX_SIZE = 0x7FFFFFFF / 3;

 private:
  std::unique_ptr<%s[]> blocks;
  uint32_t blocks_size;

 public:
  explicit Packed%dThreeBlocks(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, %d),
               blocks(),
               blocks_size(value_count) {
    if (value_count > MAX_SIZE) {
      throw ArrayIndexOutOfBoundsException("MAX_SIZE exceeded");
    }

    blocks = std::make_unique<%s[]>(value_count);
  }

  Packed%dThreeBlocks(const uint32_t packed_ints_version,
                      lucene::core::store::DataInput* in,
                      const uint32_t value_count)
    : Packed%dThreeBlocks(value_count) {""" % (TYPES[bpv],
                                               bpv,
                                               bpv * 3,
                                               TYPES[bpv],
                                               bpv,
                                               bpv))

    if bpv == 8:
      ln("    in->ReadBytes(blocks.get(), 0, 3 * value_count);");
    else:
      ln(
"""    for (uint32_t i = 0 ; i < 3 * value_count ; ++i) {
      blocks[i] = in->Read%s();
    }""" % METHODS[bpv].title())
    ln("  }")

    l(
"""
  int64_t Get(const uint32_t index) {
    const uint32_t o = index * 3;
    return ((blocks[o]%s) << %d |
            (blocks[o + 1]%s) << %d |
            (blocks[o + 2]%s));
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    uint32_t o = off;
    for (uint32_t i = index * 3, end = (index + gets) * 3 ; i < end ; i += 3) {
      arr[o++] = (blocks[i]%s << %d) |
                 (blocks[i + 1]%s) << %d |
                 (blocks[i + 2]%s);
    }

    return gets;
  }

  void Set(const uint32_t index, const int64_t value) {
    const uint32_t o = (index * 3);
    blocks[o] = %s(value >> %d);
    blocks[o + 1] = %s(value >> %d);
    blocks[o + 2] = %s(value);
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    for (uint32_t i = off, o = index * 3, end = off + sets ; i < end ; ++i) {
      const int64_t value = arr[i];
      blocks[o++] = %s(value >> %d);
      blocks[o++] = %s(value >> %d);
      blocks[o++] = %s(value);
    }

    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t value) {
    const %s block1 = %s(value >> %d);
    const %s block2 = %s(value >> %d);
    const %s block3 = %s(value);

    for (uint32_t i = from_index * 3, end = to_index * 3 ; i < end ; i += 3) {
      blocks[i] = block1;
      blocks[i + 1] = block2;
      blocks[i + 2] = block3;
    }
  }

  void Clear() {
    std::memset(blocks.get(), 0, blocks_size);
  }
""" % (MASKS[bpv],
       bpv * 2,
       MASKS[bpv],
       bpv,
       MASKS[bpv],
       MASKS[bpv],
       bpv * 2,
       MASKS[bpv],
       bpv,
       MASKS[bpv],
       CASTS[bpv],
       bpv * 2,
       CASTS[bpv],
       bpv,
       CASTS[bpv],
       CASTS[bpv],
       bpv * 2,
       CASTS[bpv],
       bpv,
       CASTS[bpv],
       TYPES[bpv],
       CASTS[bpv],
       bpv * 2,
       TYPES[bpv],
       CASTS[bpv],
       bpv,
       TYPES[bpv],
       CASTS[bpv]))

    ln("};")  # End of class definition

if __name__ == "__main__":
  start()
  gen()
  end()
