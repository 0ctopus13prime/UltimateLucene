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

#include <Util/Bits.h>

using lucene::core::util::BitUtil;

const uint64_t BitUtil::MAGIC[7] = {
  0x5555555555555555L, 0x3333333333333333L,
  0x0F0F0F0F0F0F0F0FL, 0x00FF00FF00FF00FFL,
  0x0000FFFF0000FFFFL, 0x00000000FFFFFFFFL,
  0xAAAAAAAAAAAAAAAAL
};

const uint16_t BitUtil::SHIFT[5] = {
  1, 2, 4, 8, 16
};
