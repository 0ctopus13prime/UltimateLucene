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
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class BytesRefBuilder;

class BytesRef {
 friend class BytesRefBuilder;

  char* bytes;
  uint32_t offset;
  uint32_t length;
  uint32_t capacity;

 public:
  static const uint32_t IS_OWNING_MASK = (1U << 31);
  static const uint32_t LENGTH_MASK = ((1U << 31) - 1);

 private:
  void SafeDeleteBytes() {
    if (IsOwning() && (bytes != nullptr)) {
      delete[] bytes; 
    }

    bytes = nullptr;
    offset = length = capacity = 0;
  }

  bool IsOwning() const noexcept {
    return (length & IS_OWNING_MASK) != 0;
  }

  BytesRef& SetOwning() {
    length |= IS_OWNING_MASK;
    return *this;
  }

  BytesRef& SetReference() {
    length &= LENGTH_MASK;
    return *this;
  }

  int32_t CompareTo(const BytesRef& other) const {
    const int32_t len = std::min(Length(), other.Length());

    const int32_t result = std::memcmp(Bytes() + Offset(),
                                       other.Bytes() + other.Offset(),
                                       len);

    if (result != 0) {
      return result;
    }

    // One is either a prefix of another or equivalent bytes
    return (Length() - other.Length());
  }

 public:
  static BytesRef MakeReference(const BytesRef& other) {
    return BytesRef(other.bytes,
                    other.offset,
                    other.length & LENGTH_MASK,
                    other.capacity);
  }

  static BytesRef MakeOwner(const BytesRef& other) {
    return BytesRef(ArrayUtil::CopyOf<char>(other.bytes, other.capacity),
                    other.offset,
                    other.length | IS_OWNING_MASK,
                    other.capacity);
  }

  static BytesRef MakeOwner(const std::string& text) {
    if (text.empty()) {
      return BytesRef(nullptr,
                      0,
                      IS_OWNING_MASK,
                      0);
    } else {
      return BytesRef(ArrayUtil::CopyOf<char>(text.c_str(), text.size()),
                      0,
                      text.size() | IS_OWNING_MASK,
                      text.size());
    }
  }

  static BytesRef MakeOwner(const uint32_t capacity) {
    return BytesRef(new char[capacity],
                    0,
                    IS_OWNING_MASK,
                    capacity);
  }

 public:
  // Referencing
  BytesRef()
    : bytes(nullptr),
      offset(0),
      length(0),
      capacity(0) {
  }

  // Referencing
  BytesRef(const char bytes[],
           const uint32_t offset,
           const uint32_t length,
           const uint32_t capacity)
    : bytes(const_cast<char*>(bytes)),
      offset(offset),
      length(length),
      capacity(capacity) {
  }

  // Referencing
  BytesRef(const char* bytes,
           const uint32_t offset,
           const uint32_t capacity)
    : BytesRef(const_cast<char*>(bytes), offset, capacity, offset + capacity) {
  }

  // Referencing
  BytesRef(const char* bytes, const uint32_t capacity)
    : BytesRef(const_cast<char*>(bytes), 0, capacity, capacity) {
  }

  // Copy ctor if copy = true,
  // Reference if copy = false
  BytesRef(const BytesRef& other)
    : BytesRef(other.bytes,
               other.offset,
               other.length,
               other.capacity) {
    if (other.IsOwning()) {
      bytes = ArrayUtil::CopyOf<char>(other.bytes, other.capacity);
    } 
  }

  // Move ctor, bytes ownership transfer
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
      SafeDeleteBytes();

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

  BytesRef& operator=(BytesRef&& other) {
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
    return (CompareTo(other) == 0);
  }

  bool operator!=(const BytesRef& other) const {
    return (!operator==(other));
  }

  bool operator<(const BytesRef& other) const {
    return (CompareTo(other) < 0);
  }

  bool operator<=(const BytesRef& other) const {
    return (CompareTo(other) <= 0);
  }

  bool operator>(const BytesRef& other) const {
    return (CompareTo(other) > 0);
  }

  bool operator>=(const BytesRef& other) const {
    return (CompareTo(other) >= 0);
  }

  std::string UTF8ToString() const {
    return (bytes != nullptr ? std::string(bytes, offset, Length()) : std::string());
  }

  uint32_t Length() const noexcept {
    return (length & LENGTH_MASK);
  }

  BytesRef& SetLength(const uint32_t new_length) noexcept {
    length = ((length & IS_OWNING_MASK) | new_length);
    return *this;
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
    ref.SetOwning();
  }

  char* Bytes() const noexcept {
    return ref.Bytes();
  }

  uint32_t Length() const noexcept {
    return ref.Length();
  }

  void SetLength(const uint32_t new_length) {
    ref.SetLength(new_length);
  }

