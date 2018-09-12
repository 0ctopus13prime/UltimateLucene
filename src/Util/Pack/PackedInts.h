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

#ifndef SRC_UTIL_PACK_PACKEDINTS_H_
#define SRC_UTIL_PACK_PACKEDINTS_H_

#include <Store/DataInput.h>
#include <Util/Exception.h>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace util {

class PackedInts {
 public:
  static const float FASTEST;
  static const float FAST;
  static const float DEFAULT;
  static const float COMPACT;
  static const uint32_t DEFAULT_BUFFER_SIZE = 1024;
  static const std::string CODEC_NAME;
  static const uint32_t VERSION_MONOTONIC_WITHOUT_ZIGZAG = 2;
  static const uint32_t VERSION_START = VERSION_MONOTONIC_WITHOUT_ZIGZAG;
  static const uint32_t VERSION_CURRENT = VERSION_MONOTONIC_WITHOUT_ZIGZAG;

  class Format {
   public:
    const static Format PACKED;
    const static Format PACKED_SINGLE_BLOCK;

   public:
    static Format ById(const uint32_t id) {
      switch(id) {
        case 0:
          return PACKED;
        case 1:
          return PACKED_SINGLE_BLOCK;
      }

      throw IllegalArgumentException("Unknown format id: " +
                                     std::to_string(id));
    }

   private:
    friend class PackedInts;

    uint32_t id;

    Format(const uint32_t id)
      : id(id) {
    }

   public:
    FormatAndBits& operator=(const FormatAndBits& other)
      : id(other.id) {
    }

    bool operator==(const Format& other) const noexcept {
      return (other.id == id);
    }

    bool operator!=(const Format& other) const noexcept {
      return !(*this == other);
    }

    uint32_t GetId() const noexcept {
     return id;
    }

    uint64_t ByteCount(const uint32_t packed_ints_version,
                       const uint32_t value_count,
                       const uint32_t bits_per_value) const noexcept {
      if (id == 0)  {  // PACKED
        return static_cast<uint64_t>(ceil(static_cast<double>(value_count) *
                                     bits_per_value / 8));
      } else {  // PACKED_SINGLE_BLOCK
        return (8L * LongCount(packed_ints_version,
                               value_count,
                               bits_per_value));
      }
    }

    uint32_t LongCount(const uint32_t packed_ints_version,
                       const uint32_t value_count,
                       const uint32_t bits_per_value) const noexcept {
      if (id == 0) {  // PACKED
        const uint64_t byte_count = ByteCount(packed_ints_version,
                                              value_count,
                                              bits_per_value);
        if ((byte_count % 8) == 0) {
          return static_cast<uint32_t>(byte_count / 8);
        } else {
          return static_cast<uint32_t>(byte_count / 8 + 1);
        }
      } else {  // PACKED_SINGLE_BLOCK
        const uint32_t values_per_block = (64 / bits_per_value);
        const uint32_t overhead = (64 % bits_per_value);
        return static_cast<uint32_t>(ceil(static_cast<double>(value_count) /
                                     values_per_block));
      }
    }

    virtual bool
    IsSupported(const uint32_t bits_per_value) const noexcept {
      if (id == 0) {
        return (bits_per_value >= 1 && bits_per_value <= 64);
      } else {
        // TODO(0ctopus13prime): Fix this
        // return Packed64SingleBlock.isSupported(bitsPerValue);
      }
    }

    virtual float
    OverheadPerValue(const uint32_t bits_per_value) const noexcept {
      if (id == 0) {
        return 0;
      } else {
        const uint32_t values_per_block = (64 / bits_per_value);
        const uint32_t overhead = (64 % bits_per_value);
        return (static_cast<float>(overhead) / values_per_block);
      }
    }

    virtual float
    OverheadRatio(const uint32_t bits_per_value) const noexcept {
      return OverheadPerValue(bits_per_value) / bits_per_value;
    }
  };  // class Format

  class FormatAndBits {
   public:
    Format format;
    uint32_t bits_per_value;

    FormatAndBits(Format format, const uint32_t bits_per_value)
      : format(format),
        bits_per_value(bits_per_value) {
    }
  };  // class FormatAndBits

  class Decoder {
   public:
    virtual ~Decoder() = default;

    virtual uint32_t LongBlockCount() = 0;

    virtual uint32_t LongValueCount() = 0;

    virtual uint32_t ByteBlockCount() = 0;

    virtual uint32_t ByteValueCount() = 0;

