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
using lucene::core::util::BytesRefBuilder;
using lucene::core::util::IntsRef;
using lucene::core::util::IntsRefBuilder;
using lucene::core::util::LongsRef;

TEST(BYTESREF__TESTS, BASIC__TEST) {
  // Basic test
  std::string name = "doochi";
  
  // Copy name
  BytesRef bytes_ref(name);

  ASSERT_EQ(0, bytes_ref.Offset());
  ASSERT_EQ(6, bytes_ref.Length());

  std::string got = bytes_ref.UTF8ToString();
  ASSERT_EQ("doochi", got);
}

TEST(BYTESREF__TESTS, SHALLOW__COPY__DEEP__COPY__FOR__EMPTY__INSTANCE) {
  // Shallow copy
  std::string data = "doochi";
  BytesRef bytes_ref1(data); // Owning
  ASSERT_EQ(bytes_ref1.Offset(), 0);
  ASSERT_EQ(bytes_ref1.Length(), data.size());
  ASSERT_EQ(bytes_ref1.Capacity(), data.size());

  BytesRef shared_bytes_ref2; // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref1); // Shallow copy
  ASSERT_EQ(shared_bytes_ref2.Offset(), 0);
  ASSERT_EQ(shared_bytes_ref2.Length(), data.size());
  ASSERT_EQ(shared_bytes_ref2.Capacity(), data.size());

  ASSERT_EQ(bytes_ref1, shared_bytes_ref2);

  // Deep copy case
  BytesRef bytes_ref2(bytes_ref1); // Owning
  ASSERT_EQ(bytes_ref1, bytes_ref2);

  BytesRef bytes_ref3 = bytes_ref2; //Owning
  ASSERT_EQ(bytes_ref2, bytes_ref3);
}

TEST(BYTESREF__TESTS, SHALLOW__COPY__DEEP__COPY) {
  // Shallow copy
  std::string str = "doochi";
  BytesRef bytes_ref1(str); // Owning
  BytesRef shared_bytes_ref2;
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref1); // Shallow copy

  // Shallow case, Change single character at 0 index
  bytes_ref1.Bytes()[0] = 'x';
  ASSERT_EQ('x', shared_bytes_ref2.Bytes()[0]);

  // Deep copy case, Chage single character at 0 index
  BytesRef bytes_ref2(bytes_ref1);
  bytes_ref1.Bytes()[0] = 'd';
  ASSERT_NE(bytes_ref2.Bytes()[0], bytes_ref1.Bytes()[0]);
  ASSERT_EQ('x', bytes_ref2.Bytes()[0]);

  // operator =, assign
  BytesRef bytes_ref3 = bytes_ref2;  // Deep copy
  ASSERT_EQ(bytes_ref3, bytes_ref2);

  BytesRef shared_bytes_ref3;
  BytesRef::MakeReference(shared_bytes_ref3, bytes_ref3); // Shallow copy

  ASSERT_EQ(shared_bytes_ref3, bytes_ref2);
}

TEST(BYTESREF__TESTS, EQ__COMPARE) {
  // Compare tests
  std::string str1 = "doochi stupid";

  BytesRef bytes_ref1(str1);  // Owning
  BytesRef shared_bytes_ref1;  // Reference
  BytesRef::MakeReference(shared_bytes_ref1, bytes_ref1); // Shallow copy
  BytesRef bytes_ref1_cp;
  BytesRef::MakeOwner(bytes_ref1_cp, bytes_ref1);  // Owning

  BytesRef bytes_ref2(str1); // Owning
  BytesRef shared_bytes_ref2;  // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref2); // Shallow copy
  BytesRef bytes_ref2_cp;
  BytesRef::MakeOwner(bytes_ref2_cp, bytes_ref2);  // Owning

  ASSERT_EQ(bytes_ref1, bytes_ref1);
  ASSERT_EQ(bytes_ref1, bytes_ref2);
  ASSERT_EQ(shared_bytes_ref1, shared_bytes_ref1);
  ASSERT_EQ(shared_bytes_ref1, bytes_ref2);
  ASSERT_EQ(bytes_ref1, shared_bytes_ref2);
  ASSERT_EQ(shared_bytes_ref1, shared_bytes_ref2);
  ASSERT_EQ(bytes_ref1_cp, bytes_ref1_cp);
  ASSERT_EQ(bytes_ref1_cp, bytes_ref2_cp);
  ASSERT_EQ(bytes_ref1_cp, bytes_ref1);
  ASSERT_EQ(bytes_ref1_cp, bytes_ref2);
  ASSERT_EQ(bytes_ref2_cp, bytes_ref2_cp);
  ASSERT_EQ(bytes_ref2_cp, bytes_ref2);
  ASSERT_EQ(bytes_ref2_cp, bytes_ref1);
}