  void Clear() {
    ref.offset = 0;
    SetLength(0);
  }

  char& operator[](const uint32_t idx) const noexcept {
    return ref.bytes[ref.Offset() + idx];
  }

  void Append(const char c) {
    Grow(ref.Offset() + ref.Length() + 1);
    ref.bytes[ref.Offset() + ref.Length()] = c;
    ref.length++;
  }

  void Append(const char* bytes,
              const uint32_t off,
              const uint32_t len) {
    Grow(ref.Offset() + ref.Length() + len);
    std::memcpy(ref.Bytes() + ref.Offset() + ref.Length(),
                bytes + off,
                len);
    ref.length += len;
  }

  void Append(const std::string& str) {
    Append(str.c_str(), 0, str.size());
  }

  void Append(BytesRef& ref) {
    Append(ref.Bytes(), ref.Offset(), ref.Length());
  }

  void Append(BytesRefBuilder& builder) {
    Append(builder.ref);
  }

  void Grow(uint32_t new_capacity) {
    std::pair<char*, uint32_t> new_bytes_pair =
      ArrayUtil::Grow(ref.Bytes(), ref.Capacity(), new_capacity);

    if (new_bytes_pair.first != ref.bytes) {
      ref.SafeDeleteBytes();
      ref.bytes = new_bytes_pair.first;
      ref.capacity = new_bytes_pair.second;
    }
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
    Append(builder.ref);
  }

  void CopyBytes(const std::string& str) {
    Clear();
    Append(str.c_str(), 0, str.size());
  }

  BytesRef& Get() {
    return ref;
  }

