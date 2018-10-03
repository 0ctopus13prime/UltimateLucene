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
HEADER_FILE = "Packed64.h"
CPP_FILE = "Packed64.cpp"
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

#include <Util/Pack/PackedInts.h>
#include <Util/Pack/BulkOperation.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <utility>
#include <memory>

namespace lucene {
namespace core {
namespace util {
"""

CLASS_HALF = """
class Packed64: public PackedInts::MutableImpl {
 public:
  static const uint32_t BLOCK_SIZE = 64;
  static const uint32_t BLOCK_BITS = 6;
  static const uint32_t MOD_MASK = (BLOCK_SIZE - 1);

 private:
  uint32_t blocks_size;
  std::unique_ptr<uint64_t[]> blocks; 
  uint64_t mask_right;
  int32_t bpv_minus_block_size;

 private:
  static uint32_t Gcd(uint32_t a, uint32_t b) {
    while(b) a %= b, std::swap(a, b);
    return a;
  }

 public:
  Packed64(const uint32_t value_count, const uint32_t bits_per_value)
    : PackedInts::MutableImpl(value_count, bits_per_value),
      blocks_size(PackedInts::Format::PACKED.LongCount(
        PackedInts::VERSION_CURRENT, value_count, bits_per_value)),
      blocks(std::make_unique<uint64_t[]>(blocks_size)),
      mask_right((1UL << bits_per_value) - 1),
      bpv_minus_block_size(bits_per_value - BLOCK_SIZE) {
  }

  Packed64(const uint32_t packed_ints_version,
           lucene::core::store::DataInput* in,
           const uint32_t value_count,
           const uint32_t bits_per_value)
    : PackedInts::MutableImpl(value_count, bits_per_value),
      blocks_size(PackedInts::Format::PACKED.LongCount(
        PackedInts::VERSION_CURRENT, value_count, bits_per_value)),
      blocks(std::make_unique<uint64_t[]>(blocks_size)),
      mask_right((1UL << bits_per_value) - 1),
      bpv_minus_block_size(bits_per_value - BLOCK_SIZE) {
    uint64_t byte_count =
      PackedInts::Format::PACKED.ByteCount(packed_ints_version,
                                           value_count,
                                           bits_per_value);
    for (uint64_t i = 0 ; i < (byte_count / 8) ; ++i) {
      blocks[i] = in->ReadInt64(); 
    }

    const uint32_t remaining = static_cast<uint32_t>(byte_count % 8);
    if (remaining != 0) {
      uint64_t last_long = 0;
      for (int i = 0 ; i < remaining ; ++i) {
        last_long |= (in->ReadByte() & 0xFFL) <<
                     (56 - i * 8);
      }
      blocks[blocks_size - 1] = last_long;
    }
  }

  int64_t Get(uint32_t index) {
    const uint64_t major_bit_pos =
      static_cast<uint64_t>(index) * bits_per_value;
    const uint32_t element_pos =
      static_cast<uint32_t>(major_bit_pos >> BLOCK_BITS);
    const int64_t end_bits = (major_bit_pos & MOD_MASK) + bpv_minus_block_size;

    // Single block
    if (end_bits <= 0) {
      return (blocks[element_pos] >> -end_bits) & mask_right;
    }

    // Two blocks
    return ((blocks[element_pos] << end_bits) |
            (blocks[element_pos + 1] >> (BLOCK_SIZE - end_bits))) &
            mask_right;
  }

  uint32_t Get(uint32_t index,
               int64_t arr[],
               uint32_t off,
               uint32_t len) {
    assert(index < value_count); 
    len = std::min(len, value_count - index);

    const uint32_t original_index = index;
    PackedInts::Decoder* decoder =
      BulkOperation::Of(PackedInts::Format::PACKED, bits_per_value);
    
    // Go to the next block where the value does not span across two blocks
    const uint32_t offset_in_blocks = (index % decoder->LongValueCount());
    if (offset_in_blocks != 0) {
      for (uint32_t i = offset_in_blocks ;
           i < decoder->LongValueCount() && len > 0 ;
           ++i) {
        arr[off++] = Get(index++);
        --len;
      }

      if (len == 0) {
        return (index - original_index);
      }
    }

    // Bulk get
    assert(index % decoder->LongValueCount() == 0);
    uint32_t block_index =
      static_cast<uint32_t>((static_cast<uint64_t>(index) * bits_per_value) >>
                             BLOCK_BITS);
    assert((static_cast<uint64_t>(index) * bits_per_value) & MOD_MASK == 0);
    const uint32_t iterations = (len / decoder->LongValueCount());
    decoder->Decode(blocks.get(), block_index, arr, off, iterations);
    const uint32_t got_values = iterations * decoder->LongValueCount();
    index += got_values;
    len -= got_values;

    if (index > original_index) {
      return (index - original_index);
    } else {
      assert(index == original_index);
      return PackedInts::MutableImpl::Get(index, arr, off, len);
    }
  }

