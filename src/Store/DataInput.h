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

#ifndef SRC_STORE_DATAINPUT_H_
#define SRC_STORE_DATAINPUT_H_

#include <Util/Bits.h>
#include <Util/Exception.h>
#include <Util/Numeric.h>
#include <Store/Context.h>
#include <Store/Exception.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <utility>

namespace lucene {
namespace core {
namespace store {

class RandomAccessInput {
 public:
  virtual ~RandomAccessInput() { }

  virtual char ReadByte(const uint64_t pos) = 0;

  virtual int16_t ReadInt16(const uint64_t pos) = 0;

  virtual int32_t ReadInt32(const uint64_t pos) = 0;

  virtual int64_t ReadInt64(const uint64_t pos) = 0;
};

class DataInput {
 private:
  static const uint32_t SKIP_BUFFER_SIZE;

 private:
  std::unique_ptr<char[]> skip_buffer;

 private:
  void AllocateSkipBufferIf() {
    if (!skip_buffer) {
      skip_buffer = std::make_unique<char[]>(DataInput::SKIP_BUFFER_SIZE);
    }
  }

  int64_t ReadVInt64(const bool allow_negative) {
    char b = ReadByte();
    if (b >= 0) return b;
    int64_t i = b & 0x7FL;
    b = ReadByte();
    i |= (b & 0x7FL) << 7;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 14;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 21;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 28;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 35;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 42;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 49;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7FL) << 56;
    if (b >= 0) return i;

    if (allow_negative) {
      b = ReadByte();
      i |= (b & 0x7FL) << 63;
      if (b == 0 || b == 1) return i;
      throw lucene::core::util::IOException(
            "Invalid vLong detected (more than 64 bits)");
    } else {
      throw lucene::core::util::IOException(
            "Invalid vLong detected (negative values disallowed)");
    }
  }

 public:
  DataInput() = default;

  virtual ~DataInput() = default;

  virtual char ReadByte() = 0;

  virtual void ReadBytes(char bytes[],
                         const uint32_t offset,
                         const uint32_t len) = 0;

  void ReadBytes(char bytes[],
                 const uint32_t offset,
                 const uint32_t len,
                 const bool use_buffer) {
    ReadBytes(bytes, offset, len);
  }

  virtual int16_t ReadInt16() {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[1] = ReadByte();
    iab.bytes[0] = ReadByte();
#else
    iab.bytes[0] = ReadByte();
    iab.bytes[1] = ReadByte();
#endif

    return iab.int16;
  }

  virtual int32_t ReadInt32() {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[3] = ReadByte();
    iab.bytes[2] = ReadByte();
    iab.bytes[1] = ReadByte();
    iab.bytes[0] = ReadByte();
#else
    iab.bytes[0] = ReadByte();
    iab.bytes[1] = ReadByte();
    iab.bytes[2] = ReadByte();
    iab.bytes[3] = ReadByte();
#endif

    return iab.int32;
  }

  virtual int32_t ReadVInt32() {
    char b = ReadByte();
    if (b >= 0) return b;
    int32_t i = b & 0x7F;
    b = ReadByte();
    i |= (b & 0x7F) << 7;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7F) << 14;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x7F) << 21;
    if (b >= 0) return i;
    b = ReadByte();
    i |= (b & 0x0F) << 28;
    if ((b & 0xF0) == 0) return i;
    throw lucene::core::util::IOException(
          "Invalid vInt detected (too many bits)");
  }

  virtual int32_t ReadZInt32() {
    return lucene::core::util::BitUtil::ZigZagDecode(ReadVInt32());
  }

  virtual int64_t ReadInt64() {
    return ((static_cast<int64_t>(ReadInt32()) << 32) |
            (ReadInt32() & 0xFFFFFFFFL));
  }

  virtual int64_t ReadVInt64() {
    return ReadVInt64(false);
  }

  virtual int64_t ReadZInt64() {
    return lucene::core::util::BitUtil::ZigZagDecode(ReadVInt64(true));
  }

  virtual std::string ReadString() {
    const int32_t length = ReadVInt32();
    char bytes[length];  // TODO(0ctopus13prime): alloca? built in alloca?
    ReadBytes(bytes, 0, length);

    return std::string(bytes, length/*, UTF_8 */);
  }

  virtual std::map<std::string, std::string> ReadMapOfStrings() {
    // TODO(0ctopus13prime): Implement it
    return {};
  }

  virtual std::set<std::string> ReadSetOfStrings() {
    // TODO(0ctopus13prime): Implement it
    return {};
  }

