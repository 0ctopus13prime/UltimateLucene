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

#ifndef SRC_UTIL_PACK_BULKOPERATION_H_
#define SRC_UTIL_PACK_BULKOPERATION_H_

#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <cmath>
#include <string>

namespace lucene {
namespace core {
namespace util {

class BulkOperation : public PackedInts::Decoder, public PackedInts::Encoder {
 private:
  static BulkOperation* packed_bulk_ops[];
  static BulkOperation* packed_single_block_bulk_ops[];

 protected:
  uint32_t WriteLong(const int64_t block, char blocks[],
                     const uint32_t blocks_offset) {
    char* base = blocks + blocks_offset;
    base[0] = static_cast<char>(block >> 56);
    base[1] = static_cast<char>(block >> 48);
    base[2] = static_cast<char>(block >> 40);
    base[3] = static_cast<char>(block >> 32);
    base[4] = static_cast<char>(block >> 24);
    base[5] = static_cast<char>(block >> 16);
    base[6] = static_cast<char>(block >> 8);
    base[7] = static_cast<char>(block);

    return blocks_offset;
  }

 public:
  static BulkOperation* Of(const PackedInts::Format format,
                           const uint32_t bits_per_value) {
    if (format == PackedInts::Format::PACKED) {
      return packed_bulk_ops[bits_per_value - 1];
    } else if (format == PackedInts::Format::PACKED_SINGLE_BLOCK) {
      return packed_single_block_bulk_ops[bits_per_value - 1];
    } else {
      throw IllegalArgumentException();
    }
  }

  const uint32_t ComputeIterations(const uint32_t value_count,
                                   const uint32_t ram_budget) {
    const uint32_t iterations =
      (ram_budget / (ByteBlockCount() + 8 * ByteValueCount()));

    if (iterations == 0) {
      return 1U;
    } else if ((iterations - 1) * ByteValueCount() >= value_count) {
      return
      static_cast<uint32_t>(std::ceil((static_cast<double>(value_count)) /
                            ByteValueCount()));
    } else {
      return iterations;
    }
  }
};

class BulkOperationPacked : public BulkOperation {
 private:
  uint32_t bits_per_value;
  uint32_t long_block_count;
  uint32_t long_value_count;
  uint32_t byte_block_count;
  uint32_t byte_value_count;
  uint64_t mask;
  uint32_t uint_mask;

 public:
  BulkOperationPacked(const uint32_t bits_per_value) {
    this->bits_per_value = bits_per_value;
    uint32_t blocks = bits_per_value;
    while ((blocks & 1) == 0) {
      blocks >>= 1;
    }
    this->long_block_count = blocks;
    this->long_value_count = 64 * this->long_block_count / bits_per_value;
    uint32_t byte_block_count = 8 * this->long_block_count;
    uint32_t byte_value_count = this->long_value_count;
    while ((byte_block_count & 1) == 0 && (byte_value_count & 1) == 0) {
      byte_block_count >>= 1;
      byte_value_count >>= 1;
    }

    this->byte_block_count = byte_block_count;
    this->byte_value_count = byte_value_count;
    if (bits_per_value == 64) {
      this->mask = ~0L;
    } else {
      this->mask = (1L << bits_per_value) - 1;
    }

    this->uint_mask = static_cast<uint32_t>(this->mask);
  }

  uint32_t LongBlockCount() {
    return long_block_count;
  }

  uint32_t LongValueCount() {
    return long_value_count;
  }

  uint32_t ByteBlockCount() {
    return byte_block_count;
  }

  uint32_t ByteValueCount() {
    return byte_value_count;
  }

  void Decode(const uint64_t blocks[],
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    int32_t bits_left = 64;
    for (uint32_t i = 0 ; i < long_value_count * iterations ; ++i) {
      bits_left -= bits_per_value;
      if (bits_left < 0) {
        values[values_offset++] = static_cast<int32_t>(
          ((blocks[blocks_offset++] & (((1L << (bits_per_value + bits_left)) - 1))) << -bits_left)
          | (blocks[blocks_offset] >> (64 + bits_left)));
        bits_left += 64;
      } else {
        values[values_offset++] = (blocks[blocks_offset] >> bits_left) & mask;
      }
    }
  }

  void Decode(const char blocks[],
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    int64_t next_value = 0L;
    int bits_left = bits_per_value; 
    for (uint32_t i = 0 ; i < iterations * byte_block_count ; ++i) {
      const uint64_t bytes = blocks[blocks_offset++] & 0xFFLU;
      if (bits_left > 8) {
        // Just buffer it
        bits_left -= 8;
        next_value |= (bytes << bits_left);
      } else {
        // Flush
        uint32_t bits = (8 - bits_left);
        values[values_offset++] = static_cast<int32_t>((next_value | (bytes >> bits)));
        while (bits >= bits_per_value) {
          bits -= bits_per_value;
          values[values_offset++] = static_cast<int32_t>(((bytes >> bits) & mask));
        }

        // Then buffer it again
        bits_left = (bits_per_value - bits);
        next_value = ((bytes & ((1LU << bits) - 1)) << bits_left);
      }
    }
  }

  void Decode(const uint64_t blocks[],
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    if (bits_per_value > 32) {
      throw UnsupportedOperationException("Cannot decode " + std::to_string(bits_per_value) + "-bits values into an int32_t[]");
    }

    int bits_left = 64;
    for (uint32_t i = 0 ; i < long_value_count * iterations ; ++i) {
      bits_left -= bits_per_value;
      if (bits_left < 0) {
        values[values_offset++] = static_cast<int32_t>(
          ((blocks[blocks_offset++] & ((1LU << (bits_per_value + bits_left)) - 1)) << -bits_left)
          | (blocks[blocks_offset] >> (64 + bits_left)));
      } else {
        values[values_offset++] = static_cast<int32_t>((blocks[blocks_offset] >> bits_left) & mask);
      }
    }
  }

  void Decode(const char blocks[],
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    int32_t next_value = 0; 
    int bits_left = bits_per_value;
    for (uint32_t i = 0 ; i < iterations * byte_block_count ; ++i) {
      const uint32_t bytes = blocks[blocks_offset++];
      if (bits_left > 8) {
        // Just buffer it
        bits_left -= 8;
        next_value |= (bytes << bits_left);
      } else {
        // Flush
        uint32_t bits = (8 - bits_left);
        values[values_offset++] = static_cast<int32_t>((next_value | (bytes >> bits)));
        while (bits >= bits_per_value) {
          bits -= bits_per_value;
          values[values_offset++] = static_cast<int32_t>(((bytes >> bits) & uint_mask));
        }

        // Then buffer it again
        bits_left = (bits_per_value - bits);
        next_value = ((bytes & ((1 << bits) - 1)) << bits_left);
      }
    }
  }

  void Encode(const int64_t values[],
              uint32_t values_offset,
              uint64_t blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
    uint64_t next_block = 0;
    int32_t bits_left = 64;
    for (uint32_t i = 0 ; i < long_value_count * iterations ; ++i) {
      bits_left -= bits_per_value;
      if (bits_left > 0) {
        next_block |= (values[values_offset++] << bits_left);
      } else if (bits_left == 0) {
        next_block |= values[values_offset++]; 
        blocks[blocks_offset++] = next_block;
        next_block = 0;
        bits_left = 64;
      } else {
        blocks[blocks_offset++] |= (values[values_offset] >> -bits_left);
        blocks[blocks_offset++] = next_block;
        next_block = ((values[values_offset++] & ((1UL << -bits_left) - 1)) << (64 + bits_left));
        bits_left += 64;
      }
    }
  }

  void Encode(const int32_t values[],
              uint32_t values_offset,
              uint64_t blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
    uint64_t next_block = 0;
    int32_t bits_left = 64;
    for (uint32_t i = 0 ; i < long_value_count * iterations ; ++i) {
      bits_left -= bits_per_value;
      if (bits_left > 0) {
        next_block |= (values[values_offset++] & 0xFFFFFFFFL << bits_left);
      } else if (bits_left == 0) {
        next_block |= (values[values_offset++] & 0xFFFFFFFFL);
        blocks[blocks_offset++] = next_block;
        next_block = 0;
        bits_left = 64;
      } else {
        next_block |= ((values[values_offset] & 0xFFFFFFFFL) >> -bits_left);
        blocks[blocks_offset++] = next_block;
        next_block = (values[values_offset++] & ((1L << -bits_left) - 1)) << (64 + bits_left);
        bits_left += 64;
      }
    }
  }

  void Encode(const int64_t values[],
              uint32_t values_offset,
              char blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
    uint32_t next_block = 0;
    int32_t bits_left = 8;
    for (uint32_t i = 0 ; i < byte_value_count * iterations ; ++i) {
      const int64_t v = values[values_offset++];
      if (bits_left >= 0 && bits_per_value < bits_left) {
        // Just buffer 
        next_block |= (v << (bits_left - bits_per_value));
        bits_left -= bits_per_value;
      } else {
        uint32_t bits = (bits_per_value - bits_left);
        blocks[blocks_offset++] = static_cast<char>(next_block | (v >> bits));
        while (bits >= 8) {
          bits -= 8;
          blocks[blocks_offset++] = static_cast<char>(v >> bits);
        }

        // Then buffer
        bits_left = (8 - bits);
        next_block = static_cast<uint32_t>((v & ((1LU << bits) - 1)) << bits_left);
      }
    }
  }

  void Encode(const int32_t values[],
              uint32_t values_offset,
              char blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
    uint32_t next_block = 0;  
    int32_t bits_left = 8;
    for (uint32_t i = 0 ; i < byte_value_count * iterations ; ++i) {
      const int32_t v = values[values_offset++];
      if (bits_left < 0 || bits_per_value >= bits_left) {
        uint32_t bits = (bits_per_value - bits_left); 
        blocks[blocks_offset++] = static_cast<char>(next_block | (v >> bits));
        while (bits >= 8) {
          bits -= 8;
          blocks[blocks_offset++] = static_cast<char>(v >> bits);
        }

        bits_left = (8 - bits);
        next_block = ((v & ((1U << bits) - 1)) << bits_left);
      } else {
        next_block |= (v << (bits_left - bits_per_value));
        bits_left -= bits_per_value;
      }
    }
  }
};

// TODO(0ctopus13prime): Implement it
class BulkOperationPackedSingleBlock : public BulkOperation {
 private:
  static const uint32_t BLOCK_COUNT = 1;
  
  const uint32_t bits_per_value;
  const uint32_t value_count;
  const uint64_t mask;

 public:
  BulkOperationPackedSingleBlock(const uint32_t bits_per_value)
    : bits_per_value(bits_per_value),
      value_count(64 / bits_per_value),
      mask((1L << bits_per_value) - 1) {
  }

  uint32_t LongBlockCount() {
    return 0;
  }

  uint32_t LongValueCount() {
    return 0;
  }

  uint32_t ByteBlockCount() {
    return 0;
  }

  uint32_t ByteValueCount() {
    return 0;
  }

  void Decode(const uint64_t blocks[],
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
  }

  void Decode(const char blocks[],
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
  }

  void Decode(const uint64_t blocks[],
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
  }

  void Decode(const char blocks[],
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
  }

  void Encode(const int64_t values[],
              uint32_t values_offset,
              uint64_t blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
  }

  void Encode(const int64_t values[],
              uint32_t values_offset,
              char blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
  }

  void Encode(const int32_t values[],
              uint32_t values_offset,
              uint64_t blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
  }

  void Encode(const int32_t values[],
              uint32_t values_offset,
              char blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
  }
};

class BulkOperationPacked1 : public BulkOperationPacked {
 public:
  BulkOperationPacked1()
    : BulkOperationPacked(1U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 63 ; shift >= 0 ; shift -= 1) {
        values[values_offset++] = static_cast<int32_t>((block >> shift) & 1);
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      const char block = blocks[blocks_offset++];
      values[values_offset++] = (block >> 7) & 1;
      values[values_offset++] = (block >> 6) & 1;
      values[values_offset++] = (block >> 5) & 1;
      values[values_offset++] = (block >> 4) & 1;
      values[values_offset++] = (block >> 3) & 1;
      values[values_offset++] = (block >> 2) & 1;
      values[values_offset++] = (block >> 1) & 1;
      values[values_offset++] = block & 1;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 63 ; shift >= 0 ; shift -= 1) {
        values[values_offset++] = (block >> shift) & 1;
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      const char block = blocks[blocks_offset++];
      values[values_offset++] = (block >> 7) & 1;
      values[values_offset++] = (block >> 6) & 1;
      values[values_offset++] = (block >> 5) & 1;
      values[values_offset++] = (block >> 4) & 1;
      values[values_offset++] = (block >> 3) & 1;
      values[values_offset++] = (block >> 2) & 1;
      values[values_offset++] = (block >> 1) & 1;
      values[values_offset++] = block & 1;
    }
  }
};

class BulkOperationPacked2 : public BulkOperationPacked {
 public:
  BulkOperationPacked2()
    : BulkOperationPacked(2U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 62 ; shift >= 0 ; shift -= 2) {
        values[values_offset++] = static_cast<int32_t>((block >> shift) & 3);
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      const char block = blocks[blocks_offset++];
      values[values_offset++] = (block >> 6) & 3;
      values[values_offset++] = (block >> 4) & 3;
      values[values_offset++] = (block >> 2) & 3;
      values[values_offset++] = block & 3;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 62 ; shift >= 0 ; shift -= 2) {
        values[values_offset++] = (block >> shift) & 3;
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      const char block = blocks[blocks_offset++];
      values[values_offset++] = (block >> 6) & 3;
      values[values_offset++] = (block >> 4) & 3;
      values[values_offset++] = (block >> 2) & 3;
      values[values_offset++] = block & 3;
    }
  }
};

class BulkOperationPacked3 : public BulkOperationPacked {
 public:
  BulkOperationPacked3()
    : BulkOperationPacked(3U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 61);
      values[values_offset++] = static_cast<int32_t>((block0 >> 58) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 55) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 52) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 49) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 46) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 43) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 40) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 37) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 34) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 31) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 28) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 25) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 22) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 19) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 16) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 13) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 10) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 7) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 7L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 1) & 7L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 1L) << 2) | (block1 >> 62));
      values[values_offset++] = static_cast<int32_t>((block1 >> 59) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 56) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 53) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 50) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 47) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 44) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 41) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 35) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 32) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 29) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 26) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 23) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 20) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 17) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 14) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 11) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 5) & 7L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 7L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 1) | (block2 >> 63));
      values[values_offset++] = static_cast<int32_t>((block2 >> 60) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 57) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 54) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 51) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 48) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 45) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 42) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 39) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 36) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 33) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 30) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 27) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 21) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 18) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 15) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 9) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 6) & 7L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 3) & 7L);
      values[values_offset++] = static_cast<int32_t>(block2 & 7L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 5;

      values[values_offset++] = (byte0 >> 2) & 7;

      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 3) << 1) | (byte1 >> 7);

      values[values_offset++] = (byte1 >> 4) & 7;

      values[values_offset++] = (byte1 >> 1) & 7;

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 1) << 2) | (byte2 >> 6);

      values[values_offset++] = (byte2 >> 3) & 7;

      values[values_offset++] = byte2 & 7;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 61;
      values[values_offset++] = (block0 >> 58) & 7L;
      values[values_offset++] = (block0 >> 55) & 7L;
      values[values_offset++] = (block0 >> 52) & 7L;
      values[values_offset++] = (block0 >> 49) & 7L;
      values[values_offset++] = (block0 >> 46) & 7L;
      values[values_offset++] = (block0 >> 43) & 7L;
      values[values_offset++] = (block0 >> 40) & 7L;
      values[values_offset++] = (block0 >> 37) & 7L;
      values[values_offset++] = (block0 >> 34) & 7L;
      values[values_offset++] = (block0 >> 31) & 7L;
      values[values_offset++] = (block0 >> 28) & 7L;
      values[values_offset++] = (block0 >> 25) & 7L;
      values[values_offset++] = (block0 >> 22) & 7L;
      values[values_offset++] = (block0 >> 19) & 7L;
      values[values_offset++] = (block0 >> 16) & 7L;
      values[values_offset++] = (block0 >> 13) & 7L;
      values[values_offset++] = (block0 >> 10) & 7L;
      values[values_offset++] = (block0 >> 7) & 7L;
      values[values_offset++] = (block0 >> 4) & 7L;
      values[values_offset++] = (block0 >> 1) & 7L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 1L) << 2) | (block1 >> 62);
      values[values_offset++] = (block1 >> 59) & 7L;
      values[values_offset++] = (block1 >> 56) & 7L;
      values[values_offset++] = (block1 >> 53) & 7L;
      values[values_offset++] = (block1 >> 50) & 7L;
      values[values_offset++] = (block1 >> 47) & 7L;
      values[values_offset++] = (block1 >> 44) & 7L;
      values[values_offset++] = (block1 >> 41) & 7L;
      values[values_offset++] = (block1 >> 38) & 7L;
      values[values_offset++] = (block1 >> 35) & 7L;
      values[values_offset++] = (block1 >> 32) & 7L;
      values[values_offset++] = (block1 >> 29) & 7L;
      values[values_offset++] = (block1 >> 26) & 7L;
      values[values_offset++] = (block1 >> 23) & 7L;
      values[values_offset++] = (block1 >> 20) & 7L;
      values[values_offset++] = (block1 >> 17) & 7L;
      values[values_offset++] = (block1 >> 14) & 7L;
      values[values_offset++] = (block1 >> 11) & 7L;
      values[values_offset++] = (block1 >> 8) & 7L;
      values[values_offset++] = (block1 >> 5) & 7L;
      values[values_offset++] = (block1 >> 2) & 7L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 1) | (block2 >> 63);
      values[values_offset++] = (block2 >> 60) & 7L;
      values[values_offset++] = (block2 >> 57) & 7L;
      values[values_offset++] = (block2 >> 54) & 7L;
      values[values_offset++] = (block2 >> 51) & 7L;
      values[values_offset++] = (block2 >> 48) & 7L;
      values[values_offset++] = (block2 >> 45) & 7L;
      values[values_offset++] = (block2 >> 42) & 7L;
      values[values_offset++] = (block2 >> 39) & 7L;
      values[values_offset++] = (block2 >> 36) & 7L;
      values[values_offset++] = (block2 >> 33) & 7L;
      values[values_offset++] = (block2 >> 30) & 7L;
      values[values_offset++] = (block2 >> 27) & 7L;
      values[values_offset++] = (block2 >> 24) & 7L;
      values[values_offset++] = (block2 >> 21) & 7L;
      values[values_offset++] = (block2 >> 18) & 7L;
      values[values_offset++] = (block2 >> 15) & 7L;
      values[values_offset++] = (block2 >> 12) & 7L;
      values[values_offset++] = (block2 >> 9) & 7L;
      values[values_offset++] = (block2 >> 6) & 7L;
      values[values_offset++] = (block2 >> 3) & 7L;
      values[values_offset++] = block2 & 7L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 5;

      values[values_offset++] = (byte0 >> 2) & 7;

      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 3) << 1) | (byte1 >> 7);

      values[values_offset++] = (byte1 >> 4) & 7;

      values[values_offset++] = (byte1 >> 1) & 7;

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 1) << 2) | (byte2 >> 6);

      values[values_offset++] = (byte2 >> 3) & 7;

      values[values_offset++] = byte2 & 7;
    }
  }
};

