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

SUPPORTED_BITS_PER_VALUE = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 21, 32]
HEADER_FILE = "Packed64SingleBlock.h"
CPP_FILE = "Packed64SingleBlock.cpp"
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

HEADER_HEAD = """
// This file has been automatically generated, DO NOT EDIT

#ifndef SRC_UTIL_PACK_PACKED64_H_
#define SRC_UTIL_PACK_PACKED64_H_

namespace lucene {
namespace core {
namespace util {
"""

CLASS_HALF = """
class Packed64SingleBlock : public PackedInts::MutableImpl {
 private:
  static constexpr uint32_t SUPPORTED_BITS_PER_VALUE[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 21, 32
  };

  static uint32_t RequiredCapacity(const uint32_t value_count,
                                    const uint32_t values_per_block) {
    return (value_count / values_per_block) +
           (value_count % (values_per_block == 0 ? 0 : 1));
  }

 public:
  static const uint32_t MAX_SUPPORTED_BITS_PER_VALUE = 32;

  static bool IsSupported(const uint32_t bits_per_value) {
    uint32_t l = 0;
    uint32_t h = sizeof(SUPPORTED_BITS_PER_VALUE) / sizeof(uint32_t) - 1;

    while (l <= h) {
      const uint32_t mid = (l + h) >> 1;
      const uint32_t mid_val = SUPPORTED_BITS_PER_VALUE[mid];

      if (mid_val < key) {
        l = mid + 1;
      } else {
        h = mid - 1;
      } else {
        return true;
      }
    }

    return false;
  }

  static std::unique_ptr<Packed64SingleBlock>
  Create(uint32_t value_count,
         uint32_t bits_per_value);

 private:
  std::unique_ptr<uint64_t> blocks;
  uint32_t blocks_size;

 protected:
  Packed64SingleBlock(const uint32_t value_count,
                      const uint32_t bits_per_value)
    : PackedInts::MutableImpl(value_count, bits_per_value),
      blocks(std::make_unique<uint64_t>(RequiredCapacity(value_count, values_per_block))),
      blocks_size(RequiredCapacity(value_count, values_per_block)) {
  }

 public:
  void Clear() {
    std::memset(blocks.get(), 0, blocks_size * sizeof(uint64_t));
  }

  uint32_t Get(uint32_t index,
               int64_t arr[],
               uint32_t off,
               uint32_t len) {
    len = std::min(len, value_count - index);

    const uint32_t original_index = index;

    // Go to the next block boundary
    const uint32_t values_per_block = (64 / bits_per_value);
    const uint32_t offset_in_block = (index % values_per_block);
    if (offset_in_block != 0) {
      for (uint32_t i = offset_in_block ; i < values_per_block && len > 0 ; ++i) {
        arr[off++] = Get(index++);
        --len;
      }

      if (len == 0) {
        return (index - original_index);
      }
    }

    // Bulk get
    const PackedInts::Decoder* decoder = BulkOperation.Of(PackedInts::Format::PACKED_SINGLE_BLOCK, bits_per_value);
    const uint32_t block_index = (index / values_per_block);
    const uint32_t nblocks = (index + len) / values_per_block - block_index;
    decoder->Decode(blocks.get(), block_index, arr, off, nblocks);
    const uint32_t diff = (nblocks * values_per_block);
    index += diff;
    len -= diff;

    if (index > original_index) {
      return (index - original_index);
    } else {
      return PackedInts::MutableImpl::Get(index, arr, off, len);
    }
  }

  uint32_t Set(uint32_t index,
               const int64_t arr[],
               uint32_t off,
               uint32_t len) {
    len = std::min(len, value_count - index);  
    const uint32_t original_index = index;

    // Go to the next block boundary
    const uint32_t values_per_block = (64 / bits_per_value);
    const uint32_t offset_in_block = (index % values_per_block);
    if (offset_in_block != 0) {
      for (uint32_t i = offset_in_block ; i < values_per_block && len > 0 ; ++i) {
        Set(index++, arr[off++]);
        --len;
      }

      if (len == 0) {
        return (index - original_index);
      }
    }

    // Bulk set
    BulkOperation* op = BulkOperation::Of(PackedInts::Format::PACKED_SINGLE_BLOCK, bits_per_value);
    const uint32_t block_index = (index / values_per_block);
    const uint32_t nblocks = (index + len) / values_per_block - block_index;
    op->Encode(arr, off, blocks.get(), block_index, nblocks);
    const uint32_t diff = (nblocks * values_per_block);
    index += diff;
    len -= diff;

    if (index > original_index) {
      return (index - original_index);
    } else {
      return PackedInts::MutableImple::Set(index, arr, off, len);
    }
  }

  void Fill(uint32_t from_index,
            uint32_t to_index,
            int64_t val) {
    const uint32_t values_per_block = (64 / bits_per_value);
    if (to_index - from_index <= values_per_block << 1) {
      PackedInts::MutableImpl::Fill(from_index, to_index, val);
      return;
    }

    const uint32_t from_offset_in_block = (from_index % values_per_block);
    if (from_offset_in_block != 0) {
      for (uint32_t i = from_offset_in_block ; i < values_per_block ; ++i) {
        Set(from_offset_in_block, val);
      }
    }

    // Bulk set of the inner blocks
    const uint32_t from_block = (from_index / values_per_block);
    const uint32_t to_block = (to_index / values_per_block);

    uint64_t block_value = 0L;
    for (uint32_t i = 0 ; i < values_per_block ; ++i) {
      block_value = block_value | (val << (i * bits_per_value));
    }

    std::copy(blocks.get() + from_block, blocks.get() + to_block, block_value);

    // Fill the gap
    for (uint32_t i = values_per_block * to_block ; i < to_index ; ++i) {
      Set(i, val);
    }
  }

 public:
"""

