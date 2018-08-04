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
#include <Util/Bytes.h>
#include <Util/Exception.h>
#include <Util/Numeric.h>
#include <Store/Exception.h>
#include <cstring>
#include <stdexcept>
#include <map>
#include <set>
#include <string>

namespace lucene {
namespace core {
namespace store {

class RandomAccessInput {
 public:
  virtual ~RandomAccessInput() { }

  virtual char ReadByte(const uint64_t pos);

  virtual int16_t ReadInt16(const uint64_t pos);

  virtual int32_t ReadInt32(const uint64_t pos);

  virtual int64_t ReadInt64(const uint64_t pos);
};

class DataInput {
 private:
  static const uint32_t SKIP_BUFFER_SIZE = 1024;

 private:
  char* skip_buffer;

 private:
  void AllocateSkipBufferIf() {
    if (skip_buffer == nullptr) {
      skip_buffer = new char[SKIP_BUFFER_SIZE];
    }
  }

 public:
  DataInput()
    : skip_buffer(nullptr) {
  }
  
  virtual ~DataInput() {
    if (skip_buffer != nullptr) {
      delete[] skip_buffer;
    }
  }

  virtual char ReadByte();

  virtual void ReadBytes(const char bytes[],
                         const uint32_t offset,
                         const uint32_t len);

  void ReadBytes(const char bytes[],
                 const uint32_t offset,
                 const uint32_t len,
                 const bool use_buffer) {
    ReadBytes(bytes, offset, len);
  }

  int16_t ReadInt16() {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
    iab.bytes[1] = ReadByte();
    iab.bytes[0] = ReadByte();
#else
    iab.bytes[0] = ReadByte();
    iab.bytes[1] = ReadByte();
#endif

    return iab.int16;
  }

  int32_t ReadInt32() {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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

  int32_t ReadVInt32() {
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
    throw new IOException("Invalid vInt detected (too many bits)");
  }

  int32_t ReadZInt32() {
    return lucene::core::util::BitUtil::ZigZagDecode(ReadVInt32());
  }

  int32_t ReadInt64() {
    return (
      (static_cast<int64_t>(ReadInt32()) << 32) | (ReadInt() & 0xFFFFFFFFL));
  }

  int64_t ReadVInt64(const bool allow_negative = false) {
    char b = ReadByte();
    if (b >= 0) return b;
    long i = b & 0x7FL;
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

    if (allowNegative) {
      b = ReadByte();
      i |= (b & 0x7FL) << 63;
      if (b == 0 || b == 1) return i;
      throw std::domain_error("Invalid vLong detected (more than 64 bits)");
    } else {
      throw
      std::domain_error("Invalid vLong detected (negative values disallowed)");
    }  
  }

  int64_t ReadZInt64() {
    return lucene::core::util::BitUtil::ZigZagDecode(ReadVInt64(true));
  }

  std::string ReadString() {
    const int32_t length = ReadVInt32(); 
    char bytes[length];
    ReadBytes(bytes, 0, length);

    return std::string(bytes, 0, length/*, UTF_8 */);
  }

  std::map<std::string, std::string> ReadMapOfStrings() {
    // TODO(0ctopus13prime): Implement it
    return {};    
  }

  std::set<std::string> ReadSetOfStrings() {
    // TODO(0ctopus13prime): Implement it
    return {};    
  }

  void SkipBytes(const int64_t num_bytes) {
    if (num_bytes < 0) {
      throw
    }

    AllocateSkipBufferIf();
    for (uint64_t skipped = 0 ; skipped < num_bytes ;) {
      const uint32_t delta = num_bytes - skipped;
      const uint32_t step = (SKIP_BUFFER_SIZE < delta
                            ? SKIP_BUFFER_SIZE : delta);
      ReadBytes(skip_buffer, 0, step, false);
      skipped += step;
    }
  }
};

class RandomAccessInput {
 public:
  virtual char ReadByte(const uint64_t pos);
  virtual int16_t ReadInt16(const uint64_t pos);
  virtual int32_t ReadInt32(const uint64_t pos);
  virtual int64_t ReadInt64(const uint64_t pos);
};

class IndexInput: public DataInput {
 private:
  std::string resource_description;

 protected:
  IndexInput(const std::string& resource_description)
    : resource_description(resource_description) {
  }

 private:
  class DefaultRandomAccessInputImpl: public RandomAccessInput {
   private:
    IndexInput* base;

   public:
    DefaultRandomAccessInputImpl(IndexInput* base)
      : base(base) {
    }