class BulkOperationPacked4 : public BulkOperationPacked {
 public:
  BulkOperationPacked4()
    : BulkOperationPacked(4U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 60 ; shift >= 0 ; shift -= 4) {
        values[values_offset++] = static_cast<int32_t>((block >> shift) & 15);
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      const char block = blocks[blocks_offset++];
      values[values_offset++] = (block >> 4) & 15;
      values[values_offset++] = block & 15;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 60 ; shift >= 0 ; shift -= 4) {
        values[values_offset++] = (block >> shift) & 15;
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      const char block = blocks[blocks_offset++];
      values[values_offset++] = (block >> 4) & 15;
      values[values_offset++] = block & 15;
    }
  }
};

class BulkOperationPacked5 : public BulkOperationPacked {
 public:
  BulkOperationPacked5()
    : BulkOperationPacked(5U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 59);
      values[values_offset++] = static_cast<int32_t>((block0 >> 54) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 49) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 44) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 39) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 34) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 29) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 24) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 19) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 14) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 9) & 31L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 31L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 15L) << 1) | (block1 >> 63));
      values[values_offset++] = static_cast<int32_t>((block1 >> 58) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 53) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 48) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 43) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 33) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 28) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 23) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 18) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 13) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 31L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 3) & 31L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 7L) << 2) | (block2 >> 62));
      values[values_offset++] = static_cast<int32_t>((block2 >> 57) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 52) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 47) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 42) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 37) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 32) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 27) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 22) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 17) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 7) & 31L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 2) & 31L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 3L) << 3) | (block3 >> 61));
      values[values_offset++] = static_cast<int32_t>((block3 >> 56) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 51) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 46) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 41) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 36) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 31) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 26) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 21) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 16) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 11) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 6) & 31L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 1) & 31L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 1L) << 4) | (block4 >> 60));
      values[values_offset++] = static_cast<int32_t>((block4 >> 55) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 50) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 45) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 40) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 35) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 30) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 25) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 20) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 15) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 10) & 31L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 5) & 31L);
      values[values_offset++] = static_cast<int32_t>(block4 & 31L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 3;

      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 7) << 2) | (byte1 >> 6);

      values[values_offset++] = (byte1 >> 1) & 31;

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 1) << 4) | (byte2 >> 4);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 15) << 1) | (byte3 >> 7);

      values[values_offset++] = (byte3 >> 2) & 31;

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 3) << 3) | (byte4 >> 5);

      values[values_offset++] = byte4 & 31;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 59;
      values[values_offset++] = (block0 >> 54) & 31L;
      values[values_offset++] = (block0 >> 49) & 31L;
      values[values_offset++] = (block0 >> 44) & 31L;
      values[values_offset++] = (block0 >> 39) & 31L;
      values[values_offset++] = (block0 >> 34) & 31L;
      values[values_offset++] = (block0 >> 29) & 31L;
      values[values_offset++] = (block0 >> 24) & 31L;
      values[values_offset++] = (block0 >> 19) & 31L;
      values[values_offset++] = (block0 >> 14) & 31L;
      values[values_offset++] = (block0 >> 9) & 31L;
      values[values_offset++] = (block0 >> 4) & 31L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 15L) << 1) | (block1 >> 63);
      values[values_offset++] = (block1 >> 58) & 31L;
      values[values_offset++] = (block1 >> 53) & 31L;
      values[values_offset++] = (block1 >> 48) & 31L;
      values[values_offset++] = (block1 >> 43) & 31L;
      values[values_offset++] = (block1 >> 38) & 31L;
      values[values_offset++] = (block1 >> 33) & 31L;
      values[values_offset++] = (block1 >> 28) & 31L;
      values[values_offset++] = (block1 >> 23) & 31L;
      values[values_offset++] = (block1 >> 18) & 31L;
      values[values_offset++] = (block1 >> 13) & 31L;
      values[values_offset++] = (block1 >> 8) & 31L;
      values[values_offset++] = (block1 >> 3) & 31L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 7L) << 2) | (block2 >> 62);
      values[values_offset++] = (block2 >> 57) & 31L;
      values[values_offset++] = (block2 >> 52) & 31L;
      values[values_offset++] = (block2 >> 47) & 31L;
      values[values_offset++] = (block2 >> 42) & 31L;
      values[values_offset++] = (block2 >> 37) & 31L;
      values[values_offset++] = (block2 >> 32) & 31L;
      values[values_offset++] = (block2 >> 27) & 31L;
      values[values_offset++] = (block2 >> 22) & 31L;
      values[values_offset++] = (block2 >> 17) & 31L;
      values[values_offset++] = (block2 >> 12) & 31L;
      values[values_offset++] = (block2 >> 7) & 31L;
      values[values_offset++] = (block2 >> 2) & 31L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 3L) << 3) | (block3 >> 61);
      values[values_offset++] = (block3 >> 56) & 31L;
      values[values_offset++] = (block3 >> 51) & 31L;
      values[values_offset++] = (block3 >> 46) & 31L;
      values[values_offset++] = (block3 >> 41) & 31L;
      values[values_offset++] = (block3 >> 36) & 31L;
      values[values_offset++] = (block3 >> 31) & 31L;
      values[values_offset++] = (block3 >> 26) & 31L;
      values[values_offset++] = (block3 >> 21) & 31L;
      values[values_offset++] = (block3 >> 16) & 31L;
      values[values_offset++] = (block3 >> 11) & 31L;
      values[values_offset++] = (block3 >> 6) & 31L;
      values[values_offset++] = (block3 >> 1) & 31L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 1L) << 4) | (block4 >> 60);
      values[values_offset++] = (block4 >> 55) & 31L;
      values[values_offset++] = (block4 >> 50) & 31L;
      values[values_offset++] = (block4 >> 45) & 31L;
      values[values_offset++] = (block4 >> 40) & 31L;
      values[values_offset++] = (block4 >> 35) & 31L;
      values[values_offset++] = (block4 >> 30) & 31L;
      values[values_offset++] = (block4 >> 25) & 31L;
      values[values_offset++] = (block4 >> 20) & 31L;
      values[values_offset++] = (block4 >> 15) & 31L;
      values[values_offset++] = (block4 >> 10) & 31L;
      values[values_offset++] = (block4 >> 5) & 31L;
      values[values_offset++] = block4 & 31L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 3;

      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 7) << 2) | (byte1 >> 6);

      values[values_offset++] = (byte1 >> 1) & 31;

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 1) << 4) | (byte2 >> 4);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 15) << 1) | (byte3 >> 7);

      values[values_offset++] = (byte3 >> 2) & 31;

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 3) << 3) | (byte4 >> 5);

      values[values_offset++] = byte4 & 31;
    }
  }
};

class BulkOperationPacked6 : public BulkOperationPacked {
 public:
  BulkOperationPacked6()
    : BulkOperationPacked(6U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 58);
      values[values_offset++] = static_cast<int32_t>((block0 >> 52) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 46) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 40) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 34) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 28) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 22) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 16) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 10) & 63L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 63L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 15L) << 2) | (block1 >> 62));
      values[values_offset++] = static_cast<int32_t>((block1 >> 56) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 50) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 44) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 32) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 26) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 20) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 14) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 63L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 63L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 4) | (block2 >> 60));
      values[values_offset++] = static_cast<int32_t>((block2 >> 54) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 48) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 42) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 36) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 30) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 18) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 63L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 6) & 63L);
      values[values_offset++] = static_cast<int32_t>(block2 & 63L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 2;

      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 3) << 4) | (byte1 >> 4);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 15) << 2) | (byte2 >> 6);

      values[values_offset++] = byte2 & 63;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 58;
      values[values_offset++] = (block0 >> 52) & 63L;
      values[values_offset++] = (block0 >> 46) & 63L;
      values[values_offset++] = (block0 >> 40) & 63L;
      values[values_offset++] = (block0 >> 34) & 63L;
      values[values_offset++] = (block0 >> 28) & 63L;
      values[values_offset++] = (block0 >> 22) & 63L;
      values[values_offset++] = (block0 >> 16) & 63L;
      values[values_offset++] = (block0 >> 10) & 63L;
      values[values_offset++] = (block0 >> 4) & 63L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 15L) << 2) | (block1 >> 62);
      values[values_offset++] = (block1 >> 56) & 63L;
      values[values_offset++] = (block1 >> 50) & 63L;
      values[values_offset++] = (block1 >> 44) & 63L;
      values[values_offset++] = (block1 >> 38) & 63L;
      values[values_offset++] = (block1 >> 32) & 63L;
      values[values_offset++] = (block1 >> 26) & 63L;
      values[values_offset++] = (block1 >> 20) & 63L;
      values[values_offset++] = (block1 >> 14) & 63L;
      values[values_offset++] = (block1 >> 8) & 63L;
      values[values_offset++] = (block1 >> 2) & 63L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 4) | (block2 >> 60);
      values[values_offset++] = (block2 >> 54) & 63L;
      values[values_offset++] = (block2 >> 48) & 63L;
      values[values_offset++] = (block2 >> 42) & 63L;
      values[values_offset++] = (block2 >> 36) & 63L;
      values[values_offset++] = (block2 >> 30) & 63L;
      values[values_offset++] = (block2 >> 24) & 63L;
      values[values_offset++] = (block2 >> 18) & 63L;
      values[values_offset++] = (block2 >> 12) & 63L;
      values[values_offset++] = (block2 >> 6) & 63L;
      values[values_offset++] = block2 & 63L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 2;

      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 3) << 4) | (byte1 >> 4);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 15) << 2) | (byte2 >> 6);

      values[values_offset++] = byte2 & 63;
    }
  }
};

