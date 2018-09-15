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

// This file has been automatically generated, DO NOT EDIT

#ifndef SRC_UTIL_PACK_DIRECT_H_
#define SRC_UTIL_PACK_DIRECT_H_

#include <Store/DataInput.h>
#include <Util/Pack/PackedInts.h>
#include <cstring>
#include <algorithm>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class Direct8 : public PackedInts::MutableImpl {
 private:
  std::unique_ptr<uint8_t[]> values;
  uint32_t values_size;

public:
  explicit Direct8(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, 8),
               values(std::make_unique<uint8_t[]>(value_count)),
               values_size(value_count) {
  }

  Direct8(const uint32_t packed_ints_version,
           lucene::core::store::DataInput* in,
           const uint32_t value_count) {
    : PackedInts::MutableImpl(value_count, 8),
      values(std::make_unique<uint8_t[]>(value_count)),
      values_size(value_count) {

    for (uint32_t i = 0 ; i < value_count ; ++i) {
      values[i] = in->ReadInt16();
    }

    const uint32_t remaining = static_cast<uint32_t>(
      PackedInts::Format::PACKED.ByteCount(packed_ints_version, value_count, 8) - 1L * value_count);
    for (uint32_t i = 0 ; i < remaining ; ++i) {
      in->ReadByte();
    }
  }

  int64_t Get(const uint32_t idx) {
    return values[index] & 0xFFL;
  }

  void Set(const uint32_t index, const int64_t value) {
    values[index] = static_cast<uint8_t> (value);
  }

  void Clear() {
    std::memset(values.get(), 0, sizeof(uint8_t) * values_size); 
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    std::copy(values.get() + index,
              values.get() + index + gets,
              arr + off);
    return gets;
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    std::copy(arr + off,
              arr + off + sets,
              values.get() + index);
    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t val) {
    std::fill_n(values.get() + from_index, to_index - from_index, static_cast<uint8_t> val);
  }
};

class Direct16 : public PackedInts::MutableImpl {
 private:
  std::unique_ptr<uint16_t[]> values;
  uint32_t values_size;

public:
  explicit Direct16(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, 16),
               values(std::make_unique<uint16_t[]>(value_count)),
               values_size(value_count) {
  }

  Direct16(const uint32_t packed_ints_version,
           lucene::core::store::DataInput* in,
           const uint32_t value_count) {
    : PackedInts::MutableImpl(value_count, 16),
      values(std::make_unique<uint16_t[]>(value_count)),
      values_size(value_count) {

    for (uint32_t i = 0 ; i < value_count ; ++i) {
      values[i] = in->ReadInt16();
    }

    const uint32_t remaining = static_cast<uint32_t>(
      PackedInts::Format::PACKED.ByteCount(packed_ints_version, value_count, 16) - 2L * value_count);
    for (uint32_t i = 0 ; i < remaining ; ++i) {
      in->ReadByte();
    }
  }

  int64_t Get(const uint32_t idx) {
    return values[index] & 0xFFFFL;
  }

  void Set(const uint32_t index, const int64_t value) {
    values[index] = static_cast<uint16_t> (value);
  }

  void Clear() {
    std::memset(values.get(), 0, sizeof(uint16_t) * values_size); 
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    std::copy(values.get() + index,
              values.get() + index + gets,
              arr + off);
    return gets;
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    std::copy(arr + off,
              arr + off + sets,
              values.get() + index);
    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t val) {
    std::fill_n(values.get() + from_index, to_index - from_index, static_cast<uint16_t> val);
  }
};

class Direct64 : public PackedInts::MutableImpl {
 private:
  std::unique_ptr<uint64_t[]> values;
  uint32_t values_size;

public:
  explicit Direct64(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, 64),
               values(std::make_unique<uint64_t[]>(value_count)),
               values_size(value_count) {
  }

  Direct64(const uint32_t packed_ints_version,
           lucene::core::store::DataInput* in,
           const uint32_t value_count) {
    : PackedInts::MutableImpl(value_count, 64),
      values(std::make_unique<uint64_t[]>(value_count)),
      values_size(value_count) {

    for (uint32_t i = 0 ; i < value_count ; ++i) {
      values[i] = in->ReadInt16();
    }

    const uint32_t remaining = static_cast<uint32_t>(
      PackedInts::Format::PACKED.ByteCount(packed_ints_version, value_count, 64) - 8L * value_count);
    for (uint32_t i = 0 ; i < remaining ; ++i) {
      in->ReadByte();
    }
  }

  int64_t Get(const uint32_t idx) {
    return values[index];
  }

  void Set(const uint32_t index, const int64_t value) {
    values[index] = (value);
  }

  void Clear() {
    std::memset(values.get(), 0, sizeof(uint64_t) * values_size); 
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    std::mempcy(arr + off, values.get() + index, gets);
    return gets;
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    std::memcpy(values + index, arr + off, sets);
    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t val) {
    std::fill_n(values.get() + from_index, to_index - from_index, val);
  }
};

class Direct32 : public PackedInts::MutableImpl {
 private:
  std::unique_ptr<uint32_t[]> values;
  uint32_t values_size;

public:
  explicit Direct32(const uint32_t value_count)
             : PackedInts::MutableImpl(value_count, 32),
               values(std::make_unique<uint32_t[]>(value_count)),
               values_size(value_count) {
  }

  Direct32(const uint32_t packed_ints_version,
           lucene::core::store::DataInput* in,
           const uint32_t value_count) {
    : PackedInts::MutableImpl(value_count, 32),
      values(std::make_unique<uint32_t[]>(value_count)),
      values_size(value_count) {

    for (uint32_t i = 0 ; i < value_count ; ++i) {
      values[i] = in->ReadInt16();
    }

    const uint32_t remaining = static_cast<uint32_t>(
      PackedInts::Format::PACKED.ByteCount(packed_ints_version, value_count, 32) - 4L * value_count);
    for (uint32_t i = 0 ; i < remaining ; ++i) {
      in->ReadByte();
    }
  }

  int64_t Get(const uint32_t idx) {
    return values[index] & 0xFFFFFFFFL;
  }

  void Set(const uint32_t index, const int64_t value) {
    values[index] = static_cast<uint32_t> (value);
  }

  void Clear() {
    std::memset(values.get(), 0, sizeof(uint32_t) * values_size); 
  }

  uint32_t Get(const uint32_t index,
               int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t gets = std::min(value_count - index, len);
    std::copy(values.get() + index,
              values.get() + index + gets,
              arr + off);
    return gets;
  }

  uint32_t Set(const uint32_t index,
               const int64_t arr[],
               const uint32_t off,
               const uint32_t len) {
    const uint32_t sets = std::min(value_count - index, len);
    std::copy(arr + off,
              arr + off + sets,
              values.get() + index);
    return sets;
  }

  void Fill(const uint32_t from_index,
            const uint32_t to_index,
            const int64_t val) {
    std::fill_n(values.get() + from_index, to_index - from_index, static_cast<uint32_t> val);
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_DIRECT_H_
