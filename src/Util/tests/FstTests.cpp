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

#include <Util/Fst.h>
#include <gtest/gtest.h>
#include <iostream>

using lucene::core::util::BytesStore;
using lucene::core::util::Builder;
using lucene::core::util::FST;
using lucene::core::util::FST_INPUT_TYPE;
using lucene::core::util::IntSequenceOutputs;
using lucene::core::util::IntsRef;

TEST(BYTESREF__TESTS, BASIC__TEST) {
  BytesStore bytes_store(5);
  IntSequenceOutputs outputs;
  Builder<IntsRef> builder(FST_INPUT_TYPE::BYTE1, std::make_unique<IntSequenceOutputs>());
  const uint32_t len = 5;
  IntsRef input(len);
  IntsRef output(len);

  builder.Add(input, output);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