class BulkOperationPacked7 : public BulkOperationPacked {
 public:
  BulkOperationPacked7()
    : BulkOperationPacked(7U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 57);
      values[values_offset++] = static_cast<int32_t>((block0 >> 50) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 43) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 36) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 29) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 22) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 15) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 8) & 127L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 1) & 127L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 1L) << 6) | (block1 >> 58));
      values[values_offset++] = static_cast<int32_t>((block1 >> 51) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 44) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 37) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 30) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 23) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 16) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 9) & 127L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 127L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 5) | (block2 >> 59));
      values[values_offset++] = static_cast<int32_t>((block2 >> 52) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 45) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 38) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 31) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 17) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 10) & 127L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 3) & 127L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 7L) << 4) | (block3 >> 60));
      values[values_offset++] = static_cast<int32_t>((block3 >> 53) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 46) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 39) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 32) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 25) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 18) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 11) & 127L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 4) & 127L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 15L) << 3) | (block4 >> 61));
      values[values_offset++] = static_cast<int32_t>((block4 >> 54) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 47) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 40) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 33) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 26) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 19) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 12) & 127L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 5) & 127L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 31L) << 2) | (block5 >> 62));
      values[values_offset++] = static_cast<int32_t>((block5 >> 55) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 48) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 41) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 34) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 27) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 20) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 13) & 127L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 6) & 127L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 63L) << 1) | (block6 >> 63));
      values[values_offset++] = static_cast<int32_t>((block6 >> 56) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 49) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 42) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 35) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 28) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 21) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 14) & 127L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 7) & 127L);
      values[values_offset++] = static_cast<int32_t>(block6 & 127L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 1;

      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 1) << 6) | (byte1 >> 2);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 3) << 5) | (byte2 >> 3);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 7) << 4) | (byte3 >> 4);

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 15) << 3) | (byte4 >> 5);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 31) << 2) | (byte5 >> 6);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 63) << 1) | (byte6 >> 7);

      values[values_offset++] = byte6 & 127;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 57;
      values[values_offset++] = (block0 >> 50) & 127L;
      values[values_offset++] = (block0 >> 43) & 127L;
      values[values_offset++] = (block0 >> 36) & 127L;
      values[values_offset++] = (block0 >> 29) & 127L;
      values[values_offset++] = (block0 >> 22) & 127L;
      values[values_offset++] = (block0 >> 15) & 127L;
      values[values_offset++] = (block0 >> 8) & 127L;
      values[values_offset++] = (block0 >> 1) & 127L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 1L) << 6) | (block1 >> 58);
      values[values_offset++] = (block1 >> 51) & 127L;
      values[values_offset++] = (block1 >> 44) & 127L;
      values[values_offset++] = (block1 >> 37) & 127L;
      values[values_offset++] = (block1 >> 30) & 127L;
      values[values_offset++] = (block1 >> 23) & 127L;
      values[values_offset++] = (block1 >> 16) & 127L;
      values[values_offset++] = (block1 >> 9) & 127L;
      values[values_offset++] = (block1 >> 2) & 127L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 5) | (block2 >> 59);
      values[values_offset++] = (block2 >> 52) & 127L;
      values[values_offset++] = (block2 >> 45) & 127L;
      values[values_offset++] = (block2 >> 38) & 127L;
      values[values_offset++] = (block2 >> 31) & 127L;
      values[values_offset++] = (block2 >> 24) & 127L;
      values[values_offset++] = (block2 >> 17) & 127L;
      values[values_offset++] = (block2 >> 10) & 127L;
      values[values_offset++] = (block2 >> 3) & 127L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 7L) << 4) | (block3 >> 60);
      values[values_offset++] = (block3 >> 53) & 127L;
      values[values_offset++] = (block3 >> 46) & 127L;
      values[values_offset++] = (block3 >> 39) & 127L;
      values[values_offset++] = (block3 >> 32) & 127L;
      values[values_offset++] = (block3 >> 25) & 127L;
      values[values_offset++] = (block3 >> 18) & 127L;
      values[values_offset++] = (block3 >> 11) & 127L;
      values[values_offset++] = (block3 >> 4) & 127L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 15L) << 3) | (block4 >> 61);
      values[values_offset++] = (block4 >> 54) & 127L;
      values[values_offset++] = (block4 >> 47) & 127L;
      values[values_offset++] = (block4 >> 40) & 127L;
      values[values_offset++] = (block4 >> 33) & 127L;
      values[values_offset++] = (block4 >> 26) & 127L;
      values[values_offset++] = (block4 >> 19) & 127L;
      values[values_offset++] = (block4 >> 12) & 127L;
      values[values_offset++] = (block4 >> 5) & 127L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 31L) << 2) | (block5 >> 62);
      values[values_offset++] = (block5 >> 55) & 127L;
      values[values_offset++] = (block5 >> 48) & 127L;
      values[values_offset++] = (block5 >> 41) & 127L;
      values[values_offset++] = (block5 >> 34) & 127L;
      values[values_offset++] = (block5 >> 27) & 127L;
      values[values_offset++] = (block5 >> 20) & 127L;
      values[values_offset++] = (block5 >> 13) & 127L;
      values[values_offset++] = (block5 >> 6) & 127L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 63L) << 1) | (block6 >> 63);
      values[values_offset++] = (block6 >> 56) & 127L;
      values[values_offset++] = (block6 >> 49) & 127L;
      values[values_offset++] = (block6 >> 42) & 127L;
      values[values_offset++] = (block6 >> 35) & 127L;
      values[values_offset++] = (block6 >> 28) & 127L;
      values[values_offset++] = (block6 >> 21) & 127L;
      values[values_offset++] = (block6 >> 14) & 127L;
      values[values_offset++] = (block6 >> 7) & 127L;
      values[values_offset++] = block6 & 127L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = byte0 >> 1;

      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte0 & 1) << 6) | (byte1 >> 2);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 3) << 5) | (byte2 >> 3);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 7) << 4) | (byte3 >> 4);

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 15) << 3) | (byte4 >> 5);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 31) << 2) | (byte5 >> 6);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 63) << 1) | (byte6 >> 7);

      values[values_offset++] = byte6 & 127;
    }
  }
};

class BulkOperationPacked8 : public BulkOperationPacked {
 public:
  BulkOperationPacked8()
    : BulkOperationPacked(8U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 56 ; shift >= 0 ; shift -= 8) {
        values[values_offset++] = static_cast<int32_t>((block >> shift) & 255);
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      values[values_offset++] = blocks[blocks_offset++] & 0xFF;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 56 ; shift >= 0 ; shift -= 8) {
        values[values_offset++] = (block >> shift) & 255;
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      values[values_offset++] = blocks[blocks_offset++] & 0xFF;
    }
  }
};

class BulkOperationPacked9 : public BulkOperationPacked {
 public:
  BulkOperationPacked9()
    : BulkOperationPacked(9U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 55);
      values[values_offset++] = static_cast<int32_t>((block0 >> 46) & 511L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 37) & 511L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 28) & 511L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 19) & 511L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 10) & 511L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 1) & 511L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 1L) << 8) | (block1 >> 56));
      values[values_offset++] = static_cast<int32_t>((block1 >> 47) & 511L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 511L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 29) & 511L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 20) & 511L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 11) & 511L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 511L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 7) | (block2 >> 57));
      values[values_offset++] = static_cast<int32_t>((block2 >> 48) & 511L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 39) & 511L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 30) & 511L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 21) & 511L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 511L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 3) & 511L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 7L) << 6) | (block3 >> 58));
      values[values_offset++] = static_cast<int32_t>((block3 >> 49) & 511L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 40) & 511L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 31) & 511L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 22) & 511L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 13) & 511L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 4) & 511L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 15L) << 5) | (block4 >> 59));
      values[values_offset++] = static_cast<int32_t>((block4 >> 50) & 511L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 41) & 511L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 32) & 511L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 23) & 511L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 14) & 511L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 5) & 511L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 31L) << 4) | (block5 >> 60));
      values[values_offset++] = static_cast<int32_t>((block5 >> 51) & 511L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 42) & 511L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 33) & 511L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 24) & 511L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 15) & 511L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 6) & 511L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 63L) << 3) | (block6 >> 61));
      values[values_offset++] = static_cast<int32_t>((block6 >> 52) & 511L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 43) & 511L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 34) & 511L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 25) & 511L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 16) & 511L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 7) & 511L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 127L) << 2) | (block7 >> 62));
      values[values_offset++] = static_cast<int32_t>((block7 >> 53) & 511L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 44) & 511L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 35) & 511L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 26) & 511L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 17) & 511L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 8) & 511L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 255L) << 1) | (block8 >> 63));
      values[values_offset++] = static_cast<int32_t>((block8 >> 54) & 511L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 45) & 511L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 36) & 511L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 27) & 511L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 18) & 511L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 9) & 511L);
      values[values_offset++] = static_cast<int32_t>(block8 & 511L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 1) | (byte1 >> 7);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 127) << 2) | (byte2 >> 6);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 63) << 3) | (byte3 >> 5);

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 31) << 4) | (byte4 >> 4);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 15) << 5) | (byte5 >> 3);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 7) << 6) | (byte6 >> 2);

      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 3) << 7) | (byte7 >> 1);

      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 1) << 8) | byte8;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 55;
      values[values_offset++] = (block0 >> 46) & 511L;
      values[values_offset++] = (block0 >> 37) & 511L;
      values[values_offset++] = (block0 >> 28) & 511L;
      values[values_offset++] = (block0 >> 19) & 511L;
      values[values_offset++] = (block0 >> 10) & 511L;
      values[values_offset++] = (block0 >> 1) & 511L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 1L) << 8) | (block1 >> 56);
      values[values_offset++] = (block1 >> 47) & 511L;
      values[values_offset++] = (block1 >> 38) & 511L;
      values[values_offset++] = (block1 >> 29) & 511L;
      values[values_offset++] = (block1 >> 20) & 511L;
      values[values_offset++] = (block1 >> 11) & 511L;
      values[values_offset++] = (block1 >> 2) & 511L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 7) | (block2 >> 57);
      values[values_offset++] = (block2 >> 48) & 511L;
      values[values_offset++] = (block2 >> 39) & 511L;
      values[values_offset++] = (block2 >> 30) & 511L;
      values[values_offset++] = (block2 >> 21) & 511L;
      values[values_offset++] = (block2 >> 12) & 511L;
      values[values_offset++] = (block2 >> 3) & 511L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 7L) << 6) | (block3 >> 58);
      values[values_offset++] = (block3 >> 49) & 511L;
      values[values_offset++] = (block3 >> 40) & 511L;
      values[values_offset++] = (block3 >> 31) & 511L;
      values[values_offset++] = (block3 >> 22) & 511L;
      values[values_offset++] = (block3 >> 13) & 511L;
      values[values_offset++] = (block3 >> 4) & 511L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 15L) << 5) | (block4 >> 59);
      values[values_offset++] = (block4 >> 50) & 511L;
      values[values_offset++] = (block4 >> 41) & 511L;
      values[values_offset++] = (block4 >> 32) & 511L;
      values[values_offset++] = (block4 >> 23) & 511L;
      values[values_offset++] = (block4 >> 14) & 511L;
      values[values_offset++] = (block4 >> 5) & 511L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 31L) << 4) | (block5 >> 60);
      values[values_offset++] = (block5 >> 51) & 511L;
      values[values_offset++] = (block5 >> 42) & 511L;
      values[values_offset++] = (block5 >> 33) & 511L;
      values[values_offset++] = (block5 >> 24) & 511L;
      values[values_offset++] = (block5 >> 15) & 511L;
      values[values_offset++] = (block5 >> 6) & 511L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 63L) << 3) | (block6 >> 61);
      values[values_offset++] = (block6 >> 52) & 511L;
      values[values_offset++] = (block6 >> 43) & 511L;
      values[values_offset++] = (block6 >> 34) & 511L;
      values[values_offset++] = (block6 >> 25) & 511L;
      values[values_offset++] = (block6 >> 16) & 511L;
      values[values_offset++] = (block6 >> 7) & 511L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 127L) << 2) | (block7 >> 62);
      values[values_offset++] = (block7 >> 53) & 511L;
      values[values_offset++] = (block7 >> 44) & 511L;
      values[values_offset++] = (block7 >> 35) & 511L;
      values[values_offset++] = (block7 >> 26) & 511L;
      values[values_offset++] = (block7 >> 17) & 511L;
      values[values_offset++] = (block7 >> 8) & 511L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 255L) << 1) | (block8 >> 63);
      values[values_offset++] = (block8 >> 54) & 511L;
      values[values_offset++] = (block8 >> 45) & 511L;
      values[values_offset++] = (block8 >> 36) & 511L;
      values[values_offset++] = (block8 >> 27) & 511L;
      values[values_offset++] = (block8 >> 18) & 511L;
      values[values_offset++] = (block8 >> 9) & 511L;
      values[values_offset++] = block8 & 511L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 1) | (byte1 >> 7);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 127) << 2) | (byte2 >> 6);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 63) << 3) | (byte3 >> 5);

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 31) << 4) | (byte4 >> 4);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 15) << 5) | (byte5 >> 3);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 7) << 6) | (byte6 >> 2);

      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 3) << 7) | (byte7 >> 1);

      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 1) << 8) | byte8;
    }
  }
};

class BulkOperationPacked10 : public BulkOperationPacked {
 public:
  BulkOperationPacked10()
    : BulkOperationPacked(10U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 54);
      values[values_offset++] = static_cast<int32_t>((block0 >> 44) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 34) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 24) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 14) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 1023L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 15L) << 6) | (block1 >> 58));
      values[values_offset++] = static_cast<int32_t>((block1 >> 48) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 28) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 18) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 1023L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 255L) << 2) | (block2 >> 62));
      values[values_offset++] = static_cast<int32_t>((block2 >> 52) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 42) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 32) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 22) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 2) & 1023L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 3L) << 8) | (block3 >> 56));
      values[values_offset++] = static_cast<int32_t>((block3 >> 46) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 36) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 26) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 16) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 6) & 1023L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 63L) << 4) | (block4 >> 60));
      values[values_offset++] = static_cast<int32_t>((block4 >> 50) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 40) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 30) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 20) & 1023L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 10) & 1023L);
      values[values_offset++] = static_cast<int32_t>(block4 & 1023L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 2) | (byte1 >> 6);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 63) << 4) | (byte2 >> 4);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 15) << 6) | (byte3 >> 2);

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 3) << 8) | byte4;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 54;
      values[values_offset++] = (block0 >> 44) & 1023L;
      values[values_offset++] = (block0 >> 34) & 1023L;
      values[values_offset++] = (block0 >> 24) & 1023L;
      values[values_offset++] = (block0 >> 14) & 1023L;
      values[values_offset++] = (block0 >> 4) & 1023L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 15L) << 6) | (block1 >> 58);
      values[values_offset++] = (block1 >> 48) & 1023L;
      values[values_offset++] = (block1 >> 38) & 1023L;
      values[values_offset++] = (block1 >> 28) & 1023L;
      values[values_offset++] = (block1 >> 18) & 1023L;
      values[values_offset++] = (block1 >> 8) & 1023L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 255L) << 2) | (block2 >> 62);
      values[values_offset++] = (block2 >> 52) & 1023L;
      values[values_offset++] = (block2 >> 42) & 1023L;
      values[values_offset++] = (block2 >> 32) & 1023L;
      values[values_offset++] = (block2 >> 22) & 1023L;
      values[values_offset++] = (block2 >> 12) & 1023L;
      values[values_offset++] = (block2 >> 2) & 1023L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 3L) << 8) | (block3 >> 56);
      values[values_offset++] = (block3 >> 46) & 1023L;
      values[values_offset++] = (block3 >> 36) & 1023L;
      values[values_offset++] = (block3 >> 26) & 1023L;
      values[values_offset++] = (block3 >> 16) & 1023L;
      values[values_offset++] = (block3 >> 6) & 1023L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 63L) << 4) | (block4 >> 60);
      values[values_offset++] = (block4 >> 50) & 1023L;
      values[values_offset++] = (block4 >> 40) & 1023L;
      values[values_offset++] = (block4 >> 30) & 1023L;
      values[values_offset++] = (block4 >> 20) & 1023L;
      values[values_offset++] = (block4 >> 10) & 1023L;
      values[values_offset++] = block4 & 1023L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 2) | (byte1 >> 6);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 63) << 4) | (byte2 >> 4);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 15) << 6) | (byte3 >> 2);

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 3) << 8) | byte4;
    }
  }
};

