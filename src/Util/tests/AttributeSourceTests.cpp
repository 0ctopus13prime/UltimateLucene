#include <iostream>
#include <memory>
#include <stdexcept>
#include <gtest/gtest.h>
#include <Util/Attribute.h>
#include <Analysis/AttributeImpl.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis::tokenattributes;

TEST(ATTRIBUTE__SOURCE__TEST, STATE__BASIC__CONSTRUCTOR) {
  AttributeSource::State state1;
  AttributeSource::State state2(state1);
  AttributeSource::State state3(std::move(state1));
  AttributeSource::State state4 = state3;
  AttributeSource::State state5 = std::move(state4);
}

void CompareTwoState(AttributeSource::State& expected, AttributeSource::State& target) {
    AttributeSource::State* pexpected = &expected;
    AttributeSource::State* ptarget = &target;

    BytesTermAttributeImpl* p10 = dynamic_cast<BytesTermAttributeImpl*>(pexpected->attribute);
    BytesTermAttributeImpl* p11 = dynamic_cast<BytesTermAttributeImpl*>(ptarget->attribute);
    EXPECT_EQ(*p10, *p11);

    pexpected = pexpected->next; ptarget = ptarget->next;
    CharTermAttributeImpl* p20 = dynamic_cast<CharTermAttributeImpl*>(pexpected->attribute);
    CharTermAttributeImpl* p21 = dynamic_cast<CharTermAttributeImpl*>(ptarget->attribute);
    EXPECT_EQ(*p20, *p21);

    pexpected = pexpected->next; ptarget = ptarget->next;
    FlagsAttributeImpl* p30 = dynamic_cast<FlagsAttributeImpl*>(pexpected->attribute);
    FlagsAttributeImpl* p31 = dynamic_cast<FlagsAttributeImpl*>(ptarget->attribute);
    EXPECT_EQ(*p30, *p31);

    pexpected = pexpected->next; ptarget = ptarget->next;
    EXPECT_EQ(nullptr, pexpected);
    EXPECT_EQ(nullptr, ptarget);
}

TEST(ATTRIBUTE__SOURCE__TEST, STATE__CONSTRUCTOR) {
  BytesTermAttributeImpl attr1; AttributeImpl* pattr1 = &attr1;
  CharTermAttributeImpl attr2; AttributeImpl* pattr2 = &attr2;
  FlagsAttributeImpl attr3; AttributeImpl* pattr3 = &attr3;

  AttributeSource::State state; // Read only

  AttributeSource::State* curr_state = &state;
  curr_state->attribute = pattr1;
  curr_state->next = new AttributeSource::State();
  curr_state = curr_state->next;

  curr_state->attribute = pattr2;
  curr_state->next = new AttributeSource::State();
  curr_state = curr_state->next;

  curr_state->attribute = pattr3;

  {
    AttributeSource::State replica_state(state); // Copied
    CompareTwoState(state, replica_state);
  }

  {
    AttributeSource::State replica_state = state;
    CompareTwoState(state, replica_state);
    replica_state = state; // Reassign
    CompareTwoState(state, replica_state);
  }

  {
    AttributeSource::State replica_state1 = state;
    AttributeSource::State replica_state2 = state;
    AttributeSource::State replica_state3(state);
    AttributeSource::State replica_state4(state);

    CompareTwoState(replica_state1, replica_state2);
    CompareTwoState(replica_state3, replica_state4);
    CompareTwoState(replica_state1, replica_state3);
  }
}

