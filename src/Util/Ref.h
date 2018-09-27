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

#ifndef SRC_UTIL_REF_H_
#define SRC_UTIL_REF_H_

// TEST
#include <iostream>
// TEST

#include <Util/ArrayUtil.h>
#include <Util/Exception.h>
#include <cassert>
#include <string>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class BytesRefBuilder;

class BytesRef {
 friend class BytesRefBuilder;

 private:
  static char* DEFAULT_BYTES;

  char* bytes;
  uint32_t offset;
  uint32_t length;
  uint32_t capacity;

 public:
  static const uint32_t IS_OWNING_MASK = (1U << 31);
  static const uint32_t LENGTH_MASK = ((1U << 31) - 1);

 private:
  void SafeDeleteBytes() {
    if ((length & IS_OWNING_MASK) && (bytes != nullptr)) {
      delete[] bytes; 
      offset = length = 0;
      capacity = 1;
      bytes = DEFAULT_BYTES;
    }
  }

  bool IsOwning() const noexcept {
    return (length & IS_OWNING_MASK) != 0;
  }

  BytesRef& SetOwnning() {
    length |= IS_OWNING_MASK;
    return *this;
  }

  BytesRef& SetReference() {
    length &= LENGTH_MASK;
    return *this;
  }

  uint32_t CompareTo(const BytesRef& other) const;

 public:
  uint32_t Length() {
    return (length & LENGTH_MASK);
  }

  // Referencing
  BytesRef()
    : bytes(DEFAULT_BYTES)
      offset(0),
      length(0),
      capacity(1) {
  }

  // Referencing
  BytesRef(char* bytes,
           const uint32_t offset,
           const uint32_t length,
           const uint32_t capacity)
    : bytes(bytes),
      offset(offset),
      length(length),
      capacity(capacity) {
    assert(IsValid());
  }

  // Referencing
  BytesRef(char* bytes,
           uint32_t offset,
           uint32_t length)
    : BytesRef(bytes, offset, length, offset + length) {
  }

  // Referencing
  BytesRef(char* bytes, uint32_t capacity)
    : BytesRef(bytes, 0, capacity, capacity) {
  }

  // Ownning
  BytesRef(const uint32_t capacity)
    : bytes(new char[capacity]),
      offset(0),
      length(IS_OWNING_MASK),
      capacity(capacity) {
  }

  // Referencing
  BytesRef(const std::string& text) {
    SafeDeleteBytes(); 

    if (text.empty()) {
      bytes = BytesRef::DEFAULT_BYTES;
      offset = length = 0;
      capacity = 1;
    } else {
      capacity = length = text.size();
      length |= IS_OWNING_MASK;
      offset = 0;
      bytes = new char[capacity];
      std::memcpy(bytes_ptr, text.c_str(), capacity);
    }
  }

  // Copy ctor
  BytesRef(const BytesRef& other)
    : BytesRef(other.bytes,
               other.offset,
               other.length,
               other.capacity) {
    if (other.IsOwning()) {
      bytes = ArrayUtil::CopyOf<char>(other.bytes, other.capacity);
    } 
  }

  // Move ctor
  BytesRef(BytesRef&& other)
    : bytes(other.bytes), 
      offset(other.offset),
      length(other.length),
      capacity(other.capacity) {
    // Prevent double release
    other.SetReference();
  }

  ~BytesRef() {
    SafeDeleteBytes();
  }

  BytesRef& operator=(const BytesRef& other) {
    if (this != &other) {
      SafeDeleteByte();

      bytes = other.bytes;
      offset = other.offset;
      length = other.length;
      capacity = other.capacity;

      if (other.IsOwning()) {
        bytes = ArrayUtil::CopyOf<char>(other.bytes, other.capacity);
      }
    }

    return *this;
  }

  BytesRef& BytesRef::operator=(BytesRef&& other) {
    if (this != &other) {
      SafeDeleteBytes();

      bytes = other.bytes;
      offset = other.offset;
      length = other.length;
      capacity = other.capacity;

      // Prevent double release
      other.SetReference();
    }

    return *this;
  }

  bool operator==(const BytesRef& other) const {
    return CompareTo(other) == 0;
  }

  bool operator!=(const BytesRef& other) const {
    return !operator==(other);
  }

  bool operator<(const BytesRef& other) const {
    return CompareTo(other) < 0;
  }

  bool operator<=(const BytesRef& other) const {
    return CompareTo(other) <= 0;
  }

  bool operator>(const BytesRef& other) const {
    return CompareTo(other) > 0;
  }

  bool operator>=(const BytesRef& other) const {
    return CompareTo(other) >= 0;
  }

  std::string UTF8ToString() {
    return std::string(bytes.get(), offset, length);
  }