class BulkOperationPacked11 : public BulkOperationPacked {
 public:
  BulkOperationPacked11()
    : BulkOperationPacked(11U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 53);
      values[values_offset++] = static_cast<int32_t>((block0 >> 42) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 31) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 20) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 9) & 2047L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 511L) << 2) | (block1 >> 62));
      values[values_offset++] = static_cast<int32_t>((block1 >> 51) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 40) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 29) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 18) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 7) & 2047L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 127L) << 4) | (block2 >> 60));
      values[values_offset++] = static_cast<int32_t>((block2 >> 49) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 38) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 27) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 16) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 5) & 2047L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 31L) << 6) | (block3 >> 58));
      values[values_offset++] = static_cast<int32_t>((block3 >> 47) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 36) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 25) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 14) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 3) & 2047L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 7L) << 8) | (block4 >> 56));
      values[values_offset++] = static_cast<int32_t>((block4 >> 45) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 34) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 23) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 12) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 1) & 2047L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 1L) << 10) | (block5 >> 54));
      values[values_offset++] = static_cast<int32_t>((block5 >> 43) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 32) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 21) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 10) & 2047L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 1023L) << 1) | (block6 >> 63));
      values[values_offset++] = static_cast<int32_t>((block6 >> 52) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 41) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 30) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 19) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 8) & 2047L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 255L) << 3) | (block7 >> 61));
      values[values_offset++] = static_cast<int32_t>((block7 >> 50) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 39) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 28) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 17) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 6) & 2047L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 63L) << 5) | (block8 >> 59));
      values[values_offset++] = static_cast<int32_t>((block8 >> 48) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 37) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 26) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 15) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 4) & 2047L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 15L) << 7) | (block9 >> 57));
      values[values_offset++] = static_cast<int32_t>((block9 >> 46) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 35) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 24) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 13) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 2) & 2047L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 3L) << 9) | (block10 >> 55));
      values[values_offset++] = static_cast<int32_t>((block10 >> 44) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 33) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 22) & 2047L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 11) & 2047L);
      values[values_offset++] = static_cast<int32_t>(block10 & 2047L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 3) | (byte1 >> 5);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 31) << 6) | (byte2 >> 2);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 3) << 9) | (byte3 << 1) | (byte4 >> 7);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 127) << 4) | (byte5 >> 4);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 15) << 7) | (byte6 >> 1);

      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 1) << 10) | (byte7 << 2) | (byte8 >> 6);

      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 63) << 5) | (byte9 >> 3);

      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 7) << 8) | byte10;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 53;
      values[values_offset++] = (block0 >> 42) & 2047L;
      values[values_offset++] = (block0 >> 31) & 2047L;
      values[values_offset++] = (block0 >> 20) & 2047L;
      values[values_offset++] = (block0 >> 9) & 2047L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 511L) << 2) | (block1 >> 62);
      values[values_offset++] = (block1 >> 51) & 2047L;
      values[values_offset++] = (block1 >> 40) & 2047L;
      values[values_offset++] = (block1 >> 29) & 2047L;
      values[values_offset++] = (block1 >> 18) & 2047L;
      values[values_offset++] = (block1 >> 7) & 2047L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 127L) << 4) | (block2 >> 60);
      values[values_offset++] = (block2 >> 49) & 2047L;
      values[values_offset++] = (block2 >> 38) & 2047L;
      values[values_offset++] = (block2 >> 27) & 2047L;
      values[values_offset++] = (block2 >> 16) & 2047L;
      values[values_offset++] = (block2 >> 5) & 2047L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 31L) << 6) | (block3 >> 58);
      values[values_offset++] = (block3 >> 47) & 2047L;
      values[values_offset++] = (block3 >> 36) & 2047L;
      values[values_offset++] = (block3 >> 25) & 2047L;
      values[values_offset++] = (block3 >> 14) & 2047L;
      values[values_offset++] = (block3 >> 3) & 2047L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 7L) << 8) | (block4 >> 56);
      values[values_offset++] = (block4 >> 45) & 2047L;
      values[values_offset++] = (block4 >> 34) & 2047L;
      values[values_offset++] = (block4 >> 23) & 2047L;
      values[values_offset++] = (block4 >> 12) & 2047L;
      values[values_offset++] = (block4 >> 1) & 2047L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 1L) << 10) | (block5 >> 54);
      values[values_offset++] = (block5 >> 43) & 2047L;
      values[values_offset++] = (block5 >> 32) & 2047L;
      values[values_offset++] = (block5 >> 21) & 2047L;
      values[values_offset++] = (block5 >> 10) & 2047L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 1023L) << 1) | (block6 >> 63);
      values[values_offset++] = (block6 >> 52) & 2047L;
      values[values_offset++] = (block6 >> 41) & 2047L;
      values[values_offset++] = (block6 >> 30) & 2047L;
      values[values_offset++] = (block6 >> 19) & 2047L;
      values[values_offset++] = (block6 >> 8) & 2047L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 255L) << 3) | (block7 >> 61);
      values[values_offset++] = (block7 >> 50) & 2047L;
      values[values_offset++] = (block7 >> 39) & 2047L;
      values[values_offset++] = (block7 >> 28) & 2047L;
      values[values_offset++] = (block7 >> 17) & 2047L;
      values[values_offset++] = (block7 >> 6) & 2047L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 63L) << 5) | (block8 >> 59);
      values[values_offset++] = (block8 >> 48) & 2047L;
      values[values_offset++] = (block8 >> 37) & 2047L;
      values[values_offset++] = (block8 >> 26) & 2047L;
      values[values_offset++] = (block8 >> 15) & 2047L;
      values[values_offset++] = (block8 >> 4) & 2047L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 15L) << 7) | (block9 >> 57);
      values[values_offset++] = (block9 >> 46) & 2047L;
      values[values_offset++] = (block9 >> 35) & 2047L;
      values[values_offset++] = (block9 >> 24) & 2047L;
      values[values_offset++] = (block9 >> 13) & 2047L;
      values[values_offset++] = (block9 >> 2) & 2047L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 3L) << 9) | (block10 >> 55);
      values[values_offset++] = (block10 >> 44) & 2047L;
      values[values_offset++] = (block10 >> 33) & 2047L;
      values[values_offset++] = (block10 >> 22) & 2047L;
      values[values_offset++] = (block10 >> 11) & 2047L;
      values[values_offset++] = block10 & 2047L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 3) | (byte1 >> 5);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 31) << 6) | (byte2 >> 2);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 3) << 9) | (byte3 << 1) | (byte4 >> 7);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 127) << 4) | (byte5 >> 4);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 15) << 7) | (byte6 >> 1);

      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 1) << 10) | (byte7 << 2) | (byte8 >> 6);

      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 63) << 5) | (byte9 >> 3);

      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 7) << 8) | byte10;
    }
  }
};

class BulkOperationPacked12 : public BulkOperationPacked {
 public:
  BulkOperationPacked12()
    : BulkOperationPacked(12U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 52);
      values[values_offset++] = static_cast<int32_t>((block0 >> 40) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 28) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 16) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 4095L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 15L) << 8) | (block1 >> 56));
      values[values_offset++] = static_cast<int32_t>((block1 >> 44) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 32) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 20) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 4095L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 255L) << 4) | (block2 >> 60));
      values[values_offset++] = static_cast<int32_t>((block2 >> 48) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 36) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 4095L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 4095L);
      values[values_offset++] = static_cast<int32_t>(block2 & 4095L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 4) | (byte1 >> 4);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 15) << 8) | byte2;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 52;
      values[values_offset++] = (block0 >> 40) & 4095L;
      values[values_offset++] = (block0 >> 28) & 4095L;
      values[values_offset++] = (block0 >> 16) & 4095L;
      values[values_offset++] = (block0 >> 4) & 4095L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 15L) << 8) | (block1 >> 56);
      values[values_offset++] = (block1 >> 44) & 4095L;
      values[values_offset++] = (block1 >> 32) & 4095L;
      values[values_offset++] = (block1 >> 20) & 4095L;
      values[values_offset++] = (block1 >> 8) & 4095L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 255L) << 4) | (block2 >> 60);
      values[values_offset++] = (block2 >> 48) & 4095L;
      values[values_offset++] = (block2 >> 36) & 4095L;
      values[values_offset++] = (block2 >> 24) & 4095L;
      values[values_offset++] = (block2 >> 12) & 4095L;
      values[values_offset++] = block2 & 4095L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 4) | (byte1 >> 4);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 15) << 8) | byte2;
    }
  }
};

class BulkOperationPacked13 : public BulkOperationPacked {
 public:
  BulkOperationPacked13()
    : BulkOperationPacked(13U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 51);
      values[values_offset++] = static_cast<int32_t>((block0 >> 38) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 25) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 12) & 8191L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 4095L) << 1) | (block1 >> 63));
      values[values_offset++] = static_cast<int32_t>((block1 >> 50) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 37) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 24) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 11) & 8191L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 2047L) << 2) | (block2 >> 62));
      values[values_offset++] = static_cast<int32_t>((block2 >> 49) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 36) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 23) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 10) & 8191L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 1023L) << 3) | (block3 >> 61));
      values[values_offset++] = static_cast<int32_t>((block3 >> 48) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 35) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 22) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 9) & 8191L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 511L) << 4) | (block4 >> 60));
      values[values_offset++] = static_cast<int32_t>((block4 >> 47) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 34) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 21) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 8) & 8191L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 255L) << 5) | (block5 >> 59));
      values[values_offset++] = static_cast<int32_t>((block5 >> 46) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 33) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 20) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 7) & 8191L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 127L) << 6) | (block6 >> 58));
      values[values_offset++] = static_cast<int32_t>((block6 >> 45) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 32) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 19) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 6) & 8191L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 63L) << 7) | (block7 >> 57));
      values[values_offset++] = static_cast<int32_t>((block7 >> 44) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 31) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 18) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 5) & 8191L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 31L) << 8) | (block8 >> 56));
      values[values_offset++] = static_cast<int32_t>((block8 >> 43) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 30) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 17) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 4) & 8191L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 15L) << 9) | (block9 >> 55));
      values[values_offset++] = static_cast<int32_t>((block9 >> 42) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 29) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 16) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 3) & 8191L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 7L) << 10) | (block10 >> 54));
      values[values_offset++] = static_cast<int32_t>((block10 >> 41) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 28) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 15) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 2) & 8191L);

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block10 & 3L) << 11) | (block11 >> 53));
      values[values_offset++] = static_cast<int32_t>((block11 >> 40) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 27) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 14) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 1) & 8191L);

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block11 & 1L) << 12) | (block12 >> 52));
      values[values_offset++] = static_cast<int32_t>((block12 >> 39) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 26) & 8191L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 13) & 8191L);
      values[values_offset++] = static_cast<int32_t>(block12 & 8191L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 5) | (byte1 >> 3);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 7) << 10) | (byte2 << 2) | (byte3 >> 6);

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 63) << 7) | (byte4 >> 1);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 1) << 12) | (byte5 << 4) | (byte6 >> 4);

      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 15) << 9) | (byte7 << 1) | (byte8 >> 7);

      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 127) << 6) | (byte9 >> 2);

      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 3) << 11) | (byte10 << 3) | (byte11 >> 5);

      const int32_t byte12 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 31) << 8) | byte12;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 51;
      values[values_offset++] = (block0 >> 38) & 8191L;
      values[values_offset++] = (block0 >> 25) & 8191L;
      values[values_offset++] = (block0 >> 12) & 8191L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 4095L) << 1) | (block1 >> 63);
      values[values_offset++] = (block1 >> 50) & 8191L;
      values[values_offset++] = (block1 >> 37) & 8191L;
      values[values_offset++] = (block1 >> 24) & 8191L;
      values[values_offset++] = (block1 >> 11) & 8191L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 2047L) << 2) | (block2 >> 62);
      values[values_offset++] = (block2 >> 49) & 8191L;
      values[values_offset++] = (block2 >> 36) & 8191L;
      values[values_offset++] = (block2 >> 23) & 8191L;
      values[values_offset++] = (block2 >> 10) & 8191L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 1023L) << 3) | (block3 >> 61);
      values[values_offset++] = (block3 >> 48) & 8191L;
      values[values_offset++] = (block3 >> 35) & 8191L;
      values[values_offset++] = (block3 >> 22) & 8191L;
      values[values_offset++] = (block3 >> 9) & 8191L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 511L) << 4) | (block4 >> 60);
      values[values_offset++] = (block4 >> 47) & 8191L;
      values[values_offset++] = (block4 >> 34) & 8191L;
      values[values_offset++] = (block4 >> 21) & 8191L;
      values[values_offset++] = (block4 >> 8) & 8191L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 255L) << 5) | (block5 >> 59);
      values[values_offset++] = (block5 >> 46) & 8191L;
      values[values_offset++] = (block5 >> 33) & 8191L;
      values[values_offset++] = (block5 >> 20) & 8191L;
      values[values_offset++] = (block5 >> 7) & 8191L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 127L) << 6) | (block6 >> 58);
      values[values_offset++] = (block6 >> 45) & 8191L;
      values[values_offset++] = (block6 >> 32) & 8191L;
      values[values_offset++] = (block6 >> 19) & 8191L;
      values[values_offset++] = (block6 >> 6) & 8191L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 63L) << 7) | (block7 >> 57);
      values[values_offset++] = (block7 >> 44) & 8191L;
      values[values_offset++] = (block7 >> 31) & 8191L;
      values[values_offset++] = (block7 >> 18) & 8191L;
      values[values_offset++] = (block7 >> 5) & 8191L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 31L) << 8) | (block8 >> 56);
      values[values_offset++] = (block8 >> 43) & 8191L;
      values[values_offset++] = (block8 >> 30) & 8191L;
      values[values_offset++] = (block8 >> 17) & 8191L;
      values[values_offset++] = (block8 >> 4) & 8191L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 15L) << 9) | (block9 >> 55);
      values[values_offset++] = (block9 >> 42) & 8191L;
      values[values_offset++] = (block9 >> 29) & 8191L;
      values[values_offset++] = (block9 >> 16) & 8191L;
      values[values_offset++] = (block9 >> 3) & 8191L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 7L) << 10) | (block10 >> 54);
      values[values_offset++] = (block10 >> 41) & 8191L;
      values[values_offset++] = (block10 >> 28) & 8191L;
      values[values_offset++] = (block10 >> 15) & 8191L;
      values[values_offset++] = (block10 >> 2) & 8191L;

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = ((block10 & 3L) << 11) | (block11 >> 53);
      values[values_offset++] = (block11 >> 40) & 8191L;
      values[values_offset++] = (block11 >> 27) & 8191L;
      values[values_offset++] = (block11 >> 14) & 8191L;
      values[values_offset++] = (block11 >> 1) & 8191L;

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = ((block11 & 1L) << 12) | (block12 >> 52);
      values[values_offset++] = (block12 >> 39) & 8191L;
      values[values_offset++] = (block12 >> 26) & 8191L;
      values[values_offset++] = (block12 >> 13) & 8191L;
      values[values_offset++] = block12 & 8191L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 5) | (byte1 >> 3);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 7) << 10) | (byte2 << 2) | (byte3 >> 6);

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 63) << 7) | (byte4 >> 1);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 1) << 12) | (byte5 << 4) | (byte6 >> 4);

      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 15) << 9) | (byte7 << 1) | (byte8 >> 7);

      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 127) << 6) | (byte9 >> 2);

      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 3) << 11) | (byte10 << 3) | (byte11 >> 5);

      const int64_t byte12 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 31) << 8) | byte12;
    }
  }
};