TEST(BYTESREF__TESTS, NOT__EQ__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);  // Owning
  BytesRef shared_bytes_ref1; // Reference
  BytesRef::MakeReference(shared_bytes_ref1, bytes_ref1); // Shallow copy

  BytesRef bytes_ref2(str2);  // Owning
  BytesRef shared_bytes_ref2;  // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref2); // Shallow copy

  ASSERT_NE(bytes_ref1, bytes_ref2);
  ASSERT_NE(bytes_ref1, shared_bytes_ref2);
  ASSERT_NE(shared_bytes_ref1, bytes_ref2);
  ASSERT_NE(shared_bytes_ref1, shared_bytes_ref2);
}

TEST(BYTESREF__TESTS, LESS__THEN__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);  // Owning
  BytesRef shared_bytes_ref1;  // Reference
  BytesRef::MakeReference(shared_bytes_ref1, bytes_ref1); // Shallow copy

  BytesRef bytes_ref2(str2);  // Owning
  BytesRef shared_bytes_ref2;  // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref2); // Shallow copy

  ASSERT_LT(bytes_ref1, bytes_ref2);
  ASSERT_LT(bytes_ref1, shared_bytes_ref2);
  ASSERT_LT(shared_bytes_ref1, bytes_ref2);
  ASSERT_LT(shared_bytes_ref1, shared_bytes_ref2);

  BytesRef bytes_ref3(str3);  // Owning
  BytesRef shared_bytes_ref3;  // Reference
  BytesRef::MakeReference(shared_bytes_ref3, bytes_ref3); // Shallow copy

  ASSERT_LT(bytes_ref1, bytes_ref3);
  ASSERT_LT(bytes_ref1, shared_bytes_ref3);
  ASSERT_LT(shared_bytes_ref1, bytes_ref3);
  ASSERT_LT(shared_bytes_ref1, shared_bytes_ref3);
}

TEST(BYTESREF__TESTS, LESS__OR__EQ__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1); // Owning
  BytesRef shared_bytes_ref1; // Reference
  BytesRef::MakeReference(shared_bytes_ref1, bytes_ref1); // Shallow copy

  BytesRef bytes_ref2(str2);  // Owning
  BytesRef shared_bytes_ref2;  // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref2); // Shallow copy

  ASSERT_LE(bytes_ref1, bytes_ref2);
  ASSERT_LE(bytes_ref1, shared_bytes_ref2);
  ASSERT_LE(shared_bytes_ref1, bytes_ref2);
  ASSERT_LE(shared_bytes_ref1, shared_bytes_ref2);

  BytesRef bytes_ref3(str3);  // Owning
  BytesRef shared_bytes_ref3;  // Reference
  BytesRef::MakeReference(shared_bytes_ref3, bytes_ref3); // Shallow copy
  ASSERT_LE(bytes_ref1, bytes_ref3);
  ASSERT_LE(bytes_ref1, shared_bytes_ref3);
  ASSERT_LE(shared_bytes_ref1, bytes_ref3);
  ASSERT_LE(shared_bytes_ref1, shared_bytes_ref3);
}

