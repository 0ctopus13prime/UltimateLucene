#include <typeinfo>
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
AttributeFactory::DefaultAttributeFactory* AttributeFactory::DEFAULT_ATTRIBUTE_FACTORY = new AttributeFactory::DefaultAttributeFactory();
std::unordered_map<size_t, std::function<AttributeImpl*(void)>>
AttributeFactory::ATTR_IMPL_TABLE = {
    {typeid(BytesTermAttribute).hash_code(), [](){ return new BytesTermAttributeImpl(); }},
    {typeid(CharTermAttribute).hash_code(), [](){ return new CharTermAttributeImpl(); }},
    {typeid(FlagsAttribute).hash_code(), [](){ return new FlagsAttributeImpl(); }},
    {typeid(KeywordAttribute).hash_code(), [](){ return new KeywordAttributeImpl(); }},
    {typeid(OffsetAttribute).hash_code(), [](){ return new OffsetAttributeImpl(); }},
    {typeid(PayloadAttribute).hash_code(), [](){ return new PayloadAttributeImpl(); }},
    {typeid(PositionIncrementAttribute).hash_code(), [](){ return new PositionIncrementAttributeImpl(); }},
    {typeid(PositionLengthAttribute).hash_code(), [](){ return new PositionLengthAttributeImpl(); }},
    {typeid(TermFrequencyAttribute).hash_code(), [](){ return new TermFrequencyAttributeImpl(); }},
    {typeid(TypeAttribute).hash_code(), [](){ return new TypeAttributeImpl(); }}
 };


std::function<AttributeImpl*(void)> AttributeFactory::FindAttributeImplCtor(size_t attr_hash_code) {
  auto it = AttributeFactory::ATTR_IMPL_TABLE.find(attr_hash_code);

  if(it == AttributeFactory::ATTR_IMPL_TABLE.end()) {
    throw std::runtime_error("Attribute " + std::to_string(attr_hash_code) + " implmentation was not found");
  }

  return it->second;
}

AttributeFactory::AttributeFactory() {
}

/**
 * DefaultAttributeFactory
 */

AttributeFactory::DefaultAttributeFactory::DefaultAttributeFactory() {
}

AttributeFactory::DefaultAttributeFactory::~DefaultAttributeFactory() {
}

AttributeImpl* AttributeFactory::DefaultAttributeFactory::CreateAttributeInstance(size_t attr_hash_code) {
  auto ctor = AttributeFactory::FindAttributeImplCtor(attr_hash_code);
  return ctor();
}

/**
 * AttributeSource
 */
AttributeSource::AttributeSource()
  : AttributeSource(new AttributeFactory::DefaultAttributeFactory()) {
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

void AttributeSource::AddAttributeImpl(AttributeImpl* attr_impl) {
  std::vector<size_t> attr_hash_codes = attr_impl->Attributes();
  std::shared_ptr<AttributeImpl> attr_impl_shptr(attr_impl);

  for(size_t attr_hash_code : attr_hash_codes) {
    attributes[attr_hash_code] = attr_impl_shptr;
  }

  attribute_impls[typeid(*attr_impl).hash_code()] = attr_impl_shptr;
}

bool AttributeSource::HasAttributes()  {
  return !attributes.empty();
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
    AttributeImpl* source_attr = current->attribute.get();
    size_t attr_impl_hash_code = typeid(*source_attr).hash_code();

    if(attribute_impls.find(attr_impl_hash_code) == attribute_impls.end()) {
      throw std::invalid_argument("AttributeSource::State contains AttributeImpl of type " + std::to_string(attr_impl_hash_code) + " that is not in this AttributeSource");
    }

    AttributeImpl* target_attr = attribute_impls[attr_impl_hash_code].get();
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
