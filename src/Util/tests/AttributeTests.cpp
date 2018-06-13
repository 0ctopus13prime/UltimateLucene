#include <stdexcept>
#include <typeinfo>
#include <gtest/gtest.h>
#include <Util/Attribute.h>
#include <Analysis/AttributeImpl.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis::tokenattributes;

template<typename ATTR>
void check_generated_type(AttributeFactory& attr_factory) {
  AttributeImplGenerator gen = attr_factory.FindAttributeImplGenerator(typeid(ATTR).hash_code());
  AttributeImpl* attr_impl1 = gen();
  std::unique_ptr<AttributeImpl> guard1(attr_impl1);
  EXPECT_NE(dynamic_cast<ATTR*>(attr_impl1), nullptr);

  AttributeImpl* attr_impl2 = attr_factory.CreateAttributeInstance(typeid(ATTR).hash_code());
  std::unique_ptr<AttributeImpl> guard2(attr_impl2);
  EXPECT_NE(dynamic_cast<ATTR*>(attr_impl2), nullptr);
}

TEST(ATTRIBUTE__TEST, DEFAULT__ATTRIBUTE__FACTORY) {
  AttributeFactory::DefaultAttributeFactory default_attr_factory;

  AttributeImplGenerator gen;
  check_generated_type<BytesTermAttribute>(default_attr_factory);
  check_generated_type<CharTermAttribute>(default_attr_factory);
  check_generated_type<FlagsAttribute>(default_attr_factory);
  check_generated_type<KeywordAttribute>(default_attr_factory);
  check_generated_type<OffsetAttribute>(default_attr_factory);
  check_generated_type<PayloadAttribute>(default_attr_factory);
  check_generated_type<PositionIncrementAttribute>(default_attr_factory);
  check_generated_type<PositionLengthAttribute>(default_attr_factory);
  check_generated_type<TermFrequencyAttribute>(default_attr_factory);
  check_generated_type<TypeAttribute>(default_attr_factory);
}

/*
TEST(ATTRIBUTE__TEST, STATIC__FACTORY) {
  AttributeFactory* static_factory =
    AttributeFactory::GetStaticImplementation<AttributeFactory::DefaultAttributeFactory, PackedTokenAttributeImpl>();
}
*/


int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
