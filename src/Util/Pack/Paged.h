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

#ifndef SRC_UTIL_PACK_PAGED_H_
#define SRC_UTIL_PACK_PAGED_H_

#include <Util/Etc.h>
#include <Util/Exception.h>
#include <Util/Numeric.h>
#include <Util/Pack/PackedInts.h>
#include <Util/Pack/Writer.h>
#include <cassert>
#include <string>

namespace lucene {
namespace core {
namespace util {

template <typename T>
class AbstractPagedMutable: public Int64Values {
 public:
  static const uint32_t MIN_BLOCK_SIZE = (1 << 6);
  static const uint32_t MAX_BLOCK_SIZE = (1 << 30);

 protected:
  uint64_t size;
  uint32_t page_shift;
  uint32_t page_mask;
  uint32_t sub_mutables_size;
  uint32_t bits_per_value;
  std::unique_ptr<std::unique_ptr<PackedInts::Mutable>[]> sub_mutables;

 protected:
  AbstractPagedMutable(const uint32_t bits_per_value,
                       const uint64_t size,
                       const uint32_t page_size)
    : size(size),
      page_shift(PackedInts::CheckBlockSize(page_size,
                                            MIN_BLOCK_SIZE,
                                            MAX_BLOCK_SIZE)),
      page_mask(page_size - 1),
      sub_mutables_size(PackedInts::NumBlocks(size, page_size)),
      bits_per_value(bits_per_value),
      sub_mutables(std::make_unique<std::unique_ptr<PackedInts::Mutable>[]>
                   (sub_mutables_size)) {
  }

  AbstractPagedMutable(const AbstractPagedMutable& other) = delete;

  AbstractPagedMutable(AbstractPagedMutable&& other)
    : size(other.size),
      page_shift(other.page_shift),
      page_mask(other.page_mask),
      sub_mutables_size(other.sub_mutables_size),
      bits_per_value(other.bits_per_value),
      sub_mutables(std::move(other.sub_mutables)) {
  }

  AbstractPagedMutable& operator=(AbstractPagedMutable&& other) {
    size = other.size;
    page_shift = other.page_shift;
    page_mask = other.page_mask;
    sub_mutables_size = other.sub_mutables_size;
    bits_per_value = other.bits_per_value;
    sub_mutables = std::move(other.sub_mutables);

    return *this;
  }

  void FillPages() {
    const uint32_t num_pages = PackedInts::NumBlocks(size, PageSize());
    for (uint32_t i = 0 ; i < sub_mutables_size ; ++i) {
      const uint32_t value_count = (i != num_pages - 1 ?
                                    PageSize() :
                                    LastPageSize(size));
      sub_mutables[i] = NewMutable(value_count, bits_per_value);
    }
  }

  virtual std::unique_ptr<PackedInts::Mutable>
  NewMutable(uint32_t value_count, uint32_t bits_per_value) = 0;

  uint32_t LastPageSize(const uint64_t size) const noexcept {
    const uint32_t sz = IndexInPage(size);
    return (sz == 0 ? PageSize() : sz);
  }

  uint32_t PageSize() const noexcept {
    return (page_mask + 1);
  }

  uint32_t PageIndex(const uint64_t index) const noexcept {
    return static_cast<uint32_t>(index >> page_shift);
  }

  uint32_t IndexInPage(const uint64_t index) const noexcept {
    return static_cast<uint32_t>(index & page_mask);
  }

 public:
  uint64_t Size() const noexcept {
    return size;
  }

  int64_t Get(const uint64_t index) {
    const uint32_t page_index = PageIndex(index);
    const uint32_t index_in_page = IndexInPage(index);
    return sub_mutables[page_index]->Get(index_in_page);
  }

  void Set(const uint64_t index, const uint64_t value) {
    const uint32_t page_index = PageIndex(index);
    const uint32_t index_in_page = IndexInPage(index);
    sub_mutables[page_index]->Set(index_in_page, value);
  }

  virtual std::unique_ptr<AbstractPagedMutable> NewUnfilledCopy(const uint64_t new_size) = 0;