HEADER_TAIL = """
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKED64_H_
"""

CPP_HEAD = """
#include <Util/Pack/Packed64SingleBlock.h>
#include <Util/Exception.h>

using lucene::core::util::Packed64SingleBlock;
"""

f = None

def l(line):
  global f 
  f.write(line)

def ln(line):
  global f 
  l(line + "\n")

def header_start():
  global f 
  f = open(HEADER_FILE, "w")
  l(LICENSE)
  l(HEADER_HEAD)

def header_end():
  global f
  l(HEADER_TAIL)
  f.close()

def cpp_start():
  global f
  f = open(CPP_FILE, "w")
  l(LICENSE)
  l(CPP_HEAD)

def cpp_end():
  global f
  f.close()

def gen_pack64_cpp():
  l(
"""
/**
 *  Packed64SingleBlock
 */

std::unique_ptr<Packed64SingleBlock>
Packed64SingleBlock::Create(uint32_t value_count,
                            uint32_t bits_per_value) {
  switch (bits_per_value) {""")
  for bpv in SUPPORTED_BITS_PER_VALUE:
    l(
"""
    case %d:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock%d>(value_count);
""" % (bpv, bpv))
  l(
"""
    default:
      throw
      IllegalArgumentException("Unsupported number of bits per value: " +
                               std::to_string(%d));
  }
}""" % (bpv))
  

def gen_pack64_h():
  l(CLASS_HALF)
  
  for bpv in SUPPORTED_BITS_PER_VALUE:
    ln("  class Packed64SingleBlock%d;" % bpv)
  ln("};")  # End of Packed64SingleBlock
  
  for bpv in SUPPORTED_BITS_PER_VALUE:
    log_2 = 0
    while (1 << log_2) < bpv:
      log_2 = log_2 + 1
    if (1 << log_2) != bpv:
      log_2 = None

    l(
"""
class Packed64SingleBlock::Packed64SingleBlock%d : public Packed64SingleBlock {
 public:
  Packed64SingleBlock%d(const uint32_t value_count)
    : Packed64SingleBlock(value_count, %d) {
  }

  int64_t Get(uint32_t index) {""" % (bpv, bpv, bpv))

    if log_2 is not None:
      l(
"""
      const uint32_t o = (index >> %d); 
      const uint32_t b = (index & %d);
      const uint32_t shift (b << %d);
""" % ((6 - log_2), ((1 << (6 - log_2)) - 1), (log_2)))
      
    else:
      l(
"""
      const uint32_t o =(index / %d);   
      const uint32_t b = (index %% %d);
      const uint32_t shift = (b * %d);
""" % ((64 / bpv), (64 / bpv), (bpv)))

    l(
"""
    return (blocks[o] >> shift) & %dL;
  }
""" % ((1 << bpv) - 1))
    
    l(
"""
  void Set(uint32_t index, int64_t value) {""")

    if log_2 is not None:
      l(
"""
    const uint32_t o = (index >> %d);
    const uint32_t b = (index & %d);
    const uint32_t shift = (b << %d);
""" % ((6 - log_2), ((1 << (6 - log_2)) - 1), log_2))
    else:
      l(
"""
    const uint32_t o = (index >> %d);
    const uint32_t b = (index & %d);
    const uint32_t shift = (b << %d);
""")

    l(
"""
    blocks[o] = (blocks[o] & ~(%dL << shift)) |
                (value << shift);
  }
""" % ((1 << bpv) - 1))

    ln("};")  # End of Packed64SingleBlock${bpv}

if __name__ == "__main__":
  header_start()
  gen_pack64_h()
  header_end()

  cpp_start()
  gen_pack64_cpp()
  cpp_end()
