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

import sys

MAX_SPECIALIZED_BITS_PER_VALUE = 24
PACKED_64_SINGLE_BLOCK_BPV =  [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 21, 32]
HEADER_FILE = "BulkOperation.h"
CPP_FILE = "BulkOperation.cpp"
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

#ifndef SRC_UTIL_PACK_BULKOPERATION_H_
#define SRC_UTIL_PACK_BULKOPERATION_H_

namespace lucene {
namespace core {
namespace util {

#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <cmath>
#include <string>

"""

TAIL = """

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_BULKOPERATION_H_
"""

"""
Global variable
"""
f = None

def l(line):
  global f
  f.write(line)

def ln(line):
  l(line + '\n')

def start():
  global f
  f = open(HEADER_FILE, 'w')
  l(LICENSE)
  l(HEAD)

def end():
  global f
  l(TAIL)
  f.close()

def gen_bulk_operation():
  ln(
"""
class BulkOperation : public PackedInts::Decoder, public PackedInts::Encoder {
 private:
  static const BulkOperation*[] packed_bulk_ops;
  static const BulkOperation*[] packed_single_block_bulk_ops;

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
  BulkOperation* Of(const PackedInts::Format format,
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
  const uint32_t bits_per_value;
  const uint32_t long_block_count;
  const uint32_t long_value_count;
  const uint32_t byte_block_count;
  const uint32_t byte_value_count;
  const uint64_t mask;
  const uint32_t uint_mask;

 public:
  BulkOperationPacked(const uint32_t bits_per_value) {
    this->bits_per_value = bits_per_value;
    uint32_t blocks = bits_per_value;
    while ((blocks & 1) == 0) {
      blocks >>= 1;
    }
    this->long_block_count = blocks;
    this->long_value_count = 64 * this->long_block_count / bits_per_value;
    const uint32_t byte_block_count = 8 * this->long_block_count;
    const uint32_t byte_value_count = this->long_value_count;
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

  void Decode(const uint8_t blocks[],
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
          | (blocks[blocks_offset] >>> (64 + bits_left)));
      } else {
        values[values_offset++] = static_cast<int32_t>((blocks[blocks_offset] >> bits_left) & mask);
      }
    }
  }

  void Decode(const uint8_t blocks[],
              uint32_t blocks_offset,
              int32_t values[],
              uint32_t values_offset,
              uint32_t iterations) {
    int32_t next_value = 0; 
    int bits_left = bits_per_value;
    for (uint32_t i = 0 ; i < iterations * byte_block_coutn ; ++i) {
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
          values[values_offset++] = static_cast<int32_t>(((bytes >> bits) & int_mask));
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
    int32_t bist_left = 64;
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

  void Encode(cosnt int32_t values[],
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
        next_block |= ((values[values_offset] & 0xFFFFFFFFL) >> -bits_left)
        blocks[blocks_offset++] = next_block;
        nextBlock = (values[values_offset++] & ((1L << -bits_left) - 1)) << (64 + bits_left);
        bits_left += 64;
      }
    }
  }

  void Encode(const int64_t values[],
              uint32_t values_offset,
              uint8_t blocks[],
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
        blocks[blocks_offset++] = static_cast<uint8_t>(next_block | (v >> bits));
        while (bits >= 8) {
          bits -= 8;
          blocks[blocks_offset++] = static_cast<uint8_t>(v >> bits);
        }

        // Then buffer
        bits_left = (8 - bits);
        next_block = static_cast<uint32_t>((v & ((1LU << bits) - 1)) << bits_left);
      }
    }
  }

