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
    assert(((static_cast<uint64_t>(index) * bits_per_value) & MOD_MASK) == 0);
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
    assert(((static_cast<uint64_t>(index) * bits_per_value) & MOD_MASK) == 0);
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
  class Packed64SingleBlock1;
  class Packed64SingleBlock2;
  class Packed64SingleBlock3;
  class Packed64SingleBlock4;
  class Packed64SingleBlock5;
  class Packed64SingleBlock6;
  class Packed64SingleBlock7;
  class Packed64SingleBlock8;
  class Packed64SingleBlock9;
  class Packed64SingleBlock10;
  class Packed64SingleBlock12;
  class Packed64SingleBlock16;
  class Packed64SingleBlock21;
  class Packed64SingleBlock32;
};

class Packed64SingleBlock::Packed64SingleBlock1 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock1(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 1) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o = (index >> 6); 
      const uint32_t b = (index & 63);
      const uint32_t shift (b << 0);

    return (blocks[o] >> shift) & 1L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 6);
    const uint32_t b = (index & 63);
    const uint32_t shift = (b << 0);

    blocks[o] = (blocks[o] & ~(1L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock2 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock2(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 2) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o = (index >> 5); 
      const uint32_t b = (index & 31);
      const uint32_t shift (b << 1);

    return (blocks[o] >> shift) & 3L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 5);
    const uint32_t b = (index & 31);
    const uint32_t shift = (b << 1);

    blocks[o] = (blocks[o] & ~(3L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock3 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock3(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 3) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 21);   
      const uint32_t b = (index % 21);
      const uint32_t shift = (b * 3);

    return (blocks[o] >> shift) & 7L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 21);
    const uint32_t b = (index & 21);
    const uint32_t shift = (b << 3);

    blocks[o] = (blocks[o] & ~(7L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock4 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock4(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 4) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o = (index >> 4); 
      const uint32_t b = (index & 15);
      const uint32_t shift (b << 2);

    return (blocks[o] >> shift) & 15L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 4);
    const uint32_t b = (index & 15);
    const uint32_t shift = (b << 2);

    blocks[o] = (blocks[o] & ~(15L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock5 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock5(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 5) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 12);   
      const uint32_t b = (index % 12);
      const uint32_t shift = (b * 5);

    return (blocks[o] >> shift) & 31L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 12);
    const uint32_t b = (index & 12);
    const uint32_t shift = (b << 5);

    blocks[o] = (blocks[o] & ~(31L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock6 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock6(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 6) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 10);   
      const uint32_t b = (index % 10);
      const uint32_t shift = (b * 6);

    return (blocks[o] >> shift) & 63L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 10);
    const uint32_t b = (index & 10);
    const uint32_t shift = (b << 6);

    blocks[o] = (blocks[o] & ~(63L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock7 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock7(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 7) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 9);   
      const uint32_t b = (index % 9);
      const uint32_t shift = (b * 7);

    return (blocks[o] >> shift) & 127L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 9);
    const uint32_t b = (index & 9);
    const uint32_t shift = (b << 7);

    blocks[o] = (blocks[o] & ~(127L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock8 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock8(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 8) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o = (index >> 3); 
      const uint32_t b = (index & 7);
      const uint32_t shift (b << 3);

    return (blocks[o] >> shift) & 255L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 3);
    const uint32_t b = (index & 7);
    const uint32_t shift = (b << 3);

    blocks[o] = (blocks[o] & ~(255L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock9 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock9(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 9) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 7);   
      const uint32_t b = (index % 7);
      const uint32_t shift = (b * 9);

    return (blocks[o] >> shift) & 511L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 7);
    const uint32_t b = (index & 7);
    const uint32_t shift = (b << 9);

    blocks[o] = (blocks[o] & ~(511L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock10 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock10(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 10) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 6);   
      const uint32_t b = (index % 6);
      const uint32_t shift = (b * 10);

    return (blocks[o] >> shift) & 1023L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 6);
    const uint32_t b = (index & 6);
    const uint32_t shift = (b << 10);

    blocks[o] = (blocks[o] & ~(1023L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock12 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock12(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 12) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 5);   
      const uint32_t b = (index % 5);
      const uint32_t shift = (b * 12);

    return (blocks[o] >> shift) & 4095L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 5);
    const uint32_t b = (index & 5);
    const uint32_t shift = (b << 12);

    blocks[o] = (blocks[o] & ~(4095L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock16 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock16(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 16) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o = (index >> 2); 
      const uint32_t b = (index & 3);
      const uint32_t shift (b << 4);

    return (blocks[o] >> shift) & 65535L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 2);
    const uint32_t b = (index & 3);
    const uint32_t shift = (b << 4);

    blocks[o] = (blocks[o] & ~(65535L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock21 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock21(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 21) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o =(index / 3);   
      const uint32_t b = (index % 3);
      const uint32_t shift = (b * 21);

    return (blocks[o] >> shift) & 2097151L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 3);
    const uint32_t b = (index & 3);
    const uint32_t shift = (b << 21);

    blocks[o] = (blocks[o] & ~(2097151L << shift)) |
                (value << shift);
  }
};

class Packed64SingleBlock::Packed64SingleBlock32 : public Packed64SingleBlock {
 public:
  Packed64SingleBlock32(const uint32_t value_count)
    : Packed64SingleBlock(value_count, 32) {
  }

  int64_t Get(uint32_t index) {
      const uint32_t o = (index >> 1); 
      const uint32_t b = (index & 1);
      const uint32_t shift (b << 5);

    return (blocks[o] >> shift) & 4294967295L;
  }

  void Set(uint32_t index, int64_t value) {
    const uint32_t o = (index >> 1);
    const uint32_t b = (index & 1);
    const uint32_t shift = (b << 5);

    blocks[o] = (blocks[o] & ~(4294967295L << shift)) |
                (value << shift);
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKED64_H_
