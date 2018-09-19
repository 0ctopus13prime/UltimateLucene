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

#ifndef SRC_UTIL_PACK_WRITER_H_
#define SRC_UTIL_PACK_WRITER_H_

#include <Store/DataOutput.h>
#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <Util/Pack/BulkOperation.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace util {

class PackedWriter: public PackedInts::Writer {
 private:
  PackedInts::Format format;
  BulkOperation* encoder;
  std::unique_ptr<char[]> next_blocks;
  std::unique_ptr<int64_t[]> next_values;
  uint32_t iterations;
  uint32_t off;
  uint32_t written;
  uint32_t next_blocks_size;
  uint32_t next_values_size;
  bool finished;

 private:
  void Flush() {
    encoder->Encode(next_values.get(), 0, next_blocks.get(), 0, iterations);
    const uint32_t block_count =
      static_cast<const uint32_t>(format.ByteCount(PackedInts::VERSION_CURRENT,
                                                   off,
                                                   bits_per_value));
      out->WriteBytes(next_blocks.get(), block_count); 
      std::memset(next_values.get(), 0, sizeof(int64_t) * next_values_size);
      off = 0;
  }

 public:
  PackedWriter(PackedInts::Format format,
               lucene::core::store::DataOutput* out,
               const uint32_t value_count,
               const uint32_t bits_per_value,
               const uint32_t mem)
    : PackedInts::Writer(out, value_count, bits_per_value),
      format(format),
      encoder(BulkOperation::Of(format, bits_per_value)),
      next_blocks(),
      next_values(),
      iterations(encoder->ComputeIterations(value_count, mem)),
      off(0),
      written(0),
      finished(false) {
    next_blocks_size = (iterations * encoder->ByteBlockCount());
    next_blocks =
     std::make_unique<char[]>(next_blocks_size);

    next_values_size = (iterations * encoder->ByteValueCount());
    next_values =
      std::make_unique<int64_t[]>(next_values_size);
  }

  PackedInts::Format GetFormat() {
    return format;
  }

  void Add(int64_t v) {
    assert(PackedInts::UnsignedBitsRequired(v) <= bits_per_value); 
    assert(!finished);

    if (written >= value_count) {
      throw EOFException("Writing past end of stream");
    }

    next_values[off++] = v;
    if (off == next_values_size) {
      Flush();
    }

    ++written;
  }

  void Finish() {
    assert(!finished);

    while (written < value_count) {
      Add(0);
    }

    Flush();
    finished = true;
  }

  uint32_t Ord() {
    return written - 1;
  }
};

class GrowableWriter: public PackedInts::Mutable {
 private:
  std::unique_ptr<PackedInts::Mutable> current;
  uint64_t current_mask;
  float acceptable_overhead_ratio;

 private:
  static uint64_t Mask(const uint32_t bpv) {
    return (bpv == 64 ? ~0L : PackedInts::MaxValue(bpv));
  }

  void EnsureCapacity(const int64_t value) {
    if ((value & current_mask) == value) {
      return;
    }

    const uint32_t bits_required = PackedInts::UnsignedBitsRequired(value);
    assert(bits_required > current->GetBitsPerValue());
    const uint32_t value_count = Size();
    std::unique_ptr<PackedInts::Mutable> next =
      PackedInts::GetMutable(value_count, bits_required,
                             acceptable_overhead_ratio);
    PackedInts::Copy(current.get(), 0, next.get(), 0, value_count,
                     PackedInts::DEFAULT_BUFFER_SIZE);
    current = std::move(next);
    current_mask = Mask(current->GetBitsPerValue());
  }

 public:
  GrowableWriter(const uint32_t start_bits_per_value,
                 const uint32_t value_count,
                 const float acceptable_overhead_ratio)
    : acceptable_overhead_ratio(acceptable_overhead_ratio),
      current(PackedInts::GetMutable(value_count, start_bits_per_value, acceptable_overhead_ratio)),
      current_mask(Mask(current->GetBitsPerValue())) {
  }

  int64_t Get(uint32_t index) {
    return current->Get(index);
  }

  uint32_t Size() {
    return current->Size();
  }

  uint32_t GetBitsPerValue() {
    return current->GetBitsPerValue();
  }

  PackedInts::Mutable* GetMutable() {
    return current.get();
  }

  void Set(uint32_t index, int64_t value) {
    EnsureCapacity(value);
    current->Set(index, value);
  }

  void Clear() {
    current->Clear();
  }

  uint32_t Get(uint32_t index,
               int64_t arr[],
               uint32_t off,
               uint32_t len) {
    return current->Get(index, arr, off, len);
  }

