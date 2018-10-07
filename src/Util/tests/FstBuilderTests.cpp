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
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using lucene::core::store::BufferedFileOutputStream;
using lucene::core::util::BytesRef;
using lucene::core::util::BytesRefBuilder;
using lucene::core::util::ByteSequenceOutputs;
using lucene::core::util::FstBuilder;
using lucene::core::util::IntsRef;
using lucene::core::util::IntsRefBuilder;
using lucene::core::util::IntSequenceOutputs;
using lucene::core::util::NodeHash;
using lucene::core::util::Fst;
using lucene::core::util::FST_INPUT_TYPE;
using lucene::core::util::FstUtil;

/*
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
*/

void Add(FstBuilder<IntsRef>& builder,
         const std::string& key,
         const std::string& value) {
  BytesRef key_bytes(key.c_str(), key.size());
  // BytesRef key_bytes = BytesRef::MakeOwner(key);
  IntsRefBuilder key_ints;
  key_ints.CopyUTF8Bytes(key_bytes);

  // BytesRef val_bytes = BytesRef::MakeOwner(value);
  BytesRef val_bytes(value.c_str(), value.size());
  IntsRefBuilder val_ints;
  val_ints.CopyUTF8Bytes(val_bytes);

  builder.Add(std::move(key_ints.Get()), std::move(val_ints.Get()));
}

TEST(BYTESREF__TESTS, BASIC__TEST) {
  FstBuilder<IntsRef> builder(FST_INPUT_TYPE::BYTE1,
                              std::make_unique<IntSequenceOutputs>());
  std::string key;
  std::string val;
  // std::ifstream infile("/tmp/english-words/sorted-words.txt");
  // for (int i = 0 ; i < 466544 ; ++i) {
  //   std::getline(infile, key);
  //   val = std::to_string(i % 100000);
  //   // std::cout << i << "] Key -> " << key << ", Val -> " << val << std::endl;
  //   Add(builder, key, val);
  // }

  std::ifstream infile("/tmp/fst.input");
  for (int i = 0 ; i < 100000 ; ++i) {
    std::getline(infile, key);
    std::getline(infile, val);
    Add(builder, key, val);
  }

  std::cout << "-------- summary ------- " << std::endl;
  std::cout << "Arc -> " << builder.GetArcCount() << std::endl;
  std::cout << "Node -> " << builder.GetNodeCount() << std::endl;
  std::cout << "Term -> " << builder.GetTermCount() << std::endl;

  infile.close();

  Fst<IntsRef>* fst = builder.Finish();
  std::string path("/tmp/fst.output");
  BufferedFileOutputStream bos(path);
  fst->Save(&bos);
}

/*
TEST(NODE_HASH, BASIC__TEST) {
  FstBuilder<IntsRef> builder(FST_INPUT_TYPE::BYTE1,
                              std::make_unique<IntSequenceOutputs>());
  NodeHash<IntsRef>& node_hash = builder.GetNodeHash();

  //        [A]         [B]
  //         a           b
  // Node0 ----> Node1 ----> Node2
  std::cout << "Node2" << std::endl;
  FstBuilder<IntsRef>::UnCompiledNode* node2 =
    FstBuilder<IntsRef>::UnCompiledNode::New(&builder, 2);

  std::cout << "Compile Node2" << std::endl;
  FstBuilder<IntsRef>::CompiledNode* cnode2 = 
    FstBuilder<IntsRef>::CompiledNode::New(&builder,
                                           -1);
  std::cout << "Node1" << std::endl;
  FstBuilder<IntsRef>::UnCompiledNode* node1 =
    FstBuilder<IntsRef>::UnCompiledNode::New(&builder, 1);
  std::cout << "Add arc from Node1 to Node2 with label 'b'" << std::endl;
  node1->AddArc(static_cast<int32_t>('b'), cnode2);
  
  std::cout << "Compile Node1" << std::endl;
  FstBuilder<IntsRef>::CompiledNode* cnode1 = 
    FstBuilder<IntsRef>::CompiledNode::New(&builder,
                                           node_hash.Add(&builder, node1));
  std::cout << "Node0" << std::endl;
  FstBuilder<IntsRef>::UnCompiledNode* node0 =
    FstBuilder<IntsRef>::UnCompiledNode::New(&builder, 0);
  std::cout << "Add arc from Node0 to Node1 with label 'a'" << std::endl;
  node0->AddArc(static_cast<int32_t>('a'), cnode1);
}
*/

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