TEST(BYTESREF__TESTS, GREATER__THEN__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1);  // Owning
  BytesRef shared_bytes_ref1;  // Reference
  BytesRef::MakeReference(shared_bytes_ref1, bytes_ref1); // Shallow copy

  BytesRef bytes_ref2(str2);  // Owning
  BytesRef shared_bytes_ref2;  // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref2); // Shallow copy

  ASSERT_GT(bytes_ref2, bytes_ref1);
  ASSERT_GT(bytes_ref2, shared_bytes_ref1);
  ASSERT_GT(shared_bytes_ref2, bytes_ref1);
  ASSERT_GT(shared_bytes_ref2, shared_bytes_ref1);

  BytesRef bytes_ref3(str3);  // Owning
  BytesRef shared_bytes_ref3;  // Reference
  BytesRef::MakeReference(shared_bytes_ref3, bytes_ref3); // Shallow copy

  ASSERT_GT(bytes_ref3, bytes_ref1);
  ASSERT_GT(bytes_ref3, shared_bytes_ref1);
  ASSERT_GT(shared_bytes_ref3, bytes_ref1);
  ASSERT_GT(shared_bytes_ref3, shared_bytes_ref1);
}

TEST(BYTESREF__TESTS, GREATER__OR__EQ__TEST) {
  std::string str1 = "doochi stupid";
  std::string str2 = str1 + " suffix";
  std::string str3 = "ugly doochi";

  BytesRef bytes_ref1(str1); // Owning
  BytesRef shared_bytes_ref1;  // Reference
  BytesRef::MakeReference(shared_bytes_ref1, bytes_ref1); // Shallow copy

  BytesRef bytes_ref2(str2); // Owngin
  BytesRef shared_bytes_ref2;  // Reference
  BytesRef::MakeReference(shared_bytes_ref2, bytes_ref2); // Shallow copy

  ASSERT_GE(bytes_ref2, bytes_ref1);
  ASSERT_GE(bytes_ref2, shared_bytes_ref1);
  ASSERT_GE(shared_bytes_ref2, bytes_ref1);
  ASSERT_GE(shared_bytes_ref2, shared_bytes_ref1);

  BytesRef bytes_ref3(str3);  // Owning
  BytesRef shared_bytes_ref3;  // Reference
  BytesRef::MakeReference(shared_bytes_ref3, bytes_ref3); // Shallow copy

  ASSERT_GE(bytes_ref3, bytes_ref1);
  ASSERT_GE(bytes_ref3, shared_bytes_ref1);
  ASSERT_GE(shared_bytes_ref3, bytes_ref1);
  ASSERT_GE(shared_bytes_ref3, shared_bytes_ref1);
}