  virtual void SkipBytes(const int64_t num_bytes) {
    if (num_bytes < 0) {
      throw lucene::core::util::IllegalArgumentException(
            std::string("`num_bytes` must e >= 0, got ") +
            std::to_string(num_bytes));
    }

    AllocateSkipBufferIf();
    for (int64_t skipped = 0 ; skipped < num_bytes ; /* Nothing */) {
      const uint32_t delta = num_bytes - skipped;
      const uint32_t step = std::min(SKIP_BUFFER_SIZE, delta);
      ReadBytes(skip_buffer.get(), 0, step, false);
      skipped += step;
    }
  }
};

class IndexInput: public DataInput {
 protected:
  const std::string resource_desc;

 protected:
  explicit IndexInput(const std::string& resource_desc)
    : DataInput(),
      resource_desc(resource_desc) {
  }

  explicit IndexInput(std::string&& resource_desc)
    : DataInput(),
      resource_desc(std::forward<std::string>(resource_desc)) {
  }

  explicit IndexInput(const IndexInput& other)
    : DataInput(),
      resource_desc(other.resource_desc) {
  }

 private:
  class DefaultRandomAccessInputImpl: public RandomAccessInput {
   private:
    std::unique_ptr<IndexInput> slice;

   public:
    explicit DefaultRandomAccessInputImpl(std::unique_ptr<IndexInput>&& slice)
      : slice(std::forward<std::unique_ptr<IndexInput>>(slice)) {
    }

    ~DefaultRandomAccessInputImpl() { }

    char ReadByte(const uint64_t pos) {
      slice->Seek(pos);
      return slice->ReadByte();
    }

    int16_t ReadInt16(const uint64_t pos) {
      slice->Seek(pos);
      return slice->ReadInt16();
    }

    int32_t ReadInt32(const uint64_t pos) {
      slice->Seek(pos);
      return slice->ReadInt32();
    }

    int64_t ReadInt64(const uint64_t pos) {
      slice->Seek(pos);
      return slice->ReadInt64();
    }
  };

 public:
  IndexInput() = default;

  virtual ~IndexInput() = default;

  virtual void Close() = 0;

  virtual uint64_t GetFilePointer() = 0;

  virtual void Seek(const uint64_t pos) = 0;

  virtual uint64_t Length() = 0;

  virtual std::unique_ptr<IndexInput>
  Slice(const std::string& slice_description,
        const uint64_t offset,
        const uint64_t length) = 0;

  std::unique_ptr<RandomAccessInput>
  RandomAccessSlice(const uint64_t offset,
                    const uint64_t length) {
    std::unique_ptr<IndexInput> slice = Slice("randomaccess", offset, length);

    if (RandomAccessInput* ra_input =
        dynamic_cast<RandomAccessInput*>(slice.get())) {
      slice.release();
      return std::unique_ptr<RandomAccessInput>(ra_input);
    } else {
      return std::unique_ptr<RandomAccessInput>(
      new DefaultRandomAccessInputImpl(std::move(slice)));
    }
  }
};

class ChecksumIndexInput: public IndexInput {
 protected:
  explicit ChecksumIndexInput(const std::string& resource_desc)
    : IndexInput(resource_desc) {
  }

  explicit ChecksumIndexInput(std::string&& resource_desc)
    : IndexInput(std::forward<std::string>(resource_desc)) {
  }

 public:
  virtual int64_t GetChecksum() = 0;

  void Seek(const uint64_t pos) {
    const uint64_t cur_fp = GetFilePointer();
    const int64_t skip = pos - cur_fp;

    if (skip < 0) {
      throw lucene::core::util::IllegalArgumentException(
            std::string("ChecksumIndexInput cannot seek backwards (pos = ") +
            std::to_string(pos) +
            " GetFilePointer() = " +
            std::to_string(cur_fp) +
            ")");
    }

    SkipBytes(skip);
  }
};

class BufferedChecksumIndexInput: public ChecksumIndexInput {
 private:
  std::unique_ptr<IndexInput> main;
  std::unique_ptr<lucene::core::util::Checksum> digest;

 public:
  explicit BufferedChecksumIndexInput(std::unique_ptr<IndexInput>&& main)
    : ChecksumIndexInput(std::string("BufferedChecksumIndexInput")),
      main(std::forward<std::unique_ptr<IndexInput>>(main)),
      digest(std::make_unique<BufferedChecksum>(
               std::make_unique<lucene::core::util::Crc32>())) {
  }

  char ReadByte() {
    const char b = main->ReadByte();
    digest->Update(b);
    return b;
  }

