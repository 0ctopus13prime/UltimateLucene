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
#include <Util/Attribute.h>
#include <typeinfo>
#include <sstream>
#include <stdexcept>

using lucene::core::analysis::tokenattributes::BytesTermAttribute;
using lucene::core::analysis::tokenattributes::BytesTermAttributeImpl;
using lucene::core::analysis::tokenattributes::CharTermAttribute;
using lucene::core::analysis::tokenattributes::CharTermAttributeImpl;
using lucene::core::analysis::tokenattributes::FlagsAttribute;
using lucene::core::analysis::tokenattributes::FlagsAttributeImpl;
using lucene::core::analysis::tokenattributes::KeywordAttribute;
using lucene::core::analysis::tokenattributes::KeywordAttributeImpl;
using lucene::core::analysis::tokenattributes::OffsetAttribute;
using lucene::core::analysis::tokenattributes::OffsetAttributeImpl;
using lucene::core::analysis::tokenattributes::PayloadAttribute;
using lucene::core::analysis::tokenattributes::PayloadAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionIncrementAttribute;
using lucene::core::analysis::tokenattributes::PositionIncrementAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionLengthAttribute;
using lucene::core::analysis::tokenattributes::PositionLengthAttributeImpl;
using lucene::core::analysis::tokenattributes::TermFrequencyAttribute;
using lucene::core::analysis::tokenattributes::TermFrequencyAttributeImpl;
using lucene::core::analysis::tokenattributes::TypeAttribute;
using lucene::core::analysis::tokenattributes::TypeAttributeImpl;
using lucene::core::util::AttributeFactory;
using lucene::core::util::AttributeImpl;
using lucene::core::util::AttributeImplGenerator;
using lucene::core::util::AttributeSource;

/**
 * AttributeImpl
 */
void AttributeImpl::End() {
  Clear();
}

std::string AttributeImpl::ReflectAsString(const bool prepend_att_class) {
  std::stringstream buf;
  AttributeReflector f = [&buf, &prepend_att_class]
                         (const std::string& att_class,
                          const std::string& key,
                          const std::string& value) {
    if (buf.tellp() > 0) {
      buf << ',';
    }

    if (prepend_att_class) {
      buf << att_class << '#';
    }

    buf << key << '=' << (value.empty() ? "null" : value);
  };

  ReflectWith(f);

  return buf.str();
}

/**
 * AttributeFactory
 */
std::unordered_map<std::type_index, AttributeImplGenerator>
AttributeFactory::ATTR_IMPL_TABLE = {
    {Attribute::TypeId<BytesTermAttribute>(),
      [](){ return new BytesTermAttributeImpl(); }},
    {Attribute::TypeId<CharTermAttribute>(),
      [](){ return new CharTermAttributeImpl(); }},
    {Attribute::TypeId<FlagsAttribute>(),
      [](){ return new FlagsAttributeImpl(); }},
    {Attribute::TypeId<KeywordAttribute>(),
      [](){ return new KeywordAttributeImpl(); }},
    {Attribute::TypeId<OffsetAttribute>(),
      [](){ return new OffsetAttributeImpl(); }},
    {Attribute::TypeId<PayloadAttribute>(),
      [](){ return new PayloadAttributeImpl(); }},
    {Attribute::TypeId<PositionIncrementAttribute>(),
      [](){ return new PositionIncrementAttributeImpl(); }},
    {Attribute::TypeId<PositionLengthAttribute>(),
      [](){ return new PositionLengthAttributeImpl(); }},
    {Attribute::TypeId<TermFrequencyAttribute>(),
      [](){ return new TermFrequencyAttributeImpl(); }},
    {Attribute::TypeId<TypeAttribute>(),
      [](){ return new TypeAttributeImpl(); }}
};

AttributeFactory::AttributeFactory() {
}

AttributeImplGenerator
AttributeFactory::FindAttributeImplGenerator(
const std::type_index attr_type_id) {
  auto it = AttributeFactory::ATTR_IMPL_TABLE.find(attr_type_id);

  if (it == AttributeFactory::ATTR_IMPL_TABLE.end()) {
    throw std::runtime_error("Attribute " + std::string(attr_type_id.name())
                                          + " implmentation was not found");
  }

  return it->second;
}