TEST(BYTESREF__TESTS, RELATION__TEST) {
  std::string str = "doochi test";

  // Move ctor, ownership transfer
  {
    BytesRef bytes_ref1(str);  // Owning
    BytesRef bytes_ref2(std::move(bytes_ref1));  // Move, Owning
    BytesRef bytes_ref3(std::move(bytes_ref2));  // Move, Owning

    ASSERT_EQ(bytes_ref1, bytes_ref2);
    ASSERT_EQ(bytes_ref2, bytes_ref3);
    ASSERT_EQ(bytes_ref1, bytes_ref3);
    ASSERT_EQ(str.size(), bytes_ref1.Length());
    ASSERT_EQ(str.size(), bytes_ref2.Length());
    ASSERT_EQ(str.size(), bytes_ref3.Length());
  }

  // Copy ctor, dual ownership
  {
    BytesRef bytes_ref1(str);  // Owning
    BytesRef bytes_ref2(bytes_ref1);  // Owning
    BytesRef bytes_ref3(bytes_ref2);  // Owning

    ASSERT_EQ(bytes_ref1, bytes_ref2);
    ASSERT_EQ(bytes_ref2, bytes_ref3);
    ASSERT_EQ(bytes_ref1, bytes_ref3);
    ASSERT_EQ(str.size(), bytes_ref1.Length());
    ASSERT_EQ(str.size(), bytes_ref2.Length());
    ASSERT_EQ(str.size(), bytes_ref3.Length());
  }

  // Other ctors
  {
    BytesRef bytes_ref1(str.c_str(), 0, str.size(), str.size());  // Reference
    BytesRef bytes_ref2(str.c_str(), 0, str.size());  // Reference
    BytesRef bytes_ref3(str.c_str(), str.size());  // Reference
    BytesRef bytes_ref4(str);  // Owning

    ASSERT_EQ(bytes_ref1, bytes_ref2);
    ASSERT_EQ(bytes_ref1, bytes_ref3);
    ASSERT_EQ(bytes_ref1, bytes_ref4);
    ASSERT_EQ(bytes_ref2, bytes_ref3);
    ASSERT_EQ(bytes_ref2, bytes_ref4);
    ASSERT_EQ(bytes_ref3, bytes_ref4);

    const uint32_t capacity = str.size();
    BytesRef bytes_ref5(capacity);  // Owning

    ASSERT_EQ(str.size(), bytes_ref5.Capacity());
    ASSERT_EQ(0, bytes_ref5.Offset());
  }

  // operator = copy
  {
    BytesRef bytes_ref1(str);  // Owning
    BytesRef bytes_ref2 = bytes_ref1;  // Owning
    ASSERT_EQ(bytes_ref1, bytes_ref2);
    ASSERT_EQ(bytes_ref2.Length(), bytes_ref1.Length());
    ASSERT_EQ(bytes_ref2.Offset(), bytes_ref1.Offset());
    ASSERT_EQ(bytes_ref2.Capacity(), bytes_ref1.Capacity());
    ASSERT_NE(bytes_ref2.Bytes(), bytes_ref1.Bytes());
  }

  // operator = move
  {
    BytesRef bytes_ref1(str);  // Owning
    // Ownership transfer bytes_ref1 -> bytes_ref2
    BytesRef bytes_ref2 = std::move(bytes_ref1);  
    ASSERT_EQ(bytes_ref1, bytes_ref2);
    ASSERT_EQ(bytes_ref2.Length(), bytes_ref1.Length());
    ASSERT_EQ(bytes_ref2.Offset(), bytes_ref1.Offset());
    ASSERT_EQ(bytes_ref2.Capacity(), bytes_ref1.Capacity());
    ASSERT_EQ(bytes_ref2.Bytes(), bytes_ref1.Bytes());
  }
}

TEST(INTSREF__TESTS, BASIC__TEST1) {
  // Basic test
  int32_t stream[10] = {1,3,2,4,3,5,4,6,5,9};

  // Reference
  IntsRef ints_ref1(stream, 10);

  ASSERT_EQ(0, ints_ref1.Offset());
  ASSERT_EQ(10, ints_ref1.Length());
  ASSERT_EQ(10, ints_ref1.Capacity());

  // Reference, by copy ctor
  IntsRef ints_ref2(ints_ref1);
  ASSERT_EQ(ints_ref1.Ints(), ints_ref2.Ints());
  ASSERT_EQ(0, ints_ref2.Offset());
  ASSERT_EQ(10, ints_ref2.Length());
  ASSERT_EQ(10, ints_ref2.Capacity());
  ASSERT_EQ(ints_ref1, ints_ref2);

  // Reference, by move ctor
  IntsRef ints_ref3;
  IntsRef::MakeReference(ints_ref3, ints_ref2);
  ASSERT_EQ(ints_ref2.Ints(), ints_ref3.Ints());
  ASSERT_EQ(0, ints_ref3.Offset());
  ASSERT_EQ(10, ints_ref3.Length());
  ASSERT_EQ(10, ints_ref3.Capacity());
  ASSERT_EQ(ints_ref2, ints_ref3);

  // Owning, by static helper method
  IntsRef ints_ref4;
  IntsRef::MakeOwner(ints_ref4, ints_ref3);
  ASSERT_NE(ints_ref3.Ints(), ints_ref4.Ints());
  ASSERT_EQ(0, ints_ref4.Offset());
  ASSERT_EQ(10, ints_ref4.Length());
  ASSERT_EQ(10, ints_ref4.Capacity());
  ASSERT_EQ(ints_ref3, ints_ref4);

  // Owning, by copy ctor
  IntsRef ints_ref5;
  IntsRef::MakeOwner(ints_ref5, ints_ref4);
  ASSERT_NE(ints_ref4.Ints(), ints_ref5.Ints());
  ASSERT_EQ(0, ints_ref5.Offset());
  ASSERT_EQ(10, ints_ref5.Length());
  ASSERT_EQ(10, ints_ref5.Capacity());
  ASSERT_EQ(ints_ref4, ints_ref5);
}