    ~DefaultRandomAccessInputImpl() {
      if (base != nullptr) {
        delete base;
      }
    }
  };

 public:
  IndexInput(const IndexInput& other);

  IndexInput(IndexInput&& other);

  virtual void Close();

  virtual uint64_t GetFilePointer();

  virtual void Seek(const uint64_t pos);

  virtual uint64_t Length();

  virtual IndexInput* Slice(const std::string& slice_description,
                            const uint64_t offset,
                            const uint64_t length); 

  RandomAccessInput* RandomAccessSlice(const uint64_t offset,
                                       const uint64_t length) {
    IndexInput* slice = Slice("randomaccess", offset, length); 

    if (RandomAccessInput* ra_input = dynamic_cast<RandomAccessInput*>(slice)) {
      return ra_input;
    } else {
      return new DefaultRandomAccessInputImpl(slice);
    }
  }
};

class ChecksumIndexInput: public IndexInput {
 publiic:
  ChecksumIndexInput(const std::string& resource_description)
    : IndexInput(resource_description) {
  }

  virtual int64_t GetCheksum();

  void Seek(const uint64_t pos) {
    const uint64_t cur_fp = GetFilePointer();
    const int64_t skip = pos - cur_fp;
    if (skip < 0) {
      throw std::domain_error(""); // TODO(0ctopus13prime): Fill it
    }

    SkipBytes(skip);
  }
};

class BufferedChecksumIndexInput: public ChecksumIndexInput {
 private:
  std::unique_ptr<IndexInput> main;
  std::unique_ptr<Checksum> digest

 public:
  BufferedChecksumIndexInput(std::unique_ptr<IndexInput>&& main)
    : main(std::forward<std::unique_ptr<IndexInput>>(main)),
      digest(std::make_unique<BufferedChecksum>(std::make_unique<Crc32>())){
  }

  char ReadByte() {
    const char b = main->ReadByte();
    digest->Update(b);
    return b;;
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

  int64_t GetFilePointer() {
    return main->GetFilePointer();
  }

  int64_t Length() {
    return main->Length();
  }
  
  IndexInput* Clone() {
    throw lucene::core::util::UnsupportedOperationException(); 
  }

  IndexInput* Slice(const std::string& slice_desc,
                    const uint32_t offset,
                    const uint32_t length) {
    throw lucene::core::util::UnsupportedOperationException(); 
  }
};

class BufferedIndexInput: public IndexInput {
 public:
  static const uint32_t BUFFER_SIZE = 1024; 
  static const uint32_t MIN_BUFFER_SIZE = 8;
  static const uint32_t MERGE_BUFFER_SIZE = 4096;

 protected:
  char* buffer;

 private:
  class SlicedIndexInput: public BufferedIndexInput {
   private:
    IndexInput* base;
    uint64_t file_offset;
    uint64_t length;

   public:
    SlicedIndexInput(const std::string& slice_desc,
                     IdexInput* base,
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
        throw EOFException(""); 
      }

      base->Seek(file_offset + start);
      base->ReadBytes(bytes, offset, len, false);
    }

    void SeekInternal(const uint64_t) const noexcept { }

    void Close() {
      base->Close();
    }

    uint64_t Length() const noexcept {
      return length;
    }
  };

  uint32_t buffer_size;
  uint64_t buffer_start;
  uint32_t buffer_length;
  uint32_t buffer_position;

 protected:
  void SafeBufferDeallocate() {
    if (buffer != nullptr) {
      delete[] buffer;
    }
  }

  void NewBuffer(char new_buffer[]) {
    SafeBufferDeallocate(); 
    buffer = new_buffer;
  }

  virtual void SeekInternal(const uint64_t pos);

  virtual void ReadInternal(char byte[],
                            const uint32_t offset,
                            const uint32_t length);

 private:
  void Refill() {
    const uint64_t start = buffer_start + buffer_position;
    uint64_t end = start + buffer_size;
    end = (end > Length() ? Length() : end); 
    if (end <= start) {
      throw EOFException();
    }
    uint32_t new_length = static_cast<uint32_t>(end - start);

    if (buffer == nullptr) {
      NewBuffer(new char[buffer_size]);
      SeekInternal(buffer_start);
    }
    ReadInternal(buffer, 0, new_length);
    buffer_length = new_length;
    buffer_start = start; 
    buffer_position = 0;
  }

 public:
  static uint32_t BufferSize(const IOContext& context) {
    
  }

