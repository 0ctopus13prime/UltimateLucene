#include <cstring>
#include <memory>
#include <Analysis/AttributeImpl.h>
#include <Util/Bytes.h>
#include <gtest/gtest.h>

using namespace lucene::core::analysis::tokenattributes;
using namespace lucene::core::util;

TEST(ATTRIBUTES__IMPL__TESTS, FlagsAttributeImpl) {
  FlagsAttributeImpl flags_attr_impl1;
  flags_attr_impl1.SetFlags(13);
  EXPECT_EQ(13, flags_attr_impl1.GetFlags());

  FlagsAttributeImpl* cloned = static_cast<FlagsAttributeImpl*>(flags_attr_impl1.Clone());
  std::unique_ptr<FlagsAttributeImpl> guard(cloned);
  EXPECT_EQ(13, cloned->GetFlags());
  EXPECT_EQ(flags_attr_impl1, *cloned);

  FlagsAttributeImpl flags_attr_impl2 = flags_attr_impl1;
  EXPECT_EQ(13, flags_attr_impl2.GetFlags());

  EXPECT_EQ(flags_attr_impl1, flags_attr_impl2);

  FlagsAttributeImpl flags_attr_impl3(flags_attr_impl2);
  EXPECT_EQ(13, flags_attr_impl3.GetFlags());
  flags_attr_impl3.Clear();
  EXPECT_EQ(0, flags_attr_impl3.GetFlags());

}

TEST(ATTRIBUTES__IMPL__TESTS, TypeAttributeImpl) {
  TypeAttributeImpl type_attr_impl1;
  std::string type = "Doochi_Type";
  type_attr_impl1.SetType(type);
  EXPECT_EQ(type, type_attr_impl1.Type());

  TypeAttributeImpl* cloned = static_cast<TypeAttributeImpl*>(type_attr_impl1.Clone());
  std::unique_ptr<TypeAttributeImpl> guard(cloned);
  EXPECT_EQ(type, cloned->Type());
  EXPECT_EQ(type_attr_impl1, *cloned);

  TypeAttributeImpl type_attr_impl2(type_attr_impl1);
  EXPECT_EQ(type, type_attr_impl2.Type());
  EXPECT_EQ(type_attr_impl1, type_attr_impl2);

  TypeAttributeImpl type_attr_impl3 = type_attr_impl2;
  EXPECT_EQ(type, type_attr_impl3.Type());
  EXPECT_EQ(type_attr_impl2, type_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, TermFrequencyAttributeImpl) {
  TermFrequencyAttributeImpl tf_attr_impl1;
  tf_attr_impl1.SetTermFrequency(13);
  EXPECT_EQ(13, tf_attr_impl1.GetTermFrequency());
  TermFrequencyAttributeImpl* cloned = static_cast<TermFrequencyAttributeImpl*>(tf_attr_impl1.Clone());
  std::unique_ptr<TermFrequencyAttributeImpl> guard(cloned);
  EXPECT_EQ(13, cloned->GetTermFrequency());
  EXPECT_EQ(tf_attr_impl1, *cloned);

  TermFrequencyAttributeImpl tf_attr_impl2(tf_attr_impl1);
  EXPECT_EQ(13, tf_attr_impl2.GetTermFrequency());
  EXPECT_EQ(tf_attr_impl1, tf_attr_impl2);

  TermFrequencyAttributeImpl tf_attr_impl3 = tf_attr_impl1;
  EXPECT_EQ(13, tf_attr_impl3.GetTermFrequency());
  EXPECT_EQ(tf_attr_impl2, tf_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, PositionLengthAttributeImpl) {
  PositionLengthAttributeImpl pos_len_attr_impl1;
  pos_len_attr_impl1.SetPositionLength(13);
  EXPECT_EQ(13, pos_len_attr_impl1.GetPositionLength());
  std::unique_ptr<PositionLengthAttributeImpl> cloned(static_cast<PositionLengthAttributeImpl*>(pos_len_attr_impl1.Clone()));
  EXPECT_EQ(13, cloned->GetPositionLength());
  EXPECT_EQ(pos_len_attr_impl1, *cloned);

  PositionLengthAttributeImpl pos_len_attr_impl2(pos_len_attr_impl1);
  EXPECT_EQ(13, pos_len_attr_impl2.GetPositionLength());
  EXPECT_EQ(pos_len_attr_impl1, pos_len_attr_impl2);

  PositionLengthAttributeImpl pos_len_attr_impl3 = pos_len_attr_impl2;
  EXPECT_EQ(13, pos_len_attr_impl3.GetPositionLength());
  EXPECT_EQ(pos_len_attr_impl2, pos_len_attr_impl3);
}

TEST(ATTRIBUTES__IMPL__TESTS, PositionIncrementAttributeImpl) {
  // Basic
  PositionIncrementAttributeImpl pos_incr_attr_impl1;
  pos_incr_attr_impl1.SetPositionIncrement(13);
  EXPECT_EQ(13, pos_incr_attr_impl1.GetPositionIncrement());

  // Cloned
  PositionIncrementAttributeImpl* cloned = static_cast<PositionIncrementAttributeImpl*>(pos_incr_attr_impl1.Clone());
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
  PayloadAttributeImpl payload_attr_impl1;
  BytesRef bref("Doochi's bytesref!");
  payload_attr_impl1.SetPayload(bref);
  BytesRef got_bref = payload_attr_impl1.GetPayload();
  EXPECT_EQ(bref, got_bref);

  // Cloned
  PayloadAttributeImpl* cloned = static_cast<PayloadAttributeImpl*>(payload_attr_impl1.Clone());
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
  OffsetAttributeImpl* cloned = static_cast<OffsetAttributeImpl*>(offset_attr_impl1.Clone());
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
  KeywordAttributeImpl* cloned = static_cast<KeywordAttributeImpl*>(kwd_attr_impl1.Clone());
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
  BytesTermAttributeImpl* cloned = static_cast<BytesTermAttributeImpl*>(bytes_term_attr_impl1.Clone());
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
  for(int i = 0 ; i < std::strlen(cstr1) ; ++i) {
    EXPECT_EQ(cstr1[i], buf[i]);
    EXPECT_EQ(cstr1[i], ct_attr_impl[i]);
  }

  // Resize
  ct_attr_impl.ResizeBuffer(10);
  EXPECT_EQ(std::strlen(cstr1), ct_attr_impl.Length());

  // Subsequence
  sub = ct_attr_impl.SubSequence(0, 6);
  EXPECT_EQ(std::string("Doochi"), sub);

  for(int i = 0 ; i < 10 ; ++i) {
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
  ct_attr_impl.Append(append2, 0, 7); // Just append 'another'
  EXPECT_EQ(all.size(), ct_attr_impl.Length());

  sub = ct_attr_impl.SubSequence(0, ct_attr_impl.Length());
  EXPECT_EQ(all, sub);

  // Append single character
  const char* cstr2 = "Why everything are so hard to me?";
  all += std::string(cstr2);
  for(int i = 0 ; i < std::strlen(cstr2) ; ++i) {
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

TEST(ATTRIBUTES__IMPL__TESTS, CharTermAttributeImpl__ASSIGN) {
  // TODO. Copy constructor, copy assignment, clone, shallow copied
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