TEST(ATTRIBUTE__SOURCE__TEST, STATE__HANDLING) {
  // Add 3 attributes
  AttributeSource attr_source;
  attr_source.AddAttribute<BytesTermAttribute>();
  attr_source.AddAttribute<CharTermAttribute>();
  attr_source.AddAttribute<FlagsAttribute>();

  // Count test.
  {
    AttributeSource::State* state = attr_source.CaptureState();
    std::unique_ptr<AttributeSource::State> guard(state);
    AttributeSource::State* curr = state;
    uint32_t count = 0;
    while(curr != nullptr) {
      curr = curr->next;
      count++;
    }
    EXPECT_EQ(3, count);
  }

  // Different 3 attributes check
  {
    AttributeSource::State* state = attr_source.CaptureState();
    std::unique_ptr<AttributeSource::State> guard(state);

    AttributeSource::State* curr = state;
    bool has_bytes_attr = false, has_char_attr = false, has_flags_attr = false;

    while(curr != nullptr) {
      AttributeImpl* pimpl = curr->attribute;
      if(dynamic_cast<BytesTermAttribute*>(pimpl)) {
        EXPECT_EQ(has_bytes_attr, false);
        has_bytes_attr = true;
      } else if(dynamic_cast<CharTermAttribute*>(pimpl)) {
        EXPECT_EQ(has_char_attr, false);
        has_char_attr = true;
      } else if(dynamic_cast<FlagsAttribute*>(pimpl)) {
        EXPECT_EQ(has_flags_attr, false);
        has_flags_attr = true;
      } else {
        FAIL();
      }

      curr = curr->next;
    }
  }

  // Restore state check
  {
    AttributeSource::State* target_state = attr_source.CaptureState();
    std::unique_ptr<AttributeSource::State> guard1(target_state);
    AttributeSource::State* curr = target_state;

    // Find FlagsAttribute and set flags = 13
    while(curr != nullptr && !dynamic_cast<FlagsAttribute*>(curr->attribute))
     curr = curr->next;
    FlagsAttribute* target_pflags = dynamic_cast<FlagsAttribute*>(curr->attribute);
    target_pflags->SetFlags(13); // Change state

    AttributeSource::State* org_state = attr_source.CaptureState();
    std::unique_ptr<AttributeSource::State> guard2(org_state);
    curr = org_state;
    while(curr != nullptr && !dynamic_cast<FlagsAttribute*>(curr->attribute))
     curr = curr->next;
    FlagsAttribute* org_pflags = dynamic_cast<FlagsAttribute*>(curr->attribute);

    // Ensure different
    EXPECT_NE(org_pflags->GetFlags(), target_pflags->GetFlags());

    // Restore and see flags is 13
    attr_source.RestoreState(target_state);
    AttributeSource::State* new_state = attr_source.CaptureState();
    std::unique_ptr<AttributeSource::State> guard3(new_state);
    curr = new_state;
    while(curr != nullptr && !dynamic_cast<FlagsAttribute*>(curr->attribute))
     curr = curr->next;
    FlagsAttribute* new_pflags = dynamic_cast<FlagsAttribute*>(curr->attribute);
    EXPECT_EQ(new_pflags->GetFlags(), 13);
  }
}

TEST(ATTRIBUTE__SOURCE__TEST, ETC__TESTS) {
  AttributeSource attr_source;
  EXPECT_FALSE(attr_source.HasAttributes());

  attr_source.AddAttribute<BytesTermAttribute>();
  attr_source.AddAttribute<CharTermAttribute>();
  attr_source.AddAttribute<FlagsAttribute>();
  EXPECT_TRUE(attr_source.HasAttributes());

  EXPECT_TRUE(attr_source.HasAttribute<BytesTermAttribute>());
  EXPECT_TRUE(attr_source.HasAttribute<CharTermAttribute>());
  EXPECT_TRUE(attr_source.HasAttribute<FlagsAttribute>());
  EXPECT_FALSE(attr_source.HasAttribute<TypeAttribute>());

  attr_source.RemoveAllAttributes();
  EXPECT_FALSE(attr_source.HasAttributes());
  EXPECT_FALSE(attr_source.HasAttribute<BytesTermAttribute>());
  EXPECT_FALSE(attr_source.HasAttribute<CharTermAttribute>());
  EXPECT_FALSE(attr_source.HasAttribute<FlagsAttribute>());

  AttributeSource::State* pstate = attr_source.CaptureState();
  EXPECT_EQ(nullptr, pstate);
}

