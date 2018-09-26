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

#include <Util/FstBuilder.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using lucene::core::util::BytesRef;
using lucene::core::util::BytesRefBuilder;
using lucene::core::util::ByteSequenceOutputs;
using lucene::core::util::Builder;
using lucene::core::util::IntsRef;
using lucene::core::util::IntsRefBuilder;
using lucene::core::util::IntSequenceOutputs;
using lucene::core::util::FST;
using lucene::core::util::FST_INPUT_TYPE;

void AddKeyValue(Builder<IntsRef>& builder,
                 const std::string& key,
                 const std::string& value) {
  BytesRefBuilder bytes_builder1;
  bytes_builder1.CopyChars(key);
  BytesRef key_bytes(bytes_builder1.Get());

  BytesRefBuilder bytes_builder2;
  bytes_builder2.CopyChars(value);
  BytesRef value_bytes(bytes_builder2.Get());

  IntsRefBuilder ints_ref_builder1; 
  ints_ref_builder1.CopyUTF8Bytes(key_bytes);
  IntsRef actual_key(ints_ref_builder1.Get());

  IntsRefBuilder ints_ref_builder2; 
  ints_ref_builder2.CopyUTF8Bytes(value_bytes);
  IntsRef actual_value(ints_ref_builder2.Get());

  builder.Add(std::move(actual_key), std::move(actual_value));
}

TEST(BYTESREF__TESTS, BASIC__TEST) {
  Builder<IntsRef> builder(FST_INPUT_TYPE::BYTE1,
                           std::make_unique<IntSequenceOutputs>());

  for (int i = 0 ; i < 1000 ; ++i) {
    std::cout << "i -> " << i << std::endl;
    std::string key("k-" + std::to_string(i));
    std::string val("v-" + std::to_string(i));
    AddKeyValue(builder, key, val);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
