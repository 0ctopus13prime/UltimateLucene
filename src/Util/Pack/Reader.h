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

#ifndef SRC_UTIL_PACK_READER_H_
#define SRC_UTIL_PACK_READER_H_

#include <Store/DataInput.h>
#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <string>

namespace lucene {
namespace core {
namespace util {

class DirectPackedReader: public PackedInts::ReaderImpl {
 private:
  lucene::core::store::IndexInput* in;
  uint64_t start_pointer;
  uint64_t value_mask;
  uint32_t bits_per_value;

 public:
  DirectPackedReader(const uint32_t bits_per_value,
                     const uint32_t value_count,
                     lucene::core::store::IndexInput* in)
    : PackedInts::ReaderImpl(value_count),
      in(in),
      start_pointer(in->GetFilePointer()),
      value_mask(),
      bits_per_value(bits_per_value) {
    if (bits_per_value == 64) {
      value_mask = ~0L;
    } else {
      value_mask = ((1L << bits_per_value) - 1);
    }
  }

  int64_t Get(uint32_t index) {
    const uint64_t major_bits_pos =
      static_cast<uint64_t>(index) * bits_per_value;
    const uint64_t element_pos = (major_bits_pos >> 3);

    in->Seek(start_pointer + element_pos);
    const uint32_t bit_pos = static_cast<uint32_t>(major_bits_pos & 7);
    const uint32_t rounded_bits = ((bit_pos + bits_per_value + 7) & ~7);
    uint32_t shift_right_bits = (rounded_bits - bit_pos - bits_per_value);

    int64_t raw_value;
    switch (rounded_bits >> 3) {
      case 1:
        raw_value = in->ReadByte();
        break;
      case 2:
        raw_value = in->ReadInt16();
        break;
      case 3:
        raw_value = (static_cast<int64_t>(in->ReadInt16()) << 8) |
                    (in->ReadByte() & 0xFFL);
        break;
      case 4:
        raw_value = in->ReadInt32();
        break;
      case 5:
        raw_value = (static_cast<int64_t>(in->ReadInt32()) << 8) |
                    (in->ReadByte() & 0xFFL);
        break;
      case 6:
        raw_value = (static_cast<int64_t>(in->ReadInt32()) << 16) |
                    (in->ReadByte() & 0xFFFFL);
        break;
      case 7:
        raw_value = (static_cast<int64_t>(in->ReadInt32()) << 24) |
                    ((in->ReadInt16() & 0xFFFFL) << 8) |
                    (in->ReadByte() & 0xFFL);
        break;
      case 8:
        raw_value = in->ReadInt64();
        break;
      case 9:
        raw_value = (in->ReadInt64() << (8 - shift_right_bits)) |
                    ((in->ReadByte() & 0xFFL) >> shift_right_bits);
        shift_right_bits = 0;
        break;
      default:
        throw IllegalStateException("Bits per value too large: " +
                                    std::to_string(bits_per_value));
    }

    return (raw_value >> shift_right_bits) & value_mask;
  }
};

class DirectPacked64SingleBlockReader: public PackedInts::ReaderImpl {
 private:
  lucene::core::store::IndexInput* in;
  uint32_t bits_per_value;
  uint32_t values_per_block;
  uint64_t start_pointer;
  uint64_t mask;

 public:
  DirectPacked64SingleBlockReader(const uint32_t bits_per_value,
                                  const uint32_t value_count,
                                  lucene::core::store::IndexInput* in)
    : PackedInts::ReaderImpl(value_count),
      in(in),
      bits_per_value(bits_per_value),
      values_per_block(64 / bits_per_value),
      start_pointer(in->GetFilePointer()),
      mask((1L << bits_per_value) - 1) {
  }

  int64_t Get(uint32_t index) {
    const uint32_t block_offset = (index / values_per_block);
    const uint64_t skip = (static_cast<uint64_t>(block_offset) << 3);
    in->Seek(start_pointer + skip);
    uint64_t block = in->ReadInt64();
    const uint32_t offset_in_block = (index % values_per_block);
    return ((block >> (offset_in_block * bits_per_value)) & mask);
  }

  uint32_t Size() {
    return value_count;
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_READER_H_
