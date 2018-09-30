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

TEST(OUTPUTS__TESTS, INTS__OUTPUT) {
  IntSequenceOutputs outputs;

  //////////////////////
  // No ouput
  //////////////////////
  {
    IntsRef no_output;
    ASSERT_TRUE(outputs.IsNoOutput(no_output));

    // Owner whose capacity equals 100
    IntsRef ints_ref1(IntsRef::MakeOwner(100));
    ASSERT_FALSE(outputs.IsNoOutput(ints_ref1));

    outputs.MakeNoOutput(ints_ref1);
    ASSERT_TRUE(outputs.IsNoOutput(ints_ref1));
  }

  //////////////////////
  // Prefix length
  //////////////////////
  {
    IntsRefBuilder builder1;
    IntsRefBuilder builder2;

    // Case 1. No prefix
    builder1.Append(1); builder1.Append(2);
    builder2.Append(2); builder2.Append(3);

    ASSERT_EQ(0, outputs.PrefixLen(builder1.Get(), builder2.Get()));

    // Case 2. Has common prefix
    builder1.Clear(); builder2.Clear();
    builder1.Append(1); builder1.Append(2); builder1.Append(3);
    builder2.Append(1); builder2.Append(2); builder2.Append(4);

    ASSERT_EQ(2, outputs.PrefixLen(builder1.Get(), builder2.Get()));

    // Case 3. Include another word
    builder1.Clear(); builder2.Clear();
    builder1.Append(1)
            .Append(2)
            .Append(3)
            .Append(4);

    builder2.Append(1)
            .Append(2)
            .Append(3);

    ASSERT_EQ(3, outputs.PrefixLen(builder1.Get(), builder2.Get()));
  }

  //////////////////////
  // Reference
  //////////////////////
  {
    IntsRefBuilder builder; 
    builder.Append(1)
           .Append(2)
           .Append(3)
           .Append(4)
           .Append(5);

    IntsRef prefix(outputs.PrefixReference(builder.Get(), 3));
    ASSERT_EQ(3, prefix.Length());
    ASSERT_EQ(1, prefix.Ints()[prefix.Offset()]);
    ASSERT_EQ(2, prefix.Ints()[prefix.Offset() + 1]);
    ASSERT_EQ(3, prefix.Ints()[prefix.Offset() + 2]);

    IntsRef suffix(outputs.SuffixReference(builder.Get(), 3));
    ASSERT_EQ(2, suffix.Length());
    ASSERT_EQ(4, suffix.Ints()[suffix.Offset()]);
    ASSERT_EQ(5, suffix.Ints()[suffix.Offset() + 1]);
  }

  //////////////////////
  // Manipulation
  //////////////////////
  {
    IntsRefBuilder builder1; 
    builder1.Append(1)
            .Append(2)
            .Append(3)
            .Append(4)
            .Append(5);

    outputs.DropSuffix(builder1.Get(), 3);
    ASSERT_EQ(3, builder1.Length());
    ASSERT_EQ(1, builder1[0]);
    ASSERT_EQ(2, builder1[1]);
    ASSERT_EQ(3, builder1[2]);

    // Now builder1 has [1,2,3] 
    // builder has [13, 14]
    IntsRefBuilder builder2;
    builder2.Append(13)
            .Append(14);
   
    // Case1. Prepend not empty prefix to not empty output
    // ref1 = ref2 + ref1
    outputs.Prepend(builder2.Get(), builder1.Get());
    ASSERT_EQ(5, builder1.Length());
    ASSERT_EQ(13, builder1[0]);
    ASSERT_EQ(14, builder1[1]);
    ASSERT_EQ(1, builder1[2]);
    ASSERT_EQ(2, builder1[3]);
    ASSERT_EQ(3, builder1[4]);

    // Case2. Prepend empty prefix to not empty output
    // ref1 = ref2(empty) + ref1
    builder2.Clear();
    outputs.Prepend(builder2.Get(), builder1.Get());
    ASSERT_EQ(5, builder1.Length());
    ASSERT_EQ(13, builder1[0]);
    ASSERT_EQ(14, builder1[1]);
    ASSERT_EQ(1, builder1[2]);
    ASSERT_EQ(2, builder1[3]);
    ASSERT_EQ(3, builder1[4]);

    // Case3. Prepend not empty prefix to empty output
    // ref2(empty) = ref1
    outputs.Prepend(builder1.Get(), builder2.Get());
    ASSERT_EQ(5, builder2.Length());
    ASSERT_EQ(13, builder2[0]);
    ASSERT_EQ(14, builder2[1]);
    ASSERT_EQ(1, builder2[2]);
    ASSERT_EQ(2, builder2[3]);
    ASSERT_EQ(3, builder2[4]);

    // Case4. Prepend empty prefix to empty output
    builder1.Clear();
    builder2.Clear();
    outputs.Prepend(builder1.Get(), builder2.Get());
    ASSERT_EQ(0, builder1.Length());
    ASSERT_EQ(0, builder2.Length());

    // Shift suffix 
    builder1.Clear()
            .Append(1)
            .Append(2)
            .Append(3)
            .Append(4)
            .Append(5);

    outputs.ShiftLeftSuffix(builder1.Get(), 3);
    ASSERT_EQ(2, builder1.Length());
    ASSERT_EQ(4, builder1[0]);
    ASSERT_EQ(5, builder1[1]);

    outputs.ShiftLeftSuffix(builder1.Get(), 10000);
    ASSERT_EQ(0, builder1.Length());
  }
}

TEST(BYTESREF__TESTS, BASIC__TEST) {
  Builder<IntsRef> builder(FST_INPUT_TYPE::BYTE1,
                           std::make_unique<IntSequenceOutputs>());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
