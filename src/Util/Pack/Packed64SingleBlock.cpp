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

#include <Util/Pack/Packed64SingleBlock.h>
#include <Util/Exception.h>

using lucene::core::util::Packed64SingleBlock;

/**
 *  Packed64SingleBlock
 */

std::unique_ptr<Packed64SingleBlock>
Packed64SingleBlock::Create(uint32_t value_count,
                            uint32_t bits_per_value) {
  switch (bits_per_value) {
    case 1:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock1>(value_count);

    case 2:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock2>(value_count);

    case 3:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock3>(value_count);

    case 4:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock4>(value_count);

    case 5:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock5>(value_count);

    case 6:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock6>(value_count);

    case 7:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock7>(value_count);

    case 8:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock8>(value_count);

    case 9:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock9>(value_count);

    case 10:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock10>(value_count);

    case 12:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock12>(value_count);

    case 16:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock16>(value_count);

    case 21:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock21>(value_count);

    case 32:
      return std::make_unique<Packed64SingleBlock::Packed64SingleBlock32>(value_count);

    default:
      throw
      IllegalArgumentException("Unsupported number of bits per value: " +
                               std::to_string(32));
  }
}
