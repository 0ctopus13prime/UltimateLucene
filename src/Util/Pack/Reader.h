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

#include <Index/Exception.h>
#include <Store/DataInput.h>
#include <Util/Bits.h>
#include <Util/Etc.h>
#include <Util/Numeric.h>
#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <Util/Pack/Writer.h>
#include <Util/Pack/LongValues.h>
#include <algorithm>
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
    const uint64_t major_bit_pos = static_cast<uint64_t>(index) * bits_per_value;
    const uint64_t element_pos = (major_bit_pos >> 3);
    in->Seek(start_pointer + element_pos);
    const uint32_t bit_pos = static_cast<uint32_t>(major_bit_pos & 7);
    // Round up bits to a multiple of 8 to find total bytes needed to read
    const uint32_t rounded_bits = ((bit_pos + bits_per_value + 7) & ~7U);
    // The number of extra bits read at the end to shift out
    uint32_t shift_right_bits = (rounded_bits - bit_pos - bits_per_value);

    int64_t raw_value;
    switch(rounded_bits >> 3) {
      case 1:
        raw_value = in->ReadByte();
        break;
      case 2:
        raw_value = in->ReadInt16();
        break;
      case 3:
        raw_value = (static_cast<int64_t>(in->ReadInt16()) << 8) | (in->ReadByte() & 0xFFL);
        break;
      case 4:
        raw_value = in->ReadInt32();
        break;
      case 5:
        raw_value = (static_cast<int64_t>(in->ReadInt32()) << 8) | (in->ReadByte() & 0xFFL);
        break;
      case 6:
        raw_value = (static_cast<int64_t>(in->ReadInt32()) << 16) | (in->ReadByte() & 0xFFFFL);
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
        // We must be very careful not to shift out relevant bits.
        // So we account for right shift
        // We would normally do  on return here, and reset it
        raw_value = (in->ReadInt64() << (8 - shift_right_bits)) |
                    ((in->ReadByte() & 0xFFL) >> shift_right_bits);
        shift_right_bits = 0;
        break;
      default:
        throw IllegalStateException("Bites per value too large: " + std::to_string(bits_per_value));
    }

    return ((raw_value >> shift_right_bits) & value_mask);
  }
};  // DirectPackedReader

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
};  // DirectPacked64SingleBlockReader

class BlockPackedReaderIterator {
 public:
  static int64_t ReadVInt64(lucene::core::store::DataInput* in) {
    char b = in->ReadByte();
    if (b >= 0) return b;
    int64_t i = b & 0x7FL;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 7;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 14;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 21;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 28;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 35;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 42;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 49;
    if (b >= 0) return i;
    b = in->ReadByte();
    i |= (b & 0x7FL) << 56;
    return i;
  }

 private:
  lucene::core::store::DataInput* in;
  uint32_t packed_ints_version;
  uint64_t value_count;
  uint32_t block_size;
  uint32_t values_size;
  uint32_t blocks_size;
  std::unique_ptr<int64_t[]> values;
  std::unique_ptr<char[]> blocks;
  LongsRef values_ref;
  uint32_t off;
  uint64_t ord;

 private:
  void SkipBytes(const uint64_t count) {
    if (lucene::core::store::IndexInput* iin =
        dynamic_cast<lucene::core::store::IndexInput*>(in)) {
      iin->Seek(iin->GetFilePointer() + count); 
    } else {
      if (!blocks) {
        blocks = std::make_unique<char[]>(block_size);
      }
      uint64_t skipped = 0;
      while (skipped < count) {
        const uint32_t to_skip =
          static_cast<uint32_t>(block_size, count - skipped); 
        in->ReadBytes(blocks.get(), 0, to_skip);
        skipped += to_skip;
      }
    }
  }