void CompareIdenticalAttributeSource(AttributeSource& attr_source1, AttributeSource& attr_source2, bool shared=false) {
  // Set flags = 13
  AttributeSource::State* state = attr_source1.CaptureState();
  std::unique_ptr<AttributeSource::State> guard1(state);
  AttributeSource::State* curr = state;
  while(curr != nullptr && !dynamic_cast<FlagsAttribute*>(curr->attribute))
    curr = curr->next;

  FlagsAttribute* pflags = dynamic_cast<FlagsAttribute*>(curr->attribute);
  pflags->SetFlags(13);
  attr_source1.RestoreState(state);

  // Check to see if attr_source2 that is shallow copied has identical(shared) or different flags value
  AttributeSource::State* replica_state = attr_source2.CaptureState();
  std::unique_ptr<AttributeSource::State> guard2(replica_state);
  curr = replica_state;
  while(curr != nullptr && !dynamic_cast<FlagsAttribute*>(curr->attribute))
    curr = curr->next;
  FlagsAttribute* replica_pflags = dynamic_cast<FlagsAttribute*>(curr->attribute);

  if(shared) {
    EXPECT_EQ(13, replica_pflags->GetFlags());
  } else {
    EXPECT_NE(13, replica_pflags->GetFlags());
  }
}

TEST(ATTRIBUTE__SOURCE__TEST, ATTRIBUTE__SOURCE__SHALLOW__COPY__TEST) {
  AttributeSource attr_source1;
  attr_source1.AddAttribute<BytesTermAttribute>();
  attr_source1.AddAttribute<CharTermAttribute>();
  attr_source1.AddAttribute<FlagsAttribute>();

  // Shallow copy
  AttributeSource attr_source2;
  attr_source1.ShallowCopyTo(attr_source2);

  EXPECT_TRUE(attr_source2.HasAttribute<BytesTermAttribute>());
  EXPECT_TRUE(attr_source2.HasAttribute<CharTermAttribute>());
  EXPECT_TRUE(attr_source2.HasAttribute<FlagsAttribute>());

  CompareIdenticalAttributeSource(attr_source1, attr_source2, true);
}

TEST(ATTRIBUTE__SOURCE__TEST, ATTRIBUTE__SOURCE__DEEP__COPY__CONSTRUCTOR__TEST) {
  AttributeSource attr_source1;
  attr_source1.AddAttribute<BytesTermAttribute>();
  attr_source1.AddAttribute<CharTermAttribute>();
  attr_source1.AddAttribute<FlagsAttribute>();

  // Deep copy
  AttributeSource attr_source2(attr_source1);

  EXPECT_TRUE(attr_source2.HasAttribute<BytesTermAttribute>());
  EXPECT_TRUE(attr_source2.HasAttribute<CharTermAttribute>());
  EXPECT_TRUE(attr_source2.HasAttribute<FlagsAttribute>());

  CompareIdenticalAttributeSource(attr_source1, attr_source2, false);
}

TEST(ATTRIBUTE__SOURCE__TEST, ATTRIBUTE__SOURCE__DEEP__COPY__ASSIGN__TEST) {
  AttributeSource attr_source1;
  attr_source1.AddAttribute<BytesTermAttribute>();
  attr_source1.AddAttribute<CharTermAttribute>();
  attr_source1.AddAttribute<FlagsAttribute>();

  // Deep copy
  AttributeSource attr_source2 = attr_source1;

  EXPECT_TRUE(attr_source2.HasAttribute<BytesTermAttribute>());
  EXPECT_TRUE(attr_source2.HasAttribute<CharTermAttribute>());
  EXPECT_TRUE(attr_source2.HasAttribute<FlagsAttribute>());

  CompareIdenticalAttributeSource(attr_source1, attr_source2, false);
}

TEST(ATTRIBUTE__SOURCE__TEST, ATTRIBUTE__REFLECT__TEST) {
  AttributeSource attr_source;
  attr_source.AddAttribute<BytesTermAttribute>();
  attr_source.AddAttribute<CharTermAttribute>();
  attr_source.AddAttribute<FlagsAttribute>();

  std::cout << "BytesTermAttribute, CharTermAttribute, FlagsAttribute are reflected (name included) -> "
    << attr_source.ReflectAsString(true) << std::endl;
  std::cout << "BytesTermAttribute, CharTermAttribute, FlagsAttribute are reflected -> "
    << attr_source.ReflectAsString(false) << std::endl;
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