  BytesRef DupBytesRef() const {
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
  uint32_t length;
  uint32_t capacity;

 private:
  void SafeDeleteInts() {
    if (IsOwning() && (ints != nullptr)) {
      delete[] ints; 
    }

    ints = nullptr;
    offset = capacity = length = 0;
  }

  bool IsOwning() const noexcept {
    return (length & IS_OWNING_MASK) != 0;
  }

  IntsRef& SetOwning() noexcept {
    length |= IS_OWNING_MASK;
    return *this;
  }

  IntsRef& SetReference() noexcept {
    length &= LENGTH_MASK;
    return *this;
  }

  int32_t CompareTo(const IntsRef& other) const {
    uint32_t len = std::min(Length(), other.Length());
    const int32_t* s1 = static_cast<const int32_t*>(Ints() + Offset());
    const int32_t* s2 = static_cast<const int32_t*>(other.Ints() +
                                                    other.Offset());

    while (len-- > 0) {
      if (*s1++ != *s2++) {
        return (s1[-1] < s2[-1] ? -1 : 1);
      }
    }

    // One is either a prefix of another or equivalent ints
    return (Length() - other.Length());
  }

 public:
  static IntsRef MakeReference(const IntsRef& other) {
    return IntsRef(other.ints,
                   other.offset,
                   other.length & LENGTH_MASK,
                   other.capacity);
  }

  static IntsRef MakeOwner(const IntsRef& other) {
    return IntsRef(ArrayUtil::CopyOf<int32_t>(other.ints, other.capacity),
                   other.offset,
                   other.length | IS_OWNING_MASK,
                   other.capacity);
  }

  static IntsRef MakeOwner(const int32_t ints[],
                           const uint32_t offset,
                           const uint32_t length,
                           const uint32_t capacity) {
    return IntsRef(ArrayUtil::CopyOf<int32_t>(ints, capacity),
                   offset,
                   length | IS_OWNING_MASK,
                   capacity);
  }

  static IntsRef MakeOwner(const uint32_t capacity) {
    return IntsRef(new int32_t[capacity],
                   0,
                   0,
                   capacity | IS_OWNING_MASK);
  }

 public:
  IntsRef()
    : ints(nullptr),
      offset(0),
      length(0),
      capacity(0) {
  }

  // Referencing ints
  IntsRef(const int32_t ints[],
          const uint32_t offset,
          const uint32_t length,
          const uint32_t capacity)
    : ints(const_cast<int*>(ints)),
      capacity(capacity),
      offset(offset),
      length(length) {
  }

  // Referencing ints
  IntsRef(const int32_t ints[],
          const uint32_t capacity)
    : IntsRef(ints, 0, capacity, capacity) {
  }

  // Referencing ints
  IntsRef(const int32_t ints[],
          const uint32_t offset,
          const uint32_t capacity)
    : IntsRef(ints, offset, capacity, capacity) {
  }

  // Copying ints
  // If `other` is referencing, then this will reference same address.
  // This will copy only if `other` is having own `ints`
  IntsRef(const IntsRef& other)
    : IntsRef(other.ints,
              other.offset,
              other.length,
              other.capacity) {
    // Copy if `ints` is not the reference
    if (other.IsOwning()) {
      ints = ArrayUtil::CopyOf<int32_t>(other.ints, other.capacity);
    }
  }

  // Move ctor
  IntsRef(IntsRef&& other)
    : IntsRef(other.ints,
              other.offset,
              other.length,
              other.capacity) {
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

      if (other.IsOwning()) {
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

  bool operator==(const IntsRef& other) const {
    return (CompareTo(other) == 0);
  }

  bool operator!=(const IntsRef& other) const {
    return !(operator==(other));
  }

  bool operator<(const IntsRef& other) const {
    return (CompareTo(other) < 0);
  }

  bool operator<=(const IntsRef& other) const {
    return (CompareTo(other) <= 0);
  }

  bool operator>(const IntsRef& other) const {
    return (CompareTo(other) > 0);
  }

  bool operator>=(const IntsRef& other) const {
    return (CompareTo(other) >= 0);
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
    return (length & LENGTH_MASK);
  }

  IntsRef& SetLength(const uint32_t new_length) noexcept {
    length = ((length & IS_OWNING_MASK) | new_length);
    return *this;
  }

  uint32_t Offset() const noexcept {
    return offset;
  }

  IntsRef& SetOffset(const uint32_t new_offset) noexcept {
    offset = new_offset; 
    return *this;
  }

  uint32_t Capacity() const noexcept {
    return capacity;
  }

  IntsRef& SetCapacity(const uint32_t new_capacity) noexcept {
    capacity = new_capacity; 
    return *this;
  }

  int32_t* Ints() const noexcept {
    return ints;
  }

  IntsRef& SetInts(int32_t* new_ints) noexcept {
    ints = new_ints;
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

  int32_t* Ints() const noexcept {
    return ref.Ints();
  }

  uint32_t Length() const noexcept {
    return ref.Length();
  }

  uint32_t Offset() const noexcept {
    return ref.Offset();
  }

  uint32_t Capacity() const noexcept {
    return ref.Capacity();
  }

  IntsRefBuilder& SetLength(const uint32_t length) noexcept {
    ref.SetLength(length);
    return *this;
  }

  IntsRefBuilder& Clear() noexcept {
    ref.SetOffset(0);
    ref.SetLength(0);
    return *this;
  }

  int32_t& operator[](const uint32_t idx) const noexcept {
    return ref.ints[ref.Offset() + idx];
  }

  IntsRefBuilder& Append(const int32_t i) {
    Grow(ref.Offset() + ref.Length() + 1);
    ref.ints[ref.Offset() + ref.Length()] = i;
    ref.length++;
    return *this;
  }

  IntsRefBuilder& Append(const int32_t* ints,
                         const uint32_t off,
                         const uint32_t len) {
    Grow(ref.Offset() + ref.Length() + len);
    std::memcpy(ref.Ints() + ref.Offset() + ref.Length(),
                ints + off,
                sizeof(int32_t) * len);
    ref.length += len;
    return *this;
  }

  IntsRefBuilder& Append(IntsRef& ref) {
    Append(ref.Ints(), ref.Offset(), ref.Length());
    return *this;
  }

  IntsRefBuilder& Append(IntsRefBuilder& builder) {
    Append(builder.ref);
    return *this;
  }

  IntsRefBuilder& Grow(const uint32_t new_capacity) {
    if (ref.Capacity() < new_capacity) {
      std::pair<int32_t*, uint32_t> pair =
        ArrayUtil::Grow(ref.Ints(), ref.Capacity(), new_capacity);

      delete[] ref.Ints();
      ref.SetInts(pair.first);
      ref.SetCapacity(pair.second);
      ref.SetOwning();
    }

    return *this;
  }

  // `source` must be an owner!
  // Reference type is not allowed
  IntsRefBuilder& InitInts(IntsRef&& source) {
    assert(source.IsOwning());
    ref = std::forward<IntsRef>(source); 

    return *this;
  }

  IntsRefBuilder& CopyInts(const int32_t ints[],
                const uint32_t offset,
                const uint32_t length) {
    Clear();
    Append(ints, offset, length);
    return *this;
  }

  IntsRefBuilder& CopyInts(IntsRef& ref) {
    Clear();
    Append(ref);
    return *this;
  }

  IntsRefBuilder& CopyInts(IntsRefBuilder& builder) {
    Clear();
    Append(builder.ref);
    return *this;
  }

  IntsRefBuilder& CopyUTF8Bytes(const BytesRef& bytes);

  IntsRef& Get() noexcept {
    return ref;
  }

  IntsRef DupIntsRef() const {
    return IntsRef(ref);
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

  void SafeDeleteLongs() {
    if (IsOwning()) {
      delete[] longs;
    }
  }

  LongsRef& SetOwning() noexcept {
    length |= IS_OWNING_MASK;
  }

  LongsRef& SetReference() noexcept {
    length &= LENGTH_MASK;
  }

  int32_t CompareTo(const LongsRef& other) const {
    uint32_t len = std::min(Length(), other.Length());
    const int64_t* s1 =
      static_cast<const int64_t*>(Longs() + Offset());
    const int64_t* s2 =
      static_cast<const int64_t*>(other.Longs() + other.Offset());

    while (len-- > 0) {
      if (*s1++ != *s2++) {
        return (s1[-1] < s2[-1] ? -1 : 1);
      }
    }

    // One is either a prefix of another or equivalent ints
    return (Length() - other.Length());
  }

 public:
  static LongsRef MakeReference(const LongsRef& other) {
    return LongsRef(other.longs,
                    other.offset,
                    other.length & LENGTH_MASK,
                    other.capacity);
  }

  static LongsRef MakeOwner(const LongsRef& other) {
    return LongsRef(ArrayUtil::CopyOf<int64_t>(other.longs, other.capacity),
                    other.offset,
                    other.length | IS_OWNING_MASK,
                    other.capacity);
  }

 public:
  LongsRef()
    : longs(nullptr),
      capacity(0),
      offset(0),
      length(0) {
  }

  // Owning
  LongsRef(const uint32_t capacity)
    : capacity(capacity),
      longs(new int64_t[capacity]),
      offset(0),
      length(capacity) {
  }

  // Reference
  LongsRef(const int64_t longs[],
           const uint32_t offset,
           const uint32_t length,
           const uint32_t capacity)
    : longs(const_cast<int64_t*>(longs)),
      capacity(capacity),
      offset(offset),
      length(length) {
  }

  // Reference
  LongsRef(const int64_t* longs,
           const uint32_t capacity)
    : LongsRef(longs, 0, capacity, capacity) {
  }

  // Reference
  LongsRef(const int64_t* longs,
           const uint32_t offset,
           const uint32_t capacity)
    : LongsRef(longs, offset, capacity, capacity) {
  }

  // If given other is owner, then this instance will copy longs from other
  // Otherwise this will just reference same address of other's longs
  LongsRef(const LongsRef& other)
    : LongsRef(other.longs,
               other.offset,
               other.length,
               other.capacity) {
    if (other.IsOwning()) {
      longs = ArrayUtil::CopyOf<int64_t>(other.longs,
                                         other.capacity);
    }
  }

  // Move ctor
  LongsRef(LongsRef&& other)
    : LongsRef(other.longs,
               other.offset,
               other.length,
               other.capacity) {
    // Prevent double release
    other.SetReference();
  }

  LongsRef& operator=(const LongsRef& other) {
    if (this != &other) {
      SafeDeleteLongs();

      longs = other.longs;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;

      if (other.IsOwning()) {
        longs = ArrayUtil::CopyOf<int64_t>(other.longs,
                                           other.capacity);
      }
    }

    return *this;
  }

  LongsRef& operator=(LongsRef&& other) {
    if (this != &other) {
      SafeDeleteLongs();

      longs = other.longs;
      capacity = other.capacity;
      offset = other.offset;
      length = other.length;

      // Prevent double release
      other.SetReference();
    }

    return *this;
  }

  ~LongsRef() {
    SafeDeleteLongs();
  }

  LongsRef& SetLength(const uint32_t new_length) noexcept {
    length = ((length & IS_OWNING_MASK) | new_length);
    return *this;
  }

  bool operator==(const LongsRef& other) const {
    return (CompareTo(other) == 0);
  }

  bool operator!=(const LongsRef& other) const {
    return !(operator==(other));
  }

  bool operator<(const LongsRef& other) const {
    return (CompareTo(other) < 0);
  }

  bool operator<=(const LongsRef& other) const {
    return (CompareTo(other) <= 0);
  }

  bool operator>(const LongsRef& other) const {
    return (CompareTo(other) > 0);
  }

  bool operator>=(const LongsRef& other) const {
    return (CompareTo(other) >= 0);
  }

  uint32_t Offset() const noexcept {
    return offset; 
  }

  LongsRef& Offset(const uint32_t new_offset) noexcept {
    offset = new_offset;
    return *this;
  }

  LongsRef& IncOffset(const uint32_t delta = 1) noexcept {
    offset += delta;
    return *this;
  }

  uint32_t Length() const noexcept {
    return (length & LENGTH_MASK); 
  }

  LongsRef& Length(const uint32_t new_length) {
    length = ((length & IS_OWNING_MASK) | new_length);
    return *this;
  }

  LongsRef& DecLength(const uint32_t delta = 1) noexcept {
    length -= delta;
    return *this;
  }

  uint32_t Capacity() const noexcept {
    return capacity;
  }

  int64_t* Longs() const noexcept {
    return longs;
  }
}; // LongsRef

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_REF_H_
