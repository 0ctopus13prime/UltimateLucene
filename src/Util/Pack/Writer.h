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

// TEST
#include <iostream>
// TEST

#include <Store/DataOutput.h>
#include <Util/Bits.h>
#include <Util/Exception.h>
#include <Util/Numeric.h>
#include <Util/Pack/PackedInts.h>
#include <Util/Pack/BulkOperation.h>
#include <Util/Pack/LongValues.h>
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
};  // PackedWriter

class GrowableWriter: public PackedInts::Mutable {
 private:
  std::unique_ptr<PackedInts::Mutable> current;
  uint64_t current_mask;
  float acceptable_overhead_ratio;

 private:
  static uint64_t Mask(const uint32_t bpv) {
    return (bpv == 64 ? ~0UL : PackedInts::MaxValue(bpv));
  }

  void EnsureCapacity(const int64_t value) {
    // std::cout << "Current mask -> " << current_mask
    //           << ", value -> " << value
    //           << ", ~current_mask & value -> "
    //           << (~current_mask & value) << std::endl;
    if (~current_mask & value) {
      // std::cout << "Enlarge bit-width" << std::endl;
      const uint32_t bits_required = PackedInts::UnsignedBitsRequired(value);
      assert(bits_required > current->GetBitsPerValue());
      const uint32_t value_count = Size();
      // std::cout << "Make new mutable, bits_required -> " << bits_required
      //           << ", value_count -> " << value_count << std::endl;
      std::unique_ptr<PackedInts::Mutable> next =
        PackedInts::GetMutable(value_count,
                               bits_required,
                               acceptable_overhead_ratio);
      // std::cout << "Copy current mutable to new one" << std::endl;
      PackedInts::Copy(current.get(), 0,
                       next.get(), 0,
                       value_count,
                       PackedInts::DEFAULT_BUFFER_SIZE);
      // std::cout << "Move new mutable to current" << std::endl;
      current = std::move(next);
      // std::cout << "Change current mask" << std::endl;
      current_mask = Mask(current->GetBitsPerValue());
      // std::cout << "Change mask done. current_mask -> " << current_mask << std::endl;
    } 
  }

 public:
  GrowableWriter(const uint32_t start_bits_per_value,
                 const uint32_t value_count,
                 const float acceptable_overhead_ratio)
    : acceptable_overhead_ratio(acceptable_overhead_ratio),
      current(PackedInts::GetMutable(value_count,
                                     start_bits_per_value,
                                     acceptable_overhead_ratio)),
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
      // Bitwise or is nice because either all values are positive and the
      // or-ed result will require as many bits per value as the max of the
      // values, or one of them is negative and the result will be negative,
      // forcing GrowableWriter to use 64 bits per value
      // Ex) Max 110101 -> Required bits = 6
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
};  // GrowableWriter

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

  static uint32_t RoundBits(const uint32_t bits_required) {
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
  static uint32_t BitsRequired(const int64_t max_value) {
    return RoundBits(PackedInts::BitsRequired(max_value));
  }

  static std::unique_ptr<DirectWriter>
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

  static uint32_t UnsignedBitsRequired(const int64_t max_value) {
    return RoundBits(PackedInts::UnsignedBitsRequired(max_value));
  }
};  // DirectWriter

class AbstractBlockPackedWriter {
 public:
  static const uint32_t MIN_BLOCK_SIZE = 64;
  static const uint32_t MAX_BLOCK_SIZE = 1 << (30 - 3);
  static const uint32_t MIN_VALUE_EQUALS_0 = 1 << 0;
  static const uint32_t BPV_SHIFT = 1;

 protected:
  lucene::core::store::DataOutput* out; 
  std::unique_ptr<int64_t[]> values;
  std::unique_ptr<char[]> blocks;
  uint32_t values_size;
  uint32_t off;
  uint64_t ord;
  uint32_t blocks_size;
  bool finished;

 protected:
  static void
  WriteVInt64(lucene::core::store::DataOutput* out, uint64_t i) {
    uint32_t k = 0;
    while ((i & ~0x7FL) != 0L && k++ < 8) {
      out->WriteByte(static_cast<char>((i & 0x7FL) | 0x80L));
      i >>= 7;
    }

    out->WriteByte(static_cast<char>(i));
  }

