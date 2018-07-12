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

#include <Analysis/AttributeImpl.h>
#include <gtest/gtest.h>
#include <Util/Bytes.h>
#include <cstring>
#include <memory>

using lucene::core::analysis::tokenattributes::BytesTermAttributeImpl;
using lucene::core::analysis::tokenattributes::CharTermAttributeImpl;
using lucene::core::analysis::tokenattributes::FlagsAttributeImpl;
using lucene::core::analysis::tokenattributes::KeywordAttributeImpl;
using lucene::core::analysis::tokenattributes::OffsetAttributeImpl;
using lucene::core::analysis::tokenattributes::PackedTokenAttributeImpl;
using lucene::core::analysis::tokenattributes::PayloadAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionIncrementAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionLengthAttributeImpl;
using lucene::core::analysis::tokenattributes::TermFrequencyAttributeImpl;
using lucene::core::analysis::tokenattributes::TypeAttributeImpl;
using lucene::core::util::Attribute;
using lucene::core::util::BytesRef;

TEST(ATTRIBUTES__IMPL__TESTS, FlagsAttributeImpl) {
  // Basic
  FlagsAttributeImpl flags_attr_impl1;
  flags_attr_impl1.SetFlags(13);
  EXPECT_EQ(13, flags_attr_impl1.GetFlags());

  // Clone
  FlagsAttributeImpl* cloned =
    static_cast<FlagsAttributeImpl*>(flags_attr_impl1.Clone());
  std::unique_ptr<FlagsAttributeImpl> guard(cloned);
  EXPECT_EQ(13, cloned->GetFlags());
  EXPECT_EQ(flags_attr_impl1, *cloned);

  // Copy assignment
  FlagsAttributeImpl flags_attr_impl2; flags_attr_impl2 = flags_attr_impl1;
  EXPECT_EQ(13, flags_attr_impl2.GetFlags());
  EXPECT_EQ(flags_attr_impl1, flags_attr_impl2);

  // Copy constructed
  FlagsAttributeImpl flags_attr_impl3(flags_attr_impl2);
  EXPECT_EQ(13, flags_attr_impl3.GetFlags());
  flags_attr_impl3.Clear();
  EXPECT_EQ(0, flags_attr_impl3.GetFlags());
}