  void ReadBytes(char bytes[], const uint32_t offset, const uint32_t len) {
    main->ReadBytes(bytes, offset, len);
    digest->Update(bytes, offset, len);
  }

  int64_t GetChecksum() {
    return digest->GetValue();
  }

  void Close() {
    main->Close();
  }

  uint64_t GetFilePointer() {
    return main->GetFilePointer();
  }

  uint64_t Length() {
    return main->Length();
  }

  std::unique_ptr<IndexInput>
  Slice(const std::string& slice_desc,
        const uint64_t offset,
        const uint64_t length) {
    throw lucene::core::util::UnsupportedOperationException();
  }
};

class BufferedIndexInput: public IndexInput, RandomAccessInput {
 public:
  static const uint32_t BUFFER_SIZE = 1024;
  static const uint32_t MIN_BUFFER_SIZE = 8;
  static const uint32_t MERGE_BUFFER_SIZE = 4096;

 protected:
  std::unique_ptr<char[]> buffer;

 private:
  class SlicedIndexInput;
  uint32_t buffer_size;
  uint64_t buffer_start;
  uint32_t buffer_length;
  uint32_t buffer_position;

 protected:
  void NewBuffer(char new_buffer[]) {
    buffer.reset(new_buffer);
  }

  virtual void SeekInternal(const uint64_t pos) = 0;

  virtual void ReadInternal(char byte[],
                            const uint32_t offset,
                            const uint32_t length) = 0;

 private:
  void Refill() {
    const uint64_t start = buffer_start + buffer_position;
    uint64_t end = start + buffer_size;
    end = std::min(end, Length());
    if (end <= start) {
      throw lucene::core::util::EOFException();
    }
    const uint32_t new_length = static_cast<uint32_t>(end - start);

    if (!buffer) {
      NewBuffer(new char[buffer_size]);
      SeekInternal(buffer_start);
    }
    ReadInternal(buffer.get(), 0, new_length);
    buffer_length = new_length;
    buffer_start = start;
    buffer_position = 0;
  }

 public:
  static uint32_t BufferSize(const IOContext& context) {
    switch (context.context) {
      case IOContext::Context::MERGE:
        return BufferedIndexInput::MERGE_BUFFER_SIZE;
      default:
        return BufferedIndexInput::BUFFER_SIZE;
    }
  }

  static std::unique_ptr<BufferedIndexInput>
  Wrap(const std::string& slice_desc,
       IndexInput* other,
       const uint64_t offset,
       const uint64_t length);

 public:
  explicit BufferedIndexInput(const std::string& resource_desc)
    : BufferedIndexInput(resource_desc, BufferedIndexInput::BUFFER_SIZE) {
  }

  BufferedIndexInput(const std::string& resource_desc, const IOContext& context)
    : BufferedIndexInput(resource_desc,
                         BufferedIndexInput::BufferSize(context)) {
  }

  BufferedIndexInput(const std::string& resource_desc,
                     const uint32_t buffer_size)
    : IndexInput(resource_desc),
      RandomAccessInput(),
      buffer(),
      buffer_size(buffer_size),
      buffer_start(0),
      buffer_length(0),
      buffer_position(0) {
  }

  char ReadByte() {
    if (buffer_position >= buffer_length) {
      Refill();
    }

    return buffer[buffer_position++];
  }

  void ReadBytes(char bytes[],
                 const uint32_t offset,
                 const uint32_t len) {
    ReadBytes(bytes, offset, len, true);
  }

  void ReadBytes(char bytes[],
                 const uint32_t offset,
                 const uint32_t len,
                 const bool use_buffer) {
    const uint32_t available = buffer_length - buffer_position;
    if (len <= available) {
      std::memcpy(bytes + offset, buffer.get() + buffer_position, len);
      buffer_position += len;
    } else {
      uint32_t offset_cp = offset;
      uint32_t len_cp = len;

      if (available > 0) {
        std::memcpy(bytes + offset, buffer.get() + buffer_position, available);
        offset_cp += available;
        len_cp -= available;
        buffer_position += available;
      }

      if (use_buffer && len_cp < buffer_size) {
        Refill();
        if (buffer_length < len_cp) {
          std::memcpy(bytes + offset, buffer.get(), buffer_length);
          throw lucene::core::util::EOFException();
        } else {
          std::memcpy(bytes + offset, buffer.get(), len_cp);
          buffer_position = len_cp;
        }
      } else {
        const uint64_t after = buffer_start + buffer_position + len_cp;
        if (after > Length()) {
          throw lucene::core::util::EOFException();
        }

        ReadInternal(bytes, offset, len_cp);
        buffer_start = after;
        buffer_position = 0;
        buffer_length = 0;
      }
    }
  }

