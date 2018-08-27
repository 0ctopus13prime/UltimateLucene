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
#include <Store/DataInput.h>

using lucene::core::store::DataInput;
using lucene::core::store::IndexInput;
using lucene::core::store::BufferedIndexInput;

const uint32_t DataInput::SKIP_BUFFER_SIZE = 1024;

std::unique_ptr<BufferedIndexInput>
BufferedIndexInput::Wrap(const std::string& slice_desc,
                         IndexInput* other,
                         const uint64_t offset,
                         const uint64_t length) {
  return
  std::make_unique<BufferedIndexInput::SlicedIndexInput>(slice_desc,
                                                         other,
                                                         offset,
                                                         length);
}
