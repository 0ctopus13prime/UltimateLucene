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


HEADER_FILE = "Direct.h"
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

#ifndef SRC_UTIL_PACK_DIRECT_H_
#define SRC_UTIL_PACK_DIRECT_H_

#include <Store/DataInput.h>
#include <Util/Pack/PackedInts.h>
#include <cstring>
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

#endif  // SRC_UTIL_PACK_DIRECT_H_
"""

TYPES = {8: "uint8_t", 16: "uint16_t", 32: "uint32_t", 64: "uint64_t"}
MASKS = {8: " & 0xFFL", 16: " & 0xFFFFL", 32: " & 0xFFFFFFFFL", 64: ""}
CASTS = {8: "static_cast<uint8_t> ", 16: "static_cast<uint16_t> ", 32: "static_cast<uint32_t> ", 64: ""}

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

def gen_direct():
  for bpv in TYPES.keys():
    type
    ln("\nclass Direct%d : public PackedInts::MutableImpl {" % bpv)
    ln(" private:")
    ln("  std::unique_ptr<%s[]> values;" % TYPES[bpv])
    ln("  uint32_t values_size;")
    ln("\npublic:")
    ln("  explicit Direct%d(const uint32_t value_count)" % bpv)
    ln("             : PackedInts::MutableImpl(value_count, %d)," % bpv)
    ln("               values(std::make_unique<%s[]>(value_count))," % TYPES[bpv])
    ln("               values_size(value_count) {")
    ln("  }")

    ln(
"""
  Direct%d(const uint32_t packed_ints_version,
           lucene::core::store::DataInput* in,
           const uint32_t value_count)""" % bpv)
    ln("    : PackedInts::MutableImpl(value_count, %d)," % bpv)
    ln("      values(std::make_unique<%s[]>(value_count))," % TYPES[bpv])
    ln("      values_size(value_count) {")
    ln(
"""
    for (uint32_t i = 0 ; i < value_count ; ++i) {
      values[i] = in->ReadInt16();
    }

    const uint32_t remaining = static_cast<uint32_t>(
      PackedInts::Format::PACKED.ByteCount(packed_ints_version, value_count, %d) - %dL * value_count);
    for (uint32_t i = 0 ; i < remaining ; ++i) {
      in->ReadByte();
    }
  }""" % (bpv, bpv / 8))

    ln(
"""
  int64_t Get(const uint32_t index) {
    return values[index]%s;
  }

  void Set(const uint32_t index, const int64_t value) {
    values[index] = %s(value);
  }

  void Clear() {
    std::memset(values.get(), 0, sizeof(%s) * values_size); 
  }""" % (MASKS[bpv], CASTS[bpv], TYPES[bpv]));

    if bpv == 64:
      ln(
"""
  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    std::memcpy(arr + off, values.get() + index, gets);
    return gets;
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    std::memcpy(values.get() + index, arr + off, sets);
    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t val) {
    std::fill_n(values.get() + from_index, to_index - from_index, val);
  }""")

    else:
      ln(
"""
  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    std::copy(values.get() + index,
              values.get() + index + gets,
              arr + off);
    return gets;
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    std::copy(arr + off,
              arr + off + sets,
              values.get() + index);
    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t val) {
    std::fill_n(values.get() + from_index, to_index - from_index, %s(val));
  }""" % CASTS[bpv])

    ln("};")

if __name__ == "__main__":
  start()
  gen_direct()
  end()
