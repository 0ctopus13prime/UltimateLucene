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

using lucene::core::util::BytesStore;
using lucene::core::util::FSTBytesReader;
using lucene::core::util::ForwardFSTBytesReader;
using lucene::core::util::ReverseFSTBytesReader;
using lucene::core::util::BytesStoreForwardFSTBytesReader;
using lucene::core::util::BytesStoreReverseFSTBytesReader;

TEST(BYTES__STORE__TEST, BASIC__TESTS) {
  // Write byte
  const uint32_t block_bits = 5;
  BytesStore bs(block_bits);
  bs.WriteByte(13);
  ASSERT_EQ(1, bs.GetPosition());

  for (int i = 0 ; i < 127 ; ++i) {
    bs.WriteByte(static_cast<char>(i));
  }
  ASSERT_EQ(1 + 127, bs.GetPosition());

  // Bulk write bytes
  std::unique_ptr<char[]> bulk(std::make_unique<char[]>(8 * 1231));

  bs.WriteBytes(bulk.get(), 0, 8 * 1231);
  ASSERT_EQ(1 + 127 + 8 * 1231, bs.GetPosition());
  bs.WriteBytes(bulk.get(), 0, 8 * 1231);
  ASSERT_EQ(1 + 127 + 2 * 8 * 1231, bs.GetPosition());

  // Skip bytes
  bs.SkipBytes(1234);
  ASSERT_EQ(1 + 127 + 2 * 8 * 1231 + 1234, bs.GetPosition());

  // In place byte write
  for (int i = 0 ; i < bs.GetPosition() ; ++i) {
    bs.WriteByte(i, static_cast<char>(i));
  }

  // In place bytes write
  bs.WriteBytes(229, bulk.get(), 0, 8 * 1231);

  // Write int32
  for (int i = 0 ; i < 100 ; ++i) {
    bs.WriteInt32(i, i);
  }

  // Reverse
  bs.Reverse(0, bs.GetPosition() - 1);

  // In place copy bytes
  bs.CopyBytes(0, 1231, 509);

  // Truncate
  bs.Truncate(613);
  ASSERT_EQ(613, bs.GetPosition());

  // Finish
  const uint32_t size_before_finish = bs.GetPosition();
  bs.Finish();
  ASSERT_EQ(size_before_finish, bs.GetPosition());
}

TEST(FORWARD__BYTES__READER, BASIC__TEST) {
  const uint32_t block_bits = 5;
  BytesStore bs(block_bits);
  
  // Fill 10 bytes
  for (int i = 0 ; i < 10 ; ++i) {
    bs.WriteByte(static_cast<char>(i));
  }

  std::unique_ptr<FSTBytesReader> reader =
    bs.GetForwardReader();

  ForwardFSTBytesReader* fbr = dynamic_cast<ForwardFSTBytesReader*>(reader.get());
  ASSERT_NE(nullptr, fbr);
 
  for (uint32_t i = 0 ; i < bs.GetPosition() ; ++i) {
    ASSERT_EQ(static_cast<char>(i), fbr->ReadByte());
  }

  char buffer[bs.GetPosition()];
  fbr->SetPosition(0);
  fbr->ReadBytes(buffer, 0, bs.GetPosition());

  for (uint32_t i = 0 ; i < bs.GetPosition() ; ++i) {
    ASSERT_EQ(static_cast<char>(i), buffer[i]);
  }
}