class BulkOperationPacked14 : public BulkOperationPacked {
 public:
  BulkOperationPacked14()
    : BulkOperationPacked(14U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 50);
      values[values_offset++] = static_cast<int32_t>((block0 >> 36) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 22) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 8) & 16383L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 255L) << 6) | (block1 >> 58));
      values[values_offset++] = static_cast<int32_t>((block1 >> 44) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 30) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 16) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 16383L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 12) | (block2 >> 52));
      values[values_offset++] = static_cast<int32_t>((block2 >> 38) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 10) & 16383L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 1023L) << 4) | (block3 >> 60));
      values[values_offset++] = static_cast<int32_t>((block3 >> 46) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 32) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 18) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 4) & 16383L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 15L) << 10) | (block4 >> 54));
      values[values_offset++] = static_cast<int32_t>((block4 >> 40) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 26) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 12) & 16383L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 4095L) << 2) | (block5 >> 62));
      values[values_offset++] = static_cast<int32_t>((block5 >> 48) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 34) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 20) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 6) & 16383L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 63L) << 8) | (block6 >> 56));
      values[values_offset++] = static_cast<int32_t>((block6 >> 42) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 28) & 16383L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 14) & 16383L);
      values[values_offset++] = static_cast<int32_t>(block6 & 16383L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 6) | (byte1 >> 2);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 3) << 12) | (byte2 << 4) | (byte3 >> 4);

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 15) << 10) | (byte4 << 2) | (byte5 >> 6);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 63) << 8) | byte6;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 50;
      values[values_offset++] = (block0 >> 36) & 16383L;
      values[values_offset++] = (block0 >> 22) & 16383L;
      values[values_offset++] = (block0 >> 8) & 16383L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 255L) << 6) | (block1 >> 58);
      values[values_offset++] = (block1 >> 44) & 16383L;
      values[values_offset++] = (block1 >> 30) & 16383L;
      values[values_offset++] = (block1 >> 16) & 16383L;
      values[values_offset++] = (block1 >> 2) & 16383L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 12) | (block2 >> 52);
      values[values_offset++] = (block2 >> 38) & 16383L;
      values[values_offset++] = (block2 >> 24) & 16383L;
      values[values_offset++] = (block2 >> 10) & 16383L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 1023L) << 4) | (block3 >> 60);
      values[values_offset++] = (block3 >> 46) & 16383L;
      values[values_offset++] = (block3 >> 32) & 16383L;
      values[values_offset++] = (block3 >> 18) & 16383L;
      values[values_offset++] = (block3 >> 4) & 16383L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 15L) << 10) | (block4 >> 54);
      values[values_offset++] = (block4 >> 40) & 16383L;
      values[values_offset++] = (block4 >> 26) & 16383L;
      values[values_offset++] = (block4 >> 12) & 16383L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 4095L) << 2) | (block5 >> 62);
      values[values_offset++] = (block5 >> 48) & 16383L;
      values[values_offset++] = (block5 >> 34) & 16383L;
      values[values_offset++] = (block5 >> 20) & 16383L;
      values[values_offset++] = (block5 >> 6) & 16383L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 63L) << 8) | (block6 >> 56);
      values[values_offset++] = (block6 >> 42) & 16383L;
      values[values_offset++] = (block6 >> 28) & 16383L;
      values[values_offset++] = (block6 >> 14) & 16383L;
      values[values_offset++] = block6 & 16383L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 6) | (byte1 >> 2);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 3) << 12) | (byte2 << 4) | (byte3 >> 4);

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 15) << 10) | (byte4 << 2) | (byte5 >> 6);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 63) << 8) | byte6;
    }
  }
};

class BulkOperationPacked15 : public BulkOperationPacked {
 public:
  BulkOperationPacked15()
    : BulkOperationPacked(15U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 49);
      values[values_offset++] = static_cast<int32_t>((block0 >> 34) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 19) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 32767L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 15L) << 11) | (block1 >> 53));
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 23) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 32767L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 255L) << 7) | (block2 >> 57));
      values[values_offset++] = static_cast<int32_t>((block2 >> 42) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 27) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 32767L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 4095L) << 3) | (block3 >> 61));
      values[values_offset++] = static_cast<int32_t>((block3 >> 46) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 31) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 16) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 1) & 32767L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 1L) << 14) | (block4 >> 50));
      values[values_offset++] = static_cast<int32_t>((block4 >> 35) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 20) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 5) & 32767L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 31L) << 10) | (block5 >> 54));
      values[values_offset++] = static_cast<int32_t>((block5 >> 39) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 24) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 9) & 32767L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 511L) << 6) | (block6 >> 58));
      values[values_offset++] = static_cast<int32_t>((block6 >> 43) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 28) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 13) & 32767L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 8191L) << 2) | (block7 >> 62));
      values[values_offset++] = static_cast<int32_t>((block7 >> 47) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 32) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 17) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 2) & 32767L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 3L) << 13) | (block8 >> 51));
      values[values_offset++] = static_cast<int32_t>((block8 >> 36) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 21) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 6) & 32767L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 63L) << 9) | (block9 >> 55));
      values[values_offset++] = static_cast<int32_t>((block9 >> 40) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 25) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 10) & 32767L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 1023L) << 5) | (block10 >> 59));
      values[values_offset++] = static_cast<int32_t>((block10 >> 44) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 29) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 14) & 32767L);

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block10 & 16383L) << 1) | (block11 >> 63));
      values[values_offset++] = static_cast<int32_t>((block11 >> 48) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 33) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 18) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 3) & 32767L);

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block11 & 7L) << 12) | (block12 >> 52));
      values[values_offset++] = static_cast<int32_t>((block12 >> 37) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 22) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 7) & 32767L);

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block12 & 127L) << 8) | (block13 >> 56));
      values[values_offset++] = static_cast<int32_t>((block13 >> 41) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 26) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 11) & 32767L);

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block13 & 2047L) << 4) | (block14 >> 60));
      values[values_offset++] = static_cast<int32_t>((block14 >> 45) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 30) & 32767L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 15) & 32767L);
      values[values_offset++] = static_cast<int32_t>(block14 & 32767L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 7) | (byte1 >> 1);

      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 1) << 14) | (byte2 << 6) | (byte3 >> 2);

      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 3) << 13) | (byte4 << 5) | (byte5 >> 3);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 7) << 12) | (byte6 << 4) | (byte7 >> 4);

      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 15) << 11) | (byte8 << 3) | (byte9 >> 5);

      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 31) << 10) | (byte10 << 2) | (byte11 >> 6);

      const int32_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte13 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 63) << 9) | (byte12 << 1) | (byte13 >> 7);

      const int32_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte13 & 127) << 8) | byte14;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 49;
      values[values_offset++] = (block0 >> 34) & 32767L;
      values[values_offset++] = (block0 >> 19) & 32767L;
      values[values_offset++] = (block0 >> 4) & 32767L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 15L) << 11) | (block1 >> 53);
      values[values_offset++] = (block1 >> 38) & 32767L;
      values[values_offset++] = (block1 >> 23) & 32767L;
      values[values_offset++] = (block1 >> 8) & 32767L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 255L) << 7) | (block2 >> 57);
      values[values_offset++] = (block2 >> 42) & 32767L;
      values[values_offset++] = (block2 >> 27) & 32767L;
      values[values_offset++] = (block2 >> 12) & 32767L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 4095L) << 3) | (block3 >> 61);
      values[values_offset++] = (block3 >> 46) & 32767L;
      values[values_offset++] = (block3 >> 31) & 32767L;
      values[values_offset++] = (block3 >> 16) & 32767L;
      values[values_offset++] = (block3 >> 1) & 32767L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 1L) << 14) | (block4 >> 50);
      values[values_offset++] = (block4 >> 35) & 32767L;
      values[values_offset++] = (block4 >> 20) & 32767L;
      values[values_offset++] = (block4 >> 5) & 32767L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 31L) << 10) | (block5 >> 54);
      values[values_offset++] = (block5 >> 39) & 32767L;
      values[values_offset++] = (block5 >> 24) & 32767L;
      values[values_offset++] = (block5 >> 9) & 32767L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 511L) << 6) | (block6 >> 58);
      values[values_offset++] = (block6 >> 43) & 32767L;
      values[values_offset++] = (block6 >> 28) & 32767L;
      values[values_offset++] = (block6 >> 13) & 32767L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 8191L) << 2) | (block7 >> 62);
      values[values_offset++] = (block7 >> 47) & 32767L;
      values[values_offset++] = (block7 >> 32) & 32767L;
      values[values_offset++] = (block7 >> 17) & 32767L;
      values[values_offset++] = (block7 >> 2) & 32767L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 3L) << 13) | (block8 >> 51);
      values[values_offset++] = (block8 >> 36) & 32767L;
      values[values_offset++] = (block8 >> 21) & 32767L;
      values[values_offset++] = (block8 >> 6) & 32767L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 63L) << 9) | (block9 >> 55);
      values[values_offset++] = (block9 >> 40) & 32767L;
      values[values_offset++] = (block9 >> 25) & 32767L;
      values[values_offset++] = (block9 >> 10) & 32767L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 1023L) << 5) | (block10 >> 59);
      values[values_offset++] = (block10 >> 44) & 32767L;
      values[values_offset++] = (block10 >> 29) & 32767L;
      values[values_offset++] = (block10 >> 14) & 32767L;

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = ((block10 & 16383L) << 1) | (block11 >> 63);
      values[values_offset++] = (block11 >> 48) & 32767L;
      values[values_offset++] = (block11 >> 33) & 32767L;
      values[values_offset++] = (block11 >> 18) & 32767L;
      values[values_offset++] = (block11 >> 3) & 32767L;

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = ((block11 & 7L) << 12) | (block12 >> 52);
      values[values_offset++] = (block12 >> 37) & 32767L;
      values[values_offset++] = (block12 >> 22) & 32767L;
      values[values_offset++] = (block12 >> 7) & 32767L;

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = ((block12 & 127L) << 8) | (block13 >> 56);
      values[values_offset++] = (block13 >> 41) & 32767L;
      values[values_offset++] = (block13 >> 26) & 32767L;
      values[values_offset++] = (block13 >> 11) & 32767L;

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = ((block13 & 2047L) << 4) | (block14 >> 60);
      values[values_offset++] = (block14 >> 45) & 32767L;
      values[values_offset++] = (block14 >> 30) & 32767L;
      values[values_offset++] = (block14 >> 15) & 32767L;
      values[values_offset++] = block14 & 32767L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 7) | (byte1 >> 1);

      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte1 & 1) << 14) | (byte2 << 6) | (byte3 >> 2);

      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte3 & 3) << 13) | (byte4 << 5) | (byte5 >> 3);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 7) << 12) | (byte6 << 4) | (byte7 >> 4);

      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 15) << 11) | (byte8 << 3) | (byte9 >> 5);

      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 31) << 10) | (byte10 << 2) | (byte11 >> 6);

      const int64_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte13 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 63) << 9) | (byte12 << 1) | (byte13 >> 7);

      const int64_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte13 & 127) << 8) | byte14;
    }
  }
};

class BulkOperationPacked16 : public BulkOperationPacked {
 public:
  BulkOperationPacked16()
    : BulkOperationPacked(16U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 48 ; shift >= 0 ; shift -= 16) {
        values[values_offset++] = static_cast<int32_t>((block >> shift) & 65535);
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      values[values_offset++] =
   ((blocks[blocks_offset++] & 0xFF) << 8) |
 (blocks[blocks_offset++] & 0xFF);

    }

  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = 48 ; shift >= 0 ; shift -= 16) {
        values[values_offset++] = (block >> shift) & 65535;
      }
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t j = 0 ; j < iterations ; ++j) {
      values[values_offset++] =
   ((blocks[blocks_offset++] & 0xFFL) << 8) |
 (blocks[blocks_offset++] & 0xFFL);

    }

  }
};

class BulkOperationPacked17 : public BulkOperationPacked {
 public:
  BulkOperationPacked17()
    : BulkOperationPacked(17U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 47);
      values[values_offset++] = static_cast<int32_t>((block0 >> 30) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 13) & 131071L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 8191L) << 4) | (block1 >> 60));
      values[values_offset++] = static_cast<int32_t>((block1 >> 43) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 26) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 9) & 131071L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 511L) << 8) | (block2 >> 56));
      values[values_offset++] = static_cast<int32_t>((block2 >> 39) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 22) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 5) & 131071L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 31L) << 12) | (block3 >> 52));
      values[values_offset++] = static_cast<int32_t>((block3 >> 35) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 18) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 1) & 131071L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 1L) << 16) | (block4 >> 48));
      values[values_offset++] = static_cast<int32_t>((block4 >> 31) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 14) & 131071L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 16383L) << 3) | (block5 >> 61));
      values[values_offset++] = static_cast<int32_t>((block5 >> 44) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 27) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 10) & 131071L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 1023L) << 7) | (block6 >> 57));
      values[values_offset++] = static_cast<int32_t>((block6 >> 40) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 23) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 6) & 131071L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 63L) << 11) | (block7 >> 53));
      values[values_offset++] = static_cast<int32_t>((block7 >> 36) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 19) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 2) & 131071L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 3L) << 15) | (block8 >> 49));
      values[values_offset++] = static_cast<int32_t>((block8 >> 32) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 15) & 131071L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 32767L) << 2) | (block9 >> 62));
      values[values_offset++] = static_cast<int32_t>((block9 >> 45) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 28) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 11) & 131071L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 2047L) << 6) | (block10 >> 58));
      values[values_offset++] = static_cast<int32_t>((block10 >> 41) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 24) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 7) & 131071L);

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block10 & 127L) << 10) | (block11 >> 54));
      values[values_offset++] = static_cast<int32_t>((block11 >> 37) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 20) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 3) & 131071L);

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block11 & 7L) << 14) | (block12 >> 50));
      values[values_offset++] = static_cast<int32_t>((block12 >> 33) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 16) & 131071L);

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block12 & 65535L) << 1) | (block13 >> 63));
      values[values_offset++] = static_cast<int32_t>((block13 >> 46) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 29) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 12) & 131071L);

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block13 & 4095L) << 5) | (block14 >> 59));
      values[values_offset++] = static_cast<int32_t>((block14 >> 42) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 25) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 8) & 131071L);

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block14 & 255L) << 9) | (block15 >> 55));
      values[values_offset++] = static_cast<int32_t>((block15 >> 38) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block15 >> 21) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block15 >> 4) & 131071L);

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block15 & 15L) << 13) | (block16 >> 51));
      values[values_offset++] = static_cast<int32_t>((block16 >> 34) & 131071L);
      values[values_offset++] = static_cast<int32_t>((block16 >> 17) & 131071L);
      values[values_offset++] = static_cast<int32_t>(block16 & 131071L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 9) | (byte1 << 1) | (byte2 >> 7);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 127) << 10) | (byte3 << 2) | (byte4 >> 6);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 63) << 11) | (byte5 << 3) | (byte6 >> 5);

      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 31) << 12) | (byte7 << 4) | (byte8 >> 4);

      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 15) << 13) | (byte9 << 5) | (byte10 >> 3);

      const int32_t byte11 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte12 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte10 & 7) << 14) | (byte11 << 6) | (byte12 >> 2);

      const int32_t byte13 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte12 & 3) << 15) | (byte13 << 7) | (byte14 >> 1);

      const int32_t byte15 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte16 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte14 & 1) << 16) | (byte15 << 8) | byte16;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 47;
      values[values_offset++] = (block0 >> 30) & 131071L;
      values[values_offset++] = (block0 >> 13) & 131071L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 8191L) << 4) | (block1 >> 60);
      values[values_offset++] = (block1 >> 43) & 131071L;
      values[values_offset++] = (block1 >> 26) & 131071L;
      values[values_offset++] = (block1 >> 9) & 131071L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 511L) << 8) | (block2 >> 56);
      values[values_offset++] = (block2 >> 39) & 131071L;
      values[values_offset++] = (block2 >> 22) & 131071L;
      values[values_offset++] = (block2 >> 5) & 131071L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 31L) << 12) | (block3 >> 52);
      values[values_offset++] = (block3 >> 35) & 131071L;
      values[values_offset++] = (block3 >> 18) & 131071L;
      values[values_offset++] = (block3 >> 1) & 131071L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 1L) << 16) | (block4 >> 48);
      values[values_offset++] = (block4 >> 31) & 131071L;
      values[values_offset++] = (block4 >> 14) & 131071L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 16383L) << 3) | (block5 >> 61);
      values[values_offset++] = (block5 >> 44) & 131071L;
      values[values_offset++] = (block5 >> 27) & 131071L;
      values[values_offset++] = (block5 >> 10) & 131071L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 1023L) << 7) | (block6 >> 57);
      values[values_offset++] = (block6 >> 40) & 131071L;
      values[values_offset++] = (block6 >> 23) & 131071L;
      values[values_offset++] = (block6 >> 6) & 131071L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 63L) << 11) | (block7 >> 53);
      values[values_offset++] = (block7 >> 36) & 131071L;
      values[values_offset++] = (block7 >> 19) & 131071L;
      values[values_offset++] = (block7 >> 2) & 131071L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 3L) << 15) | (block8 >> 49);
      values[values_offset++] = (block8 >> 32) & 131071L;
      values[values_offset++] = (block8 >> 15) & 131071L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 32767L) << 2) | (block9 >> 62);
      values[values_offset++] = (block9 >> 45) & 131071L;
      values[values_offset++] = (block9 >> 28) & 131071L;
      values[values_offset++] = (block9 >> 11) & 131071L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 2047L) << 6) | (block10 >> 58);
      values[values_offset++] = (block10 >> 41) & 131071L;
      values[values_offset++] = (block10 >> 24) & 131071L;
      values[values_offset++] = (block10 >> 7) & 131071L;

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = ((block10 & 127L) << 10) | (block11 >> 54);
      values[values_offset++] = (block11 >> 37) & 131071L;
      values[values_offset++] = (block11 >> 20) & 131071L;
      values[values_offset++] = (block11 >> 3) & 131071L;

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = ((block11 & 7L) << 14) | (block12 >> 50);
      values[values_offset++] = (block12 >> 33) & 131071L;
      values[values_offset++] = (block12 >> 16) & 131071L;

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = ((block12 & 65535L) << 1) | (block13 >> 63);
      values[values_offset++] = (block13 >> 46) & 131071L;
      values[values_offset++] = (block13 >> 29) & 131071L;
      values[values_offset++] = (block13 >> 12) & 131071L;

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = ((block13 & 4095L) << 5) | (block14 >> 59);
      values[values_offset++] = (block14 >> 42) & 131071L;
      values[values_offset++] = (block14 >> 25) & 131071L;
      values[values_offset++] = (block14 >> 8) & 131071L;

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = ((block14 & 255L) << 9) | (block15 >> 55);
      values[values_offset++] = (block15 >> 38) & 131071L;
      values[values_offset++] = (block15 >> 21) & 131071L;
      values[values_offset++] = (block15 >> 4) & 131071L;

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = ((block15 & 15L) << 13) | (block16 >> 51);
      values[values_offset++] = (block16 >> 34) & 131071L;
      values[values_offset++] = (block16 >> 17) & 131071L;
      values[values_offset++] = block16 & 131071L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 9) | (byte1 << 1) | (byte2 >> 7);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 127) << 10) | (byte3 << 2) | (byte4 >> 6);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 63) << 11) | (byte5 << 3) | (byte6 >> 5);

      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 31) << 12) | (byte7 << 4) | (byte8 >> 4);

      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 15) << 13) | (byte9 << 5) | (byte10 >> 3);

      const int64_t byte11 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte12 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte10 & 7) << 14) | (byte11 << 6) | (byte12 >> 2);

      const int64_t byte13 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte12 & 3) << 15) | (byte13 << 7) | (byte14 >> 1);

      const int64_t byte15 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte16 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte14 & 1) << 16) | (byte15 << 8) | byte16;
    }
  }
};

