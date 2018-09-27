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

class BytesRef {
 private:
  // TODO(0ctopus13prime): Can we remove this?.
  // Rather than owning bytes, just pointing to bytes outside?
  static std::shared_ptr<char> DEFAULT_BYTES;

 private:
  int32_t CompareTo(const BytesRef& other) const;

 public:
  std::shared_ptr<char> bytes;
  uint32_t offset;
  uint32_t length;
  uint32_t capacity;

 public:
  BytesRef();
  BytesRef(const char* bytes,
           const uint32_t offset,
           const uint32_t length);
  BytesRef(const char* bytes,
           const uint32_t offset,
           const uint32_t length,
           const uint32_t capacity);
  BytesRef(const char* bytes, const uint32_t capacity);
  BytesRef(const BytesRef& source);

  BytesRef(BytesRef&& source);

  explicit BytesRef(const uint32_t capacity);

  explicit BytesRef(const std::string& text);

  ~BytesRef();

  void ShallowCopyTo(BytesRef& target);

  BytesRef& operator=(const BytesRef& other);

  BytesRef& operator=(BytesRef&& other);

  bool operator==(const BytesRef& other) const;

  bool operator!=(const BytesRef& other) const;

  bool operator<(const BytesRef& other) const;

  bool operator<=(const BytesRef& other) const;

  bool operator>(const BytesRef& other) const;

  bool operator>=(const BytesRef& other) const;

  std::string UTF8ToString();

  bool IsValid() const;
};  // BytesRef

class BytesRefBuilder {
 private:
  BytesRef ref;

 public:
  BytesRefBuilder();
  const char* Bytes() const;
  const uint32_t Length() const;
  void SetLength(const uint32_t length);
  char& operator[](const uint32_t idx);
  void Grow(uint32_t capacity);
  void Append(const char c);
  void Append(const char* c, const uint32_t off, const uint32_t len);
  void Append(BytesRef& ref);
  void Append(BytesRefBuilder& builder);
  void Clear();
  void CopyBytes(const char* c, const uint32_t off, uint32_t len);
  void CopyBytes(BytesRef& ref);
  void CopyBytes(BytesRefBuilder& builder);
  void CopyChars(const std::string& text);
  void CopyChars(const std::string& text, const uint32_t off, const uint32_t len);
  BytesRef& Get();
  BytesRef ToBytesRef() const;
};  // BytesRefBuilder

class IntsRef {
 public:
  static const uint32_t IS_OWNING_MASK = (1U << 31);
  static const uint32_t LENGTH_MASK = ((1U << 31) - 1);

  int32_t* ints; 
  uint32_t offset;
  uint32_t capacity;
  uint32_t length;

 private:
  void SafeDeleteInts() {
    if ((length & IS_OWNING_MASK) && (ints != nullptr)) {
      std::cout << this << "] SafeDeleteInts, ints == " << ints << std::endl;
      std::cout << this << "] SafeDeleteInts, offset == " << offset << std::endl;
      std::cout << this << "] SafeDeleteInts, capacity == " << capacity << std::endl;
      std::cout << this << "] SafeDeleteInts, length == " << length << std::endl;
      delete[] ints; 
    }
  }

 public:
  IntsRef()
    : ints(nullptr),
      offset(0),
      capacity(0),
      length(0) {
    std::cout << this << "] IntsRef() == " << ints << std::endl;
  }

  // Owning ints
  explicit IntsRef(const uint32_t capacity)
    : ints(new int32_t[capacity]),
      offset(0),
      capacity(capacity),
      length(IS_OWNING_MASK) {
    std::cout << this << "] IntsRef(capacity) == " << ints << std::endl;
  }
  
  // Referencing ints
  IntsRef(int32_t ints[], const uint32_t capacity, const uint32_t offset, const uint32_t length)
    : ints(ints),
      capacity(capacity),
      offset(offset),
      length(length & LENGTH_MASK) {
    std::cout << this << "] IntsRef(...) == " << ints << std::endl;
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
    if (length & IS_OWNING_MASK) {
      ints = ArrayUtil::CopyOf<int32_t>(other.ints, capacity);
    }
    std::cout << this << "] IntsRef(const IntsRef&) == " << ints << std::endl;
  }

  IntsRef(IntsRef&& other)
    : ints(other.ints),
      offset(other.offset),
      length(other.length) {
    // Prevent double memory release
    other.length &= LENGTH_MASK;
    std::cout << this << "] IntsRef(IntsRef&&) == " << ints << std::endl;
  }

  IntsRef& operator=(const IntsRef& other) {
    if (this != &other) {
      SafeDeleteInts();
      ints = other.ints;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;

      if (length & IS_OWNING_MASK) {
        ints = ArrayUtil::CopyOf<int>(other.ints, capacity);
      }
    }

    std::cout << this << "] IntsRef operator=(const IntsRef&) == " << ints << std::endl;

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
      other.length &= LENGTH_MASK;
    }

    std::cout << this << "] IntsRef operator=(IntsRef&&) == " << ints << std::endl;

    return *this;
  }

  ~IntsRef() {
    std::cout << this << "] ~IntsRef()" << std::endl;
    SafeDeleteInts();
  }

  bool operator<(const IntsRef& other) const {
    std::cout << this << "] operator <" << std::endl;
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
    std::cout << this << "] operator ==" << std::endl;
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
    std::cout << this << "] operator !=" << std::endl;
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
      longs = ArrayUtil::CopyOfRange(other.longs,
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
}; // LongsRef

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_REF_H_
