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

#include <Util/Pack/LongValues.h>

using lucene::core::util::PackedLongValues;
using lucene::core::util::DeltaPackedLongValues;
using lucene::core::util::MonotonicLongValues;

/**
 * PackedLongValues
 */

std::unique_ptr<PackedLongValues::Builder>
PackedLongValues::PackedBuilder(const uint32_t page_size,
                                const float acceptable_overhead_ratio) {
  return std::make_unique<PackedLongValues::Builder>(
           page_size, acceptable_overhead_ratio);
}

std::unique_ptr<PackedLongValues::Builder>
PackedLongValues::PackedBuilder(const float acceptable_overhead_ratio) {
  return PackedBuilder(PackedLongValues::DEFAULT_PAGE_SIZE,
                       acceptable_overhead_ratio);
}

std::unique_ptr<PackedLongValues::Builder>
PackedLongValues::DeltaPackedBuilder(const uint32_t page_size,
                                     const float acceptable_overhead_ratio) {
  return std::make_unique<DeltaPackedLongValues::Builder>
         (page_size, acceptable_overhead_ratio);
}

std::unique_ptr<PackedLongValues::Builder>
PackedLongValues::DeltaPackedBuilder(const float acceptable_overhead_ratio) {
  return DeltaPackedBuilder(
         PackedLongValues::DEFAULT_PAGE_SIZE, acceptable_overhead_ratio);
}

std::unique_ptr<PackedLongValues::Builder>
PackedLongValues::MonotonicBuilder(const uint32_t page_size,
                                   const float acceptable_overhead_ratio) {
  return std::make_unique<PackedLongValues::Builder>(
         page_size, acceptable_overhead_ratio);
}

std::unique_ptr<PackedLongValues::Builder>
PackedLongValues::MonotonicBuilder(const float acceptable_overhead_ratio) {
  return MonotonicBuilder(PackedLongValues::DEFAULT_PAGE_SIZE,
                          acceptable_overhead_ratio);
}