class BulkOperationPacked18 : public BulkOperationPacked {
 public:
  BulkOperationPacked18()
    : BulkOperationPacked(18U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 46);
      values[values_offset++] = static_cast<int32_t>((block0 >> 28) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 10) & 262143L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 1023L) << 8) | (block1 >> 56));
      values[values_offset++] = static_cast<int32_t>((block1 >> 38) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 20) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 262143L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 16) | (block2 >> 48));
      values[values_offset++] = static_cast<int32_t>((block2 >> 30) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 262143L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 4095L) << 6) | (block3 >> 58));
      values[values_offset++] = static_cast<int32_t>((block3 >> 40) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 22) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 4) & 262143L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 15L) << 14) | (block4 >> 50));
      values[values_offset++] = static_cast<int32_t>((block4 >> 32) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 14) & 262143L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 16383L) << 4) | (block5 >> 60));
      values[values_offset++] = static_cast<int32_t>((block5 >> 42) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 24) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 6) & 262143L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 63L) << 12) | (block6 >> 52));
      values[values_offset++] = static_cast<int32_t>((block6 >> 34) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 16) & 262143L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 65535L) << 2) | (block7 >> 62));
      values[values_offset++] = static_cast<int32_t>((block7 >> 44) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 26) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 8) & 262143L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 255L) << 10) | (block8 >> 54));
      values[values_offset++] = static_cast<int32_t>((block8 >> 36) & 262143L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 18) & 262143L);
      values[values_offset++] = static_cast<int32_t>(block8 & 262143L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 10) | (byte1 << 2) | (byte2 >> 6);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 63) << 12) | (byte3 << 4) | (byte4 >> 4);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 15) << 14) | (byte5 << 6) | (byte6 >> 2);

      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 3) << 16) | (byte7 << 8) | byte8;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 46;
      values[values_offset++] = (block0 >> 28) & 262143L;
      values[values_offset++] = (block0 >> 10) & 262143L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 1023L) << 8) | (block1 >> 56);
      values[values_offset++] = (block1 >> 38) & 262143L;
      values[values_offset++] = (block1 >> 20) & 262143L;
      values[values_offset++] = (block1 >> 2) & 262143L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 16) | (block2 >> 48);
      values[values_offset++] = (block2 >> 30) & 262143L;
      values[values_offset++] = (block2 >> 12) & 262143L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 4095L) << 6) | (block3 >> 58);
      values[values_offset++] = (block3 >> 40) & 262143L;
      values[values_offset++] = (block3 >> 22) & 262143L;
      values[values_offset++] = (block3 >> 4) & 262143L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 15L) << 14) | (block4 >> 50);
      values[values_offset++] = (block4 >> 32) & 262143L;
      values[values_offset++] = (block4 >> 14) & 262143L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 16383L) << 4) | (block5 >> 60);
      values[values_offset++] = (block5 >> 42) & 262143L;
      values[values_offset++] = (block5 >> 24) & 262143L;
      values[values_offset++] = (block5 >> 6) & 262143L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 63L) << 12) | (block6 >> 52);
      values[values_offset++] = (block6 >> 34) & 262143L;
      values[values_offset++] = (block6 >> 16) & 262143L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 65535L) << 2) | (block7 >> 62);
      values[values_offset++] = (block7 >> 44) & 262143L;
      values[values_offset++] = (block7 >> 26) & 262143L;
      values[values_offset++] = (block7 >> 8) & 262143L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 255L) << 10) | (block8 >> 54);
      values[values_offset++] = (block8 >> 36) & 262143L;
      values[values_offset++] = (block8 >> 18) & 262143L;
      values[values_offset++] = block8 & 262143L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 10) | (byte1 << 2) | (byte2 >> 6);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 63) << 12) | (byte3 << 4) | (byte4 >> 4);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 15) << 14) | (byte5 << 6) | (byte6 >> 2);

      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte6 & 3) << 16) | (byte7 << 8) | byte8;
    }
  }
};

class BulkOperationPacked19 : public BulkOperationPacked {
 public:
  BulkOperationPacked19()
    : BulkOperationPacked(19U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 45);
      values[values_offset++] = static_cast<int32_t>((block0 >> 26) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 7) & 524287L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 127L) << 12) | (block1 >> 52));
      values[values_offset++] = static_cast<int32_t>((block1 >> 33) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 14) & 524287L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 16383L) << 5) | (block2 >> 59));
      values[values_offset++] = static_cast<int32_t>((block2 >> 40) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 21) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 2) & 524287L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 3L) << 17) | (block3 >> 47));
      values[values_offset++] = static_cast<int32_t>((block3 >> 28) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 9) & 524287L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 511L) << 10) | (block4 >> 54));
      values[values_offset++] = static_cast<int32_t>((block4 >> 35) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 16) & 524287L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 65535L) << 3) | (block5 >> 61));
      values[values_offset++] = static_cast<int32_t>((block5 >> 42) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 23) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 4) & 524287L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 15L) << 15) | (block6 >> 49));
      values[values_offset++] = static_cast<int32_t>((block6 >> 30) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 11) & 524287L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 2047L) << 8) | (block7 >> 56));
      values[values_offset++] = static_cast<int32_t>((block7 >> 37) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 18) & 524287L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 262143L) << 1) | (block8 >> 63));
      values[values_offset++] = static_cast<int32_t>((block8 >> 44) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 25) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 6) & 524287L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 63L) << 13) | (block9 >> 51));
      values[values_offset++] = static_cast<int32_t>((block9 >> 32) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 13) & 524287L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 8191L) << 6) | (block10 >> 58));
      values[values_offset++] = static_cast<int32_t>((block10 >> 39) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 20) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 1) & 524287L);

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block10 & 1L) << 18) | (block11 >> 46));
      values[values_offset++] = static_cast<int32_t>((block11 >> 27) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 8) & 524287L);

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block11 & 255L) << 11) | (block12 >> 53));
      values[values_offset++] = static_cast<int32_t>((block12 >> 34) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 15) & 524287L);

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block12 & 32767L) << 4) | (block13 >> 60));
      values[values_offset++] = static_cast<int32_t>((block13 >> 41) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 22) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 3) & 524287L);

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block13 & 7L) << 16) | (block14 >> 48));
      values[values_offset++] = static_cast<int32_t>((block14 >> 29) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 10) & 524287L);

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block14 & 1023L) << 9) | (block15 >> 55));
      values[values_offset++] = static_cast<int32_t>((block15 >> 36) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block15 >> 17) & 524287L);

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block15 & 131071L) << 2) | (block16 >> 62));
      values[values_offset++] = static_cast<int32_t>((block16 >> 43) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block16 >> 24) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block16 >> 5) & 524287L);

      const uint64_t block17 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block16 & 31L) << 14) | (block17 >> 50));
      values[values_offset++] = static_cast<int32_t>((block17 >> 31) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block17 >> 12) & 524287L);

      const uint64_t block18 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block17 & 4095L) << 7) | (block18 >> 57));
      values[values_offset++] = static_cast<int32_t>((block18 >> 38) & 524287L);
      values[values_offset++] = static_cast<int32_t>((block18 >> 19) & 524287L);
      values[values_offset++] = static_cast<int32_t>(block18 & 524287L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 11) | (byte1 << 3) | (byte2 >> 5);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 31) << 14) | (byte3 << 6) | (byte4 >> 2);

      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 3) << 17) | (byte5 << 9) | (byte6 << 1) | (byte7 >> 7);

      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 127) << 12) | (byte8 << 4) | (byte9 >> 4);

      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 15) << 15) | (byte10 << 7) | (byte11 >> 1);

      const int32_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte13 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 1) << 18) | (byte12 << 10) | (byte13 << 2) | (byte14 >> 6);

      const int32_t byte15 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte16 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte14 & 63) << 13) | (byte15 << 5) | (byte16 >> 3);

      const int32_t byte17 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte18 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte16 & 7) << 16) | (byte17 << 8) | byte18;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 45;
      values[values_offset++] = (block0 >> 26) & 524287L;
      values[values_offset++] = (block0 >> 7) & 524287L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 127L) << 12) | (block1 >> 52);
      values[values_offset++] = (block1 >> 33) & 524287L;
      values[values_offset++] = (block1 >> 14) & 524287L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 16383L) << 5) | (block2 >> 59);
      values[values_offset++] = (block2 >> 40) & 524287L;
      values[values_offset++] = (block2 >> 21) & 524287L;
      values[values_offset++] = (block2 >> 2) & 524287L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 3L) << 17) | (block3 >> 47);
      values[values_offset++] = (block3 >> 28) & 524287L;
      values[values_offset++] = (block3 >> 9) & 524287L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 511L) << 10) | (block4 >> 54);
      values[values_offset++] = (block4 >> 35) & 524287L;
      values[values_offset++] = (block4 >> 16) & 524287L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 65535L) << 3) | (block5 >> 61);
      values[values_offset++] = (block5 >> 42) & 524287L;
      values[values_offset++] = (block5 >> 23) & 524287L;
      values[values_offset++] = (block5 >> 4) & 524287L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 15L) << 15) | (block6 >> 49);
      values[values_offset++] = (block6 >> 30) & 524287L;
      values[values_offset++] = (block6 >> 11) & 524287L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 2047L) << 8) | (block7 >> 56);
      values[values_offset++] = (block7 >> 37) & 524287L;
      values[values_offset++] = (block7 >> 18) & 524287L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 262143L) << 1) | (block8 >> 63);
      values[values_offset++] = (block8 >> 44) & 524287L;
      values[values_offset++] = (block8 >> 25) & 524287L;
      values[values_offset++] = (block8 >> 6) & 524287L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 63L) << 13) | (block9 >> 51);
      values[values_offset++] = (block9 >> 32) & 524287L;
      values[values_offset++] = (block9 >> 13) & 524287L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 8191L) << 6) | (block10 >> 58);
      values[values_offset++] = (block10 >> 39) & 524287L;
      values[values_offset++] = (block10 >> 20) & 524287L;
      values[values_offset++] = (block10 >> 1) & 524287L;

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = ((block10 & 1L) << 18) | (block11 >> 46);
      values[values_offset++] = (block11 >> 27) & 524287L;
      values[values_offset++] = (block11 >> 8) & 524287L;

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = ((block11 & 255L) << 11) | (block12 >> 53);
      values[values_offset++] = (block12 >> 34) & 524287L;
      values[values_offset++] = (block12 >> 15) & 524287L;

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = ((block12 & 32767L) << 4) | (block13 >> 60);
      values[values_offset++] = (block13 >> 41) & 524287L;
      values[values_offset++] = (block13 >> 22) & 524287L;
      values[values_offset++] = (block13 >> 3) & 524287L;

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = ((block13 & 7L) << 16) | (block14 >> 48);
      values[values_offset++] = (block14 >> 29) & 524287L;
      values[values_offset++] = (block14 >> 10) & 524287L;

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = ((block14 & 1023L) << 9) | (block15 >> 55);
      values[values_offset++] = (block15 >> 36) & 524287L;
      values[values_offset++] = (block15 >> 17) & 524287L;

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = ((block15 & 131071L) << 2) | (block16 >> 62);
      values[values_offset++] = (block16 >> 43) & 524287L;
      values[values_offset++] = (block16 >> 24) & 524287L;
      values[values_offset++] = (block16 >> 5) & 524287L;

      const uint64_t block17 = blocks[blocks_offset++];
      values[values_offset++] = ((block16 & 31L) << 14) | (block17 >> 50);
      values[values_offset++] = (block17 >> 31) & 524287L;
      values[values_offset++] = (block17 >> 12) & 524287L;

      const uint64_t block18 = blocks[blocks_offset++];
      values[values_offset++] = ((block17 & 4095L) << 7) | (block18 >> 57);
      values[values_offset++] = (block18 >> 38) & 524287L;
      values[values_offset++] = (block18 >> 19) & 524287L;
      values[values_offset++] = block18 & 524287L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 11) | (byte1 << 3) | (byte2 >> 5);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 31) << 14) | (byte3 << 6) | (byte4 >> 2);

      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte4 & 3) << 17) | (byte5 << 9) | (byte6 << 1) | (byte7 >> 7);

      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 127) << 12) | (byte8 << 4) | (byte9 >> 4);

      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte9 & 15) << 15) | (byte10 << 7) | (byte11 >> 1);

      const int64_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte13 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 1) << 18) | (byte12 << 10) | (byte13 << 2) | (byte14 >> 6);

      const int64_t byte15 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte16 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte14 & 63) << 13) | (byte15 << 5) | (byte16 >> 3);

      const int64_t byte17 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte18 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte16 & 7) << 16) | (byte17 << 8) | byte18;
    }
  }
};

