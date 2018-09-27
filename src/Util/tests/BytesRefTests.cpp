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
#include <Util/Ref.h>
#include <iostream>

using lucene::core::util::BytesRef;

TEST(BYTESREF__TESTS, BASIC__TEST) {
  // Basic test
  std::string name = "doochi";
  BytesRef bytes_ref(name);

  EXPECT_EQ(0, bytes_ref.offset);
  EXPECT_EQ(6, bytes_ref.length);

  std::string got = bytes_ref.UTF8ToString();
  EXPECT_EQ("doochi", got);
}

TEST(BYTESREF__TESTS, SHALLOW__COPY__DEEP__COPY__FOR__EMPTY__INSTANCE) {
  // Shallow copy
  BytesRef bytes_ref1;
  BytesRef shared_bytes_ref2;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref2);  // Shallow copy
  EXPECT_EQ(bytes_ref1, shared_bytes_ref2);

  // Deep copy case
  BytesRef bytes_ref2(bytes_ref1);
  EXPECT_EQ(bytes_ref1, bytes_ref2);

  BytesRef bytes_ref3 = bytes_ref2;
  EXPECT_EQ(bytes_ref2, bytes_ref3);
}

TEST(BYTESREF__TESTS, SHALLOW__COPY__DEEP__COPY) {
  // Shallow copy
  std::string str = "doochi";
  BytesRef bytes_ref1(str);
  BytesRef shared_bytes_ref2;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref2);  // Shallow copy

  // Shallow case, Change single character at 0 index
  bytes_ref1.bytes.get()[0] = 'x';
  EXPECT_EQ('x', shared_bytes_ref2.bytes.get()[0]);

  // Deep copy case, Chage single character at 0 index
  BytesRef bytes_ref2(bytes_ref1);
  bytes_ref1.bytes.get()[0] = 'd';
  EXPECT_NE(bytes_ref2.bytes.get()[0], bytes_ref1.bytes.get()[0]);
  EXPECT_EQ('x', bytes_ref2.bytes.get()[0]);

  // operator =, assign
  BytesRef bytes_ref3 = bytes_ref2;  // Deep copy
  EXPECT_EQ(bytes_ref3, bytes_ref2);

  BytesRef shared_bytes_ref3;
  bytes_ref3.ShallowCopyTo(shared_bytes_ref3);

  EXPECT_EQ(shared_bytes_ref3, bytes_ref2);
}

TEST(BYTESREF__TESTS, EQ__COMPARE) {
  // Compare tests
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);
  BytesRef shared_bytes_ref1;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref1);
  BytesRef bytes_ref2(str1);
  BytesRef shared_bytes_ref2(bytes_ref2);

  EXPECT_EQ(bytes_ref1, bytes_ref2);
  EXPECT_EQ(shared_bytes_ref1, bytes_ref2);
  EXPECT_EQ(bytes_ref1, shared_bytes_ref2);
  EXPECT_EQ(shared_bytes_ref1, shared_bytes_ref2);
}

TEST(BYTESREF__TESTS, NOT__EQ__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);
  BytesRef shared_bytes_ref1;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

  BytesRef bytes_ref2(str2);
  BytesRef shared_bytes_ref2;
  bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

  EXPECT_NE(bytes_ref1, bytes_ref2);
  EXPECT_NE(bytes_ref1, shared_bytes_ref2);
  EXPECT_NE(shared_bytes_ref1, bytes_ref2);
  EXPECT_NE(shared_bytes_ref1, shared_bytes_ref2);
}

