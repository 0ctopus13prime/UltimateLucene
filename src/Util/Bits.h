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

#ifndef SRC_UTIL_BITS_H_
#define SRC_UTIL_BITS_H_

#include <stdint.h>

namespace lucene {
namespace core {
namespace util {

class BitUtil {
 private:
  static const uint64_t MAGIC[7];
  static const uint16_t SHIFT[5];

 private:
  BitUtil() { }

 public:
  static uint64_t PopArray(const int64_t arr[],
                            const uint32_t word_offset,
                            const uint32_t num_words) noexcept {
    uint64_t pop_count = 0;
    for (uint32_t i = word_offset, end = word_offset + num_words
         ; i < end ; ++i) {
      pop_count += __builtin_popcount(arr[i]);
    }

    return pop_count;
  }

  static uint64_t PopIntersect(const int64_t arr1[],
                                const int64_t arr2[],
                                const uint32_t word_offset,
                                const uint32_t num_words) noexcept {
    uint64_t pop_count = 0;
    for (uint32_t i = word_offset, end = word_offset + num_words
         ; i < end ; ++i) {
      pop_count += __builtin_popcount(arr1[i] & arr2[i]);
    }

    return pop_count;
  }

  static uint64_t PopUnion(const int64_t arr1[],
                            const int64_t arr2[],
                            const uint32_t word_offset,
                            const uint32_t num_words) noexcept {
    uint64_t pop_count = 0;
    for (uint32_t i = word_offset, end = word_offset + num_words
         ; i < end ; ++i) {
      pop_count += __builtin_popcount(arr1[i] | arr2[i]);
    }

    return pop_count;
  }

  static uint64_t PopAndNot(const int64_t arr1[],
                             const int64_t arr2[],
                             const uint32_t word_offset,
                             const uint32_t num_words) noexcept {
    uint64_t pop_count = 0;
    for (uint32_t i = word_offset, end = word_offset + num_words
         ; i < end ; ++i) {
      pop_count += __builtin_popcount(arr1[i] & ~arr2[i]);
    }

    return pop_count;
  }

  static uint64_t PopXor(const int64_t arr1[],
                          const int64_t arr2[],
                          const uint32_t word_offset,
                          const uint32_t num_words) noexcept {
    uint64_t pop_count = 0;
    for (uint32_t i = word_offset, end = word_offset + num_words
         ; i < end ; ++i) {
      pop_count += __builtin_popcount(arr1[i] ^ ~arr2[i]);
    }

    return pop_count;
  }

  static int64_t NextHighestPowerOfTwo(int64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;

    return v;
  }

  static int64_t Interleave(const int32_t even, const int32_t odd) {
    int64_t v1 = 0x00000000FFFFFFFFL & even;
    int64_t v2 = 0x00000000FFFFFFFFL & odd;
    v1 = (v1 | (v1 << SHIFT[4])) & MAGIC[4];
    v1 = (v1 | (v1 << SHIFT[3])) & MAGIC[3];
    v1 = (v1 | (v1 << SHIFT[2])) & MAGIC[2];
    v1 = (v1 | (v1 << SHIFT[1])) & MAGIC[1];
    v1 = (v1 | (v1 << SHIFT[0])) & MAGIC[0];
    v2 = (v2 | (v2 << SHIFT[4])) & MAGIC[4];
    v2 = (v2 | (v2 << SHIFT[3])) & MAGIC[3];
    v2 = (v2 | (v2 << SHIFT[2])) & MAGIC[2];
    v2 = (v2 | (v2 << SHIFT[1])) & MAGIC[1];
    v2 = (v2 | (v2 << SHIFT[0])) & MAGIC[0];

    return ((v2 << 1) | v1);
  }

  static int64_t Deinterleave(int64_t b) {
    b &= MAGIC[0];
    b = (b ^ (b >> SHIFT[0])) & MAGIC[1];
    b = (b ^ (b >> SHIFT[1])) & MAGIC[2];
    b = (b ^ (b >> SHIFT[2])) & MAGIC[3];
    b = (b ^ (b >> SHIFT[3])) & MAGIC[4];
    b = (b ^ (b >> SHIFT[4])) & MAGIC[5];

    return b;
  }

  static int64_t FlipFlop(const int64_t b) {
    return ((b & MAGIC[6]) >> 1) | ((b & MAGIC[0]) << 1 );
  }

  static int32_t ZigZagEncode(const int32_t i) {
    return ((i >> 31) ^ (i << 1));
  }

  static int32_t ZigZagDecode(const int32_t j) {
    uint32_t i = static_cast<uint32_t>(j);
    return ((i >> 1) ^ -(i & 1));
  }

  static int64_t ZigZagDecode(const int64_t j) {
    uint64_t l = static_cast<uint64_t>(j);
    return ((l >> 1) ^ -(l & 1));
  }
};  // BitUtil

class BitSet {

};  // BitSet

class FixedBitSet {
 public:
  void Set(const uint32_t i) {
    // TODO(0ctopus13prime): IT
  }
};  // FixedBitSet

class SparseFixedBitSet {

};  // SparseFixedBitSet

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_BITS_H_