  void Refill() {
    const int32_t token = (in->ReadByte() & 0xFF);
    const bool min_equals_0 =
      (token & AbstractBlockPackedWriter::MIN_VALUE_EQUALS_0) != 0;
    const uint32_t bits_per_value =
      (token >> AbstractBlockPackedWriter::BPV_SHIFT);

    if (bits_per_value > 64) {
      throw IOException("Corrupted");
    }
    const int64_t min_value = (min_equals_0 ? 0L :
                               BitUtil::ZigZagDecode(1L + ReadVInt64(in)));
    assert(min_equals_0 || min_value != 0);

    if (bits_per_value == 0) {
      std::fill(values.get(), values.get() + block_size, min_value); 
    } else {
      PackedInts::Decoder* decoder =
        PackedInts::GetDecoder(PackedInts::Format::PACKED,
                               packed_ints_version,
                               bits_per_value);
      const uint32_t iterations = (block_size / decoder->ByteValueCount());
      const uint32_t byte_block_size =
        (iterations * decoder->ByteBlockCount());
      if (!blocks || block_size < byte_block_size) {
        blocks = std::make_unique<char[]>(block_size);
      }

      const uint32_t value_count =
        static_cast<uint32_t>(std::min(value_count - ord,
                              static_cast<uint64_t>(block_size)));
      const uint32_t blocks_count =
        static_cast<uint32_t>(PackedInts::Format::PACKED.ByteCount(
                              packed_ints_version,
                              value_count, bits_per_value));
      in->ReadBytes(blocks.get(), 0, blocks_count);

      decoder->Decode(blocks.get(), 0, values.get(), 0, iterations);

      if (min_value != 0) {
        std::for_each(values.get(),
                      values.get() + value_count,
                      [min_value](int64_t& v){
                        v += min_value; 
                      });
      }
    }

    off = 0;
  }

 public:
  BlockPackedReaderIterator(lucene::core::store::DataInput* in,
                            const uint32_t packed_ints_version,
                            const uint32_t block_size,
                            const uint64_t value_count)
    : in(in), 
      packed_ints_version(packed_ints_version),
      value_count(0),
      block_size(block_size),
      values_size(block_size),
      blocks_size(0),
      values(std::make_unique<int64_t[]>(values_size)),
      blocks(),
      values_ref(values.get(), values_size, 0, 0),
      off(0),
      ord(0) {
    PackedInts::CheckBlockSize(block_size,
                               AbstractBlockPackedWriter::MIN_BLOCK_SIZE,
                               AbstractBlockPackedWriter::MAX_BLOCK_SIZE);
    Reset(in, value_count);
  }


  void Reset(lucene::core::store::DataInput* new_in,
             const uint64_t new_value_count) {
    in = new_in;
    value_count = new_value_count;
    off = block_size;
    ord = 0;
  }

  void Skip(uint64_t count) {
    if (ord + count > value_count || ord + count < 0) {
      throw EOFException();
    }

    // 1. Skip buffered values
    const uint32_t skip_buffer =
      static_cast<uint32_t>(std::min(count,
                                     static_cast<uint64_t>(block_size - off)));
    off += skip_buffer;
    ord += skip_buffer;
    count -= skip_buffer;
    if (count == 0) {
      return;
    }

    // 2. Skip as many blocks as necessary
    assert(off == block_size);
    while (count >= block_size) {
      const int32_t token = in->ReadByte() & 0xFF;
      const uint32_t bits_per_value =
        (token >> AbstractBlockPackedWriter::BPV_SHIFT);
      if (bits_per_value > 64) {
        throw IOException("Corrupted"); 
      }
      if ((token & AbstractBlockPackedWriter::MIN_VALUE_EQUALS_0) == 0) {
        ReadVInt64(in);
      }
      const uint64_t block_bytes =
        PackedInts::Format::PACKED.ByteCount(packed_ints_version,
                                             block_size,
                                             bits_per_value);
      SkipBytes(block_bytes);
      ord += block_size;
      count -= block_size;
    }
    if (count == 0) {
      return;
    }

    // 3. Skip last values
    assert(count < block_size);
    Refill();
    ord += count;
    off += count;
  }

  int64_t Next() {
    if (ord == value_count) {
      throw EOFException();
    }
    if (off == block_size) {
      Refill();
    }
    ++ord;
    return values[off++];
  }

  LongsRef& Next(uint32_t count) {
    if (ord == value_count) {
      throw EOFException();
    }
    if (off == block_size) {
      Refill();
    }

    count = std::min(count, block_size - off);
    count = static_cast<uint32_t>(count, value_count - ord);

    values_ref.Offset(off)
              .Length(count);

    off += count;
    ord += count;

    return values_ref;
  }

  uint64_t Ord() const noexcept {
    return ord;
  }
};  // BlockPackedReaderIterator