TEST(REVERSE__BYTES__READER, BASIC__TEST) {
  const uint32_t block_bits = 5;
  BytesStore bs(block_bits);
  
  // Fill 10 bytes
  for (int i = 0 ; i < 10 ; ++i) {
    bs.WriteByte(static_cast<char>(i));
  }

  std::unique_ptr<FSTBytesReader> reader =
    bs.GetReverseReader();

  ReverseFSTBytesReader* rbr = dynamic_cast<ReverseFSTBytesReader*>(reader.get());
  ASSERT_NE(nullptr, rbr);
  // Rewind to end
  rbr->SetPosition(bs.GetPosition() - 1);
 
  for (int32_t i = bs.GetPosition() - 1 ; i >=0 ; --i) {
    ASSERT_EQ(static_cast<char>(i), rbr->ReadByte());
  }

  rbr->SetPosition(bs.GetPosition() - 1);
  char buffer[bs.GetPosition()];
  rbr->ReadBytes(buffer, 0, bs.GetPosition());

  for (uint32_t i = 0 ; i < bs.GetPosition() ; ++i) {
    ASSERT_EQ(static_cast<char>(bs.GetPosition() - 1 - i), buffer[i]);
  }
}

TEST(REVERSE__BYTES__READER, BULK__BASIC__TEST) {
  const uint32_t block_bits = 5;
  BytesStore bs(block_bits);

  const uint32_t bulk_bytes_size = 8317;
  // Write ${bulk_bytes_size} bytes
  for (uint32_t i = 0 ; i < bulk_bytes_size ; ++i) {
    bs.WriteByte(static_cast<char>(i));
  }
 
  std::unique_ptr<FSTBytesReader> reader = bs.GetForwardReader();   
  BytesStoreForwardFSTBytesReader* fbr = dynamic_cast<BytesStoreForwardFSTBytesReader*>(reader.get());
  ASSERT_NE(nullptr, fbr);

  // Read byte
  for (uint32_t i = 0 ; i < bulk_bytes_size ; ++i) {
    ASSERT_EQ(static_cast<char>(i), fbr->ReadByte());
  }

  // Read bytes
  char buffer[bulk_bytes_size];
  fbr->SetPosition(0);
  fbr->ReadBytes(buffer, 0, bulk_bytes_size);
  for (uint32_t i = 0 ; i < bulk_bytes_size ; ++i) {
    ASSERT_EQ(static_cast<char>(i), buffer[i]);
  }

  // Skip 500 bytes
  fbr->SetPosition(500);
  ASSERT_EQ(500, fbr->GetPosition());
  for (uint32_t i = 500 ; i < bulk_bytes_size ; ++i) {
    ASSERT_EQ(static_cast<char>(i), fbr->ReadByte());
  }
}

TEST(FORWARD__BYTES__READER, BULK__BASIC__TEST) {
  const uint32_t block_bits = 5;
  BytesStore bs(block_bits);

  const uint32_t bulk_bytes_size = 8317;
  // Write ${bulk_bytes_size} bytes
  for (uint32_t i = 0 ; i < bulk_bytes_size ; ++i) {
    bs.WriteByte(static_cast<char>(i));
  }
 
  std::unique_ptr<FSTBytesReader> reader = bs.GetReverseReader();   
  BytesStoreReverseFSTBytesReader* rbr = dynamic_cast<BytesStoreReverseFSTBytesReader*>(reader.get());
  ASSERT_NE(nullptr, rbr);

  // Read byte
  rbr->SetPosition(bs.GetPosition() - 1);
  for (uint32_t i = 0 ; i < bulk_bytes_size ; ++i) {
    ASSERT_EQ(static_cast<char>(bulk_bytes_size - 1 - i), rbr->ReadByte());
  }

  // Read bytes
  char buffer[bulk_bytes_size];
  rbr->SetPosition(bs.GetPosition() - 1);
  rbr->ReadBytes(buffer, 0, bulk_bytes_size);
  for (uint32_t i = 0 ; i < bulk_bytes_size ; ++i) {
    ASSERT_EQ(static_cast<char>(bulk_bytes_size - 1 - i), buffer[i]);
  }

  // Skip 500 bytes
  rbr->SetPosition(bs.GetPosition() - 1);
  rbr->SkipBytes(500);
  ASSERT_EQ(bs.GetPosition() - 1 - 500, rbr->GetPosition());

  for (uint32_t i = 0 ; i < rbr->GetPosition() ; ++i) {
    ASSERT_EQ(static_cast<char>(bulk_bytes_size - 1 - 500 - i), rbr->ReadByte());
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
