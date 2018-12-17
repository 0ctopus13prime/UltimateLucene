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

#include <Codec/Lucene50/ForUtil.h>
#include <algorithm>

using lucene::core::codec::ForUtil;
using lucene::core::util::PackedInts;

uint32_t ForUtil::MAX_DATA_SIZE = []() {
  uint32_t max_data_size = 0;

  for (uint32_t version = PackedInts::VERSION_START ;
       version <= PackedInts::VERSION_CURRENT ;
       version++) {
    for (uint32_t bpv = 1 ; bpv <= 32 ; ++bpv) {
      if (PackedInts::Format::PACKED.IsSupported(bpv)) {
        PackedInts::Decoder* decoder =
          PackedInts::GetDecoder(PackedInts::Format::PACKED, version, bpv);
        const uint32_t iterations = ForUtil::ComputeIterations(decoder);
        max_data_size = std::max(max_data_size, iterations * decoder->ByteValueCount());
      }
    }

    for (uint32_t bpv = 1 ; bpv <= 32 ; ++bpv) {
      if (PackedInts::Format::PACKED_SINGLE_BLOCK.IsSupported(bpv)) {
        PackedInts::Decoder* decoder =
          PackedInts::GetDecoder(PackedInts::Format::PACKED_SINGLE_BLOCK, version, bpv);
        const uint32_t iterations = ForUtil::ComputeIterations(decoder);
        max_data_size = std::max(max_data_size, iterations * decoder->ByteValueCount());
      }
    }
  }

  return max_data_size;
}();
