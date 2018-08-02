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

#include <Util/Numeric.h>
#include <Util/Bits.h>
#include <stdexcept>
#include <map>
#include <set>
#include <string>

namespace lucene {
namespace core {
namespace store {

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

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_STORE_DATAINPUT_H_