  int16_t ReadInt16() {
    if (2 <= (buffer_length - buffer_position)) {
      lucene::core::util::numeric::Int16AndBytes iab;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      iab.bytes[1] = buffer[buffer_position++];
      iab.bytes[0] = buffer[buffer_position++];
#else
      iab.bytes[0] = buffer[buffer_position++];
      iab.bytes[1] = buffer[buffer_position++];
#endif

      return iab.int16;
    } else {
      return IndexInput::ReadInt16();
    }
  }

  int32_t ReadInt32() {
    if (4 <= (buffer_length - buffer_position)) {
      lucene::core::util::numeric::Int32AndBytes iab;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      iab.bytes[3] = buffer[buffer_position++];
      iab.bytes[2] = buffer[buffer_position++];
      iab.bytes[1] = buffer[buffer_position++];
      iab.bytes[0] = buffer[buffer_position++];
#else
      iab.bytes[0] = buffer[buffer_position++];
      iab.bytes[1] = buffer[buffer_position++];
      iab.bytes[2] = buffer[buffer_position++];
      iab.bytes[3] = buffer[buffer_position++];
#endif

      return iab.int32;
    } else {
      return IndexInput::ReadInt32();
    }
  }

  int64_t ReadInt64() {
    if (8 <= (buffer_length - buffer_position)) {
      lucene::core::util::numeric::Int64AndBytes iab;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      iab.bytes[7] = buffer[buffer_position++];
      iab.bytes[6] = buffer[buffer_position++];
      iab.bytes[5] = buffer[buffer_position++];
      iab.bytes[4] = buffer[buffer_position++];
      iab.bytes[3] = buffer[buffer_position++];
      iab.bytes[2] = buffer[buffer_position++];
      iab.bytes[1] = buffer[buffer_position++];
      iab.bytes[0] = buffer[buffer_position++];
#else
      iab.bytes[0] = buffer[buffer_position++];
      iab.bytes[1] = buffer[buffer_position++];
      iab.bytes[2] = buffer[buffer_position++];
      iab.bytes[3] = buffer[buffer_position++];
      iab.bytes[4] = buffer[buffer_position++];
      iab.bytes[5] = buffer[buffer_position++];
      iab.bytes[6] = buffer[buffer_position++];
      iab.bytes[7] = buffer[buffer_position++];
#endif

      return iab.int64;
    } else {
      return IndexInput::ReadInt64();
    }
  }