  virtual void Flush() = 0;

  void WriteValues(const uint32_t bits_required) {
    PackedInts::Encoder* encoder =
      PackedInts::GetEncoder(PackedInts::Format::PACKED,
                             PackedInts::VERSION_CURRENT,
                             bits_required); 
    const uint32_t iterations = values_size / encoder->ByteValueCount(); 
    const uint32_t block_size = encoder->ByteBlockCount() * iterations;
    if (!blocks || blocks_size < block_size) {
      blocks = std::make_unique<char[]>(block_size);
      blocks_size = block_size;
    }

    if (off < values_size) {
      std::memset(values.get() + off, 0, sizeof(int64_t) * values_size);
    }
    encoder->Encode(values.get(), 0, blocks.get(), 0, iterations);
    const uint32_t block_count =
      static_cast<uint32_t>(PackedInts::Format::PACKED.ByteCount(
                            PackedInts::VERSION_CURRENT, off, bits_required));
    out->WriteBytes(blocks.get(), block_count);
  }

 private:
  void CheckNotFinished() const {
    if (finished) {
      throw InvalidStateException();
    }
  }

 public:
  AbstractBlockPackedWriter(lucene::core::store::DataOutput* out,
                            const uint32_t block_size) {
    PackedInts::CheckBlockSize(block_size, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE);
    Reset(out);
    values = std::make_unique<int64_t[]>(block_size);
    values_size = block_size;
    blocks_size = 0;
  }

  virtual ~AbstractBlockPackedWriter() = 0;

  void Reset(lucene::core::store::DataOutput* out) {
    assert(out != nullptr);
    this->out = out;
    off = 0;
    ord = 0;
    finished = false;
  }

  void Add(const int64_t l) {
    CheckNotFinished();
    if (off == values_size) {
      Flush();
    }

    values[off++] = l;
    ++ord;
  }

  void Finish() {
    CheckNotFinished();
    if (off > 0) {
      Flush();
    }

    finished = true;
  }

  int64_t Ord() const noexcept {
    return ord;
  }
};  // AbstractBlockPackedWriter

class BlockPackedWriter: public AbstractBlockPackedWriter {
 public:
  BlockPackedWriter(lucene::core::store::DataOutput* out,
                    const uint32_t block_size) 
    : AbstractBlockPackedWriter(out, block_size) {
  }

 protected:
  void Flush() {
    assert(off > 0);
    int64_t min = *(std::min_element(values.get(), values.get() + values_size));
    int64_t max = *(std::max_element(values.get(), values.get() + values_size));

    const int64_t delta = (max - min);
    uint32_t bits_required = (delta == 0 ? 
                              0 : PackedInts::UnsignedBitsRequired(delta));
    if (bits_required == 64) {
      // There is no need to delta-encoding
      min = 0;
    } else if (min > 0) {
      // Make min as small as possible so that WriteVInt64 requires fewer bytes
      min = std::max(0UL, max - PackedInts::MaxValue(bits_required));
    }

    const uint32_t token = (bits_required << BPV_SHIFT) |
                           (min == 0 ? MIN_VALUE_EQUALS_0 : 0);
    out->WriteByte(static_cast<char>(token));

    if (min != 0) {
      WriteVInt64(out, BitUtil::ZigZagEncode(min) - 1);
    }

    if (bits_required > 0) {
      if (min != 0) {
        std::for_each(values.get(), values.get() + off, [min](int64_t& value){
          value -= min; 
        });
      }

      WriteValues(bits_required);
    }

    off = 0;
  }
};  // BlockPackedWriter

class MonotonicBlockPackedWriter: public AbstractBlockPackedWriter {
 public:
  MonotonicBlockPackedWriter(lucene::core::store::DataOutput* out,
                             const uint32_t block_size)
    : AbstractBlockPackedWriter(out, block_size) {
  }

  void Add(const int64_t l) {
    assert(l >= 0);
    AbstractBlockPackedWriter::Add(l);
  }