class BlockPackedReader: public Int64Values {
 private:
  uint32_t block_shift;
  uint32_t block_mask;
  uint64_t value_count;
  std::unique_ptr<int64_t[]> min_values;
  std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> sub_readers;
  uint32_t min_values_size;
  uint32_t sub_readers_size;
  uint64_t sum_bpv;

 public:
  BlockPackedReader(lucene::core::store::IndexInput* in,
                    const uint32_t packed_ints_version,
                    const uint32_t block_size,
                    const uint64_t value_count,
                    bool direct)
    : block_shift(PackedInts::CheckBlockSize(block_size,
                  AbstractBlockPackedWriter::MIN_BLOCK_SIZE,
                  AbstractBlockPackedWriter::MAX_BLOCK_SIZE)),
      block_mask(block_size - 1),
      value_count(value_count),
      min_values(),
      sub_readers(std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(
                  PackedInts::NumBlocks(value_count, block_size))),
      min_values_size(0),
      sub_readers_size(PackedInts::NumBlocks(value_count, block_size)),
      sum_bpv(0) {
    const uint32_t num_blocks = PackedInts::NumBlocks(value_count, block_size);

    int64_t sum_bpv_tmp = 0;
    for (uint32_t i = 0 ; i < num_blocks ; ++i) {
      const int32_t token = (in->ReadByte() & 0xFF);
      const uint32_t bits_per_value =
        (token >> AbstractBlockPackedWriter::BPV_SHIFT);
      sum_bpv_tmp += bits_per_value;   
      if (bits_per_value > 64) {
        throw lucene::core::index::CorruptIndexException(
              "Corrupted Block#" + std::to_string(i));
      }

      if ((token & AbstractBlockPackedWriter::MIN_VALUE_EQUALS_0) == 0) {
        if (!min_values) {
          min_values =
            std::make_unique<int64_t[]>(num_blocks);
        }
        min_values[i] =
          BitUtil::ZigZagDecode(1L + BlockPackedReaderIterator::ReadVInt64(in));
      }
      if (bits_per_value == 0) {
        sub_readers[i] = std::make_unique<PackedInts::NullReader>(block_size);
      } else {
        const uint32_t size =
          static_cast<uint32_t>(block_size, value_count -
                                static_cast<uint64_t>(i) * block_size);
        if (direct) {
          const uint64_t pointer = in->GetFilePointer();
          sub_readers[i] =
            PackedInts::GetDirectReaderNoHeader(in,
                                                PackedInts::Format::PACKED,
                                                packed_ints_version,
                                                size,
                                                bits_per_value);
            in->Seek(pointer +
                     PackedInts::Format::PACKED.ByteCount(packed_ints_version,
                                                          size,
                                                          bits_per_value));
        } else {
          sub_readers[i] =
            PackedInts::GetReaderNoHeader(in,
                                          PackedInts::Format::PACKED,
                                          packed_ints_version,
                                          size,
                                          bits_per_value);
        }
      }
    }

    sum_bpv = sum_bpv_tmp;
  }

  int64_t Get(const uint64_t index) {
    assert(index < value_count);
    const uint32_t block = static_cast<uint32_t>(index >> block_shift);
    const uint32_t idx = static_cast<uint32_t>(index & block_mask);
    return (min_values ? 0 : min_values[block]) + sub_readers[block]->Get(idx);
  }
};  // BlockPackedReader

class DirectReader {
 public: 
  static std::unique_ptr<Int64Values>
  GetInstance(lucene::core::store::RandomAccessInput* slice,
              const uint32_t bits_per_value,
              const uint64_t offset);