TEST(INTSREF__TESTS, BASIC__TEST2) {
  // Basic test
  int32_t stream[10] = {1,3,2,4,3,5,4,6,5,9};

  // Reference
  IntsRef ints_ref1(stream, 10);

  // Reference, by assigning
  IntsRef ints_ref2 = ints_ref1;

  ASSERT_EQ(ints_ref1.Ints(), ints_ref2.Ints());
  ASSERT_EQ(0, ints_ref2.Offset());
  ASSERT_EQ(10, ints_ref2.Length());
  ASSERT_EQ(10, ints_ref2.Capacity());
  ASSERT_EQ(ints_ref1, ints_ref2);

  // Owning, by helper method
  IntsRef ints_ref3;
  IntsRef::MakeOwner(ints_ref3, ints_ref2);

  // Reference, ints_ref2 is a reference
  IntsRef ints_ref4(ints_ref2);

  // Now it is having own resource, because ints_ref3 is an owner
  ints_ref4 = ints_ref3;
  ASSERT_NE(ints_ref4.Ints(), ints_ref3.Ints());
  ASSERT_EQ(0, ints_ref4.Offset());
  ASSERT_EQ(10, ints_ref4.Length());
  ASSERT_EQ(10, ints_ref4.Capacity());
  ASSERT_EQ(ints_ref3, ints_ref4);
}

TEST(INTSREF__TEST, CMP__TEST) {
  int32_t stream1[8] = {1,2,3,4,5,6,7,8};
  int32_t stream2[7] = {1,3,2,4,3,5,4};
  int32_t stream3[10] = {1,3,2,4,3,5,4,6,5,9};
  int32_t stream4[4] = {9,9,9,9};

  IntsRef ints[4] = {
    IntsRef(stream1, 8),
    IntsRef(stream2, 7),
    IntsRef(stream3, 10),
    IntsRef(stream4, 4)
  };

  // Self == Self
  for (int32_t i = 0 ; i < 4 ; ++i) {
    ASSERT_EQ(ints[i], ints[i]);
  }

  // Self != Self
  for (int32_t i = 0 ; i < 4 ; ++i) {
    for (int32_t j = i + 1 ; j < 4 ; ++j) {
      ASSERT_NE(ints[i], ints[j]);
    }
  }

  // Self <= Self
  // Self >= Self
  for (int32_t i = 0 ; i < 4 ; ++i) {
    ASSERT_LE(ints[i], ints[i]);
    ASSERT_GE(ints[i], ints[i]);
  }

  // X < Y
  for (int32_t i = 0 ; i < 4 ; ++i) {
    for (int32_t j = i + 1 ; j < 4 ; ++j) {
      ASSERT_LT(ints[i], ints[j]);
    }
  }

  // X > Y
  for (int32_t i = 0 ; i < 4 ; ++i) {
    for (int32_t j = i + 1 ; j < 4 ; ++j) {
      ASSERT_GT(ints[j], ints[i]);
    }
  }
}