/**
 * DefaultAttributeFactory
 */
AttributeFactory::DefaultAttributeFactory DEFAULT_ATTRIBUTE_FACTORY();

AttributeFactory::DefaultAttributeFactory::DefaultAttributeFactory() { }

AttributeFactory::DefaultAttributeFactory::~DefaultAttributeFactory() { }

AttributeImpl*
AttributeFactory::DefaultAttributeFactory::CreateAttributeInstance
(std::type_index attr_type_id) {
  auto generator = AttributeFactory::FindAttributeImplGenerator(attr_type_id);
  return generator();
}

/**
 * AttributeSource
 */
AttributeSource::AttributeSource()
  : AttributeSource(DEFAULT_ATTRIBUTE_FACTORY) {
}

AttributeSource::AttributeSource(const AttributeSource& other)
  : state_holder(),
    attributes(),
    attribute_impls(),
    factory(other.factory) {
  for (auto& id_attrimpl : other.attributes) {
    AddAttributeImpl(id_attrimpl.second->Clone());
  }
}

AttributeSource::AttributeSource(AttributeFactory& factory)
  : state_holder(),
    attributes(),
    attribute_impls(),
    factory(factory) {
}

AttributeFactory& AttributeSource::GetAttributeFactory() const {
  return factory;
}

void AttributeSource::AddAttributeImpl(AttributeImpl* attr_impl) {
  std::vector<std::type_index> attr_type_ids = attr_impl->Attributes();
  std::shared_ptr<AttributeImpl> attr_impl_shptr(attr_impl);

  for (const std::type_index& attr_type_id : attr_type_ids) {
    attributes[attr_type_id] = attr_impl_shptr;
  }

  attribute_impls[std::type_index(typeid(*attr_impl))] = attr_impl_shptr;
  // Invalidate state to force recomputation in CaptureState()
  state_holder.ClearState();
}

bool AttributeSource::HasAttributes()  {
  return !attributes.empty();
}

AttributeSource::State* AttributeSource::GetCurrentState() {
  AttributeSource::State* current_state = state_holder.GetState();
  if (current_state != nullptr || !HasAttributes()) {
    return current_state;
  }

  state_holder.NewState();
  AttributeSource::State* head = state_holder.GetState();
  AttributeSource::State* c = head;
  auto it = attribute_impls.begin();
  c->attribute = it->second.get();
  it++;

  while (it != attribute_impls.end()) {
    c->next = new AttributeSource::State();
    c = c->next;
    c->attribute = it->second.get();
    it++;
  }

  return head;
}

void AttributeSource::ClearAttributes() {
  for (AttributeSource::State* state = GetCurrentState()
        ; state != nullptr
        ; state = state->next) {
    state->attribute->Clear();
  }
}

void AttributeSource::EndAttributes() {
  for (AttributeSource::State* state = GetCurrentState()
        ; state != nullptr
        ; state = state->next) {
    state->attribute->End();
  }
}

void AttributeSource::RemoveAllAttributes() {
  attributes.clear();
  attribute_impls.clear();
}

AttributeSource::State* AttributeSource::CaptureState() {
  AttributeSource::State* curr_state = GetCurrentState();
  if (curr_state == nullptr) {
    return nullptr;
  } else {
    return new AttributeSource::State(*curr_state);
  }
}

void AttributeSource::RestoreState(AttributeSource::State* state) {
  if (state == nullptr || state->attribute == nullptr) return;

  AttributeSource::State* current = state;
  do {
    AttributeImpl* source_attr = current->attribute;
    std::type_index attr_impl_type_id = std::type_index(typeid(*source_attr));

    auto it = attribute_impls.find(attr_impl_type_id);
    if (it == attribute_impls.end()) {
      throw std::invalid_argument(
            "AttributeSource::State contains AttributeImpl of type "
            + std::string(attr_impl_type_id.name())
            + " that is not in this AttributeSource");
    }

    AttributeImpl* target_attr = it->second.get();
    *target_attr = *source_attr;

    current = current->next;
  } while (current != nullptr);
}