  class DirectPackedReader1: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader1(lucene::core::store::RandomAccessInput* in,
                        const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      const uint32_t shift = (7 - static_cast<uint32_t>(index & 7)); 
      return (in->ReadByte(offset + (index >> 3)) >> shift) & 0x1;
    }
  };

  class DirectPackedReader2: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader2(lucene::core::store::RandomAccessInput* in,
                        const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      const uint32_t shift = (3 - static_cast<uint32_t>(index & 3)) << 1; 
      return (in->ReadByte(offset + (index >> 2)) >> shift) & 0x3;
    }
  };

  class DirectPackedReader4: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader4(lucene::core::store::RandomAccessInput* in,
                        const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      const uint32_t shift = static_cast<uint32_t>((index + 1) & 1) << 2;
      return (in->ReadByte(offset + (index >> 1)) >> shift) & 0xF;
    }
  };

  class DirectPackedReader8: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader8(lucene::core::store::RandomAccessInput* in,
                        const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return (in->ReadByte(offset + index) & 0xFF);
    }
  };

  class DirectPackedReader12: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader12(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      const uint64_t delta_offset = (index * 12) >> 3;
      const uint32_t shift = static_cast<uint32_t>((index + 1) & 1) << 2;
      return (in->ReadInt16(offset + delta_offset) >> shift) & 0xFFF;
    }
  };

  class DirectPackedReader16: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader16(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return (in->ReadInt16(offset + (index << 1))) & 0xFFFF;
    }
  };

  class DirectPackedReader20: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader20(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      const uint64_t delta_offset = (index * 20) >> 3;
      // TODO(0ctopus13prime): Clean this up ...
      const int32_t v = (in->ReadInt32(offset + delta_offset) >> 8);
      const uint32_t shift = (static_cast<uint32_t>(index + 1) << 2);
      return (v >> shift) & 0xFFFFF;
    }
  };

  class DirectPackedReader24: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader24(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return static_cast<int64_t>(in->ReadInt32(offset + index * 3) >> 8);
    }
  };

  class DirectPackedReader28: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader28(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      const uint64_t delta_offset = ((index * 28) >> 3);
      const uint32_t shift = (static_cast<uint32_t>(index + 1) << 2);
      return ((in->ReadInt32(offset + delta_offset) >> shift) & 0xFFFFFFFL);
    }
  };

  class DirectPackedReader32: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader32(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return in->ReadInt32(offset + (index << 2)) & 0xFFFFFFFFL;
    }
  };

  class DirectPackedReader40: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader40(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return (in->ReadInt64(offset + index * 5) >> 24);
    }
  };

  class DirectPackedReader48: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader48(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return (in->ReadInt64(offset + index * 6) >> 16);
    }
  };

  class DirectPackedReader56: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader56(lucene::core::store::RandomAccessInput* in,
                         const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return (in->ReadInt64(offset + index * 7) >> 8);
    }
  };

  class DirectPackedReader64: public Int64Values {
   public:
    lucene::core::store::RandomAccessInput* in;
    uint64_t offset;

    DirectPackedReader64(lucene::core::store::RandomAccessInput* in,
                        const uint64_t offset)
      : in(in),
        offset(offset) {
    }

    int64_t Get(const uint64_t index) {
      return in->ReadInt64(offset + (index << 3));
    }
  };
};  // DirectReader

class PackedReaderIterator: public PackedInts::ReaderIteratorImpl {
 private:
  BulkOperation* bulk_operation;
  uint32_t packed_ints_version;
  uint32_t iterations;
  PackedInts::Format format;
  int32_t position;
  LongsRef next_values;
  uint32_t next_blocks_size;
  std::unique_ptr<char[]> next_blocks;

 public:
  PackedReaderIterator(PackedInts::Format format,
                       const uint32_t packed_ints_version,
                       const uint32_t value_count,
                       const uint32_t bits_per_value,
                       lucene::core::store::DataInput* in,
                       const uint32_t mem)
    : PackedInts::ReaderIteratorImpl(value_count, bits_per_value, in),
      bulk_operation(BulkOperation::Of(format, bits_per_value)),
      packed_ints_version(packed_ints_version),
      iterations(bulk_operation->ComputeIterations(value_count, mem)),
      format(format),
      position(-1),
      next_values(iterations * bulk_operation->ByteValueCount()),
      next_blocks_size(iterations * bulk_operation->ByteBlockCount()),
      next_blocks(std::make_unique<char[]>(next_blocks_size)) {
    assert(value_count == 0  || iterations > 0);
    next_values.Offset(next_values.Capacity());
  }

