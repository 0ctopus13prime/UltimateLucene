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

#include <Util/Pack/Reader.h>

using lucene::core::util::IllegalArgumentException;
using lucene::core::util::Int64Values;
using lucene::core::util::DirectReader;
using lucene::core::store::RandomAccessInput;

/*
 *  DirectReader
 */
std::unique_ptr<Int64Values>
DirectReader::GetInstance(RandomAccessInput* slice,
                          const uint32_t bits_per_value,
                          const uint64_t offset) {
  switch (bits_per_value) {
    case 1: 
      return std::make_unique<DirectReader::DirectPackedReader1>(slice, offset);
    case 2: 
      return std::make_unique<DirectReader::DirectPackedReader2>(slice, offset);
    case 4: 
      return std::make_unique<DirectReader::DirectPackedReader4>(slice, offset);
    case 8: 
      return std::make_unique<DirectReader::DirectPackedReader8>(slice, offset);
    case 12: 
      return std::make_unique<DirectReader::DirectPackedReader12>
             (slice, offset);
    case 16: 
      return std::make_unique<DirectReader::DirectPackedReader16>
             (slice, offset);
    case 20: 
      return std::make_unique<DirectReader::DirectPackedReader20>
             (slice, offset);
    case 24: 
      return std::make_unique<DirectReader::DirectPackedReader24>
             (slice, offset);
    case 28: 
      return std::make_unique<DirectReader::DirectPackedReader28>
             (slice, offset);
    case 32: 
      return std::make_unique<DirectReader::DirectPackedReader32>
             (slice, offset);
    case 40: 
      return std::make_unique<DirectReader::DirectPackedReader40>
             (slice, offset);
    case 48: 
      return std::make_unique<DirectReader::DirectPackedReader48>
             (slice, offset);
    case 56: 
      return std::make_unique<DirectReader::DirectPackedReader56>
             (slice, offset);
    case 64: 
      return std::make_unique<DirectReader::DirectPackedReader64>
             (slice, offset);
    default: throw IllegalArgumentException("unsupported bitsPerValue: " +
                                            std::to_string(bits_per_value));
  } 
}
