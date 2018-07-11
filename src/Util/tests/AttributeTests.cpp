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
#include <Util/Attribute.h>
#include <stdexcept>

using lucene::core::analysis::tokenattributes::BytesTermAttribute;
using lucene::core::analysis::tokenattributes::BytesTermAttributeImpl;
using lucene::core::analysis::tokenattributes::CharTermAttribute;
using lucene::core::analysis::tokenattributes::FlagsAttribute;
using lucene::core::analysis::tokenattributes::FlagsAttributeImpl;
using lucene::core::analysis::tokenattributes::KeywordAttribute;
using lucene::core::analysis::tokenattributes::KeywordAttributeImpl;
using lucene::core::analysis::tokenattributes::OffsetAttribute;
using lucene::core::analysis::tokenattributes::PayloadAttribute;
using lucene::core::analysis::tokenattributes::PayloadAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionIncrementAttribute;
using lucene::core::analysis::tokenattributes::PositionLengthAttribute;
using lucene::core::analysis::tokenattributes::PackedTokenAttributeImpl;
using lucene::core::analysis::tokenattributes::TermFrequencyAttribute;
using lucene::core::analysis::tokenattributes::TypeAttribute;
using lucene::core::analysis::tokenattributes::TermToBytesRefAttribute;
using lucene::core::util::Attribute;
using lucene::core::util::AttributeFactory;
using lucene::core::util::AttributeImpl;
using lucene::core::util::AttributeImplGenerator;
using lucene::core::util::AttributeReflector;

template<typename ATTR>
void check_generated_type(AttributeFactory& attr_factory) {
  AttributeImplGenerator gen =
    attr_factory.FindAttributeImplGenerator(Attribute::TypeId<ATTR>());
  AttributeImpl* attr_impl1 = gen();
  std::unique_ptr<AttributeImpl> guard1(attr_impl1);
  EXPECT_NE(dynamic_cast<ATTR*>(attr_impl1), nullptr);

  AttributeImpl* attr_impl2 =
    attr_factory.CreateAttributeInstance(Attribute::TypeId<ATTR>());
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
  AttributeImpl* ptr =
    static_factory->CreateAttributeInstance(Attribute::TypeId<ATTR>());
  std::unique_ptr<AttributeImpl> guard(ptr);
  EXPECT_NE(dynamic_cast<ATTR_IMPL*>(ptr), nullptr);
}

TEST(ATTRIBUTE__TEST, STATIC__FACTORY) {
  AttributeFactory* static_factory =
    AttributeFactory::GetStaticImplementation
      <AttributeFactory::DefaultAttributeFactory, PackedTokenAttributeImpl>();

  check_static_factory_create_result<CharTermAttribute,
                                     PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<TermToBytesRefAttribute,
                                     PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<TypeAttribute,
                                     PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<PositionIncrementAttribute,
                                     PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<PositionLengthAttribute,
                                     PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<OffsetAttribute,
                                     PackedTokenAttributeImpl>(static_factory);
  check_static_factory_create_result<TermFrequencyAttribute,
                                     PackedTokenAttributeImpl>(static_factory);

  check_static_factory_create_result<BytesTermAttribute,
                                     BytesTermAttributeImpl>(static_factory);
  check_static_factory_create_result<FlagsAttribute,
                                     FlagsAttributeImpl>(static_factory);
  check_static_factory_create_result<KeywordAttribute,
                                     KeywordAttributeImpl>(static_factory);
  check_static_factory_create_result<PayloadAttribute,
                                     PayloadAttributeImpl>(static_factory);
}

TEST(ATTRIBUTE__TEST, ATTRIBUTE__FACTORY__REGISTER) {
  class DummyCustomAttribute: public Attribute {
   public:
     virtual ~DummyCustomAttribute() {}
  };

  class DummyCustomAttributeImpl: public AttributeImpl,
                                  public DummyCustomAttribute {
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
     DummyCustomAttributeImpl&
     operator=(const DummyCustomAttributeImpl& other) {
       return
       DummyCustomAttributeImpl::operator=(
                                   static_cast<const AttributeImpl&>(other));
     }
  };

  AttributeFactory::RegisterAttributeImpl<DummyCustomAttribute,
                                          DummyCustomAttributeImpl>();
  AttributeFactory::DefaultAttributeFactory default_attr_factory;
  AttributeImpl* attr_impl1 =
    default_attr_factory.CreateAttributeInstance(
                           Attribute::TypeId<DummyCustomAttribute>());
  std::unique_ptr<AttributeImpl> guard(attr_impl1);
  EXPECT_NE(dynamic_cast<DummyCustomAttributeImpl*>(attr_impl1), nullptr);

  AttributeImplGenerator gen =
    AttributeFactory::FindAttributeImplGenerator(
                        Attribute::TypeId<DummyCustomAttribute>());
  AttributeImpl* attr_impl2 = gen();
  guard.reset(attr_impl2);
  EXPECT_NE(dynamic_cast<DummyCustomAttributeImpl*>(attr_impl2), nullptr);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