  int32_t ReadVInt32() {
    if (5 <= (buffer_length - buffer_position)) {
      char b = buffer[buffer_position++];
      if (b >= 0) return b;
      int i = b & 0x7F;
      b = buffer[buffer_position++];
      i |= (b & 0x7F) << 7;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7F) << 14;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7F) << 21;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x0F) << 28;
      if ((b & 0xF0) == 0) return i;
      throw lucene::core::util::IOException(
            "Invalid vInt detected (too many bits)");
    } else {
      return IndexInput::ReadVInt32();
    }
  }

  int64_t ReadVInt64() {
    if (9 <= buffer_length-buffer_position) {
      char b = buffer[buffer_position++];
      if (b >= 0) return b;
      int64_t i = b & 0x7FL;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 7;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 14;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 21;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 28;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 35;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 42;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 49;
      if (b >= 0) return i;
      b = buffer[buffer_position++];
      i |= (b & 0x7FL) << 56;
      if (b >= 0) return i;
      throw lucene::core::util::IOException(
            "Invalid vLong detected (negative values disallowed)");
    } else {
      return IndexInput::ReadVInt64();
    }
  }

  char ReadByte(const uint64_t pos) {
    const int64_t index = pos - buffer_start;

    if (index < 0 || index >= buffer_length) {
      buffer_start = pos;
      buffer_position = 0;
      buffer_length = 0;  // trigger refill() on read()
      SeekInternal(pos);
      Refill();
      return buffer[0];
    }

    return buffer[static_cast<int32_t>(index)];
  }

  int16_t ReadInt16(const uint64_t pos) {
    lucene::core::util::numeric::Int16AndBytes iab;
    const int64_t tmp_index = pos - buffer_start;
    int32_t idx = static_cast<int32_t>(tmp_index);

    if (tmp_index < 0 || tmp_index >= buffer_length - 1) {
      buffer_start = pos;
      buffer_position = 0;
      buffer_length = 0;  // trigger refill() on read()
      SeekInternal(pos);
      Refill();
      idx = 0;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[1] = buffer[idx++];
    iab.bytes[0] = buffer[idx++];
#else
    iab.bytes[0] = buffer[idx++];
    iab.bytes[1] = buffer[idx++];
#endif

    return iab.int16;
  }

  int32_t ReadInt32(const uint64_t pos) {
    lucene::core::util::numeric::Int32AndBytes iab;
    int64_t tmp_index = pos - buffer_start;
    int32_t idx = static_cast<int32_t>(tmp_index);

    if (tmp_index < 0 || tmp_index >= buffer_length - 3) {
      buffer_start = pos;
      buffer_position = 0;
      buffer_length = 0;  // trigger refill() on read()
      SeekInternal(pos);
      Refill();
      idx = 0;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN_
    iab.bytes[3] = buffer[idx++];
    iab.bytes[2] = buffer[idx++];
    iab.bytes[1] = buffer[idx++];
    iab.bytes[0] = buffer[idx++];
#else
    iab.bytes[0] = buffer[idx++];
    iab.bytes[1] = buffer[idx++];
    iab.bytes[2] = buffer[idx++];
    iab.bytes[3] = buffer[idx++];
#endif

    return iab.int32;
  }

  int64_t ReadInt64(const uint64_t pos) {
    lucene::core::util::numeric::Int64AndBytes iab;
    int64_t tmp_index = pos - buffer_start;
    int32_t idx = static_cast<int32_t>(tmp_index);

    if (tmp_index < 0 || tmp_index >= buffer_length - 7) {
      buffer_start = pos;
      buffer_position = 0;
      buffer_length = 0;  // trigger refill() on read()
      SeekInternal(pos);
      Refill();
      idx = 0;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN_
    iab.bytes[7] = buffer[idx++];
    iab.bytes[6] = buffer[idx++];
    iab.bytes[5] = buffer[idx++];
    iab.bytes[4] = buffer[idx++];
    iab.bytes[3] = buffer[idx++];
    iab.bytes[2] = buffer[idx++];
    iab.bytes[1] = buffer[idx++];
    iab.bytes[0] = buffer[idx++];
#else
    iab.bytes[0] = buffer[idx++];
    iab.bytes[1] = buffer[idx++];
    iab.bytes[2] = buffer[idx++];
    iab.bytes[3] = buffer[idx++];
    iab.bytes[4] = buffer[idx++];
    iab.bytes[5] = buffer[idx++];
    iab.bytes[6] = buffer[idx++];
    iab.bytes[7] = buffer[idx++];
#endif

    return iab.int64;
  }

  uint64_t GetFilePointer() {
    return buffer_start + buffer_position;
  }

  void Seek(const uint64_t pos) {
    if (pos >= buffer_start && pos < (buffer_start + buffer_length)) {
      buffer_position = static_cast<uint32_t>(pos - buffer_start);
    } else {
      buffer_start = pos;
      buffer_position = 0;
      buffer_length = 0;
      SeekInternal(pos);
    }
  }

  std::unique_ptr<IndexInput>
  Slice(const std::string& slice_desc,
        const uint64_t offset,
        const uint64_t length) {
    return std::move(BufferedIndexInput::Wrap(slice_desc,
                                              this,
                                              offset,
                                              length));
  }

  void SetBufferSize(const uint32_t new_size) {
    if (new_size != buffer_size) {
      CheckBufferSize(new_size);
      buffer_size = new_size;
      if (!buffer) {
        // TODO(0ctopus13prime): Reallocate in C?
        char* new_buffer = new char[new_size];
        const uint32_t left_in_buffer = buffer_length - buffer_position;
        const uint32_t num_to_copy = (left_in_buffer > new_size ?
                                      new_size : left_in_buffer);
        std::memcpy(new_buffer, buffer.get() + buffer_position, num_to_copy);
        buffer_start += buffer_position;
        buffer_position = 0;
        buffer_length = num_to_copy;
        NewBuffer(new_buffer);
      }
    }
  }

  const uint32_t GetBufferSize() const noexcept {
    return buffer_size;
  }

  void CheckBufferSize(const uint32_t buffer_size) {
    if (buffer_size < BufferedIndexInput::MIN_BUFFER_SIZE) {
      throw lucene::core::util::IllegalArgumentException(
            std::string("Buffer size must be at "
            "least MIN_BUFFER_SIZE = ") +
            std::to_string(BufferedIndexInput::MIN_BUFFER_SIZE));
    }
  }
};

class BufferedIndexInput::SlicedIndexInput: public BufferedIndexInput {
 private:
  IndexInput* base;
  uint64_t file_offset;
  uint64_t length;

 protected:
  void SeekInternal(const uint64_t) { }

 public:
  SlicedIndexInput(const std::string& slice_desc,
                   IndexInput* base,
                   const uint64_t offset,
                   const uint64_t length)
    : BufferedIndexInput(slice_desc, BufferedIndexInput::BUFFER_SIZE),
      base(base),
      file_offset(offset),
      length(length) {
  }

  void ReadInternal(char bytes[],
                    const uint32_t offset,
                    const uint32_t len) {
    const uint64_t start = GetFilePointer();
    if (start + len > length) {
      throw lucene::core::util::EOFException("Read past eof");
    }

    base->Seek(file_offset + start);
    base->ReadBytes(bytes, offset, len, false);
  }

  void Close() {
    base->Close();
  }

  uint64_t Length() {
    return length;
  }
};

class ByteArrayReferenceDataInput : public DataInput {
 private:
  char* bytes;
  uint32_t pos;
  uint32_t limit;

 public:
  ByteArrayReferenceDataInput(char bytes[], const uint32_t bytes_length) {
    Reset(bytes, 0, bytes_length);
  }

  ByteArrayReferenceDataInput(char bytes[],
                              const uint32_t offset,
                              const uint32_t len) {
    Reset(bytes, offset, len);
  }

  ByteArrayReferenceDataInput() {
    Reset(nullptr, 0, 0);
  }

  void Reset(char new_bytes[],
             const uint32_t offset,
             const uint32_t len) noexcept {
    bytes = new_bytes;
    pos = offset;
    limit = offset + len;
  }

  void Rewind() noexcept {
    pos = 0;
  }

  uint32_t GetPosition() const noexcept {
    return pos;
  }

  void SetPosition(const uint32_t new_pos) noexcept {
    pos = new_pos;
  }

  uint64_t Length() const noexcept {
    return limit;
  }

  bool Eof() const noexcept {
    return (pos >= limit);
  }

  void SkipBytes(const uint64_t count) {
    pos += count;
  }

  int16_t ReadInt16() {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
#endif

    return iab.int16;
  }

  int32_t ReadInt32() {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[3] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[3] = bytes[pos++];
#endif

    return iab.int32;
  }

  int64_t ReadInt64() {
    lucene::core::util::numeric::Int64AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[7] = bytes[pos++];
    iab.bytes[6] = bytes[pos++];
    iab.bytes[5] = bytes[pos++];
    iab.bytes[4] = bytes[pos++];
    iab.bytes[3] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[3] = bytes[pos++];
    iab.bytes[4] = bytes[pos++];
    iab.bytes[5] = bytes[pos++];
    iab.bytes[6] = bytes[pos++];
    iab.bytes[7] = bytes[pos++];
#endif

    return iab.int64;
  }

  int32_t ReadVInt32() {
    char b = bytes[pos++];
    if (b >= 0) return b;
    int32_t i = b & 0x7F;
    b = bytes[pos++];
    i |= (b & 0x7F) << 7;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7F) << 14;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7F) << 21;
    if (b >= 0) return i;
    b = bytes[pos++];
    // Warning: the next ands use 0x0F / 0xF0 - beware copy/paste errors:
    i |= (b & 0x0F) << 28;
    if ((b & 0xF0) == 0) return i;
    throw lucene::core::util::InvalidStateException(
          "Invalid vInt detected (too many bits)");
  }

  int64_t ReadVInt64() {
    char b = bytes[pos++];
    if (b >= 0) return b;
    int64_t i = b & 0x7FL;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 7;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 14;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 21;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 28;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 35;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 42;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 49;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 56;
    if (b >= 0) return i;
    throw lucene::core::util::InvalidStateException(
          "Invalid vLong detected (negative values disallowed)");
  }

  char ReadByte() {
    return bytes[pos++];
  }

  void ReadBytes(char b[],
                 const uint32_t offset,
                 const uint32_t len) {
    std::memcpy(b + offset, bytes + pos, len);
    pos += len;
  }
};