  LongsRef& Next(uint32_t count) {
    assert(next_values.Length() >= 0); 

    next_values.IncOffset(next_values.Length());

    const int32_t remaining_check = (value_count - position - 1);
    if (remaining_check <= 0) {
      throw EOFException();
    }
    const uint32_t remaining = static_cast<uint32_t>(remaining_check);
    count = std::min(remaining, count);

    if (next_values.Offset() == next_values.Capacity()) {
      const uint64_t remaining_blocks =
        format.ByteCount(packed_ints_version, remaining, bits_per_value);
      const uint32_t blocks_to_read =
        std::min(static_cast<uint32_t>(remaining_blocks), next_blocks_size);
      in->ReadBytes(next_blocks.get(), 0, blocks_to_read);
      if (blocks_to_read < next_blocks_size) {
        std::memset(next_blocks.get() + blocks_to_read,
                    next_blocks_size, 0);
      }

      bulk_operation->Decode(next_blocks.get(), 0,
                             next_values.Longs(), 0, iterations);
      next_values.Offset(0);
    } 

    next_values.Length(std::min(next_values.Capacity() - next_values.Offset(),
                                count));
    position += next_values.Length();
    return next_values;
  }

  uint32_t Ord() {
    return position;
  }
};  // PackedReaderIterator

class MonotonicBlockPackedReader: public Int64Values {
 private:
  uint32_t block_shift; 
  uint32_t block_mask;
  uint64_t value_count;
  uint32_t num_blocks;
  std::unique_ptr<int64_t[]> min_values;
  std::unique_ptr<float[]> averages;
  std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> sub_readers;
  uint64_t sum_bpv;

  MonotonicBlockPackedReader(lucene::core::store::IndexInput* in,
                             const uint32_t packed_ints_version,
                             const uint32_t block_size,
                             const uint32_t value_count,
                             bool direct)
    : block_shift(PackedInts::CheckBlockSize(block_size,
                              AbstractBlockPackedWriter::MIN_BLOCK_SIZE,
                              AbstractBlockPackedWriter::MAX_BLOCK_SIZE)),
      block_mask(block_size - 1),
      value_count(value_count),
      num_blocks(PackedInts::NumBlocks(value_count, block_size)),
      min_values(std::make_unique<int64_t[]>(num_blocks)),
      averages(std::make_unique<float[]>(num_blocks)),
      sub_readers(std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(
                  num_blocks)),
      sum_bpv(0) {
    uint64_t sum_bpv_tmp = 0; 
    for (uint32_t i = 0 ; i < num_blocks ; ++i) {
      min_values[i] = in->ReadZInt64();
      averages[i] =
        lucene::core::util::numeric::Float::IntBitsToFloat(in->ReadInt32());
      uint32_t bpv = static_cast<uint32_t>(in->ReadVInt32());
      sum_bpv_tmp += bpv;
      if (bpv > 64) {
        throw IOException("Corrupted");
      }
      if (bpv == 0) {
        sub_readers[i] = std::make_unique<PackedInts::NullReader>(block_size);
      } else {
        const uint32_t size = static_cast<uint32_t>(
                 std::min(static_cast<uint64_t>(block_size),
                          value_count - static_cast<uint64_t>(i) * block_size));
        if (direct) {
          const uint64_t pointer = in->GetFilePointer();
          sub_readers[i] =
            PackedInts::GetDirectReaderNoHeader(in,
                                                PackedInts::Format::PACKED,
                                                packed_ints_version,
                                                size,
                                                bpv);
          in->Seek(pointer + PackedInts::Format::PACKED.ByteCount(
                             packed_ints_version, size, bpv));
        } else {
          sub_readers[i] =
            PackedInts::GetReaderNoHeader(in,
                                          PackedInts::Format::PACKED,
                                          packed_ints_version,
                                          size,
                                          bpv);
        }
      }
    }  // End for

    sum_bpv = sum_bpv_tmp;
  }

  int64_t Get(const uint64_t index) {
    assert(index < value_count);
    const uint32_t block = static_cast<uint32_t>(index >> block_shift);
    const uint32_t idx = static_cast<uint32_t>(index & block_mask);
    return MonotonicLongValues::Expected(min_values[block],
                                         averages[block],
                                         idx) +
                                         sub_readers[block]->Get(idx);
  }

  uint64_t Size() const noexcept {
    return value_count;
  }
};  // MonotonicBlockPackedReader

class DirectMonotonicReader {
 public:
  class EMPTY: public Int64Values {
   public:
    int64_t Get(const uint64_t index) {
      return 0;
    }
  };