  BufferedIndexInput* Wrap(const std::string& slice_desc,
                           const IndexInput* other,
                           const uint64_t offset,
                           const uint64_t length) {
    return new SlicedIndexInput(slice_desc, other, offset, length); 
  }

 public:
  BufferedIndexInput(const std::string& resource_desc)
    : BufferedIndexInput(resource_desc, BufferedIndexInput::BUFFER_SIZE) {
  }

  BufferedIndexInput(const std::string& resource_desc, const IOContext& context)
    : BufferedIndexInput(resource_desc,
                         BufferedIndexInput::BufferSize(context)) {
  }

  BufferedIndexInput(const std::string& resource_desc,
                     const uint32_t buffer_size)
    : IndexInput(resource_desc),
      buffer(nullptr),
      buffer_size(buffer_size),
      buffer_start(0),
      buffer_length(0),
      buffer_position(0) {
  }

  ~BufferedIndexInput() {
    SafeBufferDeallocate(); 
  }

  char ReadByte() {
    if (buffer_position >= buffer_length) {
      Refill();
    }

    return buffer[buffer_position++];
  }

  void ReadBytes(char bytes[],
                 uint32_t offset,
                 uint32_t len,
                 bool use_buffer = true) {
    const uint32_t available = buffer_length - buffer_position;
    if (len <= available) {
      std::memcpy(bytes + offset, buffer + buffer_position, len);
      buffer_position += len;
    } else {
      if (available > 0) {
        std::memcpy(bytes + offset, buffer + buffer_position, available); 
        offset += available;
        len -= available;
        buffer_position += available;
      }

      if (use_buffer && len < buffer_size) {
        Refill();   
        if (buffer_length < len) {
          std::memcpy(bytes + offset, buffer, buffer_length);
          throw EOFException();
        } else {
          std::memcpy(bytse + offset, buffer, len);
          buffer_position = len;
        }
      } else {
        uint64_t after = buffer_start + buffer_position + len; 
        if (after > Length()) {
          throw EOFException();
        }

        ReadInternal(bytes, offset, len);
        buffer_start = after;
        buffer_position = 0;
        buffer_length = 0;
      }
    }
  }