class BytesArrayReferenceIndexInput : public IndexInput {
 private:
  char* bytes;
  uint32_t pos;
  uint32_t limit;

 public:
  BytesArrayReferenceIndexInput(const std::string& desc,
                                char bytes[],
                                const uint32_t bytes_len)
    : IndexInput(desc),
      bytes(bytes),
      pos(0),
      limit(bytes_len) {
  }

  uint64_t GetFilePointer() {
    return pos;
  }

  void Seek(const uint64_t new_pos) {
    pos = static_cast<int32_t>(new_pos);
  }

  void Reset(char new_bytes[],
             const uint32_t offset,
             const uint32_t len) noexcept {
    bytes = new_bytes;
    pos = offset;
    limit = offset + len;
  }

  uint64_t Length() {
    return limit;
  }

  bool Eof() const noexcept {
    return (pos == limit);
  }

  void SkipBytes(const uint64_t count) noexcept {
    pos += count;
  }

  int16_t ReadInt16() {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
#endif

    return iab.int16;
  }

  int32_t ReadInt32() {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[3] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[3] = bytes[pos++];
#endif

    return iab.int32;
  }

  int64_t ReadInt64() {
    lucene::core::util::numeric::Int64AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[7] = bytes[pos++];
    iab.bytes[6] = bytes[pos++];
    iab.bytes[5] = bytes[pos++];
    iab.bytes[4] = bytes[pos++];
    iab.bytes[3] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
    iab.bytes[2] = bytes[pos++];
    iab.bytes[3] = bytes[pos++];
    iab.bytes[4] = bytes[pos++];
    iab.bytes[5] = bytes[pos++];
    iab.bytes[6] = bytes[pos++];
    iab.bytes[7] = bytes[pos++];
#endif

    return iab.int64;
  }