TEST(BYTESREF__TESTS, LESS__THEN__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);
  BytesRef shared_bytes_ref1;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

  BytesRef bytes_ref2(str2);
  BytesRef shared_bytes_ref2;
  bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

  EXPECT_LT(bytes_ref1, bytes_ref2);
  EXPECT_LT(bytes_ref1, shared_bytes_ref2);
  EXPECT_LT(shared_bytes_ref1, bytes_ref2);
  EXPECT_LT(shared_bytes_ref1, shared_bytes_ref2);

  BytesRef bytes_ref3(str3);
  BytesRef shared_bytes_ref3(bytes_ref3);
  EXPECT_LT(bytes_ref1, bytes_ref3);
  EXPECT_LT(bytes_ref1, shared_bytes_ref3);
  EXPECT_LT(shared_bytes_ref1, bytes_ref3);
  EXPECT_LT(shared_bytes_ref1, shared_bytes_ref3);
}

TEST(BYTESREF__TESTS, LESS__OR__EQ__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);
  BytesRef shared_bytes_ref1;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

  BytesRef bytes_ref2(str2);
  BytesRef shared_bytes_ref2;
  bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

  EXPECT_LE(bytes_ref1, bytes_ref2);
  EXPECT_LE(bytes_ref1, shared_bytes_ref2);
  EXPECT_LE(shared_bytes_ref1, bytes_ref2);
  EXPECT_LE(shared_bytes_ref1, shared_bytes_ref2);

  BytesRef bytes_ref3(str3);
  BytesRef shared_bytes_ref3(bytes_ref3);
  EXPECT_LE(bytes_ref1, bytes_ref3);
  EXPECT_LE(bytes_ref1, shared_bytes_ref3);
  EXPECT_LE(shared_bytes_ref1, bytes_ref3);
  EXPECT_LE(shared_bytes_ref1, shared_bytes_ref3);
}

TEST(BYTESREF__TESTS, GREATER__THEN__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);
  BytesRef shared_bytes_ref1;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

  BytesRef bytes_ref2(str2);
  BytesRef shared_bytes_ref2;
  bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

  EXPECT_GT(bytes_ref2, bytes_ref1);
  EXPECT_GT(bytes_ref2, shared_bytes_ref1);
  EXPECT_GT(shared_bytes_ref2, bytes_ref1);
  EXPECT_GT(shared_bytes_ref2, shared_bytes_ref1);

  BytesRef bytes_ref3(str3);
  BytesRef shared_bytes_ref3(bytes_ref3);
  EXPECT_GT(bytes_ref3, bytes_ref1);
  EXPECT_GT(bytes_ref3, shared_bytes_ref1);
  EXPECT_GT(shared_bytes_ref3, bytes_ref1);
  EXPECT_GT(shared_bytes_ref3, shared_bytes_ref1);
}

TEST(BYTESREF__TESTS, GREATER__OR__EQ__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);
  BytesRef shared_bytes_ref1;
  bytes_ref1.ShallowCopyTo(shared_bytes_ref1);

  BytesRef bytes_ref2(str2);
  BytesRef shared_bytes_ref2;
  bytes_ref2.ShallowCopyTo(shared_bytes_ref2);

  EXPECT_GE(bytes_ref2, bytes_ref1);
  EXPECT_GE(bytes_ref2, shared_bytes_ref1);
  EXPECT_GE(shared_bytes_ref2, bytes_ref1);
  EXPECT_GE(shared_bytes_ref2, shared_bytes_ref1);

  BytesRef bytes_ref3(str3);
  BytesRef shared_bytes_ref3(bytes_ref3);
  EXPECT_GE(bytes_ref3, bytes_ref1);
  EXPECT_GE(bytes_ref3, shared_bytes_ref1);
  EXPECT_GE(shared_bytes_ref3, bytes_ref1);
  EXPECT_GE(shared_bytes_ref3, shared_bytes_ref1);
}

TEST(BYTESREF__TESTS, VALIDATION__TEST) {
  BytesRef bytes_ref;
  bytes_ref.offset = 1;  // nullptr && offset > length
  try {
    bytes_ref.IsValid();
    assert(false);  // Fail here
  } catch(...) {
    // Ignore, Exception must be caught in here.
  }
}


int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
