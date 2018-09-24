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

#ifndef SRC_UTIL_ETC_H_
#define SRC_UTIL_ETC_H_

#include <Util/ArrayUtil.h>
#include <Util/Exception.h>
#include <Util/Unicode.h>
#include <Util/ZlibCrc32.h>
#include <cassert>
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace util {

class Version {
 public:
  static Version LATEST;

 private:
  uint8_t major;
  uint8_t minor;
  uint8_t bugfix;
  uint8_t prerelease;
  uint32_t encoded_value;

 private:
  Version(const uint8_t major, const uint8_t minor, const uint8_t bugfix);
  Version(const uint8_t major,
          const uint8_t minor,
          const uint8_t bugfix,
          const uint8_t prerelease);
  bool EncodedIsValid() const;

 public:
  Version(const Version& other);
  Version(Version&& other);
  bool OnOrAfter(const Version& other) const;
  bool OnOrAfter(Version&& other) const;
  std::string ToString() const;
  Version& operator=(const Version& other);
  Version& operator=(Version&& other);
  bool operator==(const Version& other) const;
  bool operator==(Version&& other) const;
  uint8_t GetMajor() const {
    return major;
  }
  uint8_t GetMinor() const {
    return minor;
  }
  uint8_t GetBugfix() const {
    return bugfix;
  }
  uint8_t GetPreRelease() const {
    return prerelease;
  }

 public:
  static Version Parse(const std::string& version);
  static Version Parse(std::string&& version);
  static Version ParseLeniently(const std::string& version);
  static Version ParseLeniently(std::string&& version);
  static Version FromBits(const uint8_t major,
                          const uint8_t minor,
                          const uint8_t bugfix);
};

class Checksum {
 public:
  Checksum() { }
  virtual ~Checksum() { }
  virtual void Update(const char b) = 0;
  virtual void Update(const char bytes[],
                      const uint32_t off,
                      const uint32_t len) = 0;
  virtual int64_t GetValue() = 0;
  virtual void Reset() = 0;
};

class Crc32: public Checksum {
 private:
  int32_t crc;

 public:
  Crc32()
    : Checksum(),
      crc(0) {
  }

  ~Crc32() { }

  void Update(const char b) {
    Update(&b, 0, 1);
  }

  void Update(const char bytes[], const uint32_t off, const uint32_t len) {
    crc = _l_crc32(crc,
                  reinterpret_cast<const unsigned char*>(bytes + off),
                  len);
  }

  int64_t GetValue() {
    return (static_cast<int64_t>(crc)) & 0xFFFFFFFFL;
  }

  void Reset() {
    crc = 0;
  }
};

// TODO(0ctopus13prime): Remove `is_reference`, using MSB in length instead.
class IntsRef {
 public:
  int32_t* ints; 
  uint32_t offset;
  uint32_t capacity;
  uint32_t length;
  bool is_reference;

 public:
  IntsRef()
    : ints(nullptr),
      offset(0),
      capacity(0),
      length(0),
      is_reference(true) {
  }

  explicit IntsRef(const uint32_t capacity)
    : ints(new int[capacity]),
      offset(0),
      capacity(capacity),
      length(0),
      is_reference(false) {
  }
  
  IntsRef(int32_t ints[], const uint32_t capacity, const uint32_t offset, const uint32_t length)
    : ints(ints),
      capacity(capacity),
      offset(offset),
      length(length),
      is_reference(true) {
  }

  IntsRef(const IntsRef& other)
    : ints(other.ints),
      capacity(other.capacity),
      offset(other.offset),
      length(other.length),
      is_reference(other.is_reference) {
    if (!is_reference) {
      ints = lucene::core::util::arrayutil::CopyOf<int>(ints, capacity);
    }
  }

  IntsRef(IntsRef&& other)
    : ints(other.ints),
      offset(other.offset),
      length(other.length),
      is_reference(other.is_reference) {
    // Prevent double memory release
    other.is_reference = true;
  }

  IntsRef& operator=(const IntsRef& other) {
    if (this != &other) {
      ints = other.ints;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;
      is_reference = other.is_reference;

      if (!is_reference) {
        ints = lucene::core::util::arrayutil::CopyOf<int>(ints, capacity);
      }
    }
  }

  IntsRef& operator=(IntsRef&& other) {
    if (this != &other) {
      ints = other.ints;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;
      is_reference = other.is_reference;

      // Prevent double memory release
      other.is_reference = true;
    }
  }

  bool operator<(const IntsRef& other) const {
    if (this != &other) {
      const uint32_t len = std::min(length, other.length);
      int32_t* my_base = (ints + offset);
      int32_t* other_base = (other.ints + other.offset);
      for (uint32_t i = 0 ; i < len ; ++i) {
        if (my_base[i] > other_base[i]) {
          return false;
        }
      }

      return (length < other.length);
    }

    return false;
  }

  bool operator==(const IntsRef& other) const {
    if (this != &other) {
      if (length != other.length) {
        return false;    
      } else {
        int32_t* my_base = (ints + offset);
        int32_t* other_base = (other.ints + other.offset);
        uint32_t idx = 0;  
        while (idx < length && (my_base[idx] == other_base[idx])) {
          ++idx;
        }

        return (idx == length);
      }
    }

    return true;
  }

  bool operator!=(const IntsRef& other) const {
    return !(operator==(other));
  }

  ~IntsRef() {
    if (!is_reference && ints != nullptr) {
      delete[] ints; 
    }
  }
};  // IntsRef

class IntsRefBuilder {
 private:
  IntsRef ref;

 public:
  IntsRefBuilder()
    : ref() {
  }

  int* Ints() {
    return ref.ints;
  }

  uint32_t Length() {
    return ref.length;
  }

  void SetLength(const uint32_t length) {
    ref.length = length;
  }

  void Clear() {
    SetLength(0);
  }

  int32_t IntAt(const uint32_t offset) {
    return ref.ints[offset];
  }

  void SetIntAt(const uint32_t offset, const int32_t b) {
    ref.ints[offset] = b;
  }

  void Append(const int32_t i) {
    Grow(ref.length + 1);
    ref.ints[ref.length++] = i;
  }

  void Grow(const uint32_t new_length) {
    std::pair<int*, uint32_t> pair =
    lucene::core::util::arrayutil::Grow(ref.ints, ref.capacity, new_length);

    if (pair.first != ref.ints) {
      delete[] ref.ints;
      ref.ints = pair.first;
      ref.capacity = pair.second;
    }
  }

  void CopyInts(const int32_t other_ints[],
                const uint32_t other_offset,
                const uint32_t other_length) {
    Grow(other_length);
    std::memcpy(ref.ints, other_ints + other_offset, other_length);
    ref.length = other_length;
  }

  void CopyInts(IntsRef& ints) {
    CopyInts(ints.ints, ints.offset, ints.length);
  }

  void CopyUTF8Bytes(BytesRef& bytes) {
    Grow(bytes.length);
    ref.length = UnicodeUtil::UTF8toUTF32(bytes, ref.ints);
  }

  IntsRef& Get() {
    return ref;
  }
};

class LongsRef {
 private:
  static const uint32_t IS_OWNING_MASK = (1U << 31);
  static const uint32_t LENGTH_MASK = ((1U << 31) - 1);

 private:
  int64_t* longs;
  uint32_t capacity;
  uint32_t offset;
  uint32_t length;

 private:
  void ClearIf() {
    if (length & IS_OWNING_MASK) {
      delete[] longs;
    }
  }

 public:
  LongsRef()
    : longs(nullptr),
      capacity(0),
      offset(0),
      length(0) {
  }

  LongsRef(const uint32_t capacity)
    : capacity(capacity),
      longs(new int64_t[capacity]),
      offset(0),
      length(IS_OWNING_MASK) {
  }

  LongsRef(int64_t* longs,
           const uint32_t capacity,
           const uint32_t offset,
           const uint32_t length)
    : longs(longs),
      capacity(capacity),
      offset(offset),
      length(length) {
    assert(IsValid());
  }

  LongsRef(const LongsRef& other) {
    operator=(other);
  }

  LongsRef(LongsRef&& other) {
    operator=(std::forward<LongsRef>(other));
  }

  LongsRef& operator=(const LongsRef& other) {
    if (other.length & IS_OWNING_MASK) {
      longs =
      lucene::core::util::arrayutil::CopyOfRange(other.longs,
                                                 other.offset,
                                                 other.offset + other.length);
      capacity = other.length;
      offset = 0;
      length = other.length;
    } else {
      longs = other.longs;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;
    }
  }

  LongsRef& operator=(LongsRef&& other) {
    longs = other.longs;
    capacity = other.capacity;
    offset = other.offset;
    length = other.length;

    other.length &= LENGTH_MASK;
  }

  ~LongsRef() {
    ClearIf();
  }

  uint32_t Offset() const noexcept {
    return offset; 
  }

  LongsRef& DecOffset() noexcept {
    --offset; 
    return *this; 
  }

  LongsRef& DecOffset(const uint32_t delta) noexcept {
    offset -= delta; 
    return *this; 
  }

  LongsRef& IncOffset() noexcept {
    offset++;
    return *this; 
  }

  LongsRef& IncOffset(const uint32_t delta) noexcept {
    offset += delta;
    return *this; 
  }

  LongsRef& Offset(const uint32_t new_offset) noexcept {
    offset = new_offset;
    return *this; 
  }

  uint32_t Length() const noexcept {
    return (length & LENGTH_MASK); 
  }

  LongsRef& DecLength() noexcept {
    --length;
    return *this;
  }

  LongsRef& DecLength(const uint32_t delta) noexcept {
    length -= delta;
    return *this;
  }

  LongsRef& IncLength() noexcept {
    ++length;
    return *this;
  }

  LongsRef& IncLength(const uint32_t delta) noexcept {
    length += delta;
    return *this;
  }

  LongsRef& Length(const uint32_t new_length) noexcept {
    length = (length & IS_OWNING_MASK) | new_length;
    return *this;
  }

  uint32_t Capacity() const noexcept {
    return capacity;
  }

  int64_t* Longs() const noexcept {
    return longs;
  }

  LongsRef& Reference(int64_t* new_longs,
                      const uint32_t new_capacity) noexcept {
    ClearIf();
    longs = new_longs;
    capacity = new_capacity;
    length &= LENGTH_MASK;

    return *this;
  }

  LongsRef& Own(int64_t* new_longs,
                const uint32_t new_capacity) noexcept {
    ClearIf();
    longs = new_longs;
    capacity = new_capacity;
    length |= IS_OWNING_MASK;

    return *this;
  }

  bool IsValid() {
    if (longs == nullptr) {
      throw IllegalStateException("`longs` is null"); 
    }

    if (length > capacity) {
      throw IllegalStateException("Length is out of bounds " +
                                  std::to_string(length) +
                                  ",capacity=" +
                                  std::to_string(capacity));
    }

    if (offset > capacity) {
      throw IllegalStateException("Length is out of bounds " +
                                  std::to_string(offset) +
                                  ",capacity=" +
                                  std::to_string(capacity));
    }

    if (offset + length > capacity) {
      throw IllegalStateException("offset+length out of bounds: offset=" +
                                  std::to_string(offset) +
                                  ",length=" +
                                  std::to_string(length) +
                                  ",capacity=" +
                                  std::to_string(capacity));
    }
  }
};

class Int64Values {
 public:
  virtual ~Int64Values() = default;

  virtual int64_t Get(const uint64_t index) = 0; 
};

class IdentityInt64Values : public Int64Values {
 public:
  int64_t Get(const uint64_t index) {
    return index;
  }
};

class ZeroInt64Values : public Int64Values {
  int64_t Get(const uint64_t index) {
    return 0;
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_ETC_H_