  uint32_t Set(uint32_t index,
               const int64_t arr[],
               uint32_t off,
               uint32_t len) {
    uint64_t max = 0;
    for (uint32_t i = off, end = off + len ; i < end ; ++i) {
      max |= arr[i];
    }

    EnsureCapacity(max);
    return current->Set(index, arr, off, len);
  }

  void Fill(uint32_t from_index,
            uint32_t to_index,
            int64_t val) {
    EnsureCapacity(val);
    current->Fill(from_index, to_index, val);
  }

  void Save(lucene::core::store::DataOutput* out) {
    current->Save(out);
  }
};

class DirectWriter {
 private:
  static constexpr uint32_t SUPPORTED_BITS_PER_VALUE[] = {
    1, 2, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64
  };

  uint32_t bits_per_value;
  uint32_t num_values;
  lucene::core::store::DataOutput* output;
  uint64_t count;
  bool finished;

  BulkOperation* encoder;
  uint32_t iterations;
  uint32_t off;
  uint32_t next_blocks_size;
  std::unique_ptr<char[]> next_blocks;
  uint32_t next_values_size;
  std::unique_ptr<int64_t[]> next_values;

 private:
  void Flush() {
    encoder->Encode(next_values.get(), 0, next_blocks.get(), 0, iterations);
    const uint32_t block_count =
      static_cast<uint32_t>(PackedInts::Format::PACKED.ByteCount(
                            PackedInts::VERSION_CURRENT, off, bits_per_value));
    output->WriteBytes(next_blocks.get(), block_count);
    std::memset(next_values.get(), 0, next_values_size);
    off = 0;
  }

  static int32_t BinarySearchSupportedBits(const uint32_t bits_per_value) {
    uint32_t l = 0;
    uint32_t h = sizeof(SUPPORTED_BITS_PER_VALUE) / sizeof(uint32_t) - 1;

    while (l <= h) {
      const uint32_t mid = (l + h) >> 1;
      const uint32_t mid_val = SUPPORTED_BITS_PER_VALUE[mid];

      if (mid_val < bits_per_value) {
        l = mid + 1;
      } else if (mid_val > bits_per_value) {
        h = mid - 1;
      } else {
        return mid;
      }
    }

    return -(l + 1);
  }

  uint32_t RoundBits(const uint32_t bits_required) {
    int32_t idx = BinarySearchSupportedBits(bits_required);
    if(idx < 0) {
      return SUPPORTED_BITS_PER_VALUE[-idx - 1];
    } else {
      return bits_required;
    } 
  }

 public:
  DirectWriter(lucene::core::store::DataOutput* output,
               const uint64_t num_values,
               const uint32_t bits_per_value)
    : bits_per_value(bits_per_value),
      num_values(num_values),
      output(output),
      count(0),
      finished(false),
      encoder(BulkOperation::Of(PackedInts::Format::PACKED, bits_per_value)),
      iterations(encoder->ComputeIterations(static_cast<uint32_t>(
                 std::min(num_values, static_cast<uint64_t>(
                          std::numeric_limits<int32_t>::min()))),
                 PackedInts::DEFAULT_BUFFER_SIZE)),
      off(0),
      next_blocks_size(iterations * encoder->ByteBlockCount()),
      next_blocks(std::make_unique<char[]>(next_blocks_size)),
      next_values_size(iterations * encoder->ByteValueCount()),
      next_values(std::make_unique<int64_t[]>(next_values_size)) {
  }

  void Add(const int64_t l) {
    if (count >= num_values) {
      throw EOFException("Writing past end of stream");
    }
    next_values[off++] = l;
    if (off == next_values_size) {
      Flush();
    }

    count++;
  }

  void Finish() {
    if (count != num_values) {
      throw IllegalStateException("Wrong number of values added, expected: " +
                                  std::to_string(num_values) +
                                  ", got: " +
                                  std::to_string(count));
    }

    assert(!finished);
    Flush();
    // Pad for fast io: we actually only need this for certain BPV,
    // but its just 3 bytes...
    for (int i = 0; i < 3; i++) {
      output->WriteByte(0);
    }
    finished = true;
  }

 public:
  std::unique_ptr<DirectWriter>
  GetInstance(lucene::core::store::DataOutput* output,
              const uint64_t num_values,
              const uint32_t bits_per_value) {
    if (BinarySearchSupportedBits(bits_per_value) < 0) {
      throw IllegalArgumentException("Unsupported bits_per_value " +
                                     std::to_string(bits_per_value) +
                                     ". Did you use bits_required?");
    }

    return std::make_unique<DirectWriter>(output, num_values, bits_per_value);
  }

  uint32_t BitsRequired(const int64_t max_value) {
    return RoundBits(PackedInts::BitsRequired(max_value));
  }

  uint32_t UnsignedBitsRequired(const int64_t max_value) {
    return RoundBits(PackedInts::UnsignedBitsRequired(max_value));
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_WRITER_H_