  std::unique_ptr<T> Resize(const uint64_t new_size) {
    std::unique_ptr<T> copy(dynamic_cast<T*>(NewUnfilledCopy(new_size).release()));
    const uint32_t num_common_pages =
      std::min(copy->sub_mutables_size, sub_mutables_size);
    uint64_t copy_buffer[1024];  // TODO(0ctopus13prime): alloca?

    for (uint32_t i = 0 ; i < copy->sub_mutables_size ; ++i) {
      const uint32_t value_count =
      (i != copy->sub_mutables_size - 1 ? PageSize() : LastPageSize(new_size));
      const uint32_t bpv = (i < num_common_pages ?
                            sub_mutables[i]->GetBitsPerValue() :
                            bits_per_value);
      copy->sub_mutables[i] = NewMutable(value_count, bpv);

      if (i < num_common_pages) {
        const uint32_t copy_length = std::min(value_count,
                                              sub_mutables[i]->Size());

        PackedInts::Copy(sub_mutables[i].get(),
                         0,
                         copy->sub_mutables[i].get(),
                         0,
                         copy_length,
                         copy_buffer);
      }
    }

    return std::move(copy);
  }

  std::unique_ptr<T> Grow(const uint64_t min_size) {
    if (min_size <= Size()) {
      return std::unique_ptr<T>();
    }

    const uint64_t extra = (min_size < 3 * 8 ? 3 : min_size >> 3);
    const uint64_t new_size = (min_size + extra);
    return Resize(new_size);
  }

  std::unique_ptr<T> Grow() {
    return Grow(Size() + 1);
  }
};

class PagedGrowableWriter: public AbstractPagedMutable<PagedGrowableWriter> {
 private:
  float acceptable_overhead_ratio;

 public:
  PagedGrowableWriter(const uint64_t size,
                      const uint32_t page_size,
                      const uint32_t start_bits_per_value,
                      const float acceptable_overhead_ratio,
                      const bool fill_pages=true)
    : AbstractPagedMutable<PagedGrowableWriter>(start_bits_per_value,
                                                size,
                                                page_size),
      acceptable_overhead_ratio(acceptable_overhead_ratio) {
    if (fill_pages) {
      FillPages();
    }
  }

  PagedGrowableWriter(PagedGrowableWriter&& other)
    : AbstractPagedMutable<PagedGrowableWriter>(std::move(other)),
      acceptable_overhead_ratio(other.acceptable_overhead_ratio) {
  }

  PagedGrowableWriter& operator=(PagedGrowableWriter&& other) {
    AbstractPagedMutable<PagedGrowableWriter>::operator=(std::move(other));
    acceptable_overhead_ratio = other.acceptable_overhead_ratio;

    return *this;
  }

  std::unique_ptr<PackedInts::Mutable>
  NewMutable(uint32_t value_count, uint32_t bits_per_value) {
    return std::make_unique<GrowableWriter>(bits_per_value,
                                            value_count,
                                            acceptable_overhead_ratio);
  }

  std::unique_ptr<AbstractPagedMutable> NewUnfilledCopy(uint64_t new_size) {
    return std::make_unique<PagedGrowableWriter>(new_size,
                                                 PageSize(),
                                                 bits_per_value,
                                                 acceptable_overhead_ratio,
                                                 false);  
  }
};

class PagedMutable: public AbstractPagedMutable<PagedMutable> {
 private:
  PackedInts::Format format;

 private:
  PagedMutable(const uint64_t size,
               const uint32_t page_size,
               const uint32_t bits_per_value,
               PackedInts::Format format)
    : AbstractPagedMutable<PagedMutable>(bits_per_value, size, page_size),
      format(format) {
  }

  PagedMutable(const uint64_t size,
               const uint32_t page_size,
               PackedInts::FormatAndBits format_and_bits)
    : PagedMutable(size,
                   page_size,
                   format_and_bits.bits_per_value,
                   format_and_bits.format) {
  }

 public:
  PagedMutable(const uint64_t size,
               const uint32_t page_size,
               const uint32_t bits_per_value,
               const float acceptable_overhead_ratio)
    : PagedMutable(size,
                   page_size,
                   PackedInts::FastestFormatAndBits(page_size,
                                                  bits_per_value,
                                                  acceptable_overhead_ratio)) {
    FillPages();
  }

  PagedMutable(PagedMutable&& other)
    : AbstractPagedMutable<PagedMutable>(std::move(other)),
      format(other.format) {
  }

  PagedMutable& operator=(PagedMutable&& other) {
    AbstractPagedMutable<PagedMutable>::operator=(std::move(other));
    format = other.format;
    return *this;
  }

  std::unique_ptr<PackedInts::Mutable>
  NewMutable(uint32_t value_count, uint32_t bits_per_value) {
    assert(this->bits_per_value >= bits_per_value);
    return PackedInts::GetMutable(value_count, this->bits_per_value, format);
  }

  std::unique_ptr<AbstractPagedMutable> NewUnfilledCopy(uint64_t new_size) {
    return std::unique_ptr<PagedMutable>(
           new PagedMutable(new_size,
                            PageSize(),
                            bits_per_value,
                            format));
  }
};  // PagedMutable

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PAGED_H_