  bool IsValid() const {
    // C++ BytesRef allows null bytes
    if (!bytes.get() && (offset != 0 || length != 0) && capacity > 1) {
      throw std::runtime_error("bytes is nullptr, offset="
                                + std::to_string(offset)
                                + ", length="
                                + std::to_string(length)
                                + ", capacity=" + std::to_string(capacity));
    }

    if (offset > length) {
      throw std::overflow_error("Offset out of bounds: "
                                + std::to_string(offset)
                                + ", length=" + std::to_string(length));
    }

    return true;
  }

  char* Bytes() const noexcept {
    return bytes;
  }

  uint32_t Offset() const noexcept {
    return offset;
  }

  uint32_t Capacity() const noexcept {
    return capacity;
  }
};  // BytesRef

class BytesRefBuilder {
 private:
  BytesRef ref;

 public:
  BytesRefBuilder()
    : ref() {
  }

  char* Bytes() const {
    return ref.Bytes();
  }

  uint32_t Length() const {
    return ref.Length();
  }

  void SetLength(const uint32_t new_length) {
    ref.length = new_length;
  }

  char& operator[](const uint32_t idx) {
    return ref.bytes[idx];
  }

  void Grow(uint32_t new_capacity) {
    std::pair<char*, uint32_t> new_bytes_pair =
      arrayutil::Grow(ref.bytes, ref.capacity, new_capacity);

    if (new_bytes_pair.first) {
      ref.SafeDeleteByte();
      ref.bytes = new_bytes_pair.first;
      ref.length = ref.capacity = new_bytes_pair.second;
      ref.offset = 0;
      ref.SetOwning(); 
    }
  }

  void Append(const char c) {
    Grow(ref.length + 1);
    ref.bytes[ref.length++] = c;
  }

  void Append(const char* bytes,
              const uint32_t off,
              const uint32_t len) {
    Grow(ref.Length() + len);
    std::memcpy(ref.bytes + ref.Length(), bytes + off, len);
    ref.length += len;
  }

  void Append(BytesRef& ref) {
    Append(ref.bytes, ref.offset, ref.length);
  }

  void Append(BytesRefBuilder& builder) {
    Append(builder.ref);
  }

  void Clear() {
    SetLength(0);
  }

  void CopyBytes(const char* bytes,
                 const uint32_t off,
                 const uint32_t len) {
    Clear();
    Append(bytes, off, len);
  }

  void CopyBytes(BytesRef& ref) {
    Clear();
    Append(ref);
  }

  void CopyBytes(BytesRefBuilder& builder) {
    Clear();
    Append(builder);
  }

  void CopyChars(const std::string& text) {
    CopyChars(text, 0, text.size());
  }

  void CopyChars(const std::string& text,
                 const uint32_t off,
                 const uint32_t len) {
    Grow(len);
    std::memcpy(ref.bytes, text.c_str() + off, len);
    ref.length = len;
    ref.SetOwning();
  }

  BytesRef& Get() {
    return ref;
  }

  BytesRef ToBytesRef() const {
    return BytesRef(ref);
  }
};  // BytesRefBuilder

class IntsRefBuilder;

class IntsRef {
 friend class IntsRefBuilder;

 private:
  static const uint32_t IS_OWNING_MASK = (1U << 31);
  static const uint32_t LENGTH_MASK = ((1U << 31) - 1);

  int32_t* ints; 
  uint32_t offset;
  uint32_t capacity;
  uint32_t length;

 private:
  void SafeDeleteInts() {
    if ((length & IS_OWNING_MASK) && (ints != nullptr)) {
      delete[] ints; 
      ints = nullptr;
      length = offset = capacity = 0;
    }
  }

  bool IsOwning() {
    return (length & IS_OWNING_MASK) != 0;
  }

  IntsRef& SetOwning() {
    length |= IS_OWNING_MASK;
    return *this;
  }

  IntsRef& SetReference() {
    length &= LENGTH_MASK;
    return *this;
  }

 public:
  IntsRef()
    : ints(nullptr),
      offset(0),
      capacity(0),
      length(0) {
  }

  // Owning ints
  explicit IntsRef(const uint32_t capacity)
    : ints(new int32_t[capacity]),
      offset(0),
      capacity(capacity),
      length(capacity) {
    SetOwnning();
  }
  
  // Referencing ints
  IntsRef(int32_t ints[],
          const uint32_t capacity,
          const uint32_t offset,
          const uint32_t length)
    : ints(ints),
      capacity(capacity),
      offset(offset),
      length(length & LENGTH_MASK) {
  }