  void Set(uint32_t index, int64_t value) {
    const uint64_t major_bit_pos =
      static_cast<uint64_t>(index) * bits_per_value;
    const uint32_t element_pos =
      static_cast<uint32_t>(major_bit_pos >> BLOCK_BITS);
    const int64_t end_bits = (major_bit_pos & MOD_MASK) + bpv_minus_block_size;

    // Single block
    if (end_bits <= 0) {
      blocks[element_pos] = blocks[element_pos] &
                            ~(mask_right << -end_bits) |
                            (value << -end_bits);
    } else {
      // Two blocks
      blocks[element_pos] = blocks[element_pos] &
                            ~(mask_right >> end_bits) |
                            (value >> end_bits);
      blocks[element_pos + 1] = blocks[element_pos + 1] &
                                (~0L >> end_bits) |
                                (value << (BLOCK_SIZE - end_bits));
    }
  }

  uint32_t Set(uint32_t index,
               const int64_t arr[],
               uint32_t off,
               uint32_t len) {
    assert(index < value_count);
    len = std::min(len, value_count - index);

    const uint32_t original_index = index;
    PackedInts::Encoder* encoder =
      BulkOperation::Of(PackedInts::Format::PACKED, bits_per_value);

    const uint32_t offset_in_blocks = (index % encoder->LongValueCount());
    if (offset_in_blocks != 0) {
      for (uint32_t i = offset_in_blocks ;
           i < encoder->LongValueCount() && len > 0 ;
           ++i) {
        Set(index++, arr[off++]);
        --len;
      }

      if (len == 0) {
        return (index - original_index);
      }
    }

    // Bulk set
    assert(index % encoder->LongValueCount() == 0);
    uint32_t block_index =
      static_cast<uint32_t>((static_cast<uint64_t>(index) * bits_per_value) >>
                            BLOCK_BITS);
    assert((static_cast<uint64_t>(index) * bits_per_value) & MOD_MASK == 0);
    const uint32_t iterations = (len / encoder->LongValueCount());
    encoder->Encode(arr, off, blocks.get(), block_index, iterations);
    const uint32_t set_values = (iterations * encoder->LongValueCount());
    index += set_values;
    len -= set_values;

    if (index > original_index) {
      return (index - original_index);
    } else {
      assert(index == original_index);
      return PackedInts::MutableImpl::Set(index, arr, off, len);
    }
  }