TEST(LONGSREF__TESTS, BASIC__TEST1) {
  // Basic test
  int64_t stream[10] = {1,3,2,4,3,5,4,6,5,9};

  // Reference
  LongsRef longs_ref1(stream, 10);

  ASSERT_EQ(0, longs_ref1.Offset());
  ASSERT_EQ(10, longs_ref1.Length());
  ASSERT_EQ(10, longs_ref1.Capacity());

  // Reference, by copy ctor
  LongsRef longs_ref2(longs_ref1);
  ASSERT_EQ(longs_ref1.Longs(), longs_ref2.Longs());
  ASSERT_EQ(0, longs_ref2.Offset());
  ASSERT_EQ(10, longs_ref2.Length());
  ASSERT_EQ(10, longs_ref2.Capacity());
  ASSERT_EQ(longs_ref1, longs_ref2);

  // Reference, by move ctor
  LongsRef longs_ref3;
  LongsRef::MakeReference(longs_ref3, longs_ref2);
  ASSERT_EQ(longs_ref2.Longs(), longs_ref3.Longs());
  ASSERT_EQ(0, longs_ref3.Offset());
  ASSERT_EQ(10, longs_ref3.Length());
  ASSERT_EQ(10, longs_ref3.Capacity());
  ASSERT_EQ(longs_ref2, longs_ref3);

  // Owning, by static helper method
  LongsRef longs_ref4;
  LongsRef::MakeOwner(longs_ref4, longs_ref3);
  ASSERT_NE(longs_ref3.Longs(), longs_ref4.Longs());
  ASSERT_EQ(0, longs_ref4.Offset());
  ASSERT_EQ(10, longs_ref4.Length());
  ASSERT_EQ(10, longs_ref4.Capacity());
  ASSERT_EQ(longs_ref3, longs_ref4);

  // Owning, by copy ctor
  LongsRef longs_ref5;
  LongsRef::MakeOwner(longs_ref5, longs_ref4);
  ASSERT_NE(longs_ref4.Longs(), longs_ref5.Longs());
  ASSERT_EQ(0, longs_ref5.Offset());
  ASSERT_EQ(10, longs_ref5.Length());
  ASSERT_EQ(10, longs_ref5.Capacity());
  ASSERT_EQ(longs_ref4, longs_ref5);
}

TEST(LONGSREF__TESTS, BASIC__TEST2) {
  // Basic test
  int64_t stream[10] = {1,3,2,4,3,5,4,6,5,9};

  // Reference
  LongsRef longs_ref1(stream, 10);

  // Reference, by assigning
  LongsRef longs_ref2 = longs_ref1;

  ASSERT_EQ(longs_ref1.Longs(), longs_ref2.Longs());
  ASSERT_EQ(0, longs_ref2.Offset());
  ASSERT_EQ(10, longs_ref2.Length());
  ASSERT_EQ(10, longs_ref2.Capacity());
  ASSERT_EQ(longs_ref1, longs_ref2);

  // Owning, by helper method
  LongsRef longs_ref3;
  LongsRef::MakeOwner(longs_ref3, longs_ref2);

  // Reference, longs_ref2 is a reference
  LongsRef longs_ref4(longs_ref2);

  // Now it is having own resource, because longs_ref3 is an owner
  longs_ref4 = longs_ref3;
  ASSERT_NE(longs_ref4.Longs(), longs_ref3.Longs());
  ASSERT_EQ(0, longs_ref4.Offset());
  ASSERT_EQ(10, longs_ref4.Length());
  ASSERT_EQ(10, longs_ref4.Capacity());
  ASSERT_EQ(longs_ref3, longs_ref4);
}

