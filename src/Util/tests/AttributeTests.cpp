#include <stdexcept>
#include <gtest/gtest.h>
#include <Util/Attribute.h>
#include <Analysis/AttributeImpl.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis::tokenattributes;

template<typename ATTR>
void check_generated_type(AttributeFactory& attr_factory) {
  AttributeImplGenerator gen = attr_factory.FindAttributeImplGenerator(Attribute::TypeId<ATTR>());
  AttributeImpl* attr_impl1 = gen();
  std::unique_ptr<AttributeImpl> guard1(attr_impl1);
  EXPECT_NE(dynamic_cast<ATTR*>(attr_impl1), nullptr);

  AttributeImpl* attr_impl2 = attr_factory.CreateAttributeInstance(Attribute::TypeId<ATTR>());
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

template<typename ATTR, typename ATTR_IMPL>
void check_static_factory_create_result(AttributeFactory* static_factory) {
  AttributeImpl* ptr = static_factory->CreateAttributeInstance(Attribute::TypeId<ATTR>());
  std::unique_ptr<AttributeImpl> guard(ptr);
  EXPECT_NE(dynamic_cast<ATTR_IMPL*>(ptr), nullptr);
}

TEST(ATTRIBUTE__TEST, STATIC__FACTORY) {
  AttributeFactory* static_factory =
    AttributeFactory::GetStaticImplementation<AttributeFactory::DefaultAttributeFactory, PackedTokenAttributeImpl>();

  check_static_factory_create_result<CharTermAttribute, PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<TermToBytesRefAttribute, PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<TypeAttribute, PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<PositionIncrementAttribute, PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<PositionLengthAttribute, PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<OffsetAttribute, PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<TermFrequencyAttribute, PackedTokenAttributeImpl>(static_factory);

  check_static_factory_create_result<BytesTermAttribute, BytesTermAttributeImpl>(static_factory);
  check_static_factory_create_result<FlagsAttribute, FlagsAttributeImpl>(static_factory);
  check_static_factory_create_result<KeywordAttribute, KeywordAttributeImpl>(static_factory);
  check_static_factory_create_result<PayloadAttribute, PayloadAttributeImpl>(static_factory);
}

TEST(ATTRIBUTE__TEST, ATTRIBUTE__FACTORY__REGISTER) {
  class DummyCustomAttribute: public Attribute {
    public:
      virtual ~DummyCustomAttribute() {}
   };

  class DummyCustomAttributeImpl: public AttributeImpl, public DummyCustomAttribute {
    public:
      DummyCustomAttributeImpl() {}
      virtual ~DummyCustomAttributeImpl() {}
      void ReflectWith(AttributeReflector& reflector) override {}
      void Clear() override {}
      std::vector<std::type_index> Attributes() override {
        return {
          Attribute::TypeId<DummyCustomAttribute>()
        };
      }
      void ShallowCopyTo(AttributeImpl& attr_impl) override {
      }
      AttributeImpl* Clone() override {
        return new DummyCustomAttributeImpl();
      }
      DummyCustomAttributeImpl& operator=(const AttributeImpl& other) {
        return *this;
      }
      DummyCustomAttributeImpl& operator=(const DummyCustomAttributeImpl& other) {
        return DummyCustomAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
      }
  };

  AttributeFactory::RegisterAttributeImpl<DummyCustomAttribute, DummyCustomAttributeImpl>();
  AttributeFactory::DefaultAttributeFactory default_attr_factory;
  AttributeImpl* attr_impl1 = default_attr_factory.CreateAttributeInstance(Attribute::TypeId<DummyCustomAttribute>());
  std::unique_ptr<AttributeImpl> guard(attr_impl1);
  EXPECT_NE(dynamic_cast<DummyCustomAttributeImpl*>(attr_impl1), nullptr);

  AttributeImplGenerator gen = AttributeFactory::FindAttributeImplGenerator(Attribute::TypeId<DummyCustomAttribute>());
  AttributeImpl* attr_impl2 = gen();
  guard.reset(attr_impl2);
  EXPECT_NE(dynamic_cast<DummyCustomAttributeImpl*>(attr_impl2), nullptr);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
