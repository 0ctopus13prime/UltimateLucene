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

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
