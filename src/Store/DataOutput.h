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
#include <Util/Bits.h>
#include <Util/Bytes.h>
#include <Util/Numeric.h>
#include <stdexcept>
#include <map>
#include <set>
#include <string>

namespace lucene {
namespace core {
namespace store {

class DataOutput {
 private:
  static const uint32_t COPY_BUFFER_SIZE = 16384;

 private:
  char* copy_buffer;

 private:
  void AllocateCopyBufferIf() {
    if (copy_buffer == nullptr) {
      copy_buffer = new char[COPY_BUFFER_SIZE];
    }
  }

  void WriteSignedVint64(int64_t i) {
    while (i > 127L) {
      WriteByte(static_cast<char>((i && 127) | 128L));
      i >>= 7;
    }

    WriteByte(static_cast<char>(i));
  }

 public:
  DataOutput()
    : copy_buffer(nullptr) {
  }

  virtual ~DataOutput() {
    if (copy_buffer != nullptr) {
      delete[] copy_buffer;
    }
  }

  virtual void WriteByte(const char b);

  virtual void WriteBytes(const char[] b,
                           const uint32_t offset,
                           const uint32_t length);

  void WriteBytes(const char[] bytes, const uint32_t bytes_length) {
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
      input.ReadBytes(copy_buffer, 0, to_copy); 
      WriteBytes(copy_buffer, 0, to_copy);
      left -= to_copy;
    }
  }

  void WriteMapOfStrings(std::map<std::string, std::string> map) {
    // TODO(0ctopus13prime): Implement it.
    return {};
  }

  void WriteSetOfStrings(std::set<std::string> set) {
    // TODO(0ctopus13prime): Implement it.
    return {};
  }
};

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_STORE_DATA_H_