TEST(ATTRIBUTES__IMPL__TESTS, TypeAttributeImpl) {
  // Baisc
  TypeAttributeImpl type_attr_impl1;
  std::string type = "Doochi_Type";
  type_attr_impl1.SetType(type);
  EXPECT_EQ(type, type_attr_impl1.Type());

  // Clone
  TypeAttributeImpl* cloned =
    static_cast<TypeAttributeImpl*>(type_attr_impl1.Clone());
  std::unique_ptr<TypeAttributeImpl> guard(cloned);
  EXPECT_EQ(type, cloned->Type());
  EXPECT_EQ(type_attr_impl1, *cloned);

  // Copy constructed
  TypeAttributeImpl type_attr_impl2(type_attr_impl1);
  EXPECT_EQ(type, type_attr_impl2.Type());
  EXPECT_EQ(type_attr_impl1, type_attr_impl2);

  // Copy assignment
  TypeAttributeImpl type_attr_impl3; type_attr_impl3 = type_attr_impl2;
  EXPECT_EQ(type, type_attr_impl3.Type());
  EXPECT_EQ(type_attr_impl2, type_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, TermFrequencyAttributeImpl) {
  // Basic
  TermFrequencyAttributeImpl tf_attr_impl1;
  tf_attr_impl1.SetTermFrequency(13);
  EXPECT_EQ(13, tf_attr_impl1.GetTermFrequency());
  TermFrequencyAttributeImpl* cloned =
    static_cast<TermFrequencyAttributeImpl*>(tf_attr_impl1.Clone());
  std::unique_ptr<TermFrequencyAttributeImpl> guard(cloned);
  EXPECT_EQ(13, cloned->GetTermFrequency());
  EXPECT_EQ(tf_attr_impl1, *cloned);

  // Copy constructed
  TermFrequencyAttributeImpl tf_attr_impl2(tf_attr_impl1);
  EXPECT_EQ(13, tf_attr_impl2.GetTermFrequency());
  EXPECT_EQ(tf_attr_impl1, tf_attr_impl2);

  // Copy assignment
  TermFrequencyAttributeImpl tf_attr_impl3; tf_attr_impl3 = tf_attr_impl1;
  EXPECT_EQ(13, tf_attr_impl3.GetTermFrequency());
  EXPECT_EQ(tf_attr_impl2, tf_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, PositionLengthAttributeImpl) {
  // Basic
  PositionLengthAttributeImpl pos_len_attr_impl1;
  pos_len_attr_impl1.SetPositionLength(13);
  EXPECT_EQ(13, pos_len_attr_impl1.GetPositionLength());
  std::unique_ptr<PositionLengthAttributeImpl>
  cloned(static_cast<PositionLengthAttributeImpl*>(pos_len_attr_impl1.Clone()));
  EXPECT_EQ(13, cloned->GetPositionLength());
  EXPECT_EQ(pos_len_attr_impl1, *cloned);

  // Copy constructed
  PositionLengthAttributeImpl pos_len_attr_impl2(pos_len_attr_impl1);
  EXPECT_EQ(13, pos_len_attr_impl2.GetPositionLength());
  EXPECT_EQ(pos_len_attr_impl1, pos_len_attr_impl2);

  // Copy assignment
  PositionLengthAttributeImpl pos_len_attr_impl3; pos_len_attr_impl3 =
    pos_len_attr_impl2;
  EXPECT_EQ(13, pos_len_attr_impl3.GetPositionLength());
  EXPECT_EQ(pos_len_attr_impl2, pos_len_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, PositionIncrementAttributeImpl) {
  // Basic
  PositionIncrementAttributeImpl pos_incr_attr_impl1;
  pos_incr_attr_impl1.SetPositionIncrement(13);
  EXPECT_EQ(13, pos_incr_attr_impl1.GetPositionIncrement());

  // Cloned
  PositionIncrementAttributeImpl* cloned =
    static_cast<PositionIncrementAttributeImpl*>(pos_incr_attr_impl1.Clone());
  std::unique_ptr<PositionIncrementAttributeImpl> guard(cloned);
  EXPECT_EQ(13, cloned->GetPositionIncrement());
  EXPECT_EQ(pos_incr_attr_impl1, *cloned);

  // Copy constructor
  PositionIncrementAttributeImpl pos_incr_attr_impl2(pos_incr_attr_impl1);
  EXPECT_EQ(13, pos_incr_attr_impl2.GetPositionIncrement());
  EXPECT_EQ(pos_incr_attr_impl1, pos_incr_attr_impl2);

  // Copy assignment
  PositionIncrementAttributeImpl pos_incr_attr_impl3 = pos_incr_attr_impl2;
  EXPECT_EQ(13, pos_incr_attr_impl3.GetPositionIncrement());
  EXPECT_EQ(pos_incr_attr_impl2, pos_incr_attr_impl2);
}

TEST(ATTRIBUTES__IMPL__TESTS, PayloadAttributeImpl) {
  // Basic
  PayloadAttributeImpl payload_attr_impl1;
  BytesRef bref("Doochi's bytesref!");
  payload_attr_impl1.SetPayload(bref);
  BytesRef got_bref = payload_attr_impl1.GetPayload();
  EXPECT_EQ(bref, got_bref);

  // Cloned
  PayloadAttributeImpl* cloned =
    static_cast<PayloadAttributeImpl*>(payload_attr_impl1.Clone());
  std::unique_ptr<PayloadAttributeImpl> guard(cloned);
  EXPECT_EQ(bref, cloned->GetPayload());
  EXPECT_EQ(payload_attr_impl1, *cloned);

  // Shallow copied
  PayloadAttributeImpl shallow_copied;
  payload_attr_impl1.ShallowCopyTo(shallow_copied);
  EXPECT_EQ(bref, shallow_copied.GetPayload());
  EXPECT_EQ(payload_attr_impl1, shallow_copied);

  // Copy constructor
  PayloadAttributeImpl payload_attr_impl2(payload_attr_impl1);
  EXPECT_EQ(bref, payload_attr_impl2.GetPayload());
  EXPECT_EQ(payload_attr_impl1, payload_attr_impl2);

  // Copy assignment
  PayloadAttributeImpl payload_attr_impl3 = payload_attr_impl2;
  EXPECT_EQ(bref, payload_attr_impl3.GetPayload());
  EXPECT_EQ(payload_attr_impl2, payload_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, OffsetAttributeImpl) {
  // Basic
  OffsetAttributeImpl offset_attr_impl1;
  offset_attr_impl1.SetOffset(13, 1313);
  EXPECT_EQ(13, offset_attr_impl1.StartOffset());
  EXPECT_EQ(1313, offset_attr_impl1.EndOffset());

  // Cloned
  OffsetAttributeImpl* cloned =
    static_cast<OffsetAttributeImpl*>(offset_attr_impl1.Clone());
  std::unique_ptr<OffsetAttributeImpl> guard(cloned);
  EXPECT_EQ(13, cloned->StartOffset());
  EXPECT_EQ(1313, cloned->EndOffset());
  EXPECT_EQ(offset_attr_impl1, *cloned);

  // Copy constructor
  OffsetAttributeImpl offset_attr_impl2(offset_attr_impl1);
  EXPECT_EQ(13, offset_attr_impl2.StartOffset());
  EXPECT_EQ(1313, offset_attr_impl2.EndOffset());
  EXPECT_EQ(offset_attr_impl1, offset_attr_impl2);

  // Copy assignment
  OffsetAttributeImpl offset_attr_impl3 = offset_attr_impl2;
  EXPECT_EQ(13, offset_attr_impl3.StartOffset());
  EXPECT_EQ(1313, offset_attr_impl3.EndOffset());
  EXPECT_EQ(offset_attr_impl2, offset_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, KeywordAttributeImpl) {
  // Basic
  KeywordAttributeImpl kwd_attr_impl1;
  kwd_attr_impl1.SetKeyword(true);
  EXPECT_TRUE(kwd_attr_impl1.IsKeyword());

  // Cloned
  KeywordAttributeImpl* cloned =
    static_cast<KeywordAttributeImpl*>(kwd_attr_impl1.Clone());
  std::unique_ptr<KeywordAttributeImpl> guard(cloned);
  EXPECT_TRUE(cloned->IsKeyword());
  EXPECT_EQ(kwd_attr_impl1, *cloned);

  // Copy constructor
  KeywordAttributeImpl kwd_attr_impl2(kwd_attr_impl1);
  EXPECT_TRUE(kwd_attr_impl2.IsKeyword());
  EXPECT_EQ(kwd_attr_impl1, kwd_attr_impl2);

  // Copy assignment
  KeywordAttributeImpl kwd_attr_impl3 = kwd_attr_impl2;
  EXPECT_TRUE(kwd_attr_impl3.IsKeyword());
  EXPECT_EQ(kwd_attr_impl2, kwd_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, BytesTermAttributeImpl) {
  // Basic
  BytesTermAttributeImpl bytes_term_attr_impl1;
  BytesRef bref("Doochi's bytes ref!");
  bytes_term_attr_impl1.SetBytesRef(bref);
  BytesRef& got = bytes_term_attr_impl1.GetBytesRef();
  EXPECT_EQ(bref, got);

  // Cloned
  BytesTermAttributeImpl* cloned =
    static_cast<BytesTermAttributeImpl*>(bytes_term_attr_impl1.Clone());
  std::unique_ptr<BytesTermAttributeImpl> guard(cloned);
  EXPECT_EQ(bref, cloned->GetBytesRef());
  EXPECT_EQ(bytes_term_attr_impl1, *cloned);

  // Shallow copied
  BytesTermAttributeImpl shallow_copied;
  bytes_term_attr_impl1.ShallowCopyTo(shallow_copied);
  got = shallow_copied.GetBytesRef();
  EXPECT_EQ(bref, got);
  EXPECT_EQ(bytes_term_attr_impl1, shallow_copied);

  // Copy constructor
  BytesTermAttributeImpl bytes_term_attr_impl2(bytes_term_attr_impl1);
  got = bytes_term_attr_impl2.GetBytesRef();
  EXPECT_EQ(bref, got);
  EXPECT_EQ(bytes_term_attr_impl1, bytes_term_attr_impl2);

  // Copy assignment
  BytesTermAttributeImpl bytes_term_attr_impl3 = bytes_term_attr_impl2;
  got = bytes_term_attr_impl3.GetBytesRef();
  EXPECT_EQ(bref, got);
  EXPECT_EQ(bytes_term_attr_impl2, bytes_term_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, CharTermAttributeImpl__BASIC) {
  // Copy buffer
  CharTermAttributeImpl ct_attr_impl;
  const char* cstr1 = "Doochi! Will that day come to me?";
  ct_attr_impl.CopyBuffer(cstr1, 0, std::strlen(cstr1));
  EXPECT_EQ(std::strlen(cstr1), ct_attr_impl.Length());
  std::string sub = ct_attr_impl.SubSequence(0, 6);
  EXPECT_EQ(std::string("Doochi"), sub);

  // operator[], buffer access
  char* buf = ct_attr_impl.Buffer();
  for (int i = 0 ; i < std::strlen(cstr1) ; ++i) {
    EXPECT_EQ(cstr1[i], buf[i]);
    EXPECT_EQ(cstr1[i], ct_attr_impl[i]);
  }

  // Resize
  ct_attr_impl.ResizeBuffer(10);
  EXPECT_EQ(std::strlen(cstr1), ct_attr_impl.Length());

  // Subsequence
  sub = ct_attr_impl.SubSequence(0, 6);
  EXPECT_EQ(std::string("Doochi"), sub);

  for (int i = 0 ; i < 10 ; ++i) {
    EXPECT_EQ(cstr1[i], buf[i]);
    EXPECT_EQ(cstr1[i], ct_attr_impl[i]);
  }

  // Append std::string
  std::string append1("append1");
  std::string all = std::string(cstr1) + append1;
  ct_attr_impl.Append(append1);
  EXPECT_EQ(all.size(), ct_attr_impl.Length());

  sub = ct_attr_impl.SubSequence(0, all.size());
  EXPECT_EQ(all, sub);

  // Append sub string
  std::string append2("another append2");
  all += append2.substr(0, 7);
  ct_attr_impl.Append(append2, 0, 7);  // Just append 'another'
  EXPECT_EQ(all.size(), ct_attr_impl.Length());

  sub = ct_attr_impl.SubSequence(0, ct_attr_impl.Length());
  EXPECT_EQ(all, sub);

  // Append single character
  const char* cstr2 = "Why everything are so hard to me?";
  all += std::string(cstr2);
  for (int i = 0 ; i < std::strlen(cstr2) ; ++i) {
    ct_attr_impl.Append(cstr2[i]);
  }
  EXPECT_EQ(all.size(), ct_attr_impl.Length());

  sub = ct_attr_impl.SubSequence(0, ct_attr_impl.Length());
  EXPECT_EQ(all, sub);

  // Append another CharTermAttributeImpl
  CharTermAttributeImpl another_ct_attr_impl;
  std::string append3("Albeit it does move");
  all += append3;
  another_ct_attr_impl.Append(append3);
  ct_attr_impl.Append(another_ct_attr_impl);
  EXPECT_EQ(all.size(), ct_attr_impl.Length());

  sub = ct_attr_impl.SubSequence(0, ct_attr_impl.Length());
  EXPECT_EQ(all, sub);

  // GetBytesRef, Clear
  BytesRef& got_bref = ct_attr_impl.GetBytesRef();
  EXPECT_EQ(BytesRef(all), got_bref);

  ct_attr_impl.Clear();
  EXPECT_EQ(0, ct_attr_impl.Length());

  // SetLength
  ct_attr_impl.SetLength(5);
  EXPECT_EQ(5, ct_attr_impl.Length());

  // SetEmpty
  ct_attr_impl.SetEmpty();
  EXPECT_EQ(0, ct_attr_impl.Length());
}

TEST(ATTRIBUTES__IMPL__TESTS, CharTermAttributeImpl__GROW__TEST) {
  CharTermAttributeImpl ct_attr_impl;
  for (int i = 0 ; i < 1000 ; ++i) {
    std::string line("line - " + std::to_string(i));
    ct_attr_impl.Append(line);
  }
}

TEST(ATTRIBUTES__IMPL__TESTS, CharTermAttributeImpl__ASSIGN) {
  // Prepare
  CharTermAttributeImpl ct_attr_impl1;
  std::string str1 = "";
  ct_attr_impl1.Append(str1);

  // Cloned
  CharTermAttributeImpl* cloned =
    static_cast<CharTermAttributeImpl*>(ct_attr_impl1.Clone());
  std::unique_ptr<CharTermAttributeImpl> guard(cloned);
  EXPECT_EQ(ct_attr_impl1, *cloned);
  EXPECT_EQ(ct_attr_impl1.Length(), cloned->Length());
  BytesRef& org_bref = ct_attr_impl1.GetBytesRef();
  BytesRef& new_bref = cloned->GetBytesRef();
  EXPECT_EQ(org_bref, new_bref);

  // Copy constructed
  CharTermAttributeImpl copy_constructed(ct_attr_impl1);
  EXPECT_EQ(ct_attr_impl1, copy_constructed);
  EXPECT_EQ(ct_attr_impl1.Length(), copy_constructed.Length());
  org_bref = ct_attr_impl1.GetBytesRef();
  new_bref = copy_constructed.GetBytesRef();
  EXPECT_EQ(org_bref, new_bref);

  // Copy assignment
  CharTermAttributeImpl copy_assigned; copy_assigned = ct_attr_impl1;
  EXPECT_EQ(ct_attr_impl1, copy_assigned);
  EXPECT_EQ(ct_attr_impl1.Length(), copy_assigned.Length());
  org_bref = ct_attr_impl1.GetBytesRef();
  new_bref = copy_assigned.GetBytesRef();
  EXPECT_EQ(org_bref, new_bref);

  // Shallow copied
  CharTermAttributeImpl shallow_copied;
  ct_attr_impl1.ShallowCopyTo(shallow_copied);
  EXPECT_EQ(ct_attr_impl1, shallow_copied);
  EXPECT_EQ(ct_attr_impl1.Length(), shallow_copied.Length());
  org_bref = ct_attr_impl1.GetBytesRef();
  new_bref = shallow_copied.GetBytesRef();
  EXPECT_EQ(org_bref, new_bref);
}

TEST(ATTRIBUTES__IMPL__TESTS, PackedTokenAttributeImpl__BASIC) {
  // Prepare
  PackedTokenAttributeImpl packed_attr_impl;
  std::string type = "Doochi-Type";
  packed_attr_impl.SetType(type);
  EXPECT_EQ(type, packed_attr_impl.Type());

  // Position
  packed_attr_impl.SetPositionIncrement(13);
  packed_attr_impl.SetPositionLength(1313);
  EXPECT_EQ(13, packed_attr_impl.GetPositionIncrement());
  EXPECT_EQ(1313, packed_attr_impl.GetPositionLength());

  // Offset
  packed_attr_impl.SetOffset(123, 12345);
  EXPECT_EQ(123, packed_attr_impl.StartOffset());
  EXPECT_EQ(12345, packed_attr_impl.EndOffset());

  // Term frequency
  packed_attr_impl.SetTermFrequency(1010);
  EXPECT_EQ(1010, packed_attr_impl.GetTermFrequency());
}

TEST(ATTRIBUTES__IMPL__TESTS, PackedTokenAttributeImpl__ASSIGN) {
  // Prepare
  PackedTokenAttributeImpl packed_attr_impl1;
  packed_attr_impl1.SetPositionIncrement(13);
  packed_attr_impl1.SetPositionLength(1313);
  packed_attr_impl1.SetOffset(123, 12345);
  packed_attr_impl1.SetTermFrequency(1010);
  std::string str("Doochi! Tests are never finish!");
  packed_attr_impl1.Append(str);
  BytesRef& org_bref = packed_attr_impl1.GetBytesRef();

  {
    // Copy constructed
    PackedTokenAttributeImpl copy_constructed(packed_attr_impl1);
    EXPECT_EQ(packed_attr_impl1, copy_constructed);
    EXPECT_EQ(13, copy_constructed.GetPositionIncrement());
    EXPECT_EQ(1313, copy_constructed.GetPositionLength());
    EXPECT_EQ(123, copy_constructed.StartOffset());
    EXPECT_EQ(12345, copy_constructed.EndOffset());
    BytesRef& copied_bref = copy_constructed.GetBytesRef();
    EXPECT_EQ(org_bref, copied_bref);
  }

  {
    // Cloned
    PackedTokenAttributeImpl* cloned =
      static_cast<PackedTokenAttributeImpl*>(packed_attr_impl1.Clone());
    std::unique_ptr<PackedTokenAttributeImpl> guard(cloned);
    EXPECT_EQ(packed_attr_impl1, *cloned);
    EXPECT_EQ(13, cloned->GetPositionIncrement());
    EXPECT_EQ(1313, cloned->GetPositionLength());
    EXPECT_EQ(123, cloned->StartOffset());
    EXPECT_EQ(12345, cloned->EndOffset());
    BytesRef& copied_bref = cloned->GetBytesRef();
    EXPECT_EQ(org_bref, copied_bref);
  }

  {
    // Shallow copied
    PackedTokenAttributeImpl shallow_copied;
    packed_attr_impl1.ShallowCopyTo(shallow_copied);
    EXPECT_EQ(packed_attr_impl1, shallow_copied);
    EXPECT_EQ(13, shallow_copied.GetPositionIncrement());
    EXPECT_EQ(1313, shallow_copied.GetPositionLength());
    EXPECT_EQ(123, shallow_copied.StartOffset());
    EXPECT_EQ(12345, shallow_copied.EndOffset());
    BytesRef& copied_bref = shallow_copied.GetBytesRef();
    EXPECT_EQ(org_bref, copied_bref);
  }

  {
    // Copy assigned
    PackedTokenAttributeImpl copy_assigned; copy_assigned = packed_attr_impl1;
    EXPECT_EQ(packed_attr_impl1, copy_assigned);
    EXPECT_EQ(13, copy_assigned.GetPositionIncrement());
    EXPECT_EQ(1313, copy_assigned.GetPositionLength());
    EXPECT_EQ(123, copy_assigned.StartOffset());
    EXPECT_EQ(12345, copy_assigned.EndOffset());
    BytesRef& copied_bref = copy_assigned.GetBytesRef();
    EXPECT_EQ(org_bref, copied_bref);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