  class Meta {
   public:
    uint32_t block_shift;
    uint32_t num_blocks;
    std::unique_ptr<int64_t[]> mins;
    std::unique_ptr<float[]> avgs;
    std::unique_ptr<char[]> bpvs;
    std::unique_ptr<int64_t[]> offsets;

    Meta(const uint64_t num_values,
         const uint32_t block_shift) {
      this->block_shift = block_shift;
      uint64_t num_blocks_tmp = (num_values >> block_shift);
      if ((num_blocks << block_shift) < num_values) {
        ++num_blocks_tmp;
      }
      num_blocks = static_cast<uint32_t>(num_blocks_tmp);
      mins = std::make_unique<int64_t[]>(num_blocks);
      avgs = std::make_unique<float[]>(num_blocks);
      bpvs = std::make_unique<char[]>(num_blocks);
      offsets = std::make_unique<int64_t[]>(num_blocks);
    }

    Meta(const Meta& other) = delete;

    Meta(Meta&& other)
      : block_shift(other.block_shift),
        num_blocks(other.num_blocks),
        mins(std::move(other.mins)),
        avgs(std::move(other.avgs)),
        bpvs(std::move(other.bpvs)),
        offsets(std::move(other.offsets)) {
    }

    Meta& operator=(const Meta& other) = delete;

    Meta& operator=(Meta&& other) {
      block_shift = other.block_shift;
      num_blocks = other.num_blocks;
      mins = std::move(other.mins);
      avgs = std::move(other.avgs);
      bpvs = std::move(other.bpvs);
      offsets = std::move(other.offsets);
    }
  };  // Meta

 private:
  class SlicedInt64Values: public Int64Values {
   private:
     std::unique_ptr<std::unique_ptr<Int64Values>[]> readers;
     int64_t* mins;
     float* avgs;
     uint32_t num_blocks;
     uint32_t block_shift;
     uint32_t mask;

   public:
    SlicedInt64Values(const uint32_t block_shift,
                      std::unique_ptr<std::unique_ptr<Int64Values>[]>&& readers,
                      int64_t* mins,
                      float* avgs,
                      const uint32_t num_blocks)
      : readers(std::move(readers)),
        mins(mins),
        avgs(avgs),
        num_blocks(num_blocks),
        block_shift(block_shift),
        mask((1 << block_shift) - 1) {
    }

    int64_t Get(const uint64_t index) {
      const uint32_t block = static_cast<uint32_t>(index >> block_shift);
      const uint64_t block_index = (index & mask);
      const int64_t delta = readers[block]->Get(block_index);

      return mins[block] +
             static_cast<int64_t>(avgs[block] * block_index) +
             delta;
    }
  };

 public:
  static Meta LoadMeta(lucene::core::store::IndexInput* meta_in,
                       const uint64_t num_values,
                       const uint32_t block_shift) {
    Meta meta(num_values, block_shift);
    for (uint32_t i = 0 ; i < meta.num_blocks ; ++i) {
      meta.mins[i] = meta_in->ReadInt64();
      meta.avgs[i] =
        lucene::core::util::numeric::Float::IntBitsToFloat
        (meta_in->ReadInt32());
      meta.offsets[i] = meta_in->ReadInt64();
      meta.bpvs[i] = meta_in->ReadByte();
    }

    return meta;
  }

  static std::unique_ptr<Int64Values>
  GetInstance(Meta& meta,
              lucene::core::store::RandomAccessInput* data) {
    std::unique_ptr<std::unique_ptr<Int64Values>[]> readers =
      std::make_unique<std::unique_ptr<Int64Values>[]>(meta.num_blocks);

    for (uint32_t i = 0 ; i < meta.num_blocks ; ++i) {
      if (meta.bpvs[i] == 0) {
        readers[i] = std::make_unique<EMPTY>();
      } else {
        readers[i] =
          DirectReader::GetInstance(data, meta.bpvs[i], meta.offsets[i]);
      }
    }

    uint32_t block_shift = meta.block_shift;
    int64_t* mins = meta.mins.get(); 
    float* avgs = meta.avgs.get();

    return std::make_unique<SlicedInt64Values>(block_shift,
                                               std::move(readers),
                                               mins,
                                               avgs,
                                               meta.num_blocks);
  }
};  // DirectMonotonicReader

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_READER_H_
