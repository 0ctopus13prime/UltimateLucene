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

#include <assert.h>
#include <gtest/gtest.h>
#include <Store/Directory.h>
#include <Util/File.h>
#include <iostream>
#include <memory>
#include <string>

using lucene::core::store::ByteArrayReferenceDataInput;
using lucene::core::store::MMapDirectory;
using lucene::core::store::IndexInput;
using lucene::core::store::IndexOutput;
using lucene::core::store::IOContext;
using lucene::core::store::BytesArrayReferenceIndexInput;
using lucene::core::store::FileIndexOutput;
using lucene::core::store::GrowableByteArrayDataOutput;
using lucene::core::store::BufferedChecksumIndexInput;
using lucene::core::util::FileUtil;

TEST(DATA__OUTPUT__TESTS, FILE__INDEX__OUT) {
  FileUtil::Delete("/tmp/kdy");
  FileIndexOutput fio("A file index output",
                      "For testing",
                      "/tmp/kdy");

  const uint32_t buf_size = 100000;
  char buf[buf_size];
  for (int i = 0 ; i < buf_size ; ++i) {
    buf[i] = static_cast<char>(i);
  }

  fio.WriteBytes(buf, 0, buf_size);

  EXPECT_EQ(buf_size, fio.GetFilePointer());
  // Crc32 value from Java implementation
  EXPECT_EQ(2865713097, fio.GetChecksum());
}

TEST(DATA__INPUT__TESTS, BYTE__ARRAY__REFERENCE__DATA__INPUT) {
    char buf[] = {0x1, 0x2, 0x3, 0x4};
    ByteArrayReferenceDataInput bar_input(buf, 4);

  {
    // Read bytes and rewind
    ASSERT_EQ(0x1, bar_input.ReadByte());
    ASSERT_FALSE(bar_input.Eof());

    ASSERT_EQ(0x2, bar_input.ReadByte());
    ASSERT_FALSE(bar_input.Eof());

    ASSERT_EQ(0x3, bar_input.ReadByte());
    ASSERT_FALSE(bar_input.Eof());

    ASSERT_EQ(0x4, bar_input.ReadByte());
    ASSERT_TRUE(bar_input.Eof());

    bar_input.Rewind();
    ASSERT_EQ(0x1, bar_input.ReadByte());
    ASSERT_FALSE(bar_input.Eof());

    ASSERT_EQ(0x2, bar_input.ReadByte());
    ASSERT_FALSE(bar_input.Eof());

    ASSERT_EQ(0x3, bar_input.ReadByte());
    ASSERT_FALSE(bar_input.Eof());

    ASSERT_EQ(0x4, bar_input.ReadByte());
    ASSERT_TRUE(bar_input.Eof());
  }

  {
    bar_input.Rewind();
    char read_buf[4];
    bar_input.ReadBytes(read_buf, 0, 4);
    ASSERT_TRUE(bar_input.Eof());
    for (int i = 0 ; i < 4 ; ++i) {
      ASSERT_EQ(buf[i], read_buf[i]);
    }
  }
}

