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
#include <string>

namespace lucene {
namespace core {
namespace util {

template <typename T>
class AbstractPagedMutable : LongValues {
 public:
  static const uint32_t MIN_BLOCK_SIZE = 1 << 6;
  static const uint32_t MAX_BLOCK_SIZE = 1 << 30;

 protected:
  uint64_t size;
  uint32_t page_shift;
  uint32_t page_mask;
  uint32_t sub_mutables_size;
  std::unique_ptr<PackedInts::Mutable*,
                  std::function<void(PackedInts::Mutable**)>> sub_mutables;
  uint32_t bits_per_value;

 protected:
  AbstractPagedMutable(const uint32_t bits_per_value,
                       const uint64_t size,
                       const uint32_t page_size)
    : size(size),
      page_shift(PackedInts::CheckBlockSize(page_size,
                                            MIN_BLOCK_SIZE,
                                            MAX_BLOCK_SIZE))
      page_mask(page_size - 1),
      sub_mutables_size(PackedInts::NumBlocks(size, page_size)),
      sub_mutables(new PackedInts::Mutable*[sub_mutables_size](),
                   [this] (PackedInts::Mutable** target) {
                     std::for_each(target,
                                 target + sub_mutables_size,
                                 std::default_delete<PackedInts::Mutable[]>());
                   }) {
  }

  AbstractPagedMutable(const AbstractPagedMutable& other) = delete;

  AbstractPagedMutable(AbstractPagedMutable&& other)
    : size(other.size),
      page_shift(other.page_shift),
      page_mask(other.page_mask),
      sub_mutables_size(other.sub_mutables_size),
      sub_mutables(std::move(other.sub_mutables)) {
  }

  AbstractPagedMutable& operator=(AbstractPagedMutable&& other) {
    size = other.size;
    page_shift = other.page_shift;
    page_mask = other.page_mask;
    sub_mutables_size = other.sub_mutables_size;
    sub_mutables = std::move(other.sub_mutables);
    bits_per_value = other.bits_per_value;

    return *this;
  }

  void FillPages() {
    for (uint32_t i = 0 ; i < sub_mutables_size ; ++i) {
      const uint32_t value_count = (i != num_pages - 1 ?
                                    PageSize() :
                                    LastPageSize(size));
      sub_mutables[i] = NewMutable(value_count, bits_per_value).release(); 
    }
  }

  virtual std::unique_ptr<PackedInts::Mutable>
  NewMutable(const uint32_t value_count, const uint32_t bits_per_value) = 0;

  uint32_t LastPageSize(const uint64_t size) const noexcept {
    const uint32_t size = IndexInPage(size);
    return (size == 0 ? PageSize() : size);
  }

  uint32_t PageSize() const noexcept {
    return (page_mask + 1);
  }

  uint64_t Size() const noexcept {
    return size;
  }

  uint32_t PageIndex(const uint64_t index) {
    return static_cast<uint32_t>(index >> page_shift);
  }

  uint32_t IndexInPage(const uint64_t index) {
    return static_cast<uint32_t>(index & page_mask);
  }

  virtual std::unique_ptr<T> NewUnfilledCopy(const uint64_t new_size) = 0;

 public:
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

  std::unique_ptr<T> Resize(const uint64_t new_size) {
    std::unique_ptr<T> copy = NewUnfilledCopy(new_size);
    const uint32_t num_common_pages = std::min(copy->sub_mutables);
    uint64_t copy_buffer[1024];  // TODO(0ctopus13prime): alloca?

    for (uint32_t i = 0 ; i < copy->sub_mutables_size ; ++i) {
      const uint32_t value_count =
      (i != copy->sub_mutables_size - 1 ? PageSize() : LastPageSize(new_size));
      const uint32_t bpv = (i < num_common_pages ?
                            sub_mutables[i].GetBitsPerValue() :
                            bits_per_value);
      copy->sub_mutables[i] = NewMutable(value_count, bpv).release();

      if (i < num_common_pages) {
        const uint32_t copy_length = std::min(value_count,
                                              sub_mutables[i]->Size());

        PackedInts::Copy(sub_mutables[i],
                         0 ,
                         copy->sub_mutables[i],
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

class PagedGrowableWriter : AbstractPagedMutable<PagedGrowableWriter> {

};

class PagedMutable : AbstractPagedMutable<PagedMutable> {

};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PAGED_H_