  // Copying ints
  // If `other` is referencing, then this will reference same address.
  // This will copy only if `other` is having own `ints`
  IntsRef(const IntsRef& other)
    : ints(other.ints),
      capacity(other.capacity),
      offset(other.offset),
      length(other.length) {
    // Copy if `ints` is not the reference
    if (other.IsOwning()) {
      ints = ArrayUtil::CopyOf<int32_t>(other.ints, capacity);
    }
  }

  // Move ctor
  IntsRef(IntsRef&& other)
    : ints(other.ints),
      capacity(other.capacity),
      offset(other.offset),
      length(other.length) {
    // Prevent double memory release
    other.SetReference();
  }

  IntsRef& operator=(const IntsRef& other) {
    if (this != &other) {
      SafeDeleteInts();

      ints = other.ints;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;

      if (other.IsOwning() {
        ints = ArrayUtil::CopyOf<int>(other.ints, capacity);
      }
    }

    return *this;
  }

  IntsRef& operator=(IntsRef&& other) {
    if (this != &other) {
      SafeDeleteInts();

      ints = other.ints;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;

      // Prevent double memory release
      other.SetReference();
    }

    return *this;
  }

  ~IntsRef() {
    SafeDeleteInts();
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
    if (this != &other && length == other.length) {
      int32_t* my_base = (ints + offset);
      int32_t* other_base = (other.ints + other.offset);
      uint32_t idx = 0;  
      while (idx < length && (my_base[idx] == other_base[idx])) {
        ++idx;
      }

      return (idx == length);
    }

    return (length == other.length);
  }

  bool operator!=(const IntsRef& other) const {
    return !(operator==(other));
  }

  int32_t HashCode() {
    const int32_t prime = 31;
    int32_t result = 0;
    const int end = (offset + length);
    for (int i = offset ; i < end ; ++i) {
      result = ((prime * result) + ints[i]);
    }

    return result;
  }

  uint32_t Length() const noexcept {
    return length;
  }

  uint32_t Offset() const noexcept {
    return offset;
  }

  uint32_t Capacity() const noexcept {
    return capacity;
  }

  int32_t* Ints() {
    return ints;
  }
};  // IntsRef

class IntsRefBuilder {
 private:
  IntsRef ref;

 public:
  IntsRefBuilder()
    : ref() {
    ref.length |= IntsRef::IS_OWNING_MASK;
  }

  int32_t* Ints() {
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

  void SetIntAt(const uint32_t offset, const int32_t i) {
    ref.ints[offset] = i;
  }

  void Append(const int32_t i) {
    Grow(ref.capacity + 1);
    ref.ints[ref.length++] = i;
  }

  void Grow(const uint32_t new_length) {
    std::pair<int32_t*, uint32_t> pair =
      ArrayUtil::Grow(ref.ints, ref.capacity, new_length);

    if (pair.first != ref.ints) {
      delete[] ref.ints;
      ref.ints = pair.first;
      ref.capacity = pair.second;

      std::cout << "IntsRefBuilder::Grow, ints -> " << ref.ints << std::endl;
    }
  }

  void InitInts(IntsRef&& source) {
    ref = std::forward<IntsRef>(source); 
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

  void CopyUTF8Bytes(const BytesRef& bytes);

  IntsRef& Get() {
    return ref;
  }
};  // IntsRefBuilder

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
  bool IsOwning() const noexcept {
    return (length & IS_OWNING_MASK) != 0;
  }

  void ClearIf() {
    if (length & IS_OWNING_MASK) {
      delete[] longs;
    }
  }

  void SetOwning() {
    length |= IS_OWNING_MASK;
  }

  void SetReference() {
    length &= LENGTH_MASK;
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
      length(capacity) {
    SetOwning();
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
    SetReference();
  }

  LongsRef(const LongsRef& other) {
    operator=(other);
  }

  LongsRef(LongsRef&& other) {
    operator=(std::forward<LongsRef>(other));
  }

  LongsRef& operator=(const LongsRef& other) {
    if (this != &other) {
      longs = other.longs;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;

      if (other.IsOwning()) {
        longs = ArrayUtil::CopyOfRange(other.longs,
                                       other.offset,
                                       other.offset + other.length);
      }
    }

    return *this;
  }

  LongsRef& operator=(LongsRef&& other) {
    longs = other.longs;
    capacity = other.capacity;
    offset = other.offset;
    length = other.length;

    other.SetReference();
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
    if (longs != new_longs) {
      ClearIf();
      longs = new_longs;
      capacity = new_capacity;
      length = new_capacity;
      SetReference();
    }

    return *this;
  }

  LongsRef& Own(int64_t* new_longs,
                const uint32_t new_capacity) noexcept {
    if (longs != new_longs) {
      ClearIf();
      longs = new_longs;
      capacity = new_capacity;
      length = new_capacity;
      SetOwning();
    }

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
}; // LongsRef

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_REF_H_