  int32_t ReadVInt32() {
    char b = bytes[pos++];
    if (b >= 0) return b;
    int32_t i = b & 0x7F;
    b = bytes[pos++];
    i |= (b & 0x7F) << 7;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7F) << 14;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7F) << 21;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x0F) << 28;
    if ((b & 0xF0) == 0) return i;
    throw lucene::core::util::InvalidStateException(
          "Invalid vInt detected (too many bits)");
  }

  int64_t ReadVInt64() {
    char b = bytes[pos++];
    if (b >= 0) return b;
    int64_t i = b & 0x7FL;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 7;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 14;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 21;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 28;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 35;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 42;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 49;
    if (b >= 0) return i;
    b = bytes[pos++];
    i |= (b & 0x7FL) << 56;
    if (b >= 0) return i;
    throw new lucene::core::util::InvalidStateException(
              "Invalid vLong detected (negative values disallowed)");
  }

  char ReadByte() {
    return bytes[pos++];
  }

  void ReadBytes(char b[],
                 const uint32_t offset,
                 const uint32_t len) {
    std::memcpy(b + offset, bytes + pos, len);
    pos += len;
  }

  void Close() { }

  std::unique_ptr<IndexInput>
  Slice(const std::string& slice_desc,
        const uint64_t offset,
        const uint64_t length) {
    throw lucene::core::util::UnsupportedOperationException();
  }
};


class ByteBufferIndexInput: public IndexInput, public RandomAccessInput {
 protected:
  const uint64_t length;
  uint64_t idx;
  const char* base;
  bool* isClosed;
  bool isClone;

 private:
  void EnsureValid() const {
    if (*isClosed) {
      throw AlreadyClosedException();
    }
  }

 public:
  ByteBufferIndexInput(const std::string& resource_desc,
                       const char* base,
                       const uint64_t length,
                       const bool isClone = false)
    : IndexInput(resource_desc),
      length(length),
      idx(0),
      base(base),
      isClosed(!isClone ? new bool(false) : nullptr),
      isClone(isClone) {
  }

  ByteBufferIndexInput(const ByteBufferIndexInput& other)
    : IndexInput(other),
      length(other.length),
      idx(0),
      base(other.base),
      isClosed(other.isClosed),
      isClone(true) {
  }

  ByteBufferIndexInput(ByteBufferIndexInput&& other)
    : IndexInput(other),
      length(other.length),
      idx(other.idx),
      base(other.base),
      isClosed(other.isClosed),
      isClone(other.isClone) {
    // Prevent double memory deallocation
    other.isClone = true;
  }

  ~ByteBufferIndexInput() {
    try {
      Close();
    } catch(...) {
      // Ignore
    }

    if (!isClone) {
      delete isClosed;
    }
  }

  char ReadByte() {
    return base[idx++];
  }

  void ReadBytes(char b[], const uint32_t offset, const uint32_t len) {
    std::memcpy(b + offset, base + idx, len);
    idx += len;
  }

  uint64_t GetFilePointer() {
    return idx;
  }

  void Seek(const uint64_t pos) {
    idx = pos;
  }

  char ReadByte(const uint64_t pos) {
    return base[pos];
  }

