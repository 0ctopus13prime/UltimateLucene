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
 * AttributeSource
 */
AttributeSource::AttributeSource()
  : AttributeSource(new AttributeFactory::DefaultAttributeImpl()) {
}

AttributeSource::AttributeSource(const AttributeSource& other)
  : current_state(other.current_state),
    current_state_dirty(other.current_state_dirty),
    attributes(other.attributes),
    attribute_impls(other.attribute_impls),
    factory(other.factory) {
  if(factory == nullptr) {
    throw std::invalid_argument("AttributeFactory must not be null");
  }
}

AttributeSource::AttributeSource(AttributeFactory* factory)
  : current_state(new AttributeSource::State()),
    current_state_dirty(true),
    attributes(),
    attribute_impls(),
    factory(factory) {
  if(factory == nullptr) {
    throw std::invalid_argument("AttributeFactory must not be null");
  }
}

AttributeFactory& AttributeSource::GetAttributeFactory() const {
  return *factory;
}

void AttributeSource::AddAttributeImpl(const std::string& att_name, const std::string& att_impl_name, std::function<AttributeImpl*(void)>& att_impl_factory) {
  if(attribute_impls.find(att_name) != attribute_impls.end()) {
    return;
  }

  AttributeImpl* att_impl = att_impl_factory();
  attributes[att_name].reset(att_impl);
  attribute_impls[att_name].reset(att_impl);
}

bool AttributeSource::HasAttributes()  {
  return !attributes.empty();
}

bool AttributeSource::HasAttribute(const std::string& att_name)  {
  return (attributes.find(att_name) != attributes.end());
}

AttributeSource::State* AttributeSource::GetCurrentState() {
  current_state_dirty = false;
  if(!current_state_dirty || !HasAttributes()) {
    return current_state.get();
  }

  AttributeSource::State* head = current_state.get();
  AttributeSource::State* c = head;
  auto it = attribute_impls.begin();
  c->attribute.reset(it->second.get());

  while(it != attribute_impls.end()) {
    c->next = (c->next == nullptr ? new AttributeSource::State() : c->next);
    c = c->next;
    c->attribute.reset(it->second.get());
  }

  if(c->next != nullptr) {
    c->next->~State();
  }

  return head;
}

void AttributeSource::ClearAttributes() {
  for(AttributeSource::State* state = GetCurrentState() ; state != nullptr ; state = state->next) {
    state->attribute->Clear();
  }
}

void AttributeSource::EndAttributes() {
  for(AttributeSource::State* state = GetCurrentState() ; state != nullptr ; state = state->next) {
    state->attribute->End();

  }
}

void AttributeSource::RemoveAllAttributes() {
  attributes.clear();
  attribute_impls.clear();
}

AttributeSource::State* AttributeSource::CaptureState(AttributeSource::State& target) {
  AttributeSource::State* state = GetCurrentState();
  return new AttributeSource::State(*state);
}

void AttributeSource::RestoreState(AttributeSource::State* state) {
  if(state->attribute.use_count() <= 0) return;

  AttributeSource::State* current = state;
  do {
    std::string attr_impl_name = state->attribute->AttributeImplName();
    AttributeImpl* source_attr = current->attribute.get();

    if(attribute_impls.find(attr_impl_name) == attribute_impls.end()) {
      throw std::invalid_argument("AttributeSource::State contains AttributeImpl of type " + attr_impl_name + " that is not in this AttributeSource");
    }

    AttributeImpl* target_attr = attribute_impls[attr_impl_name].get();
    *target_attr = *source_attr;

    current = current->next;
  } while(current != nullptr);
}

std::string AttributeSource::ReflectAsString(const bool prepend_att) {
  // TODO Implement this.
  return "NOOP";
}

void AttributeSource::ReflectWith(AttributeReflector& reflector)  {
  // TODO Implement this
}

AttributeSource& AttributeSource::operator=(const AttributeSource& other) {
  current_state = other.current_state;
  current_state_dirty = other.current_state_dirty;
  attributes = other.attributes;
  attribute_impls = other.attribute_impls;
  factory = other.factory;
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
  if(next != nullptr) {
    next->~State();
  }
}

AttributeSource::State& AttributeSource::State::operator=(const AttributeSource::State& other) {
  attribute = other.attribute;
  next = other.next;
}
