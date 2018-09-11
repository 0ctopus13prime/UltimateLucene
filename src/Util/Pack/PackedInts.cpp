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

using lucene::core::util::PackedInts;

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