TEST(DATA__INPUT__TESTS, BYTE__ARRAY__REFERENCE__DATA__READ) {
  const uint32_t capacity = 1024;
  GrowableByteArrayDataOutput out(capacity);
  std::string str("content-");
  const uint32_t str_len = str.length();

  for (int i = 0 ; i < 100000 ; ++i) {
    out.WriteByte(static_cast<char>(i));
    out.WriteInt16(static_cast<int16_t>(i));
    out.WriteInt32(static_cast<int32_t>(i));
    out.WriteInt64(static_cast<int64_t>(i));
    out.WriteVInt32(static_cast<int32_t>(i));
    out.WriteZInt32(static_cast<int32_t>(i));
    out.WriteVInt64(static_cast<int64_t>(i));
    out.WriteZInt64(static_cast<int64_t>(i));
    str.resize(str_len);
    str += std::to_string(i);
    out.WriteString(str);
  }

  ByteArrayReferenceDataInput bardi(out.GetBytes(), 0, out.GetPosition()); 
  for (int i = 0 ; i < 100000 ; ++i) {
    ASSERT_EQ(static_cast<char>(i), bardi.ReadByte());
    ASSERT_EQ(static_cast<int16_t>(i), bardi.ReadInt16());
    ASSERT_EQ(static_cast<int32_t>(i), bardi.ReadInt32());
    ASSERT_EQ(static_cast<int64_t>(i), bardi.ReadInt64());
    ASSERT_EQ(static_cast<int32_t>(i), bardi.ReadVInt32());
    ASSERT_EQ(static_cast<int32_t>(i), bardi.ReadZInt32());
    ASSERT_EQ(static_cast<int64_t>(i), bardi.ReadVInt64());
    ASSERT_EQ(static_cast<int64_t>(i), bardi.ReadZInt64());
    str.resize(str_len);
    str += std::to_string(i);
    ASSERT_EQ(str, bardi.ReadString());
  }

  BytesArrayReferenceIndexInput barii("BytesArrayReferenceIndexInput",
                                      out.GetBytes(),
                                      out.GetPosition());
  for (int i = 0 ; i < 100000 ; ++i) {
    ASSERT_EQ(static_cast<char>(i), barii.ReadByte());
    ASSERT_EQ(static_cast<int16_t>(i), barii.ReadInt16());
    ASSERT_EQ(static_cast<int32_t>(i), barii.ReadInt32());
    ASSERT_EQ(static_cast<int64_t>(i), barii.ReadInt64());
    ASSERT_EQ(static_cast<int32_t>(i), barii.ReadVInt32());
    ASSERT_EQ(static_cast<int32_t>(i), barii.ReadZInt32());
    ASSERT_EQ(static_cast<int64_t>(i), barii.ReadVInt64());
    ASSERT_EQ(static_cast<int64_t>(i), barii.ReadZInt64());
    str.resize(str_len);
    str += std::to_string(i);
    ASSERT_EQ(str, barii.ReadString());
  }
}

TEST(DATA__INPUT__TESTS, BYTES__ARRAY__REFERENCE__INDEX__INPUT) {
  std::string name("BytesArrayReferenceIndexInput");
  char buf[] = {0x1, 0x2, 0x3, 0x4, 0x5};
  uint32_t buf_len = 5;
  BytesArrayReferenceIndexInput bar_ii(name, buf, buf_len);
}

TEST(DATA__INPUT__TESTS, CHECK__SUM__INDEX__INPUT) {
  const size_t elem_num = 130;
  const std::string base("/tmp");
  const std::string name("mmap_out_test");
  FileUtil::Delete(base + '/' + name);
  std::string str("content-");
  const uint32_t str_len = str.length();

  MMapDirectory dir(base);
  IOContext io_ctx;
  std::unique_ptr<IndexOutput> out_ptr = dir.CreateOutput(name, io_ctx);

  for (size_t i = 0 ; i < elem_num ; ++i) {
    out_ptr->WriteByte(static_cast<char>(i));
    out_ptr->WriteInt32(static_cast<int32_t>(i));
    out_ptr->WriteInt64(static_cast<int64_t>(i));
    out_ptr->WriteVInt32(static_cast<int32_t>(i));
    out_ptr->WriteVInt64(static_cast<int64_t>(i));
    out_ptr->WriteInt16(static_cast<int16_t>(i));
    out_ptr->WriteZInt64(static_cast<int64_t>(i));
    str.resize(str_len);
    str += std::to_string(i); 
    out_ptr->WriteString(str);
  }
  out_ptr->Close();

  // Read afterward
  std::unique_ptr<IndexInput> in_ptr = dir.OpenInput(name, io_ctx);
  BufferedChecksumIndexInput checksum_in(std::move(in_ptr));

  for (size_t i = 0 ; i < elem_num ; ++i) {
    ASSERT_EQ(static_cast<char>(i), checksum_in.ReadByte());
    ASSERT_EQ(static_cast<int32_t>(i), checksum_in.ReadInt32());
    ASSERT_EQ(static_cast<int64_t>(i), checksum_in.ReadInt64());
    ASSERT_EQ(static_cast<int32_t>(i), checksum_in.ReadVInt32());
    ASSERT_EQ(static_cast<int64_t>(i), checksum_in.ReadVInt64());
    ASSERT_EQ(static_cast<int16_t>(i), checksum_in.ReadInt16());
    ASSERT_EQ(static_cast<int64_t>(i), checksum_in.ReadZInt64());
    str.resize(str_len);
    str += std::to_string(i); 
    ASSERT_EQ(str, checksum_in.ReadString());
  }

  ASSERT_EQ(out_ptr->GetChecksum(), checksum_in.GetChecksum());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