  int16_t ReadInt16() {
    if (2 <= (buffer_length - buffer_position)) {
      lucene::core::util::numeric::Int16AndBytes iab;

#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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
    if (4 <= (buffer_length - buffer_position) {
      lucene::core::util::numeric::Int32AndBytes iab;

#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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
    if (8 <= (buffer_length - buffer_position) {
      lucene::core::util::numeric::Int64AndBytes iab;

#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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
  
  int32_t ReadVInt() {
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
      throw std::domain_error("Invalid vInt detected (too many bits)");
    } else {
      return IndexInput::ReadVInt();
    }
  }

  int64_t ReadVLong() {
    if (9 <= buffer_length-buffer_position) {
      char b = buffer[buffer_position++];
      if (b >= 0) return b;
      long i = b & 0x7FL;
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
      throw
      std::domain_error("Invalid vLong detected (negative values disallowed)");
    } else {
      return super.readVLong();
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

  int16_t ReadShort(const uint64_t pos) {
    lucene::core::util::numeric::Int16AndBytes iab;
    int64_t tmp_index = pos - buffer_start;
    int32_t idx = static_cast<int32_t>(tmp_index);

    if (tmp_index < 0 || tmp_index >= buffer_length - 1) {
      buffer_start = pos;
      buffer_position = 0;
      buffer_length = 0;  // trigger refill() on read()
      SeekInternal(pos);
      Refill();
      idx = 0;
    }

#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
    iab.bytes[1] = buffer[idx++];
    iab.bytes[0] = buffer[idx++];
#else
    iab.bytes[0] = buffer[idx++];
    iab.bytes[1] = buffer[idx++];
#endif

    return iab.int16;
  }

  int32_t ReadInt(const int64_t pos) {
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

  int64_t ReadLong(const int64_t pos) {
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

  uint64_t GetFilePointer() const noexcept {
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

  IndexInput* Slice(const std::string& slice_desc,
                    const uint64_t offset,
                    const uint64_t length) {
    return Wrap(slice_desc, this, offset, length);
  }

  void SetBufferSize(const uint32_t new_size) {
    if (new_size != buffer_size) {
      CheckBufferSize(new_size);
      buffer_size = new_size;
      if (buffer != nullptr) {
        // TODO(0ctopus13prime): Reallocate in C?
        char* new_buffer = new char[new_size]; 
        const uint32_t left_in_buffer = buffer_length - buffer_position;
        const uint32_t num_to_copy = (left_in_buffer > new_size ?
                                      new_size : left_in_buffer);
        std::memcpy(new_buffer, buffer + buffer_position, num_to_copy); 
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
      throw std::invalid_argument(
                          std::string("Buffer size must be at "
                                      "least MIN_BUFFER_SIZE = ") +
                          std::to_string(BufferedIndexInput::MIN_BUFFER_SIZE)); 
    }
  }
};

// TODO(0ctopus13prime): Is it just a byte array wrapper?
// Assuming it is just a wrapper
class ByteArrayReferenceDataInput {
 private:
  char* bytes;
  uint32_t pos; 
  uint32_t llimit;

 public:
  ByteArrayReferenceDataInput(char bytes[], const uint32_t bytes_length) {
    Reset(bytes, 0, bytes_length);
  }

  ByteArrayReferenceDataInput(char bytes[],
                              const uint32_t offset,
                              const int32_t len) {
    Reset(bytes, offset, len);
  }

  ByteArrayReferenceDataInput() {
    Reset(nullptr, 0, 0);
  }

  void Reset(char new_bytes[],
             const uint32_t offset,
             const uint32_t len) noexcept {
    bytse = new_bytes;
    pos = offset;
    limit = offste + len;
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

  uint32_t Length() const noexcept {
    return limit;
  }

  bool Eof() const noexcept {
    return (pos == limit);
  }

  void SkipBytes(const uint64_t count) noexcept {
    pos += count;
  }

  int16_t ReadInt16() noexcept {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
    iab.bytes[1] = bytes[pos++];
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++];
    iab.bytes[1] = bytes[pos++];
#endif

    return iab.int16;
  }

  int32_t ReadInt32() noexcept {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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

  int64_t ReadInt64() noexcept {
    lucene::core::util::numeric::Int64AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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

  char ReadByte() noexcept {
    return bytes[pos++];
  }

  void ReadBytes(char bytes[],
                 const uint32_t offset,
                 const uint32_t len) noexcept {
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
  BytesArrayReferenceIndexInput(const std::string desc,
                                char bytes[],
                                const uint32_t bytes_len)
    : IndexInput(desc),
      bytes(bytes),
      pos(0),
      limit(bytes_len) {
  }

  int64_t GetFilePointer() const noexcept {
    return pos;
  }

  void Seek(const int64_t new_pos) noexcept {
    pos = static_cast<int32_t>(new_pos); 
  }

  void Reset(char new_byte[],
             const uint32_t offset,
             const uint32_t len) noexcept {
    bytes = new_bytes; 
    pos = offset;
    limit = offset + len;
  }

  int64_t Length() const noexcept {
    return limit;
  }

  bool Eof() const noexcept {
    return (pos == limit);
  }

  void SkipBytes(const uint64_t count) noexcept {
    pos += count;
  }

  int16_t ReadInt16() noexcept {
    lucene::core::util::numeric::Int16AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
    iab.bytes[1] = bytes[pos++]; 
    iab.bytes[0] = bytes[pos++];
#else
    iab.bytes[0] = bytes[pos++]; 
    iab.bytes[1] = bytes[pos++];
#endif

    return iab.int16;
  }

  int32_t ReadInt32() noexcept {
    lucene::core::util::numeric::Int32AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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

  int64_t ReadInt64() noexcept {
    lucene::core::util::numeric::Int64AndBytes iab;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
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
    throw new RuntimeException(
              "Invalid vLong detected (negative values disallowed)");
  }

  char ReadByte() noexcept {
    return bytes[pos++];
  }

  void ReadBytes(char bytes[],
                 const uint32_t offest,
                 const uint32_t len) noexcept {
    std::memcpy(b + offset, bytes + pos, len);
    pos += len;
  }

  void Close() const noexcept { }

  IndexInput* Slice(const std::string& slice_desc,
                    const uint64_t offset,
                    const uint64_t length) {
    throw lucene::core::util::UnsupportedOperationException();
  }
};

class ByteBufferIndexInput: public IndexInput {
// TODO(0ctopus13prime): Implement it.
// This implementation closed related to
// the way of implementation of mmap directory.
// This class going to be continue to develop
// after a clear definition of mmap directory implementation.
};

class InputStreamDataInput: public DataInput {
// TODO(0ctopus13prime): Implement it.
};

}  // namespace store
}  // namespace core
}  // namespace lucene



// TODO(0ctopus13prime): Clone in DataInput