std::string AttributeSource::ReflectAsString(const bool prepend_att) {
  std::stringstream buf;
  AttributeReflector reflector =
  [&buf, &prepend_att](const std::string& class_name,
                       const std::string& key,
                       const std::string& value){
    if (buf.tellp() > 0) {
      buf << ',';
    }
    if (prepend_att) {
      buf << class_name << '#';
    }
    buf << key << '=' << (value.empty() ? "null" : value);
  };
  ReflectWith(reflector);
  return buf.str();
}

void AttributeSource::ReflectWith(AttributeReflector& reflector)  {
  for (AttributeSource::State* state = GetCurrentState()
        ; state != nullptr
        ; state = state->next) {
    state->attribute->ReflectWith(reflector);
  }
}

AttributeSource& AttributeSource::operator=(const AttributeSource& other) {
  for (auto& id_attrimpl : other.attributes) {
    AddAttributeImpl(id_attrimpl.second->Clone());
  }
  factory = other.factory;
}

void AttributeSource::ShallowCopyTo(AttributeSource& other) const {
  other.state_holder = state_holder;
  other.attributes.clear();
  other.attributes = attributes;
  other.attribute_impls = attribute_impls;
  other.factory = factory;
}


/**
 *  AttributeSource::State
 */
AttributeSource::State::State(bool delete_attribute/*=false*/)
  : attribute(nullptr),
    next(nullptr),
    delete_attribute(delete_attribute) {
}

AttributeSource::State::State(const AttributeSource::State& other)
  : attribute(other.attribute == nullptr ? nullptr : other.attribute->Clone()),
    next(nullptr),
    delete_attribute(true) {
  AttributeSource::State* curr = this;
  for (AttributeSource::State* from = other.next
        ; from != nullptr
        ; from = from->next) {
    curr->next = new AttributeSource::State(true);
    curr->next->attribute = from->attribute->Clone();
    curr = curr->next;
  }
}

AttributeSource::State::State(AttributeSource::State&& other)
  : attribute(other.attribute),
    next(other.next),
    delete_attribute(other.delete_attribute) {
  other.delete_attribute = false;
  other.next = nullptr;
}

AttributeSource::State::~State() {
  CleanAttribute();
  if (next != nullptr) {
    next->~State();
  }
}

AttributeSource::State&
AttributeSource::State::operator=(const AttributeSource::State& other) {
  this->~State();

  attribute = other.attribute->Clone();
  delete_attribute = true;
  AttributeSource::State* curr = this;

  for (AttributeSource::State* from = other.next
        ; from != nullptr
        ; from = from->next) {
    curr->next = new AttributeSource::State(true);
    curr->next->attribute = from->attribute->Clone();
    curr = curr->next;
  }
}

AttributeSource::State&
AttributeSource::State::operator=(AttributeSource::State&& other) {
  this->~State();

  attribute = other.attribute;
  next = other.next;

  other.delete_attribute = false;
  other.next = nullptr;
}

void AttributeSource::State::CleanAttribute() noexcept {
  if (delete_attribute && attribute) {
    delete attribute;
    attribute = nullptr;
  }
}

/**
 * AttributeSource::StateHolder
 */
std::function<void(AttributeSource::State**)>
AttributeSource::StateHolder::SAFE_DELETER = [](State** state_ptr){
  if (*state_ptr != nullptr) {
    delete *state_ptr;
  }

  delete state_ptr;
};

AttributeSource::StateHolder::StateHolder()
  : state_ptr(new State*(nullptr), AttributeSource::StateHolder::SAFE_DELETER) {
}

void AttributeSource::StateHolder::ResetState(State* state) {
  if (*state_ptr != nullptr) {
    delete *state_ptr;
  }

  *state_ptr = state;
}

AttributeSource::State* AttributeSource::StateHolder::GetState() {
  return *state_ptr;
}

void AttributeSource::StateHolder::NewState() {
  ResetState(new AttributeSource::State());
}

void AttributeSource::StateHolder::ClearState() {
  ResetState(nullptr);
}