  int16_t ReadInt16(const uint64_t pos) {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[1] = base[pos];
    iab.bytes[0] = base[pos + 1];
#else
    iab.bytes[0] = base[pos];
    iab.bytes[1] = base[pos + 1];
#endif

    return iab.int16;
  }

  int32_t ReadInt32(const uint64_t pos) {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[3] = base[pos];
    iab.bytes[2] = base[pos + 1];
    iab.bytes[1] = base[pos + 2];
    iab.bytes[0] = base[pos + 3];
#else
    iab.bytes[0] = base[pos];
    iab.bytes[1] = base[pos + 1];
    iab.bytes[2] = base[pos + 2];
    iab.bytes[3] = base[pos + 3];
#endif

    return iab.int32;
  }

  int64_t ReadInt64(const uint64_t pos) {
    return (static_cast<int64_t>(ReadInt32(pos)) << 32 |
            ReadInt32(pos + 4) & 0xFFFFFFFFL);
  }

  uint64_t Length() {
    return length;
  }

  std::unique_ptr<IndexInput>
  Slice(const std::string& slice_description,
        const uint64_t offset,
        const uint64_t length) {
    return std::make_unique<ByteBufferIndexInput>(resource_desc,
                                                  base + offset,
                                                  length,
                                                  true);
  }

  void Close() {
    if (!isClone && *isClosed == false) {
      const int result = munmap(const_cast<char*>(base), length);
      *isClosed = true;
      // What happens if clear-cache-feature is not supported?
      // Imagine this scenario. When destruct this instance
      // isClosed variable was setted to false and be released by delete.
      // And there are still remaining read threads out there try to read data
      // Then is it possible for them to meet segment fault?
      __builtin___clear_cache(reinterpret_cast<char*>(&isClosed),
                              reinterpret_cast<char*>(&isClosed) + 1);
      std::this_thread::yield();
      if (result != 0) {
        throw lucene::core::util::IOException(std::string("Failed to close ") +
                                              resource_desc);
      }
    }
  }
};

class BufferedFileInputStreamDataInput: public DataInput {
 private:
  static const uint32_t BUFFER_SIZE = 8 * 1024;

  std::string path;
  int32_t fd;
  uint32_t idx;
  uint32_t limit;
  char buffer[BUFFER_SIZE];

 private:
  void FillBufferIf() {
    if (idx >= limit) {
      size_t read_size = read(fd, buffer, BUFFER_SIZE);
      if (read_size == -1) {
        throw lucene::core::util::EOFException(); 
      }

      limit = static_cast<uint32_t>(read_size);
      idx = 0;
    }
  }

  void EnsureNotClosed() const {
    if (fd == -1) {
      throw lucene::core::util::IOException("Already closed()");
    }
  }

 public:
  explicit BufferedFileInputStreamDataInput(const std::string& path)
    : path(path),
      fd(-1),
      idx(0),
      limit(0) {
    fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
      throw lucene::core::util::IOException (
              "Open file failed. Path -> " +
              path + ", Error -> " + std::strerror(errno));
    }
  }
  
  BufferedFileInputStreamDataInput
  (const BufferedFileInputStreamDataInput&) = delete;

  BufferedFileInputStreamDataInput&
  operator=(const BufferedFileInputStreamDataInput&) = delete;

  ~BufferedFileInputStreamDataInput() {
    Close();
  }

  char ReadByte() {
    EnsureNotClosed();
    FillBufferIf();  
    return buffer[idx++];
  }

  void ReadBytes(char bytes[],
                 const uint32_t offset,
                 const uint32_t len) {
    EnsureNotClosed();
    uint32_t left_len = len;
    uint32_t actual_offset = offset;

    // Read from own buffer
    if (idx < limit) {
      const uint32_t read_len = std::min(limit - idx, len);
      std::memcpy(bytes + offset, buffer + idx, read_len);
      left_len -= read_len;
      idx += read_len;
      actual_offset += read_len;
    }

    // Read from file
    while (left_len > 0) {
      const uint32_t cnt = read(fd, bytes + actual_offset, left_len);
      if (cnt < 0) {
        throw lucene::core::util::EOFException();
      }
      left_len -= cnt;
      actual_offset += cnt;
    }
  }

  void Close() {
    if (fd != -1) {
      close(fd);
      fd = -1;
    }
  }
}; // BufferedFileInputStreamDataInput


}  // namespace store
}  // namespace core
}  // namespace lucene

// TODO(0ctopus13prime): Clone in DataInput

#endif  // SRC_STORE_DATAINPUT_H_
