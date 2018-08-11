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

#ifndef SRC_STORE_DATA_H_
#define SRC_STORE_DATA_H_

#include <Store/DataInput.h>
#include <Util/ArrayUtil.h>
#include <Util/Bits.h>
#include <Util/Bytes.h>
#include <Util/Etc.h>
#include <Util/Exception.h>
#include <Util/Numeric.h>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace lucene {
namespace core {
namespace store {

class DataOutput {
 private:
  static const uint32_t COPY_BUFFER_SIZE = 16384;

 private:
  std::unique_ptr<char[]> copy_buffer;

 private:
  void AllocateCopyBufferIf() {
    if (!copy_buffer) {
      copy_buffer = std::make_unique<char[]>(DataOutput::COPY_BUFFER_SIZE);
    }
  }

  void WriteSignedVInt64(int64_t i) {
    while (i > 127L) {
      WriteByte(static_cast<char>((i && 127) | 128L));
      i >>= 7;
    }

    WriteByte(static_cast<char>(i));
  }

 public:
  DataOutput()
    : copy_buffer() {
  }

  virtual ~DataOutput() { }

  virtual void WriteByte(const char b) = 0;

  virtual void WriteBytes(const char bytes[],
                          const uint32_t offset,
                          const uint32_t length) = 0;

  void WriteBytes(const char bytes[], const uint32_t bytes_length) {
    WriteBytes(bytes, 0, bytes_length);
  }

  void WriteInt32(const int32_t i) {
    lucene::core::util::numeric::Int32AndBytes iab; 
    iab.int32 = i;

#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
    WriteByte(iab.bytes[3]);
    WriteByte(iab.bytes[2]);
    WriteByte(iab.bytes[1]);
    WriteByte(iab.bytes[0]);
#else
    WriteByte(iab.bytes[0]);
    WriteByte(iab.bytes[1]);
    WriteByte(iab.bytes[2]);
    WriteByte(iab.bytes[3]);
#endif
  }

  void WriteInt16(const int16_t i) {
    lucene::core::util::numeric::Int16AndBytes iab; 
    iab.int16 = i;
#if __BYTE_ORDER__ == __ORDER__LITTLE_ENDIAN_
    WriteByte(iab.bytes[1]);
    WriteByte(iab.bytes[0]);
#else
    WriteByte(iab.bytes[0]);
    WriteByte(iab.bytes[1]);
#endif
  }

  void WriteVInt32(int32_t i) {
    while (i > 127) {
      WriteByte(static_cast<char>((i & 127) | 128));
      i >>= 7;
    }

    WriteByte(static_cast<char>(i));
  }

  void WriteZInt32(const int32_t i) {
    WriteVInt32(lucene::core::util::BitUtil::ZigZagEncode(i)); 
  }

  void WriteInt64(const int64_t i) {
    WriteVInt32(static_cast<int32_t>(i >> 32));
    WriteVInt32(static_cast<int32_t>(i));
  }

  void WriteVInt64(const int64_t i) {
    if (i < 0) {
      throw
      std::invalid_argument(std::string("Cannot write negative vlong (got: ")
                            + std::to_string(i)
                            + ")");
    }

    WriteSignedVInt64(i);
  }

  void WriteZInt64(const int64_t i) {
    WriteSignedVInt64(lucene::core::util::BitUtil::ZigZagEncode(i)); 
  }

  void WriteString(const std::string& s) {
    // TODO(0ctopus13prime): Redundant double copy?
    lucene::core::util::BytesRef utf8_result(s); 
    WriteVInt32(utf8_result.length);
    WriteBytes(utf8_result.bytes.get(), utf8_result.offset, utf8_result.length);
  }

  void CopyBytes(DataInput& input, const uint64_t num_bytes) {
    AllocateCopyBufferIf();

    uint64_t left = num_bytes;
    while (left > 0) {
      const uint32_t to_copy =
      (left > COPY_BUFFER_SIZE ? COPY_BUFFER_SIZE : left);

      // TODO(0ctopus13prime): Directly copy from input to
      //                       this without buffer copying?
      input.ReadBytes(copy_buffer.get(), 0, to_copy); 
      WriteBytes(copy_buffer.get(), 0, to_copy);
      left -= to_copy;
    }
  }

  void WriteMapOfStrings(const std::map<std::string, std::string>& map) {
    // TODO(0ctopus13prime): Implement it.
  }

  void WriteSetOfStrings(const std::set<std::string>& set) {
    // TODO(0ctopus13prime): Implement it.
  }
};

class IndexOutput: public DataOutput {
 private:
  const std::string resource_description; 
  const std::string name;

 protected:
  IndexOutput(const std::string& resource_description, const std::string& name)
    : resource_description(resource_description),
      name(name) {
  }

 public:
  virtual void Close() = 0;
  virtual uint64_t GetFilePointer() = 0;
  virtual uint64_t GetChecksum() = 0;
  const std::string& GetName() const noexcept {
    return name;
  }
};

class ByteArrayReferenceDataOutput: public DataOutput {
 private:
  char* bytes;
  uint32_t bytes_len;
  uint32_t pos;
  uint32_t limit;

