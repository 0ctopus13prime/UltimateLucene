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

#include <Codec/CodecUtil.h>
#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Util/Etc.h>
#include <Util/Exception.h>
#include <Util/Numeric.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace util {

class PackedInts {
 public:
  static const float FASTEST;  // 7F
  static const float FAST;    // 0.5F
  static const float DEFAULT;  // 0.25F
  static const float COMPACT;  // 0F
  static const uint32_t DEFAULT_BUFFER_SIZE = 1024;
  static const std::string CODEC_NAME;  // "PackedInts"
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
    Format& operator=(const Format& other) {
      id = other.id;
      return *this;
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
        return
        static_cast<uint64_t>(std::ceil(static_cast<double>(value_count) *
                              bits_per_value / 8));
      } else {  // PACKED_SINGLE_BLOCK
        assert(bits_per_value >= 0 && bits_per_value <= 64);
        return (8L * LongCount(packed_ints_version,
                               value_count,
                               bits_per_value));
      }
    }

    uint32_t LongCount(const uint32_t packed_ints_version,
                       const uint32_t value_count,
                       const uint32_t bits_per_value) const noexcept {
      if (id == 0) {  // PACKED
        assert(bits_per_value >= 0 && bits_per_value <= 64);
        const uint64_t byte_count = ByteCount(packed_ints_version,
                                              value_count,
                                              bits_per_value);
        assert(byte_count < 8L * std::numeric_limits<int32_t>::max());
        if ((byte_count % 8) == 0) {
          return static_cast<uint32_t>(byte_count / 8);
        } else {
          return static_cast<uint32_t>(byte_count / 8 + 1);
        }
      } else {  // PACKED_SINGLE_BLOCK
        const uint32_t values_per_block = (64 / bits_per_value);
        const uint32_t overhead = (64 % bits_per_value);
        return
        static_cast<uint32_t>(std::ceil(static_cast<double>(value_count) /
                              values_per_block));
      }
    }

    bool IsSupported(const uint32_t bits_per_value) const noexcept;

    float OverheadPerValue(const uint32_t bits_per_value) const noexcept {
      if (id == 0) {  // PACKED
        return 0;
      } else {  // PACKED_SINGLE_BLOCK
        assert(IsSupported(bits_per_value));
        const uint32_t values_per_block = (64 / bits_per_value);
        const uint32_t overhead = (64 % bits_per_value);
        return (static_cast<float>(overhead) / values_per_block);
      }
    }
    
    float OverheadRatio(const uint32_t bits_per_value) const noexcept {
      assert(IsSupported(bits_per_value));
      return OverheadPerValue(bits_per_value) / bits_per_value;
    }
  };  // class Format

  class FormatAndBits {
   public:
    Format format;
    uint32_t bits_per_value;

    FormatAndBits(const Format format, const uint32_t bits_per_value)
      : format(format),
        bits_per_value(bits_per_value) {
    }
  };  // class FormatAndBits

  class BlockBase {
   public:
    virtual ~BlockBase() = default;

    virtual uint32_t LongBlockCount() = 0;

    virtual uint32_t LongValueCount() = 0;

    virtual uint32_t ByteBlockCount() = 0;

    virtual uint32_t ByteValueCount() = 0;
  };

  class Decoder: public virtual BlockBase {
   public:
    virtual ~Decoder() = default;

    virtual void Decode(const uint64_t blocks[],
                        uint32_t blocks_offset,
                        int64_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;

    virtual void Decode(const char blocks[],
                        uint32_t blocks_offset,
                        int64_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;

    virtual void Decode(const uint64_t blocks[],
                        uint32_t blocks_offset,
                        int32_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;

    virtual void Decode(const char blocks[],
                        uint32_t blocks_offset,
                        int32_t values[],
                        uint32_t values_offset,
                        uint32_t iterations) = 0;
  };  // class Decoder

  class Encoder: public virtual BlockBase {
   public:
    virtual ~Encoder() = default;

    virtual void Encode(const int64_t values[],
                        uint32_t values_offset,
                        uint64_t blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

    virtual void Encode(const int64_t values[],
                        uint32_t values_offset,
                        char blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

    virtual void Encode(const int32_t values[],
                        uint32_t values_offset,
                        uint64_t blocks[],
                        uint32_t blocks_offset,
                        uint32_t iterations) = 0;

    virtual void Encode(const int32_t values[],
                        uint32_t values_offset,
                        char blocks[],
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

    virtual int64_t Get(uint32_t index) = 0;

    virtual uint32_t Size() = 0;
  };  // class Reader

  class ReaderIterator {
   public:
    virtual ~ReaderIterator() = default;

    virtual int64_t Next() = 0;

    virtual LongsRef& Next(uint32_t count) = 0;

    virtual uint32_t GetBitsPerValue() = 0;

    virtual uint32_t Size() = 0;

    virtual uint32_t Ord() = 0;
  };  // ReaderIterator

  class ReaderImpl : public Reader {
   protected:
    uint32_t value_count;

    ReaderImpl(const uint32_t value_count)
      : Reader(),
        value_count(value_count) {
    }

    ~ReaderImpl() = default;

    virtual int64_t Get(uint32_t index) = 0;

    virtual uint32_t Size() {
      return value_count;
    }
  };  // class ReaderImpl

  class ReaderIteratorImpl : public ReaderIterator {
   protected:
    lucene::core::store::DataInput* in;
    uint32_t bits_per_value;
    uint32_t value_count;

   public:
    ReaderIteratorImpl(const uint32_t value_count,
                       const uint32_t bits_per_value,
                       lucene::core::store::DataInput* in)
      : ReaderIterator(),
        in(in),
        bits_per_value(bits_per_value),
        value_count(value_count) {
    }

    int64_t Next() {
      LongsRef& next_values = ReaderIterator::Next(1);
      const int64_t result = next_values.Longs()[next_values.Offset()];
      next_values.IncOffset();
      next_values.DecLength();

      return result;
    }

    uint32_t GetBitsPerValue() {
      return bits_per_value;
    }

    uint32_t Size() {
      return value_count;
    }
  };  // ReaderIteratorImpl

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
      assert(len > 0);
      assert(index >= 0 && index < value_count);
      len = std::min(len, value_count - index);
      std::memset(arr + off, 0, sizeof(int64_t) * len);
      return len;
    }

    uint32_t Size() {
      return value_count;
    }
  };  // class NullReader

  class Writer {
   protected:
    lucene::core::store::DataOutput* out;
    uint32_t value_count;
    uint32_t bits_per_value;

    Writer(lucene::core::store::DataOutput* out,
           const uint32_t value_count,
           const uint32_t bits_per_value)
      : out(out),
        value_count(value_count),
        bits_per_value(bits_per_value) {
      assert(bits_per_value <= 64);
      assert(value_count >= 0 || value_count == -1);
    }

    virtual PackedInts::Format GetFormat() = 0;

   public:
    virtual void Add(int64_t v) = 0;

    int32_t BitsPerValue() const noexcept {
      return bits_per_value;
    }

    void WriteHeader() {
      lucene::core::codec::CodecUtil::WriteHeader(out,
                                                  CODEC_NAME,
                                                  VERSION_CURRENT);
      out->WriteVInt32(bits_per_value);
      out->WriteVInt32(value_count);
      out->WriteVInt32(GetFormat().GetId());
    }

    virtual void Finish() = 0;

    virtual uint32_t Ord() = 0;
  };  // class Writer

  class Mutable : public Reader {
   public:
    virtual ~Mutable() = default;

    virtual uint32_t GetBitsPerValue() = 0;

    virtual void Set(uint32_t index, int64_t value) = 0;

    virtual uint32_t Set(uint32_t index,
                         const int64_t arr[],
                         uint32_t off,
                         uint32_t len) {
      assert(len > 0);
      assert(index >= 0 && index <= Size());
      len = std::min(len, Size() - index);

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

    void Save(lucene::core::store::DataOutput* out) {
      std::unique_ptr<Writer> writer =
      GetWriterNoHeader(out,
                        GetFormat(),
                        Size(),
                        GetBitsPerValue(),
                        DEFAULT_BUFFER_SIZE);
      writer->WriteHeader();
      for (uint32_t i = 0 ; i < Size() ; ++i) {
        writer->Add(Get(i));
      }

      writer->Finish();
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

   public:
    virtual ~MutableImpl() = default;
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

    return lucene::core::util::numeric::NumericUtils
           ::NumberOfTrailingZeros(block_size);
  }

  static uint32_t NumBlocks(const uint64_t size, const uint32_t block_size) {
    const uint32_t num_blocks =
      static_cast<const uint32_t>(size / block_size) +
      ((size % block_size) == 0 ? 0 : 1);

    if (static_cast<const uint64_t>(num_blocks) * block_size < size) {
      throw IllegalArgumentException("Size is too large for this block size");
    }

    return num_blocks;
  }

  static uint32_t BitsRequired(const uint64_t max_value) {
    if (max_value < 0) {
      throw IllegalArgumentException("`max_value` must be non-negative (Got: " +
                                     std::to_string(max_value) + ")");
    }

    return UnsignedBitsRequired(max_value);
  }

  static void Copy(PackedInts::Reader* src,
                   uint32_t src_pos,
                   PackedInts::Mutable* dest,
                   uint32_t dest_pos,
                   uint32_t len,
                   uint32_t mem) {
    assert(src_pos + len <= src->Size());
    assert(dest_pos + len <= dest->Size());
    const uint32_t capacity = (mem >> 3);
    if (capacity == 0) {
      for (uint32_t i = 0 ; i < len ; ++i) {
        dest->Set(dest_pos++, src->Get(src_pos++));
      }
    } else if (len > 0) {
      // Use bulk operations
      const uint32_t buf_size = std::min(capacity, len);
      int64_t buf[buf_size];
      Copy(src, src_pos, dest, dest_pos, len, buf, buf_size);
    }
  }

  static void Copy(PackedInts::Reader* src,
                   uint32_t src_pos,
                   PackedInts::Mutable* dest,
                   uint32_t dest_pos,
                   uint32_t len,
                   int64_t buf[],
                   uint32_t buf_size) {
    uint32_t remaining = 0;
    while (len > 0) {
      const uint32_t read =
        src->Get(src_pos, buf, remaining, std::min(len, buf_size));
      assert(read > 0);
      src_pos += read;
      len -= read;
      remaining += read;
      const uint32_t written = dest->Set(dest_pos, buf, 0, remaining);
      assert(written > 0);
      dest_pos += written;
      if (written < remaining) {
        std::memcpy(buf, buf + written,
                    sizeof(int64_t) * (remaining - written));
      }
      remaining -= written;
    }

    while (remaining > 0) {
      const uint32_t written = dest->Set(dest_pos, buf, 0, remaining);
      dest_pos += written;
      remaining -= written;
      std::memcpy(buf, buf + written,
                  sizeof(int64_t) * remaining);
    }
  }

  static PackedInts::FormatAndBits
  FastestFormatAndBits(uint32_t value_count,
                       uint32_t bits_per_value,
                       float acceptable_overhead_ratio);

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
             const uint32_t bits_per_value);

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

  static std::unique_ptr<PackedInts::ReaderIterator>
  GetReaderIterator(lucene::core::store::DataInput* in, const uint32_t mem);

  static std::unique_ptr<PackedInts::ReaderIterator>
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

  static const uint64_t MaxValue(const uint32_t bits_per_value) {
    return (bits_per_value == 64 ?
            std::numeric_limits<int64_t>::max() :
            ((1L << bits_per_value) - 1));
  }

  static const uint32_t UnsignedBitsRequired(const int64_t bits) {
    return std::max(1U, 64 - lucene::core::util::numeric
                            ::NumericUtils::NumberOfLeadingZeros(bits));
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKEDINTS_H_