class BulkOperationPacked20 : public BulkOperationPacked {
 public:
  BulkOperationPacked20()
    : BulkOperationPacked(20U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 44);
      values[values_offset++] = static_cast<int32_t>((block0 >> 24) & 1048575L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 4) & 1048575L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 15L) << 16) | (block1 >> 48));
      values[values_offset++] = static_cast<int32_t>((block1 >> 28) & 1048575L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 1048575L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 255L) << 12) | (block2 >> 52));
      values[values_offset++] = static_cast<int32_t>((block2 >> 32) & 1048575L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 12) & 1048575L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 4095L) << 8) | (block3 >> 56));
      values[values_offset++] = static_cast<int32_t>((block3 >> 36) & 1048575L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 16) & 1048575L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 65535L) << 4) | (block4 >> 60));
      values[values_offset++] = static_cast<int32_t>((block4 >> 40) & 1048575L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 20) & 1048575L);
      values[values_offset++] = static_cast<int32_t>(block4 & 1048575L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 12) | (byte1 << 4) | (byte2 >> 4);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 15) << 16) | (byte3 << 8) | byte4;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 44;
      values[values_offset++] = (block0 >> 24) & 1048575L;
      values[values_offset++] = (block0 >> 4) & 1048575L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 15L) << 16) | (block1 >> 48);
      values[values_offset++] = (block1 >> 28) & 1048575L;
      values[values_offset++] = (block1 >> 8) & 1048575L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 255L) << 12) | (block2 >> 52);
      values[values_offset++] = (block2 >> 32) & 1048575L;
      values[values_offset++] = (block2 >> 12) & 1048575L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 4095L) << 8) | (block3 >> 56);
      values[values_offset++] = (block3 >> 36) & 1048575L;
      values[values_offset++] = (block3 >> 16) & 1048575L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 65535L) << 4) | (block4 >> 60);
      values[values_offset++] = (block4 >> 40) & 1048575L;
      values[values_offset++] = (block4 >> 20) & 1048575L;
      values[values_offset++] = block4 & 1048575L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 12) | (byte1 << 4) | (byte2 >> 4);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 15) << 16) | (byte3 << 8) | byte4;
    }
  }
};

class BulkOperationPacked21 : public BulkOperationPacked {
 public:
  BulkOperationPacked21()
    : BulkOperationPacked(21U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 43);
      values[values_offset++] = static_cast<int32_t>((block0 >> 22) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block0 >> 1) & 2097151L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 1L) << 20) | (block1 >> 44));
      values[values_offset++] = static_cast<int32_t>((block1 >> 23) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 2) & 2097151L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 3L) << 19) | (block2 >> 45));
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 3) & 2097151L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 7L) << 18) | (block3 >> 46));
      values[values_offset++] = static_cast<int32_t>((block3 >> 25) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 4) & 2097151L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 15L) << 17) | (block4 >> 47));
      values[values_offset++] = static_cast<int32_t>((block4 >> 26) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 5) & 2097151L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 31L) << 16) | (block5 >> 48));
      values[values_offset++] = static_cast<int32_t>((block5 >> 27) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 6) & 2097151L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 63L) << 15) | (block6 >> 49));
      values[values_offset++] = static_cast<int32_t>((block6 >> 28) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 7) & 2097151L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 127L) << 14) | (block7 >> 50));
      values[values_offset++] = static_cast<int32_t>((block7 >> 29) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 8) & 2097151L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 255L) << 13) | (block8 >> 51));
      values[values_offset++] = static_cast<int32_t>((block8 >> 30) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 9) & 2097151L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 511L) << 12) | (block9 >> 52));
      values[values_offset++] = static_cast<int32_t>((block9 >> 31) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 10) & 2097151L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 1023L) << 11) | (block10 >> 53));
      values[values_offset++] = static_cast<int32_t>((block10 >> 32) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 11) & 2097151L);

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block10 & 2047L) << 10) | (block11 >> 54));
      values[values_offset++] = static_cast<int32_t>((block11 >> 33) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 12) & 2097151L);

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block11 & 4095L) << 9) | (block12 >> 55));
      values[values_offset++] = static_cast<int32_t>((block12 >> 34) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 13) & 2097151L);

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block12 & 8191L) << 8) | (block13 >> 56));
      values[values_offset++] = static_cast<int32_t>((block13 >> 35) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block13 >> 14) & 2097151L);

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block13 & 16383L) << 7) | (block14 >> 57));
      values[values_offset++] = static_cast<int32_t>((block14 >> 36) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 15) & 2097151L);

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block14 & 32767L) << 6) | (block15 >> 58));
      values[values_offset++] = static_cast<int32_t>((block15 >> 37) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block15 >> 16) & 2097151L);

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block15 & 65535L) << 5) | (block16 >> 59));
      values[values_offset++] = static_cast<int32_t>((block16 >> 38) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block16 >> 17) & 2097151L);

      const uint64_t block17 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block16 & 131071L) << 4) | (block17 >> 60));
      values[values_offset++] = static_cast<int32_t>((block17 >> 39) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block17 >> 18) & 2097151L);

      const uint64_t block18 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block17 & 262143L) << 3) | (block18 >> 61));
      values[values_offset++] = static_cast<int32_t>((block18 >> 40) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block18 >> 19) & 2097151L);

      const uint64_t block19 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block18 & 524287L) << 2) | (block19 >> 62));
      values[values_offset++] = static_cast<int32_t>((block19 >> 41) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block19 >> 20) & 2097151L);

      const uint64_t block20 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block19 & 1048575L) << 1) | (block20 >> 63));
      values[values_offset++] = static_cast<int32_t>((block20 >> 42) & 2097151L);
      values[values_offset++] = static_cast<int32_t>((block20 >> 21) & 2097151L);
      values[values_offset++] = static_cast<int32_t>(block20 & 2097151L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 13) | (byte1 << 5) | (byte2 >> 3);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 7) << 18) | (byte3 << 10) | (byte4 << 2) | (byte5 >> 6);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 63) << 15) | (byte6 << 7) | (byte7 >> 1);

      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 1) << 20) | (byte8 << 12) | (byte9 << 4) | (byte10 >> 4);

      const int32_t byte11 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte13 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte10 & 15) << 17) | (byte11 << 9) | (byte12 << 1) | (byte13 >> 7);

      const int32_t byte14 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte15 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte13 & 127) << 14) | (byte14 << 6) | (byte15 >> 2);

      const int32_t byte16 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte17 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte18 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte15 & 3) << 19) | (byte16 << 11) | (byte17 << 3) | (byte18 >> 5);

      const int32_t byte19 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte20 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte18 & 31) << 16) | (byte19 << 8) | byte20;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 43;
      values[values_offset++] = (block0 >> 22) & 2097151L;
      values[values_offset++] = (block0 >> 1) & 2097151L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 1L) << 20) | (block1 >> 44);
      values[values_offset++] = (block1 >> 23) & 2097151L;
      values[values_offset++] = (block1 >> 2) & 2097151L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 3L) << 19) | (block2 >> 45);
      values[values_offset++] = (block2 >> 24) & 2097151L;
      values[values_offset++] = (block2 >> 3) & 2097151L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 7L) << 18) | (block3 >> 46);
      values[values_offset++] = (block3 >> 25) & 2097151L;
      values[values_offset++] = (block3 >> 4) & 2097151L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 15L) << 17) | (block4 >> 47);
      values[values_offset++] = (block4 >> 26) & 2097151L;
      values[values_offset++] = (block4 >> 5) & 2097151L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 31L) << 16) | (block5 >> 48);
      values[values_offset++] = (block5 >> 27) & 2097151L;
      values[values_offset++] = (block5 >> 6) & 2097151L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 63L) << 15) | (block6 >> 49);
      values[values_offset++] = (block6 >> 28) & 2097151L;
      values[values_offset++] = (block6 >> 7) & 2097151L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 127L) << 14) | (block7 >> 50);
      values[values_offset++] = (block7 >> 29) & 2097151L;
      values[values_offset++] = (block7 >> 8) & 2097151L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 255L) << 13) | (block8 >> 51);
      values[values_offset++] = (block8 >> 30) & 2097151L;
      values[values_offset++] = (block8 >> 9) & 2097151L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 511L) << 12) | (block9 >> 52);
      values[values_offset++] = (block9 >> 31) & 2097151L;
      values[values_offset++] = (block9 >> 10) & 2097151L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 1023L) << 11) | (block10 >> 53);
      values[values_offset++] = (block10 >> 32) & 2097151L;
      values[values_offset++] = (block10 >> 11) & 2097151L;

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = ((block10 & 2047L) << 10) | (block11 >> 54);
      values[values_offset++] = (block11 >> 33) & 2097151L;
      values[values_offset++] = (block11 >> 12) & 2097151L;

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = ((block11 & 4095L) << 9) | (block12 >> 55);
      values[values_offset++] = (block12 >> 34) & 2097151L;
      values[values_offset++] = (block12 >> 13) & 2097151L;

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = ((block12 & 8191L) << 8) | (block13 >> 56);
      values[values_offset++] = (block13 >> 35) & 2097151L;
      values[values_offset++] = (block13 >> 14) & 2097151L;

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = ((block13 & 16383L) << 7) | (block14 >> 57);
      values[values_offset++] = (block14 >> 36) & 2097151L;
      values[values_offset++] = (block14 >> 15) & 2097151L;

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = ((block14 & 32767L) << 6) | (block15 >> 58);
      values[values_offset++] = (block15 >> 37) & 2097151L;
      values[values_offset++] = (block15 >> 16) & 2097151L;

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = ((block15 & 65535L) << 5) | (block16 >> 59);
      values[values_offset++] = (block16 >> 38) & 2097151L;
      values[values_offset++] = (block16 >> 17) & 2097151L;

      const uint64_t block17 = blocks[blocks_offset++];
      values[values_offset++] = ((block16 & 131071L) << 4) | (block17 >> 60);
      values[values_offset++] = (block17 >> 39) & 2097151L;
      values[values_offset++] = (block17 >> 18) & 2097151L;

      const uint64_t block18 = blocks[blocks_offset++];
      values[values_offset++] = ((block17 & 262143L) << 3) | (block18 >> 61);
      values[values_offset++] = (block18 >> 40) & 2097151L;
      values[values_offset++] = (block18 >> 19) & 2097151L;

      const uint64_t block19 = blocks[blocks_offset++];
      values[values_offset++] = ((block18 & 524287L) << 2) | (block19 >> 62);
      values[values_offset++] = (block19 >> 41) & 2097151L;
      values[values_offset++] = (block19 >> 20) & 2097151L;

      const uint64_t block20 = blocks[blocks_offset++];
      values[values_offset++] = ((block19 & 1048575L) << 1) | (block20 >> 63);
      values[values_offset++] = (block20 >> 42) & 2097151L;
      values[values_offset++] = (block20 >> 21) & 2097151L;
      values[values_offset++] = block20 & 2097151L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 13) | (byte1 << 5) | (byte2 >> 3);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 7) << 18) | (byte3 << 10) | (byte4 << 2) | (byte5 >> 6);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 63) << 15) | (byte6 << 7) | (byte7 >> 1);

      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte7 & 1) << 20) | (byte8 << 12) | (byte9 << 4) | (byte10 >> 4);

      const int64_t byte11 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte13 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte10 & 15) << 17) | (byte11 << 9) | (byte12 << 1) | (byte13 >> 7);

      const int64_t byte14 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte15 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte13 & 127) << 14) | (byte14 << 6) | (byte15 >> 2);

      const int64_t byte16 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte17 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte18 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte15 & 3) << 19) | (byte16 << 11) | (byte17 << 3) | (byte18 >> 5);

      const int64_t byte19 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte20 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte18 & 31) << 16) | (byte19 << 8) | byte20;
    }
  }
};

class BulkOperationPacked22 : public BulkOperationPacked {
 public:
  BulkOperationPacked22()
    : BulkOperationPacked(22U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 42);
      values[values_offset++] = static_cast<int32_t>((block0 >> 20) & 4194303L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 1048575L) << 2) | (block1 >> 62));
      values[values_offset++] = static_cast<int32_t>((block1 >> 40) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 18) & 4194303L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 262143L) << 4) | (block2 >> 60));
      values[values_offset++] = static_cast<int32_t>((block2 >> 38) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 16) & 4194303L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 65535L) << 6) | (block3 >> 58));
      values[values_offset++] = static_cast<int32_t>((block3 >> 36) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 14) & 4194303L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 16383L) << 8) | (block4 >> 56));
      values[values_offset++] = static_cast<int32_t>((block4 >> 34) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block4 >> 12) & 4194303L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 4095L) << 10) | (block5 >> 54));
      values[values_offset++] = static_cast<int32_t>((block5 >> 32) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 10) & 4194303L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 1023L) << 12) | (block6 >> 52));
      values[values_offset++] = static_cast<int32_t>((block6 >> 30) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 8) & 4194303L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 255L) << 14) | (block7 >> 50));
      values[values_offset++] = static_cast<int32_t>((block7 >> 28) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 6) & 4194303L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 63L) << 16) | (block8 >> 48));
      values[values_offset++] = static_cast<int32_t>((block8 >> 26) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 4) & 4194303L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 15L) << 18) | (block9 >> 46));
      values[values_offset++] = static_cast<int32_t>((block9 >> 24) & 4194303L);
      values[values_offset++] = static_cast<int32_t>((block9 >> 2) & 4194303L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 3L) << 20) | (block10 >> 44));
      values[values_offset++] = static_cast<int32_t>((block10 >> 22) & 4194303L);
      values[values_offset++] = static_cast<int32_t>(block10 & 4194303L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 14) | (byte1 << 6) | (byte2 >> 2);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 3) << 20) | (byte3 << 12) | (byte4 << 4) | (byte5 >> 4);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 15) << 18) | (byte6 << 10) | (byte7 << 2) | (byte8 >> 6);

      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 63) << 16) | (byte9 << 8) | byte10;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 42;
      values[values_offset++] = (block0 >> 20) & 4194303L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 1048575L) << 2) | (block1 >> 62);
      values[values_offset++] = (block1 >> 40) & 4194303L;
      values[values_offset++] = (block1 >> 18) & 4194303L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 262143L) << 4) | (block2 >> 60);
      values[values_offset++] = (block2 >> 38) & 4194303L;
      values[values_offset++] = (block2 >> 16) & 4194303L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 65535L) << 6) | (block3 >> 58);
      values[values_offset++] = (block3 >> 36) & 4194303L;
      values[values_offset++] = (block3 >> 14) & 4194303L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 16383L) << 8) | (block4 >> 56);
      values[values_offset++] = (block4 >> 34) & 4194303L;
      values[values_offset++] = (block4 >> 12) & 4194303L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 4095L) << 10) | (block5 >> 54);
      values[values_offset++] = (block5 >> 32) & 4194303L;
      values[values_offset++] = (block5 >> 10) & 4194303L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 1023L) << 12) | (block6 >> 52);
      values[values_offset++] = (block6 >> 30) & 4194303L;
      values[values_offset++] = (block6 >> 8) & 4194303L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 255L) << 14) | (block7 >> 50);
      values[values_offset++] = (block7 >> 28) & 4194303L;
      values[values_offset++] = (block7 >> 6) & 4194303L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 63L) << 16) | (block8 >> 48);
      values[values_offset++] = (block8 >> 26) & 4194303L;
      values[values_offset++] = (block8 >> 4) & 4194303L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 15L) << 18) | (block9 >> 46);
      values[values_offset++] = (block9 >> 24) & 4194303L;
      values[values_offset++] = (block9 >> 2) & 4194303L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 3L) << 20) | (block10 >> 44);
      values[values_offset++] = (block10 >> 22) & 4194303L;
      values[values_offset++] = block10 & 4194303L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 14) | (byte1 << 6) | (byte2 >> 2);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 3) << 20) | (byte3 << 12) | (byte4 << 4) | (byte5 >> 4);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 15) << 18) | (byte6 << 10) | (byte7 << 2) | (byte8 >> 6);

      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 63) << 16) | (byte9 << 8) | byte10;
    }
  }
};

