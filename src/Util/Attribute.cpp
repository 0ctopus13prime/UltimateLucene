#include <stdexcept>
#include <Util/Attribute.h>
#include <Analysis/AttributeImpl.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis::tokenattributes;

/**
 * AttributeImpl
 */

std::string AttributeImpl::ReflectAsString(const bool prepend_att_class) {
  // TODO Implement it.
  return "";
}

void AttributeImpl::End() {
  Clear();
}

/**
 * AttributeFactory
 */
std::unordered_map<std::string, std::function<AttributeImpl*(void)>>
 AttributeFactory::ATTR_IMPL_TABLE = {
    {"BytesTermAttribute", [](){ return new BytesTermAttributeImpl(); }},
    {"CharTermAttribute", [](){ return new CharTermAttributeImpl(); }},
    {"FlagsAttribute", [](){ return new FlagsAttributeImpl(); }},
    {"KeywordAttribute", [](){ return new KeywordAttributeImpl(); }},
    {"OffsetAttribute", [](){ return new OffsetAttributeImpl(); }},
    {"PayloadAttribute", [](){ return new PayloadAttributeImpl(); }},
    {"PositionIncrementAttribute", [](){ return new PositionIncrementAttributeImpl(); }},
    {"PositionLengthAttribute", [](){ return new PositionLengthAttributeImpl(); }},
    {"TermFrequencyAttribute", [](){ return new TermFrequencyAttributeImpl(); }},
    {"TypeAttribute", [](){ return new TypeAttributeImpl(); }}
 };

AttributeFactory::AttributeFactory() {
}

std::function<AttributeImpl*(void)> AttributeFactory::FindAttributeImplCtor(const std::string& attr_name) {
  auto it = AttributeFactory::ATTR_IMPL_TABLE.find(attr_name);

  if(it == AttributeFactory::ATTR_IMPL_TABLE.end()) {
    throw std::runtime_error("Attribute " + attr_name + " implmentation was not found");
  }

  return it->second;
}

/**
 * DefaultAttributeImpl
 */

AttributeFactory::DefaultAttributeImpl::DefaultAttributeImpl() {
}

AttributeFactory::DefaultAttributeImpl::~DefaultAttributeImpl() {
}

AttributeImpl* AttributeFactory::DefaultAttributeImpl::CreateAttributeInstance(const std::string& attr_name) {
  auto ctor = AttributeFactory::FindAttributeImplCtor(attr_name);
  return ctor();
}

/**
 *  AttributeSource::State
 */
AttributeSource::State::State()
  : attribute(nullptr),
    next(nullptr) {
}

AttributeSource::State::State(const AttributeSource::State& other)
  : attribute(other.attribute),
    next(other.next) {
}

AttributeSource::State::~State() {
}

AttributeSource::State& AttributeSource::State::operator=(const AttributeSource::State& other) {
  attribute = other.attribute;
  next = other.next;
}
