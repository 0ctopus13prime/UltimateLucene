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
#include <Store/DataInput.h>
#include <iostream>
#include <string>

using lucene::core::store::ByteArrayReferenceDataInput;
using lucene::core::store::BytesArrayReferenceIndexInput;

TEST(DATA__INPUT__TESTS, ByteArrayReferenceDataInput) {
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

TEST(DATA__INPUT__TESTS, BytesArrayReferenceIndexInput) {
  std::string name("BytesArrayReferenceIndexInput");
  char buf[] = {0x1, 0x2, 0x3, 0x4, 0x5};
  uint32_t buf_len = 5;
  BytesArrayReferenceIndexInput bar_ii(name, buf, buf_len);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