    virtual void Decode(const uint64_t blocks[],
                        uint32_t blocks_offset,
                        int64_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;

    virtual void Decode(const uint8_t blocks[],
                        uint32_t blocks_offset,
                        int64_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;

    virtual void Decode(const uint64_t blocks[],
                        uint32_t blocks_offset,
                        int32_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;

    virtual void Decode(const uint8_t blocks[],
                        uint32_t blocks_offset,
                        int32_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;
  };  // class Decoder

  class Encoder {
   public:
    virtual ~Encoder() = default;

    virtual uint32_t LongBlockCount() = 0;

    virtual uint32_t LongValueCount() = 0;

    virtual uint32_t ByteBlockCount() = 0;

    virtual uint32_t ByteValueCount() = 0;

    virtual void Encode(const int64_t values[],
                        uint32_t values_offset,
                        uint64_t blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

    virtual void Encode(const int64_t values[],
                        uint32_t values_offset,
                        uint8_t blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

    virtual void Encode(const int32_t values[],
                        uint32_t values_offset,
                        uint64_t blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

    virtual void Encode(const int32_t values[],
                        uint32_t values_offset,
                        uint8_t blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

  };  // class Encoder

  class Reader {
   public:
    virtual ~Reader() = default;

    virtual uint32_t Get(uint32_t index,
                         int64_t arr[],
                         uint32_t off,
                         uint32_t len) {
      const uint32_t gets = std::min(Size() - index, len);
      for (uint32_t i = index, o = off, end = index + gets ;
           i < end ;
           ++i, ++o) {
        arr[o] = Get(i);
      }
    }

    virtual int64_t Get(uint32_t docID) = 0;

    virtual uint32_t Size() = 0;
  };  // class Reader

  class ReaderImpl : public Reader {
   protected:
    uint32_t value_count;

    ReaderImpl(const uint32_t value_count)
      : value_count(value_count) {
    }

    virtual int64_t Get(uint32_t index) = 0;

    uint32_t Size() {
      return value_count;
    }
  };  // class ReaderImpl

  class NullReader : public Reader {
   private:
    uint32_t value_count;

   public:
    NullReader(const uint32_t value_count)
      : value_count(value_count) {
    }

    int64_t Get(uint32_t index) {
      return 0;
    }

    uint32_t Get(uint32_t index,
                int64_t arr[],
                uint32_t off,
                uint32_t len) {
      const uint32_t actual_len = std::min(len, value_count - index);
      memset(arr + offset, 0, sizeof(int64_t) * actual_len);
      return actual_len;
    }

    uint32_t Size() {
      return value_count;
    }
  };  // class NullReader

  class ReaderIterator {
   public:
    virtual ~ReaderIterator() = default;

    virtual int64_t Next() = 0;

    virtual LongsRef& Next(const uint32_t count) = 0;

    virtual uint32_t GetBitsPerValue() = 0;

    virtual uint32_t Size() = 0;

    virtual uint32_t Ord() = 0;
  };  // ReaderIterator

  class ReaderIteratorImpl : public ReaderIterator {
   protected:
    std::unique_ptr<DataInput> in;
    uint32_t bits_per_value;
    uint32_t value_count;

   public:
    ReaderIteratorImpl(const uint32_t value_count,
                       const uint32_t bits_per_value,
                       std::unique_ptr<DataInput>&& in)
      : in(std::forward<std::unique_ptr<DataInput>>(in)),
        bits_per_value(bits_per_value),
        value_count(value_count) {
    }

    int64_t Next() {
      LongsRef& next_values = Next(1);
      const int64_t result = next_values.longs[next_values.offset];
      ++next_values.offset;
      --next_values.length;

      return result;
    }

    uint32_t GetBitsPerValue() {
      return bits_per_value;
    }

    uint32_t Size() {
      return value_count;
    }
  };  // ReaderIteratorImpl

  class Writer {
   protected:
    std::unique_ptr<DataOutput> out;
    uint32_t value_count;
    uint32_t bits_per_value;

    Writer(std::unique_ptr<DataOut>&& out,
           uint32_t value_count,
           uint32_t bits_per_value)
      : out(std::forward<std::unique_ptr<DataOut>>(out)),
        value_count(value_count),
        bits_per_value(bits_per_value) {
    }

   public:
    void WriteHeader() {
      CodecUtil::WriteHeader(out.get(), CODEC_NAME, VERSION_CURRENT);
      out->WriteVInt(bits_per_value);
      out->WriteVInt(value_count);
      out->WriteVInt(GetFormat().GetId());
    }

    virtual void Add(int64_t v) = 0;

    int32_t BitsPerValue() const noexcept {
      return bits_per_value;
    }

    virtual void Finishi() = 0;

    virtual uint32_t Ord() = 0;
  };  // class Writer

  class Mutable : public Reader {
   public:
    virtual uint32_t GetBitsPerValue() = 0;

    virtual void Set(uint32_t index, int64_t value) = 0;

    virtual uint32_t Set(uint32_t index,
                         const int64_t arr[],
                         uint32_t off,
                         uint32_t len) {
      const uint32_t actual_len = std::min(len, Size() - index);

      for (uint32_t i = index, o = off, end = index + len ;
           i < end ;
           ++i, ++o) {
        Set(i, arr[o]);
      }
    }

    virtual void Fill(uint32_t from_index,
                      uint32_t to_index,
                      int64_t val) {
      for (uint32_t i = from_index ; i < to_index ; ++i) {
        Set(i, val);
      }
    }

    virtual void Clear() {
      // TODO (0ctopus13prime) : Using native approach
      // std::fill( array, array + sizeof( array ), 0 );
      // or memset
      Fill(0, Size(), 0);
    }

    void Save(DataOutput* out) {
      // TODO(0ctopus13prime) : Implement this
    }

    Format GetFormat() const noexcept {
      return Format::PACKED;
    }
  };  // class Mutable

  class MutableImpl : public Mutable {
   protected:
    uint32_t value_count;
    uint32_t bits_per_value;

    MutableImpl(const uint32_t value_count, const uint32_t bits_per_value)
      : value_count(value_count),
        bits_per_value(bits_per_value) {
    }

    uint32_t GetBitsPerValue() {
      return bits_per_value;
    }

    uint32_t Size() {
      return value_count;
    }
  };  // MutableImpl

 public:
  static void CheckVersion(const uint32_t version) {
    if (version < VERSION_START) {
      throw IllegalArgumentException("Version is too old, should be at least " +
                                     std::to_string(VERSION_START) +
                                     " (got " +
                                     std::to_string(version) +
                                     ")");
    } else if (version > VERSION_START) {
      throw IllegalArgumentException("Version is too new, should be at most " +
                                     std::to_string(VERSION_CURRENT) +
                                     " (got " +
                                     std::to_string(version) +
                                     ")");
    }
  }

  static uint32_t CheckBlockSize(const uint32_t block_size,
                                 const uint32_t min_block_size,
                                 const uint32_t max_block_size) {
    if (block_size < min_block_size || block_size > max_block_size) {
      throw IllegalArgumentException("Block size must be >= " +
                                     std::to_string(min_block_size) +
                                     " and <= " +
                                     std::to_string(max_block_size) +
                                     ", got " +
                                     std::to_string(block_size));
    }

    if ((block_size & (block_size - 1)) != 0) {
      throw IllegalArgumentException("Block size must be a power of two, got " +
                                     std::to_string(block_size));
    }

    return NumericUtils::NumberOfTrailingZeros(block_size);
  }

  static uint32_t NumBlocks(const uint64_t size, const uint32_t block_size) {
    const uint32_t num_blocks =
      static_cast<const uint32_t>(size / block_size) +
      ((size % block_size) & 1);

    if (static_cast<const uint64_t>(num_blocks) * block_size < size) {
      throw IllegalArgumentException("Size is too large for this block size");
    }

    return num_blocks;
  }

  static uint32_t BitsRequired(const uint64_t max_value);

  static void Copy(PackedInts::Reader* src,
                   const uint32_t src_pos,
                   PackedInts::Mutable* dest,
                   const uint32_t dest_pos,
                   const uint32_t len,
                   const uint32_t mem);

  static PackedInts::FormatAndBits
  FastestFormatAndBits(const uint32_t value_count,
                       const uint32_t bits_per_value,
                       const float acceptable_overhead_ratio);

  static PackedInts::Decoder* GetDecoder(PackedInts::Format format,
                                  const uint32_t version,
                                  const uint32_t bits_per_value);

  static std::unique_ptr<PackedInts::Reader>
  GetDirectReader(lucene::core::store::IndexInput* in);

  static std::unique_ptr<PackedInts::Reader>
  GetDirectReaderNoHeader(lucene::core::store::IndexInput* in,
                          PackedInts::Format format,
                          const uint32_t version,
                          const uint32_t value_count,
                          const uint32_t bits_per_value);

  static PackedInts::Encoder*
  GetEncoder(PackedInts::Format format,
             const uint32_t version,
             cosnt uint32_t bits_per_value);

  static std::unique_ptr<PackedInts::Mutable>
  GetMutable(const uint32_t value_count,
             const uint32_t bits_per_value,
             const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedInts::Mutable>
  GetMutable(const uint32_t value_count,
             const uint32_t bits_per_value,
             PackedInts::Format format);

  static std::unique_ptr<PackedInts::Reader>
  GetReader(lucene::core::store::DataInput* in);

  static std::unique_ptr<PackedInts::Reader>
  GetReaderNoHeader(lucene::core::store::DataInput* in,
                    PackedInts::Format format,
                    const uint32_t version,
                    const uint32_t value_count,
                    const uint32_t bits_per_value);

  static PackedInts::ReaderIterator
  GetReaderIterator(lucene::core::store::DataInput* in, const uint32_t mem);

  static PackedInts::ReaderIterator
  GetReaderIteratorNoHeader(lucene::core::store::DataInput* in,
                            PackedInts::Format format,
                            const uint32_t version,
                            const uint32_t value_count,
                            const uint32_t bits_per_value,
                            const uint32_t mem);

  static std::unique_ptr<PackedInts::Writer>
  GetWriter(lucene::core::store::DataOutput* out,
            const uint32_t value_count,
            const uint32_t bits_per_value,
            const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedInts::Writer>
  GetWriterNoHeader(lucene::core::store::DataOutput* out,
                    PackedInts::Format format,
                    const uint32_t version_count,
                    const uint32_t bits_per_value,
                    const uint32_t mem);

  static const uint64_t MaxValue(const uint32_t bits_per_value);

  static const uint32_t UnsignedBitsRequired(const int64_t bits);
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKEDINTS_H_