 protected:
  void Flush() {
    assert(off > 0);

    const float avg = (off == 1 ? 
                       0 :
                       static_cast<float>(
                         values[off - 1] - values[0] / (off - 1) 
                       ));

    int64_t min = values[0];
    // Adjust min so that all deltas will be positive
    for (uint32_t i = 1 ; i < off ; ++i) {
      const int64_t actual = values[i];
      const int64_t expected = MonotonicLongValues::Expected(min, avg, i);
      if (expected > actual) {
        min -= (expected - actual);
      }
    }

    int64_t max_delta = 0;
    for (uint32_t i = 0 ; i < off ; ++i) {
      values[i] = (values[i] - MonotonicLongValues::Expected(min, avg, i));
      max_delta = std::max(max_delta, values[i]);
    }

    out->WriteZInt64(min);
    out->WriteInt32(lucene::core::util::Float::FloatToIntBits(avg));
    if (max_delta == 0) {
      out->WriteVInt32(0);
    } else {
      const uint32_t bits_required = PackedInts::BitsRequired(max_delta);
      out->WriteVInt32(bits_required);
      WriteValues(bits_required);
    }

    off = 0;
  }
};  // MonotonicBlockPackedWriter

class DirectMonotonicWriter {
 public:
  static const uint32_t MIN_BLOCK_SHIFT = 2;
  static const uint32_t MAX_BLOCK_SHIFT = 22;

 protected:
  lucene::core::store::IndexOutput* meta;
  lucene::core::store::IndexOutput* data;
  uint64_t num_values;
  uint64_t base_data_pointer;
  std::unique_ptr<int64_t[]> buffer;
  uint64_t count;
  int64_t previous;
  uint32_t buffer_off;
  uint32_t buffer_size;
  bool finished;

 private:
  void Flush() {
    assert(buffer_size != 0);

    const float avg_inc = static_cast<float>(
      static_cast<double>(buffer[buffer_off - 1] - buffer[0]) /
      std::max(1U, buffer_off - 1));
    
    for (int64_t i = 0 ; i < buffer_off ; ++i) {
      const int64_t expected = static_cast<int64_t>(avg_inc * i);
      buffer[i] -= expected;
    }

    const int64_t min =
      *(std::min_element(buffer.get(), buffer.get() + buffer_off));
    int64_t max_delta = 0;
    std::for_each(
      buffer.get(),
      buffer.get() + buffer_off,
      [min, &max_delta](int64_t& v){
        v -= min;   
        max_delta |= v;
      });

    meta->WriteInt64(min);
    meta->WriteInt32(lucene::core::util::
                     Float::FloatToIntBits(avg_inc));
    meta->WriteInt64(data->GetFilePointer() - base_data_pointer);
    if (max_delta == 0) {
      meta->WriteByte(0);
    } else {
      const uint32_t bits_required =
        DirectWriter::UnsignedBitsRequired(max_delta);
      std::unique_ptr<DirectWriter> writer =
        DirectWriter::GetInstance(data, buffer_off, bits_required);
        for (uint32_t i = 0 ; i < buffer_size ; ++i) {
          writer->Add(buffer[i]);
        }
        writer->Finish();
        meta->WriteByte(static_cast<char>(bits_required));
    }

    buffer_off = 0;
  }

 public:
  DirectMonotonicWriter(lucene::core::store::IndexOutput* meta,
                        lucene::core::store::IndexOutput* data,
                        const uint64_t num_values,
                        const uint32_t block_shift)
    : meta(meta),
      data(data),
      num_values(num_values),
      base_data_pointer(data->GetFilePointer()),
      buffer(std::make_unique<int64_t[]>(1 << block_shift)),
      count(0),
      previous(std::numeric_limits<int64_t>::min()),
      buffer_off(0),
      buffer_size(1 << block_shift),
      finished(false) {
  }

  void add(const int64_t v) {
    assert(v >= previous);

    if (buffer_off == buffer_size) {
      Flush();
    }

    buffer[buffer_off++] = v;
    previous = v;
    count++;
  }

  void Finish() {
    assert(count == num_values);
    assert(!finished);

    if (buffer_off > 0) {
      Flush();
    }

    finished = true;
  }
};  // DirectMonotonicWriter 

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_WRITER_H_
