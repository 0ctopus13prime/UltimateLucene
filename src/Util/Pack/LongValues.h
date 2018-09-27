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
#ifndef SRC_UTIL_PACK_LONGVALUES_H_
#define SRC_UTIL_PACK_LONGVALUES_H_

#include <Util/ArrayUtil.h>
#include <Util/Pack/PackedInts.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class PackedLongValues {
 public:
  class Builder;
  class Iterator;

 private:
  static const uint32_t DEFAULT_PAGE_SIZE = 1024;
  static const uint32_t MIN_PAGE_SIZE = 64;
  static const uint32_t MAX_PAGE_SIZE = (1 << 20);

 public:
  static std::unique_ptr<PackedLongValues::Builder>
  PackedBuilder(const uint32_t page_size,
                const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedLongValues::Builder>
  PackedBuilder(const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedLongValues::Builder>
  DeltaPackedBuilder(const uint32_t page_size,
                     const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedLongValues::Builder>
  DeltaPackedBuilder(const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedLongValues::Builder>
  MonotonicBuilder(const uint32_t page_size,
                   const float acceptable_overhead_ratio);

  static std::unique_ptr<PackedLongValues::Builder>
  MonotonicBuilder(const float acceptable_overhead_ratio);

 protected:
  uint32_t page_shift;
  uint32_t page_mask;
  std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> values;
  uint32_t values_size;

 private:
  uint64_t size;

 protected:
  virtual uint32_t DecodeBlock(const uint32_t block, int64_t dest[]) {
    std::unique_ptr<PackedInts::Reader>& vals = values[block];
    const uint32_t size = vals->Size();
    for (uint32_t k = 0 ; k < size ; ) {
      k += vals->Get(k, dest, k, size - k);
    }

    return size;
  }

  virtual int64_t Get(const uint32_t block, const uint32_t element) noexcept {
    return values[block]->Get(element);
  }

 public:
  PackedLongValues(const uint32_t page_shift,
               const uint32_t page_mask,
               std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]>&& values,
               const uint32_t values_size,
               const uint64_t size)
    : page_shift(page_shift),
      page_mask(page_mask),
      values(std::move(values)),
      values_size(values_size),
      size(size) {
  }

  virtual ~PackedLongValues() = default;

  uint64_t Size() const noexcept {
    return size;
  }

  int64_t Get(uint32_t index) {
    assert(index < Size());
    const uint32_t block =
      static_cast<uint32_t>(index >> page_shift);
    const uint32_t element =
      static_cast<uint32_t>(index & page_mask);

    return Get(block, element);
  }

  Iterator iterator();
};

class PackedLongValues::Iterator {
 private:
  friend class PackedLongValues;

  PackedLongValues* ref;
  std::unique_ptr<int64_t[]> current_values;
  uint32_t v_off;
  uint32_t p_off;
  uint32_t current_count;

 private:
  Iterator(PackedLongValues* ref)
    : ref(ref), 
      current_values(std::make_unique<int64_t[]>(ref->page_mask + 1)),
      v_off(0),
      p_off(0),
      current_count(0) {
    FillBlock();
  }

  void FillBlock() {
    if (v_off == ref->values_size) {
      current_count = 0;
    } else {
      current_count = ref->DecodeBlock(v_off, current_values.get());
      assert(current_count > 0);
    }
  }

 public:
  bool HasNext() const noexcept {
    return (p_off < current_count); 
  }

  int64_t Next() {
    assert(HasNext());
    const int64_t result = current_values[p_off++];
    if (p_off == current_count) {
      ++v_off;
      p_off = 0;
      FillBlock();
    }

    return result;
  }
};

class PackedLongValues::Builder {
 private:
  static const uint32_t INITIAL_PAGE_COUNT = 16;

 protected:
  uint32_t page_shift;
  uint32_t page_mask;
  float acceptable_overhead_ratio;
  uint32_t pending_size;
  std::unique_ptr<int64_t[]> pending;
  uint64_t size;

  std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> values;
  uint32_t values_size;
  uint32_t values_off;
  uint32_t pending_off;

 private:
  void Pack() {
    Pack(pending.get(), pending_off, values_off, acceptable_overhead_ratio);
    ++values_off;
    // Reset pending buffer
    pending_off = 0;
  }

  virtual std::unique_ptr<PackedLongValues> DoBuild() {
    Finish();
    pending.reset(); pending_size = 0;

    std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> new_values =
      std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(
        values_off);
    for (uint32_t i = 0 ; i < values_off ; ++i) {
      new_values[i] = std::move(values[i]);
    }

    values = std::move(new_values);
    values_size = values_off;

    return std::make_unique<PackedLongValues>(page_shift,
                                              page_mask,
                                              std::move(values),
                                              values_size,
                                              size);
  }

 protected:
  void Finish() {
    if (pending_off > 0) {
      if (values_size == values_off) {
        Grow(values_off + 1);
      }
      Pack();
    }
  }

  virtual void Pack(int64_t input_values[],
                    uint32_t num_values,
                    uint32_t block,
                    float acceptable_overhead_ratio) {
    assert(num_values != 0);
    // Compute max delta
    int64_t min_value = input_values[0];
    int64_t max_value = input_values[0];

    for (uint32_t i = 0 ; i < num_values ; ++i) {
      min_value = std::min(min_value, input_values[i]);
      max_value = std::max(max_value, input_values[i]);
    }

    // Build a new packed reader
    if (min_value == 0 && max_value == 0) {
      values[block] = std::make_unique<PackedInts::NullReader>(num_values);
    } else {
      const uint32_t bits_required =
        (min_value < 0 ? 64 : PackedInts::BitsRequired(max_value));

      std::unique_ptr<PackedInts::Mutable> mmutable =
        PackedInts::GetMutable(num_values,
                               bits_required,
                               acceptable_overhead_ratio);
      for (uint32_t i = 0 ; i < num_values ; ) {
        i += mmutable->Set(i, input_values, i, num_values - i);
      }

      values[block] = std::move(mmutable);
    }
  }

  virtual void Grow(const uint32_t new_block_count) {
    assert(new_block_count > values_size);
    std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> new_values =
      std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(
        INITIAL_PAGE_COUNT);

    for (uint32_t i = 0 ; i < values_size ; ++i) {
      new_values[i] = std::move(values[i]);
    }

    values = std::move(new_values);
  }

 public:
  Builder(const uint32_t page_size,
          const float acceptable_overhead_ratio)
    : page_shift(PackedInts::CheckBlockSize(page_size,
                 PackedLongValues::MIN_PAGE_SIZE,
                 PackedLongValues::MAX_PAGE_SIZE)),
      page_mask(page_size - 1),
      acceptable_overhead_ratio(acceptable_overhead_ratio),
      pending_size(page_size),
      pending(std::make_unique<int64_t[]>(pending_size)),
      size(0),
      values(std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(
             INITIAL_PAGE_COUNT)),
      values_size(INITIAL_PAGE_COUNT),
      values_off(0),
      pending_off(0) {
  }

  virtual ~Builder() = default;

  std::unique_ptr<PackedLongValues> Build() {
    return DoBuild();
  }

  uint64_t Size() const noexcept {
    return size;
  }

  Builder& Add(const int64_t l) {
    if (pending_off == pending_size) {
      // Check size
      if (values_size == values_off) {
        const uint32_t new_length =
          ArrayUtil::Oversize<uint32_t>(values_off);
        Grow(new_length);
      }

      Pack();
    }
    pending[pending_off++] = l;
    ++size;

    return *this;
  }
};

class DeltaPackedLongValues: public PackedLongValues {
 public:
  class Builder;

 protected:
  std::unique_ptr<int64_t[]> mins;
  uint32_t mins_size;

 public:
  DeltaPackedLongValues(const uint32_t page_shift,
                        const uint32_t page_mask,
                        std::unique_ptr<std::unique_ptr<PackedInts::
                                        Reader>[]>&& values,
                        const uint32_t values_size,
                        std::unique_ptr<int64_t[]>&& mins,
                        const uint32_t mins_size,
                        const uint64_t size)
    : PackedLongValues(page_shift, page_mask,
                       std::move(values), values_size, size),
      mins(std::move(mins)),
      mins_size(mins_size) {
    assert(values_size == mins_size);
  }

  virtual ~DeltaPackedLongValues() = default;

  int64_t Get(const uint32_t block, const uint32_t element) noexcept {
    return (mins[block] + values[block]->Get(element));
  }

  uint32_t DecodeBlock(const uint32_t block, int64_t dest[]) {
    const uint32_t count = PackedLongValues::DecodeBlock(block, dest);
    const uint32_t min = mins[block];
    for (uint32_t i = 0 ; i < count ; ++i) {
      dest[i] += min;
    }

    return count;
  }
};

class DeltaPackedLongValues::Builder: public PackedLongValues::Builder {
 protected:
  std::unique_ptr<int64_t[]> mins;
  uint32_t mins_size;

 protected:
  void Pack(int64_t input_values[],
            uint32_t num_values,
            uint32_t block,
            float acceptable_overhead_ratio) {
    const int64_t min =
      *(std::min_element(input_values, input_values + num_values));

    std::for_each(input_values,
                  input_values + num_values,
                  [min](int64_t& v){
                    v -= min; 
                  });

    PackedLongValues::Builder::Pack(input_values,
                                    num_values,
                                    block,
                                    acceptable_overhead_ratio);
    
    mins[block] = min;
  }

  void Grow(const uint32_t new_block_count) {
    PackedLongValues::Builder::Grow(new_block_count); 
    std::unique_ptr<int64_t[]> new_mins =
      std::make_unique<int64_t[]>(new_block_count);
    std::memcpy(new_mins.get(), mins.get(), sizeof(int64_t) * new_block_count);

    mins = std::move(new_mins);
    mins_size = new_block_count;
  }

  std::unique_ptr<PackedLongValues> DoBuild() {
    return Build();
  }

 public:
  Builder(const uint32_t page_size, const float acceptable_overhead_ratio)
    : PackedLongValues::Builder(page_size, acceptable_overhead_ratio),
      mins(std::make_unique<int64_t[]>(values_size)),
      mins_size(values_size) {
  }

  virtual ~Builder() = default;

  std::unique_ptr<DeltaPackedLongValues> Build() {
    Finish();
    pending.reset();

    std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> new_values =
      std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(values_off);
    for (uint32_t i = 0 ; i < values_off ; ++i) {
      new_values[i] = std::move(values[i]);
    }

    std::unique_ptr<int64_t[]> new_mins =
      std::make_unique<int64_t[]>(values_off);
    std::memcpy(new_mins.get(), mins.get(), sizeof(int64_t) * values_off);

    return std::make_unique<DeltaPackedLongValues>(page_shift,
                                                   page_mask,
                                                   std::move(new_values),
                                                   values_off,
                                                   std::move(new_mins),
                                                   values_off,
                                                   size);
  }
};

class MonotonicLongValues: public DeltaPackedLongValues {
 protected:
  std::unique_ptr<float[]> averages; 
  uint32_t averages_size;

 public:
  class Builder;

 public:
  MonotonicLongValues(const uint32_t page_shift,
                      const uint32_t page_mask,
                      std::unique_ptr<std::unique_ptr<PackedInts::
                                                      Reader>[]>&& values,
                      const uint32_t values_size,
                      std::unique_ptr<int64_t[]>&& mins,
                      const uint32_t mins_size,
                      std::unique_ptr<float[]>&& averages,
                      const uint32_t averages_size,
                      const uint64_t size)
    : DeltaPackedLongValues(page_shift, page_mask,
                            std::move(values), values_size,
                            std::move(mins), mins_size,
                            size),
      averages(std::move(averages)),
      averages_size(averages_size) {
    assert(values_size == averages_size);
  }

  int64_t Get(const uint32_t block, const uint32_t element) noexcept {
    return (Expected(mins[block], averages[block], element) +
            values[block]->Get(element));
  }

  uint32_t DecodeBlock(const uint32_t block, int64_t dest[]) {
    const uint32_t count = DeltaPackedLongValues::DecodeBlock(block, dest);
    const float average = averages[block];
    for (uint32_t i = 0 ; i < count ; ++i) {
      dest[i] += Expected(0, average, i);
    }

    return count;
  }

  static int64_t
  Expected(const int64_t origin, const float average, const uint32_t index) {
    return (origin + static_cast<int64_t>(
           average * static_cast<int64_t>(index)));
  }
};

class MonotonicLongValues::Builder: public DeltaPackedLongValues::Builder {
 protected:
  std::unique_ptr<float[]> averages;
  uint32_t averages_size;

 private:
  std::unique_ptr<PackedLongValues> DoBuild() {
    return Build();
  }

 public:
  Builder(const uint32_t page_size, const float acceptable_overhead_ratio)
    : DeltaPackedLongValues::Builder(page_size, acceptable_overhead_ratio),
      averages(std::make_unique<float[]>(values_size)),
      averages_size(values_size) {
  }

  std::unique_ptr<MonotonicLongValues> Build() {
    Finish();
    pending.reset();
    
    // Filtering values
    std::unique_ptr<std::unique_ptr<PackedInts::Reader>[]> new_values =
      std::make_unique<std::unique_ptr<PackedInts::Reader>[]>(values_off);
    for (uint32_t i = 0 ; i < values_off ; ++i) {
      new_values[i] = std::move(values[i]);
    }

    // Filtering mins
    std::unique_ptr<int64_t[]> new_mins =
      std::make_unique<int64_t[]>(values_off);
    std::memcpy(new_mins.get(), mins.get(), sizeof(int64_t) * values_off);

    // Filtering avgs
    std::unique_ptr<float[]> new_avgs =
      std::make_unique<float[]>(values_off);
    std::memcpy(new_avgs.get(), averages.get(), sizeof(float) * values_off);
   
    return std::make_unique<MonotonicLongValues>(
             page_shift,
             page_mask,
             std::move(new_values),
             values_off,
             std::move(new_mins),
             values_off,
             std::move(new_avgs),
             values_off,
             size);
  }

 protected:
  void Pack(int64_t input_values[],
            uint32_t num_values,
            uint32_t block,
            float acceptable_overhead_ratio) {
    const float average = (num_values == 1 ? 0 :
                       static_cast<float>(input_values[num_values - 1] -
                                          input_values[0]) /
                       (num_values - 1));
    for (uint32_t i = 0 ; i < num_values ; ++i) {
      input_values[i] -= Expected(0, average, i);
    }

    DeltaPackedLongValues::Builder::Pack(input_values,
                                         num_values,
                                         block,
                                         acceptable_overhead_ratio);
    averages[block] = average;
  }

  void Grow(const uint32_t new_block_count) {
    DeltaPackedLongValues::Builder::Grow(new_block_count);

    std::unique_ptr<float[]> new_avgs =
      std::make_unique<float[]>(new_block_count);
    std::memcpy(new_avgs.get(), averages.get(),
                sizeof(float) * new_block_count);

    averages = std::move(new_avgs);
    averages_size = new_block_count;
  }
};

}  // util
}  // core
}  // lucene


#endif  // SRC_UTIL_PACK_LONGVALUES_H_