  void Encode(const int32_t values[],
              uint32_t values_offset,
              uint8_t blocks[],
              uint32_t blocks_offset,
              uint32_t iterations) {
    uint32_t next_block = 0;  
    int32_t bits_left = 8;
    for (uint32_t i = 0 ; i < byte_value_count * iterations ; ++i) {
      const int32_t v = values[values_offset++];
      if (bits_left < 0 || bits_per_value >= bits_left) {
        uint32_t bits = (bits_per_value - bits_left); 
        blocks[blocks_offset++] = static_cast<uint8_t>(next_block | (v >> bits));
        while (bits >= 8) {
          bits -= 8;
          blocks[blocks_offset++] = static_cast<uint8_t>(v >> bits);
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

class BulkOperationPackedSingleBlock : public BulkOperation {

};
""".strip())

def gen_bulk_operation_packed():
  def gen_head(bpv):
    l(
"""
class BulkOperationPacked%d : public BulkOperationPacked {
 public:
  BulkOperationPacked%d()
    : BulkOperationPacked(%dU) {
  }
""" % (bpv, bpv, bpv))

  def block_value_count(bpv, bits=64):
    blocks = bpv
    values = blocks * bits / bpv
    while blocks % 2 == 0 and values % 2 == 0:
      blocks /= 2
      values /= 2
    assert values * bpv == bits * blocks, "%d values, %d blocks, %d bits per value" % (values, blocks, bpv)
    return (blocks, values)

  def get_type(bits):
    if bits == 8:
      return "char"
    elif bits == 16:
      return "int16_t"
    elif bits == 32:
      return "int32_t"
    elif bits == 64:
      return "int64_t"
    else:
      assert False

  def casts(typ):
    cast_start = "static_cast<%s>(" % typ
    cast_end = ")"
    if typ == "int64_t":
      cast_start = ""
      cast_end = ""
    return cast_start, cast_end

  def is_power_of_two(n):
      return n & (n - 1) == 0

  def gen_decode(bpv, f, bits):
    blocks, values = block_value_count(bpv)
    typ = get_type(bits)
    cast_start, cast_end = casts(typ) 
    l(
"""
  void Decode(const uint64_t blocks[], 
              uint32_t blocks_offset,
              %s values[],
              uint32_t values_offset,
              uint32_t iterations) {
""" % typ)
    if bits < bpv:
      ln("    throw UnsupportedOperationException();")
    else:
      ln("    for (uint32_t i = 0 ; i < iterations ; ++i) {")
      mask = (1 << bpv) - 1

      if is_power_of_two(bpv):
        l(
"""      const uint64_t block = blocks[blocks_offset++];
      for (int64_t shift = %d ; shift >= 0 ; shift -= %d) {
        values[value_offset++] = %s(block >> shift) & %d%s;
      }
""" % (64 - bpv, bpv, cast_start, mask, cast_end))
      else:
        for i in xrange(0, values):
          block_offset = i * bpv / 64
          bit_offset = (i * bpv) % 64
          if bit_offset == 0:
            # Start of block
            ln("      const uint64_t block%d = blocks[block_offset++];" % block_offset)
            ln("      values[values_offset++] = %sblock%d >> %d%s" % (cast_start, block_offset, 64 - bpv, cast_end))
          elif bit_offset + bpv == 64:
            # End of block
            ln("      values[vlaue_offset++] = %sblock%d & %dL%s;" % (cast_start, block_offset, mask, cast_end))
          elif bit_offset + bpv < 64:
            # Middle of block
            ln("      values[values_offset++] = %s(block%d >> %d) & %dL%s;" % (cast_start, block_offset, 64 - bit_offset - bpv, mask, cast_end))
          else:
            mask1 = (1 << (64 - bit_offset)) - 1
            shift1 = bit_offset + bpv - 64
            shift2 = 64 - shift1
            ln("\n      const uint64_t block%d = blocks[blocks_offset++];" % (block_offset + 1))
            ln("      values[values_offset++] = %s((block%d & %dL) << %d) | (block%d >> %d)%s;" % (cast_start, block_offset, mask1, shift1, block_offset + 1, shift2, cast_end))
      ln("    }")
    ln("  }\n") 
    
    byte_blocks, byte_values = block_value_count(bpv, 8)

    l(
"""
void Decode(const uint8_t blocks[],
            uint32_t blocks_offset,
            %s values[],
            uint32_t values_offset,
            uint32_t iterations) {\n""" % typ)
    if bits < bpv:
      ln("    throw UnsupportedOperationException();")
    else:
      if is_power_of_two(bpv) and bpv < 8:
        ln("    for (uint32_t j = 0 ; j < iterations ; ++j) {")
        ln("      const uint8_t block = blocks[blocks_offset++];")
        for shift in xrange(8 - bpv, 0, -bpv):
          ln("      values[values_offset++] = (block >> %d) & %d;" % (shift, mask))
        ln("      values[values_offset++] = block & %d;" % mask)
        ln("    }")
      elif bpv == 8:
        ln("    for (uint32_t j = 0 ; j < iterations ; ++j) {")
        ln("      values[values_offset++] = blocks[blocks_offset++] & 0xFF;")
        ln("    }")
      elif is_power_of_two(bpv) and bpv > 8:
        ln("    for (uint32_t j = 0 ; j < iterations ; ++j) {")
        m = bits <= 32 and "0xFF" or "0xFFL"
        ln("      values[values_offset++] =")
        for i in xrange(bpv / 8 - 1):
          ln("   ((blocks[blocks_offset++] & %s) << %d) |" % (m, bpv - 8))
        ln(" (blocks[blocks_offset++] & %s);\n" % m)
        ln("    }\n")
      else:
        ln("    for (uint32_t i = 0 ; i < iterations ; ++i) {")
        for i in xrange(0, byte_values):
          byte_start = i * bpv / 8
          bit_start = (i * bpv) % 8
          byte_end = ((i + 1) * bpv - 1) / 8
          bit_end = ((i + 1) * bpv - 1) % 8
          shift = lambda b: 8 * (byte_end - b - 1) + 1 + bit_end
          if i > 0:
            ln("")
          if bit_start == 0:
            ln("      const %s byte%d = blocks[blocks_offset++] & 0xFF;" % (typ, byte_start))
          for b in xrange(byte_start + 1, byte_end + 1):
            ln("      const %s byte%d = blocks[blocks_offset++] & 0xFF;" % (typ, b))
          l("      values[values_offset++] =")
          if byte_start == byte_end:
            if bit_start == 0:
              if bit_end == 7:
                l(" byte%d" % byte_start)
              else:
                l(" byte%d >> %d" % (byte_start, 7 - bit_end))
            else:
              if bit_end == 7:
                l(" byte%d & %d" % (byte_start, 2 ** (8 - bit_start) - 1))
              else:
                l(" (byte%d >> %d) & %d" % (byte_start, 7 - bit_end, 2 ** (bit_end - bit_start + 1) - 1))
          else:
            if bit_start == 0:
              l(" (byte%d << %d)" % (byte_start, shift(byte_start)))
            else:
              l(" ((byte%d & %d) << %d)" % (byte_start, 2 ** (8 - bit_start) - 1, shift(byte_start)))
            for b in xrange(byte_start + 1, byte_end):
              l(" | (byte%d << %d)" % (b, shift(b)))
            if bit_end == 7:
              l(" | byte%d" % byte_end)
            else:
              l(" | (byte%d >> %d)" % (byte_end, 7 - bit_end))
          ln(";")
        ln("    }") 
    ln("  }")

  for bpv in xrange(1, 25):
    gen_head(bpv)
    gen_decode(bpv, f, 32)
    gen_decode(bpv, f, 64)
    ln("};")

def gen_cpp():
  global f
  f = open(CPP_FILE, "w")
  l(LICENSE)
  l(
"""

#include <Util/Pack/BulkOperation.h>

using lucene::core::util::BulkOperation;
using lucene::core::util::BulkOperationPackedSingleBlock;

/**
 *  BulkOperation
 */
BulkOperation* BulkOperation::packed_bulk_ops[] = {
""")

  for bpv in xrange(1, 65):
    if bpv > MAX_SPECIALIZED_BITS_PER_VALUE:
      l("  new BulkOperationPacked(%d)" % bpv)
      ln(", " if bpv != 64 else "")
    else:
      ln("  new BulkOperationPacked%d()," % bpv)
  ln("};")
  ln("")
  ln("BulkOperation* BulkOperation::packed_single_block_bulk_ops[] = {")

  for bpv in xrange(1, max(PACKED_64_SINGLE_BLOCK_BPV) + 1):
    if bpv in PACKED_64_SINGLE_BLOCK_BPV:
      l("  new BulkOperationPackedSingleBlock(%d)" % bpv)
    else:
      l("  nullptr")

    if bpv == max(PACKED_64_SINGLE_BLOCK_BPV):
      ln("")
    else:
      ln(",")

  ln("};")
  f.close()

if __name__ == '__main__':
  start()
  gen_bulk_operation()
  gen_bulk_operation_packed()
  end()

  gen_cpp()