  void Fill(uint32_t from_index,
            uint32_t to_index,
            int64_t val) {
    assert(PackedInts::UnsignedBitsRequired(val) <= GetBitsPerValue());
    assert(from_index <= to_index);

    // Minimum number of values that use an exact number of full blocks
    const uint32_t n_aligned_values = (64 / Gcd(64, bits_per_value));
    const uint32_t span = (to_index - from_index);
    if (span <= 3 * n_aligned_values) {
      // There needs be at least 2 * n_aligned_values aligned values for the
      // lock approach to be worth trying
      PackedInts::MutableImpl::Fill(from_index, to_index, val);
      return;
    }

    // Fill the first values naively until the next block start
    const uint32_t from_index_mod_n_aligned_values =
      (from_index % n_aligned_values);
    if (from_index_mod_n_aligned_values != 0) {
      for (uint32_t i = from_index_mod_n_aligned_values ;
           i < n_aligned_values ;
           ++i) {
        Set(from_index++, val);
      }
    }
    assert(from_index % n_aligned_values == 0);

    // Compute the uint64_t[] blocks for n_aligned_values consecutive values and
    // use them to set as many values as possible without applying any mask
    // or shift
    const uint32_t n_aligned_blocks = (n_aligned_values * bits_per_value) >> 6;
    std::unique_ptr<uint64_t[]> n_aligned_values_blocks;
    {
      Packed64 values(n_aligned_values, bits_per_value);
      for (uint32_t i = 0; i < n_aligned_values; ++i) {
        values.Set(i, val);
      }
      n_aligned_values_blocks = std::move(values.blocks);
      assert(n_aligned_blocks <= values.blocks_size);
    }
    const uint32_t start_block =
      static_cast<uint32_t>((static_cast<uint64_t>(from_index) *
                            bits_per_value) >> 6);
    const uint32_t end_block =
      static_cast<uint32_t>((static_cast<uint64_t>(to_index) *
                            bits_per_value) >> 6);
    for (uint32_t block = start_block ; block < end_block; ++block) {
      const uint64_t block_value =
        n_aligned_values_blocks[block % n_aligned_blocks];
      blocks[block] = block_value;
    }

    // Fill the gap
    for (uint32_t i = static_cast<uint32_t>((static_cast<uint64_t>(end_block)
                                            << 6) / bits_per_value) ;
         i < to_index ;
         ++i) {
      Set(i, val);
    }
  }

  void Clear() {
    std::memset(blocks.get(), 0, blocks_size * sizeof(uint64_t));
  }
};


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

      if (mid_val < bits_per_value) {
        l = mid + 1;
      } else if (mid_val > bits_per_value) {
        h = mid - 1;
      } else {
        return true;
      }
    }

    return false;
  }

  static std::unique_ptr<Packed64SingleBlock>
  Create(lucene::core::store::DataInput* in,
         uint32_t value_count,
         uint32_t bits_per_value);

  static std::unique_ptr<Packed64SingleBlock>
  Create(uint32_t value_count,
         uint32_t bits_per_value);

 protected:
  uint32_t blocks_size;
  std::unique_ptr<uint64_t[]> blocks;

 protected:
  Packed64SingleBlock(const uint32_t value_count,
                      const uint32_t bits_per_value)
    : PackedInts::MutableImpl(value_count, bits_per_value),
      blocks_size(RequiredCapacity(value_count, 64 / bits_per_value)),
      blocks(std::make_unique<uint64_t[]>(RequiredCapacity(value_count, blocks_size))) {
  }

 public:
  using PackedInts::Mutable::Set;
  using PackedInts::Reader::Get;

  virtual ~Packed64SingleBlock() = default;

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
    PackedInts::Decoder* decoder = BulkOperation::Of(PackedInts::Format::PACKED_SINGLE_BLOCK, bits_per_value);
    const uint32_t block_index = (index / values_per_block);
    const uint32_t nblocks = (index + len) / values_per_block - block_index;
    decoder->Decode(blocks.get(), block_index, arr, off, nblocks);
    const uint32_t diff = (nblocks * values_per_block);
    index += diff;
    len -= diff;

    if (index > original_index) {
      return (index - original_index);
    } else {
      return Get(index, arr, off, len);
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
      return Set(index, arr, off, len);
    }
  }

  void Fill(uint32_t from_index,
            uint32_t to_index,
            int64_t val) {
    const uint32_t values_per_block = (64 / bits_per_value);
    if (to_index - from_index <= values_per_block << 1) {
      Fill(from_index, to_index, val);
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

    std::fill(blocks.get() + from_block, blocks.get() + to_block, block_value);

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
// This file has been automatically generated, DO NOT EDIT

#include <Util/Pack/Packed64.h>
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

  l(
"""

std::unique_ptr<Packed64SingleBlock>
Packed64SingleBlock::Create(lucene::core::store::DataInput* in,
                            uint32_t value_count,
                            uint32_t bits_per_value) {
  std::unique_ptr<Packed64SingleBlock> reader =
    Create(value_count, bits_per_value);
  for (uint32_t i = 0 ; i < reader->blocks_size ; ++i) {
    reader->blocks[i] = in->ReadInt64();
  }

  return std::move(reader);
}
""")
  

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
""" % ((64 / bpv), (64 / bpv), (bpv)))

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
