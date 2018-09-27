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
#include <Util/FstUtil.h>
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
using lucene::core::util::FSTUtil;

void AddKeyValue(Builder<IntsRef>& builder,
                 const std::string& key,
                 const std::string& value) {
  BytesRefBuilder bytes_builder1;
  bytes_builder1.CopyChars(key);
  BytesRef& key_bytes = bytes_builder1.Get();

  BytesRefBuilder bytes_builder2;
  bytes_builder2.CopyChars(value);
  BytesRef& value_bytes = bytes_builder2.Get();

  IntsRefBuilder ints_ref_builder1; 
  ints_ref_builder1.CopyUTF8Bytes(key_bytes);
  IntsRef& actual_key = ints_ref_builder1.Get();

  IntsRefBuilder ints_ref_builder2; 
  ints_ref_builder2.CopyUTF8Bytes(value_bytes);
  IntsRef& actual_value = ints_ref_builder2.Get();

  std::cout << "Actual key's ints -> " << actual_key.ints << std::endl;
  std::cout << "Actual value's ints -> " << actual_value.ints << std::endl;

  builder.Add(std::move(actual_key), std::move(actual_value));

  std::cout << "kkkkkkkkkkkkkkkkkkkkk" << std::endl;
  std::cout << "&actual_key -> " << &actual_key << std::endl;
  std::cout << "&ints_ref_builder1's ref -> "
            << &ints_ref_builder1.Get() << std::endl;

  std::cout << "&actual_value -> " << &actual_value << std::endl;
  std::cout << "&ints_ref_builder2's ref -> "
            << &ints_ref_builder2.Get() << std::endl;
  std::cout << "kkkkkkkkkkkkkkkkkkkkk" << std::endl;
}

IntsRef ReadValue(FST<IntsRef>& fst, const std::string& key) {
  BytesRefBuilder bytes_builder;
  bytes_builder.CopyChars(key);
  BytesRef key_bytes(bytes_builder.Get());

  return FSTUtil::Get(fst, key_bytes);
}

TEST(BYTESREF__TESTS, BASIC__TEST) {
  Builder<IntsRef> builder(FST_INPUT_TYPE::BYTE1,
                           std::make_unique<IntSequenceOutputs>());

  for (int i = 0 ; i < 5 ; ++i) {
    std::cout << "[Write] i -> " << i << std::endl;
    std::string key("k-" + std::to_string(i));
    std::string val("v-" + std::to_string(i));
    AddKeyValue(builder, key, val);
  }

  FST<IntsRef>* fst = builder.Finish();

  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;

  ASSERT_NE(fst, nullptr);

  for (int i = 0 ; i < 5 ; ++i) {
    std::cout << "[Read] i -> " << i << std::endl;
    std::string key("k-" + std::to_string(i));
    IntsRef val = ReadValue(*fst, key);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