TEST(LONGSREF__TEST, CMP__TEST) {
  int64_t stream1[8] = {1,2,3,4,5,6,7,8};
  int64_t stream2[7] = {1,3,2,4,3,5,4};
  int64_t stream3[10] = {1,3,2,4,3,5,4,6,5,9};
  int64_t stream4[4] = {9,9,9,9};

  LongsRef longs[4] = {
    LongsRef(stream1, 8),
    LongsRef(stream2, 7),
    LongsRef(stream3, 10),
    LongsRef(stream4, 4)
  };

  // Self == Self
  for (int32_t i = 0 ; i < 4 ; ++i) {
    ASSERT_EQ(longs[i], longs[i]);
  }

  // Self != Self
  for (int32_t i = 0 ; i < 4 ; ++i) {
    for (int32_t j = i + 1 ; j < 4 ; ++j) {
      ASSERT_NE(longs[i], longs[j]);
    }
  }

  // Self <= Self
  // Self >= Self
  for (int32_t i = 0 ; i < 4 ; ++i) {
    ASSERT_LE(longs[i], longs[i]);
    ASSERT_GE(longs[i], longs[i]);
  }

  // X < Y
  for (int32_t i = 0 ; i < 4 ; ++i) {
    for (int32_t j = i + 1 ; j < 4 ; ++j) {
      ASSERT_LT(longs[i], longs[j]);
    }
  }

  // X > Y
  for (int32_t i = 0 ; i < 4 ; ++i) {
    for (int32_t j = i + 1 ; j < 4 ; ++j) {
      ASSERT_GT(longs[j], longs[i]);
    }
  }
}

TEST(BYTES__BUILDER__TEST, BASIC__TESTS) {
  BytesRefBuilder builder; 
  std::string data("doochi BytesRefBuilder");
  for (const char ch : data) {
    builder.Append(ch);
  }

  std::string got(builder.Get().UTF8ToString());
  ASSERT_EQ(got, data);

  builder.Clear();
  builder.CopyBytes(data.c_str(), 0, data.size());
  got = builder.Get().UTF8ToString();
  ASSERT_EQ(got, data);

  BytesRef duped(builder.DupBytesRef());
  ASSERT_EQ(duped, builder.Get());

  std::string another_data("doochi doochi doochi!!");
  // Reference
  BytesRef bytes_ref1(another_data.c_str(), another_data.size());
  builder.CopyBytes(bytes_ref1);
  got = builder.Get().UTF8ToString();
  ASSERT_EQ(got, another_data);

  builder.Append(bytes_ref1);
  got = builder.Get().UTF8ToString();
  ASSERT_EQ(got, another_data + another_data);

  std::string final_data("Why are you crying?");
  BytesRefBuilder builder2;
  builder2.CopyBytes(final_data.c_str(), 0, final_data.size());
  builder.CopyBytes(builder2);
  got = builder.Get().UTF8ToString();
  ASSERT_EQ(got, final_data);
}

TEST(INTS__BUILDER__TEST, BASIC__TESTS) {
  std::string data("doochi BytesRefBuilder");
  BytesRef bytes_ref1(data);
  IntsRefBuilder builder; 
  builder.CopyUTF8Bytes(bytes_ref1);
  // Ascii. Size must be equal
  ASSERT_EQ(data.size(), builder.Length());

  for (int i = 0 ; i < data.size() ; ++i) {
    ASSERT_EQ(data[i], builder[i]);
  }

  builder.Clear();
  for (int i = 0 ; i < 100 ; ++i) {
    builder.Append(i);
  }
  ASSERT_EQ(100, builder.Length());
  for (int i = 0 ; i < 100 ; ++i) {
    ASSERT_EQ(i, builder[i]);
  }

  // Move ref of builder1 to builder2
  IntsRefBuilder builder1;
  builder1.InitInts(std::move(builder.Get()));
  ASSERT_EQ(100, builder1.Length());
  for (int i = 0 ; i < 100 ; ++i) {
    ASSERT_EQ(i, builder1[i]);
  }

  // Copy of builder1
  IntsRefBuilder builder2;
  builder2.CopyInts(builder1);
  ASSERT_EQ(100, builder2.Length());
  for (int i = 0 ; i < 100 ; ++i) {
    ASSERT_EQ(i, builder2[i]);
  }

  IntsRefBuilder builder3;
  for (int i = 0 ; i < 100 ; ++i) {
    builder3.Append(100 + i);
  }
  builder2.Append(builder3);
  for (int i = 0 ; i < 200 ; ++i) {
    ASSERT_EQ(i, builder2[i]);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
