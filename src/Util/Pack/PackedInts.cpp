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

#include <Util/Pack/PackedInts.h>
#include <Util/Pack/Packed64SingleBlock.h>
#include <Util/Pack/Writer.h>

using lucene::core::util::PackedInts;
using lucene::core::util::Packed64SingleBlock;
using lucene::core::util::PackedWriter;

/**
 *  PackedInts
 */

const float PackedInts::FASTEST = 7;
const float PackedInts::FAST = 0.5F;
const float PackedInts::DEFAULT = 0.25F;
const float PackedInts::COMPACT = 0;
const std::string CODEC_NAME("PackedInts");

const PackedInts::Format PackedInts::Format::PACKED(0);
const PackedInts::Format PackedInts::Format::PACKED_SINGLE_BLOCK(1);

bool PackedInts::Format::IsSupported(const uint32_t bits_per_value) const noexcept {
  if (id == 0) {  // PACKED
    return (bits_per_value >= 1 && bits_per_value <= 64);
  } else {  // PACKED_SINGLE_BLOCK
    return Packed64SingleBlock::IsSupported(bits_per_value);
  }
}

std::unique_ptr<PackedInts::Writer>
PackedInts::GetWriterNoHeader(lucene::core::store::DataOutput* out,
                              PackedInts::Format format,
                              const uint32_t value_count,
                              const uint32_t bits_per_value,
                              const uint32_t mem) {
  return std::make_unique<PackedWriter>(format, out, value_count, bits_per_value, mem);
}