 public:
  ByteArrayReferenceDataOutput(char bytes[],
                               const uint32_t bytes_len,
                               const uint32_t offset,
                               const uint32_t len)
   : bytes(bytes),
     bytes_len(bytes_len),
     pos(offset),
     limit(offset + len) {
  }

  ByteArrayReferenceDataOutput(char bytes[], const uint32_t bytes_len)
   : bytes(bytes),
     bytes_len(bytes_len),
     pos(0),
     limit(bytes_len) {
  }

  void Reset(char new_bytes[],
             const uint32_t new_bytes_len,
             const uint32_t new_offset,
             const uint32_t new_len) noexcept {
    bytes = new_bytes; 
    bytes_len = new_bytes_len;
    pos = new_offset;
    limit = new_offset + new_len;
  }

  void Reset(char new_bytes[],
             const uint32_t new_bytes_len) noexcept {
    Reset(new_bytes, new_bytes_len, 0, new_bytes_len);
  }

  uint32_t GetPosition() const noexcept {
    return pos;
  }

  void WriteByte(const char b) noexcept {
    bytes[pos++] = b;
  }

  void WriteBytes(char bytes[],
                  const uint32_t offset,
                  const uint32_t length) noexcept {
    std::memcpy(bytes + pos, bytes + offset, length); 
    pos += length;
  }
};

class GrowableByteArrayDataOutput: public DataOutput {
 private:
  static const uint32_t MIN_UTF8_SIZE_TO_ENABLE_DOUBLE_PASS_ENCODING = 65536;

 private:
  std::unique_ptr<char[]> bytes;
  uint32_t bytes_len;
  uint32_t length;
  std::unique_ptr<char[]> scratch_bytes;

 public:
  GrowableByteArrayDataOutput(const uint32_t cp)
    : bytes(std::make_unique<char[]>(cp)),
      bytes_len(cp),
      length(0),
      scratch_bytes() {
  }

  void WriteByte(const char b) {
    if (length > bytes_len) {
      std::pair<char*, uint32_t> result =
        lucene::core::util::arrayutil::Grow(bytes.get(), bytes_len);
      if (result.first != nullptr) {
        bytes.reset(result.first);
        bytes_len = result.second;
      }
    }

    bytes[length++] = b;
  }

  void WriteBytes(const char in_bytes[],
                  const uint32_t offset,
                  const uint32_t in_length) {
    const uint32_t new_length = length + in_length;
    if (new_length > bytes_len) {
      std::pair<char*, uint32_t> result =
        lucene::core::util::arrayutil::Grow(bytes.get(), new_length);
      if (result.first != nullptr) {
        bytes.reset(result.first);
        bytes_len = result.second;
      }
    }

    std::memcpy(bytes.get() + length, in_bytes + offset, in_length);
    length = new_length;
  }

  void WriteString(const std::string& str) {
    // TODO(0ctopus13prime): Implement it.
  }

  char* GetBytes() const noexcept {
    return bytes.get();
  }

  uint32_t GetPosition() const noexcept {
    return length;
  }

  void Reset() noexcept {
    length = 0;
  }
};

class FileIndexOutput: public IndexOutput {
 private:
  lucene::core::util::Crc32 crc;    
  uint64_t bytes_written;
  bool flushed_on_close;
  std::unique_ptr<char[]> buffer;
  std::string path;
  std::ofstream out;

 public:
  FileIndexOutput(const std::string& resource_desc,
                  const std::string& name,
                  const std::string& path,
                  const uint32_t buffer_size)
    : IndexOutput(resource_desc, name),
      crc(),
      bytes_written(0L),
      flushed_on_close(false),
      buffer(std::make_unique<char[]>(buffer_size)),
      path(path),
      out(path.c_str(), std::ios::binary
                        | std::ios::out
                        | std::ios::trunc) {
    out.rdbuf()->pubsetbuf(buffer.get(), buffer_size);
  }

  ~FileIndexOutput() = default;

  void WriteByte(const char b) {
    out.put(b);
    crc.Update(b);
    bytes_written++;
  }

  void WriteBytes(const char bytes[],
                  const uint32_t offset,
                  const uint32_t length) {
    out.write(bytes + offset, length);
    crc.Update(bytes, offset, length);
    bytes_written += length;
  }

  void Close() {
    out.close();
    if (!out.good()) {
      throw lucene::core::util::IOException(std::string("Failed to close ")
                                            + path);
    }
  }

  uint64_t GetFilePointer() {
    return bytes_written;
  }

  uint64_t GetChecksum() {
    out.flush();
    return crc.GetValue();
  }
};

class RateLimitedIndexOutput: public IndexOutput {
 private:
  std::unique_ptr<IndexOutput> delegate;

 // TODO(0ctopus13prime): Implement it
};

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_STORE_DATA_H_