class BulkOperationPacked23 : public BulkOperationPacked {
 public:
  BulkOperationPacked23()
    : BulkOperationPacked(23U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 41);
      values[values_offset++] = static_cast<int32_t>((block0 >> 18) & 8388607L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 262143L) << 5) | (block1 >> 59));
      values[values_offset++] = static_cast<int32_t>((block1 >> 36) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 13) & 8388607L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 8191L) << 10) | (block2 >> 54));
      values[values_offset++] = static_cast<int32_t>((block2 >> 31) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block2 >> 8) & 8388607L);

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block2 & 255L) << 15) | (block3 >> 49));
      values[values_offset++] = static_cast<int32_t>((block3 >> 26) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block3 >> 3) & 8388607L);

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block3 & 7L) << 20) | (block4 >> 44));
      values[values_offset++] = static_cast<int32_t>((block4 >> 21) & 8388607L);

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block4 & 2097151L) << 2) | (block5 >> 62));
      values[values_offset++] = static_cast<int32_t>((block5 >> 39) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block5 >> 16) & 8388607L);

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block5 & 65535L) << 7) | (block6 >> 57));
      values[values_offset++] = static_cast<int32_t>((block6 >> 34) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block6 >> 11) & 8388607L);

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block6 & 2047L) << 12) | (block7 >> 52));
      values[values_offset++] = static_cast<int32_t>((block7 >> 29) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block7 >> 6) & 8388607L);

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block7 & 63L) << 17) | (block8 >> 47));
      values[values_offset++] = static_cast<int32_t>((block8 >> 24) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block8 >> 1) & 8388607L);

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block8 & 1L) << 22) | (block9 >> 42));
      values[values_offset++] = static_cast<int32_t>((block9 >> 19) & 8388607L);

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block9 & 524287L) << 4) | (block10 >> 60));
      values[values_offset++] = static_cast<int32_t>((block10 >> 37) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block10 >> 14) & 8388607L);

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block10 & 16383L) << 9) | (block11 >> 55));
      values[values_offset++] = static_cast<int32_t>((block11 >> 32) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block11 >> 9) & 8388607L);

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block11 & 511L) << 14) | (block12 >> 50));
      values[values_offset++] = static_cast<int32_t>((block12 >> 27) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block12 >> 4) & 8388607L);

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block12 & 15L) << 19) | (block13 >> 45));
      values[values_offset++] = static_cast<int32_t>((block13 >> 22) & 8388607L);

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block13 & 4194303L) << 1) | (block14 >> 63));
      values[values_offset++] = static_cast<int32_t>((block14 >> 40) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block14 >> 17) & 8388607L);

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block14 & 131071L) << 6) | (block15 >> 58));
      values[values_offset++] = static_cast<int32_t>((block15 >> 35) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block15 >> 12) & 8388607L);

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block15 & 4095L) << 11) | (block16 >> 53));
      values[values_offset++] = static_cast<int32_t>((block16 >> 30) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block16 >> 7) & 8388607L);

      const uint64_t block17 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block16 & 127L) << 16) | (block17 >> 48));
      values[values_offset++] = static_cast<int32_t>((block17 >> 25) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block17 >> 2) & 8388607L);

      const uint64_t block18 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block17 & 3L) << 21) | (block18 >> 43));
      values[values_offset++] = static_cast<int32_t>((block18 >> 20) & 8388607L);

      const uint64_t block19 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block18 & 1048575L) << 3) | (block19 >> 61));
      values[values_offset++] = static_cast<int32_t>((block19 >> 38) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block19 >> 15) & 8388607L);

      const uint64_t block20 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block19 & 32767L) << 8) | (block20 >> 56));
      values[values_offset++] = static_cast<int32_t>((block20 >> 33) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block20 >> 10) & 8388607L);

      const uint64_t block21 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block20 & 1023L) << 13) | (block21 >> 51));
      values[values_offset++] = static_cast<int32_t>((block21 >> 28) & 8388607L);
      values[values_offset++] = static_cast<int32_t>((block21 >> 5) & 8388607L);

      const uint64_t block22 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block21 & 31L) << 18) | (block22 >> 46));
      values[values_offset++] = static_cast<int32_t>((block22 >> 23) & 8388607L);
      values[values_offset++] = static_cast<int32_t>(block22 & 8388607L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 15) | (byte1 << 7) | (byte2 >> 1);

      const int32_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 1) << 22) | (byte3 << 14) | (byte4 << 6) | (byte5 >> 2);

      const int32_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 3) << 21) | (byte6 << 13) | (byte7 << 5) | (byte8 >> 3);

      const int32_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 7) << 20) | (byte9 << 12) | (byte10 << 4) | (byte11 >> 4);

      const int32_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte13 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 15) << 19) | (byte12 << 11) | (byte13 << 3) | (byte14 >> 5);

      const int32_t byte15 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte16 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte17 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte14 & 31) << 18) | (byte15 << 10) | (byte16 << 2) | (byte17 >> 6);

      const int32_t byte18 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte19 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte20 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte17 & 63) << 17) | (byte18 << 9) | (byte19 << 1) | (byte20 >> 7);

      const int32_t byte21 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte22 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte20 & 127) << 16) | (byte21 << 8) | byte22;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 41;
      values[values_offset++] = (block0 >> 18) & 8388607L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 262143L) << 5) | (block1 >> 59);
      values[values_offset++] = (block1 >> 36) & 8388607L;
      values[values_offset++] = (block1 >> 13) & 8388607L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 8191L) << 10) | (block2 >> 54);
      values[values_offset++] = (block2 >> 31) & 8388607L;
      values[values_offset++] = (block2 >> 8) & 8388607L;

      const uint64_t block3 = blocks[blocks_offset++];
      values[values_offset++] = ((block2 & 255L) << 15) | (block3 >> 49);
      values[values_offset++] = (block3 >> 26) & 8388607L;
      values[values_offset++] = (block3 >> 3) & 8388607L;

      const uint64_t block4 = blocks[blocks_offset++];
      values[values_offset++] = ((block3 & 7L) << 20) | (block4 >> 44);
      values[values_offset++] = (block4 >> 21) & 8388607L;

      const uint64_t block5 = blocks[blocks_offset++];
      values[values_offset++] = ((block4 & 2097151L) << 2) | (block5 >> 62);
      values[values_offset++] = (block5 >> 39) & 8388607L;
      values[values_offset++] = (block5 >> 16) & 8388607L;

      const uint64_t block6 = blocks[blocks_offset++];
      values[values_offset++] = ((block5 & 65535L) << 7) | (block6 >> 57);
      values[values_offset++] = (block6 >> 34) & 8388607L;
      values[values_offset++] = (block6 >> 11) & 8388607L;

      const uint64_t block7 = blocks[blocks_offset++];
      values[values_offset++] = ((block6 & 2047L) << 12) | (block7 >> 52);
      values[values_offset++] = (block7 >> 29) & 8388607L;
      values[values_offset++] = (block7 >> 6) & 8388607L;

      const uint64_t block8 = blocks[blocks_offset++];
      values[values_offset++] = ((block7 & 63L) << 17) | (block8 >> 47);
      values[values_offset++] = (block8 >> 24) & 8388607L;
      values[values_offset++] = (block8 >> 1) & 8388607L;

      const uint64_t block9 = blocks[blocks_offset++];
      values[values_offset++] = ((block8 & 1L) << 22) | (block9 >> 42);
      values[values_offset++] = (block9 >> 19) & 8388607L;

      const uint64_t block10 = blocks[blocks_offset++];
      values[values_offset++] = ((block9 & 524287L) << 4) | (block10 >> 60);
      values[values_offset++] = (block10 >> 37) & 8388607L;
      values[values_offset++] = (block10 >> 14) & 8388607L;

      const uint64_t block11 = blocks[blocks_offset++];
      values[values_offset++] = ((block10 & 16383L) << 9) | (block11 >> 55);
      values[values_offset++] = (block11 >> 32) & 8388607L;
      values[values_offset++] = (block11 >> 9) & 8388607L;

      const uint64_t block12 = blocks[blocks_offset++];
      values[values_offset++] = ((block11 & 511L) << 14) | (block12 >> 50);
      values[values_offset++] = (block12 >> 27) & 8388607L;
      values[values_offset++] = (block12 >> 4) & 8388607L;

      const uint64_t block13 = blocks[blocks_offset++];
      values[values_offset++] = ((block12 & 15L) << 19) | (block13 >> 45);
      values[values_offset++] = (block13 >> 22) & 8388607L;

      const uint64_t block14 = blocks[blocks_offset++];
      values[values_offset++] = ((block13 & 4194303L) << 1) | (block14 >> 63);
      values[values_offset++] = (block14 >> 40) & 8388607L;
      values[values_offset++] = (block14 >> 17) & 8388607L;

      const uint64_t block15 = blocks[blocks_offset++];
      values[values_offset++] = ((block14 & 131071L) << 6) | (block15 >> 58);
      values[values_offset++] = (block15 >> 35) & 8388607L;
      values[values_offset++] = (block15 >> 12) & 8388607L;

      const uint64_t block16 = blocks[blocks_offset++];
      values[values_offset++] = ((block15 & 4095L) << 11) | (block16 >> 53);
      values[values_offset++] = (block16 >> 30) & 8388607L;
      values[values_offset++] = (block16 >> 7) & 8388607L;

      const uint64_t block17 = blocks[blocks_offset++];
      values[values_offset++] = ((block16 & 127L) << 16) | (block17 >> 48);
      values[values_offset++] = (block17 >> 25) & 8388607L;
      values[values_offset++] = (block17 >> 2) & 8388607L;

      const uint64_t block18 = blocks[blocks_offset++];
      values[values_offset++] = ((block17 & 3L) << 21) | (block18 >> 43);
      values[values_offset++] = (block18 >> 20) & 8388607L;

      const uint64_t block19 = blocks[blocks_offset++];
      values[values_offset++] = ((block18 & 1048575L) << 3) | (block19 >> 61);
      values[values_offset++] = (block19 >> 38) & 8388607L;
      values[values_offset++] = (block19 >> 15) & 8388607L;

      const uint64_t block20 = blocks[blocks_offset++];
      values[values_offset++] = ((block19 & 32767L) << 8) | (block20 >> 56);
      values[values_offset++] = (block20 >> 33) & 8388607L;
      values[values_offset++] = (block20 >> 10) & 8388607L;

      const uint64_t block21 = blocks[blocks_offset++];
      values[values_offset++] = ((block20 & 1023L) << 13) | (block21 >> 51);
      values[values_offset++] = (block21 >> 28) & 8388607L;
      values[values_offset++] = (block21 >> 5) & 8388607L;

      const uint64_t block22 = blocks[blocks_offset++];
      values[values_offset++] = ((block21 & 31L) << 18) | (block22 >> 46);
      values[values_offset++] = (block22 >> 23) & 8388607L;
      values[values_offset++] = block22 & 8388607L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 15) | (byte1 << 7) | (byte2 >> 1);

      const int64_t byte3 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte4 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte5 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte2 & 1) << 22) | (byte3 << 14) | (byte4 << 6) | (byte5 >> 2);

      const int64_t byte6 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte7 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte8 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte5 & 3) << 21) | (byte6 << 13) | (byte7 << 5) | (byte8 >> 3);

      const int64_t byte9 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte10 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte11 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte8 & 7) << 20) | (byte9 << 12) | (byte10 << 4) | (byte11 >> 4);

      const int64_t byte12 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte13 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte14 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte11 & 15) << 19) | (byte12 << 11) | (byte13 << 3) | (byte14 >> 5);

      const int64_t byte15 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte16 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte17 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte14 & 31) << 18) | (byte15 << 10) | (byte16 << 2) | (byte17 >> 6);

      const int64_t byte18 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte19 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte20 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte17 & 63) << 17) | (byte18 << 9) | (byte19 << 1) | (byte20 >> 7);

      const int64_t byte21 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte22 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = ((byte20 & 127) << 16) | (byte21 << 8) | byte22;
    }
  }
};

class BulkOperationPacked24 : public BulkOperationPacked {
 public:
  BulkOperationPacked24()
    : BulkOperationPacked(24U) {
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(block0 >> 40);
      values[values_offset++] = static_cast<int32_t>((block0 >> 16) & 16777215L);

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block0 & 65535L) << 8) | (block1 >> 56));
      values[values_offset++] = static_cast<int32_t>((block1 >> 32) & 16777215L);
      values[values_offset++] = static_cast<int32_t>((block1 >> 8) & 16777215L);

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = static_cast<int32_t>(((block1 & 255L) << 16) | (block2 >> 48));
      values[values_offset++] = static_cast<int32_t>((block2 >> 24) & 16777215L);
      values[values_offset++] = static_cast<int32_t>(block2 & 16777215L);
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int32_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int32_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int32_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 16) | (byte1 << 8) | byte2;
    }
  }

  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              int64_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const uint64_t block0 = blocks[blocks_offset++];
      values[values_offset++] = block0 >> 40;
      values[values_offset++] = (block0 >> 16) & 16777215L;

      const uint64_t block1 = blocks[blocks_offset++];
      values[values_offset++] = ((block0 & 65535L) << 8) | (block1 >> 56);
      values[values_offset++] = (block1 >> 32) & 16777215L;
      values[values_offset++] = (block1 >> 8) & 16777215L;

      const uint64_t block2 = blocks[blocks_offset++];
      values[values_offset++] = ((block1 & 255L) << 16) | (block2 >> 48);
      values[values_offset++] = (block2 >> 24) & 16777215L;
      values[values_offset++] = block2 & 16777215L;
    }
  }


void Decode(const char blocks[],
            uint32_t blocks_offset,
            int64_t values[],
            uint32_t values_offset,
            uint32_t iterations) {
    for (uint32_t i = 0 ; i < iterations ; ++i) {
      const int64_t byte0 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte1 = blocks[blocks_offset++] & 0xFF;
      const int64_t byte2 = blocks[blocks_offset++] & 0xFF;
      values[values_offset++] = (byte0 << 16) | (byte1 << 8) | byte2;
    }
  }
};


}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_BULKOPERATION_H_
