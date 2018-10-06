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

#ifndef SRC_UTIL_PACK_PACKEDTHREEBLOCKS_H_
#define SRC_UTIL_PACK_PACKEDTHREEBLOCKS_H_

#include <Store/DataInput.h>
#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <algorithm>
#include <limits>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class Packed8ThreeBlocks : public PackedInts::MutableImpl {
 public:
  static const uint32_t MAX_SIZE = std::numeric_limits<int32_t>::max() / 3;

 private:
  std::unique_ptr<char[]> blocks;
  uint32_t blocks_size;

 public:
  explicit Packed8ThreeBlocks(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, 24),
               blocks(),
               blocks_size(value_count) {
    if (value_count > MAX_SIZE) {
      throw ArrayIndexOutOfBoundsException("MAX_SIZE exceeded");
    }

    blocks = std::make_unique<char[]>(3 * value_count);
  }

  Packed8ThreeBlocks(const uint32_t packed_ints_version,
                      lucene::core::store::DataInput* in,
                      const uint32_t value_count)
    : Packed8ThreeBlocks(value_count) {
    in->ReadBytes(blocks.get(), 0, 3 * value_count);
  }

  int64_t Get(const uint32_t index) {
    const uint32_t o = index * 3;
    return ((blocks[o] & 0xFFL) << 16 |
            (blocks[o + 1] & 0xFFL) << 8 |
            (blocks[o + 2] & 0xFFL));
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    uint32_t o = off;
    for (uint32_t i = index * 3, end = (index + gets) * 3 ; i < end ; i += 3) {
      arr[o++] = (blocks[i] & 0xFFL << 16) |
                 (blocks[i + 1] & 0xFFL) << 8 |
                 (blocks[i + 2] & 0xFFL);
    }

    return gets;
  }

  void Set(const uint32_t index, const int64_t value) {
    const uint32_t o = (index * 3);
    blocks[o] = static_cast<char> (value >> 16);
    blocks[o + 1] = static_cast<char> (value >> 8);
    blocks[o + 2] = static_cast<char> (value);
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    for (uint32_t i = off, o = index * 3, end = off + sets ; i < end ; ++i) {
      const int64_t value = arr[i];
      blocks[o++] = static_cast<char> (value >> 16);
      blocks[o++] = static_cast<char> (value >> 8);
      blocks[o++] = static_cast<char> (value);
    }

    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t value) {
    const char block1 = static_cast<char> (value >> 16);
    const char block2 = static_cast<char> (value >> 8);
    const char block3 = static_cast<char> (value);

    for (uint32_t i = from_index * 3, end = to_index * 3 ; i < end ; i += 3) {
      blocks[i] = block1;
      blocks[i + 1] = block2;
      blocks[i + 2] = block3;
    }
  }

  void Clear() {
    std::memset(blocks.get(), 0, blocks_size);
  }
};

class Packed16ThreeBlocks : public PackedInts::MutableImpl {
 public:
  static const uint32_t MAX_SIZE = std::numeric_limits<int32_t>::max() / 3;

 private:
  std::unique_ptr<uint16_t[]> blocks;
  uint32_t blocks_size;

 public:
  explicit Packed16ThreeBlocks(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, 48),
               blocks(),
               blocks_size(value_count) {
    if (value_count > MAX_SIZE) {
      throw ArrayIndexOutOfBoundsException("MAX_SIZE exceeded");
    }

    blocks = std::make_unique<uint16_t[]>(3 * value_count);
  }

  Packed16ThreeBlocks(const uint32_t packed_ints_version,
                      lucene::core::store::DataInput* in,
                      const uint32_t value_count)
    : Packed16ThreeBlocks(value_count) {
    for (uint32_t i = 0 ; i < 3 * value_count ; ++i) {
      blocks[i] = in->ReadInt16();
    }
  }

  int64_t Get(const uint32_t index) {
    const uint32_t o = index * 3;
    return ((blocks[o] & 0xFFFFL) << 32 |
            (blocks[o + 1] & 0xFFFFL) << 16 |
            (blocks[o + 2] & 0xFFFFL));
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    uint32_t o = off;
    for (uint32_t i = index * 3, end = (index + gets) * 3 ; i < end ; i += 3) {
      arr[o++] = (blocks[i] & 0xFFFFL << 32) |
                 (blocks[i + 1] & 0xFFFFL) << 16 |
                 (blocks[i + 2] & 0xFFFFL);
    }

    return gets;
  }

  void Set(const uint32_t index, const int64_t value) {
    const uint32_t o = (index * 3);
    blocks[o] = static_cast<uint16_t> (value >> 32);
    blocks[o + 1] = static_cast<uint16_t> (value >> 16);
    blocks[o + 2] = static_cast<uint16_t> (value);
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    for (uint32_t i = off, o = index * 3, end = off + sets ; i < end ; ++i) {
      const int64_t value = arr[i];
      blocks[o++] = static_cast<uint16_t> (value >> 32);
      blocks[o++] = static_cast<uint16_t> (value >> 16);
      blocks[o++] = static_cast<uint16_t> (value);
    }

    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t value) {
    const uint16_t block1 = static_cast<uint16_t> (value >> 32);
    const uint16_t block2 = static_cast<uint16_t> (value >> 16);
    const uint16_t block3 = static_cast<uint16_t> (value);

    for (uint32_t i = from_index * 3, end = to_index * 3 ; i < end ; i += 3) {
      blocks[i] = block1;
      blocks[i + 1] = block2;
      blocks[i + 2] = block3;
    }
  }

  void Clear() {
    std::memset(blocks.get(), 0, blocks_size);
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKEDTHREEBLOCKS_H_
