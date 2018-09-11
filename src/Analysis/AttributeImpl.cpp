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
#include <Util/ArrayUtil.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using lucene::core::util::BytesRef;
using lucene::core::util::AttributeImpl;
using lucene::core::util::AttributeReflector;
using lucene::core::util::arrayutil::CopyOfRange;
using lucene::core::util::arrayutil::Oversize;
using lucene::core::util::arrayutil::Grow;
using lucene::core::util::arrayutil::CheckFromToIndex;
using lucene::core::util::arrayutil::CheckIndex;
using lucene::core::util::arrayutil::CheckFromIndexSize;
using lucene::core::analysis::tokenattributes::BytesTermAttributeImpl;
using lucene::core::analysis::tokenattributes::BytesTermAttribute;
using lucene::core::analysis::tokenattributes::PackedTokenAttributeImpl;
using lucene::core::analysis::tokenattributes::CharTermAttributeImpl;
using lucene::core::analysis::tokenattributes::CharTermAttribute;
using lucene::core::analysis::tokenattributes::TermFrequencyAttribute;
using lucene::core::analysis::tokenattributes::TermFrequencyAttributeImpl;
using lucene::core::analysis::tokenattributes::OffsetAttribute;
using lucene::core::analysis::tokenattributes::OffsetAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionLengthAttribute;
using lucene::core::analysis::tokenattributes::PositionLengthAttributeImpl;
using lucene::core::analysis::tokenattributes::PositionIncrementAttribute;
using lucene::core::analysis::tokenattributes::PositionIncrementAttributeImpl;
using lucene::core::analysis::tokenattributes::TypeAttribute;
using lucene::core::analysis::tokenattributes::TypeAttributeImpl;
using lucene::core::analysis::tokenattributes::TermToBytesRefAttribute;
using lucene::core::analysis::tokenattributes::PackedTokenAttributeImpl;
using lucene::core::analysis::tokenattributes::PayloadAttribute;
using lucene::core::analysis::tokenattributes::PayloadAttributeImpl;
using lucene::core::analysis::tokenattributes::KeywordAttribute;
using lucene::core::analysis::tokenattributes::KeywordAttributeImpl;
using lucene::core::analysis::tokenattributes::FlagsAttribute;
using lucene::core::analysis::tokenattributes::FlagsAttributeImpl;

/**
 * BytesTermAttributeImpl
 */

BytesTermAttributeImpl::BytesTermAttributeImpl()
  : bytes() {
}

BytesTermAttributeImpl::BytesTermAttributeImpl(
                                  const BytesTermAttributeImpl& other)
  : bytes(other.bytes) {
}

BytesRef& BytesTermAttributeImpl::GetBytesRef() {
  return bytes;
}

BytesTermAttributeImpl::~BytesTermAttributeImpl() {
}

void BytesTermAttributeImpl::SetBytesRef(BytesRef& other_bytes) {
  bytes = other_bytes;
}

void BytesTermAttributeImpl::Clear() {
  bytes = BytesRef();
}

void BytesTermAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("BytesTermAttributeImpl", "bytes", bytes.UTF8ToString());
}

bool
BytesTermAttributeImpl::operator==(const BytesTermAttributeImpl& other) const {
  if (this == &other) {
    return true;
  }

  return (bytes == other.bytes);
}

std::vector<std::type_index> BytesTermAttributeImpl::Attributes() {
  return {Attribute::TypeId<BytesTermAttribute>()};
}

void BytesTermAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    BytesTermAttributeImpl& other =
      dynamic_cast<BytesTermAttributeImpl&>(attr_impl);
    bytes.ShallowCopyTo(other.bytes);
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

BytesTermAttributeImpl&
BytesTermAttributeImpl::operator=(const AttributeImpl& other) {
  const BytesTermAttributeImpl& other_impl =
    dynamic_cast<const BytesTermAttributeImpl&>(other);
  bytes = other_impl.bytes;
  return *this;
}

BytesTermAttributeImpl&
BytesTermAttributeImpl::operator=(const BytesTermAttributeImpl& other) {
  return
    BytesTermAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
}

AttributeImpl* BytesTermAttributeImpl::Clone() {
  return new BytesTermAttributeImpl(*this);
}


/**
 * FlagsAttributeImpl
 */

FlagsAttributeImpl::FlagsAttributeImpl()
  : flags(0) {
}

FlagsAttributeImpl::FlagsAttributeImpl(const FlagsAttributeImpl& other)
  : flags(other.flags) {
}

FlagsAttributeImpl::~FlagsAttributeImpl() {
}

int32_t FlagsAttributeImpl::GetFlags() {
  return flags;
}

void FlagsAttributeImpl::SetFlags(const int32_t new_flags) {
  flags = new_flags;
}

void FlagsAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("FlagsAttribute", "flags", std::to_string(flags));
}

void FlagsAttributeImpl::Clear() {
  flags = 0;
}

bool FlagsAttributeImpl::operator==(const FlagsAttributeImpl& other) const {
  return (flags == other.flags);
}

std::vector<std::type_index> FlagsAttributeImpl::Attributes() {
  return {Attribute::TypeId<FlagsAttribute>()};
}

void FlagsAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    FlagsAttributeImpl& other = dynamic_cast<FlagsAttributeImpl&>(attr_impl);
    other.flags = flags;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

FlagsAttributeImpl& FlagsAttributeImpl::operator=(const AttributeImpl& other) {
  const FlagsAttributeImpl& other_impl
    = dynamic_cast<const FlagsAttributeImpl&>(other);
  flags = other_impl.flags;
  return *this;
}

FlagsAttributeImpl&
FlagsAttributeImpl::operator=(const FlagsAttributeImpl& other) {
  return
    FlagsAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
}

AttributeImpl* FlagsAttributeImpl::Clone() {
  return new FlagsAttributeImpl(*this);
}

/**
 * KeywordAttributeImpl
 */

KeywordAttributeImpl::KeywordAttributeImpl()
  : keyword(false) {
}

KeywordAttributeImpl::KeywordAttributeImpl(const KeywordAttributeImpl& other)
  : keyword(other.keyword) {
}

KeywordAttributeImpl::~KeywordAttributeImpl() {
}

bool KeywordAttributeImpl::IsKeyword() {
  return keyword;
}

void KeywordAttributeImpl::SetKeyword(const bool new_keyword) {
  keyword = new_keyword;
}

void KeywordAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("KeywordAttribute", "keyword", (keyword ? "true" : "false"));
}

void KeywordAttributeImpl::Clear() {
  keyword = false;
}

bool KeywordAttributeImpl::operator==(const KeywordAttributeImpl& other) const {
  return (keyword == other.keyword);
}

std::vector<std::type_index> KeywordAttributeImpl::Attributes() {
  return {Attribute::TypeId<KeywordAttribute>()};
}

void KeywordAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    KeywordAttributeImpl& other
      = dynamic_cast<KeywordAttributeImpl&>(attr_impl);
    other.keyword = keyword;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

KeywordAttributeImpl&
KeywordAttributeImpl::operator=(const AttributeImpl& other) {
  const KeywordAttributeImpl& other_impl
    = dynamic_cast<const KeywordAttributeImpl&>(other);
  keyword = other_impl.keyword;

  return *this;
}

KeywordAttributeImpl&
KeywordAttributeImpl::operator=(const KeywordAttributeImpl& other) {
  return
    KeywordAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
}

AttributeImpl* KeywordAttributeImpl::Clone() {
  return new KeywordAttributeImpl(*this);
}

/**
 * OffsetAttributeImpl
 */

OffsetAttributeImpl::OffsetAttributeImpl()
  : start_offset(0),
    end_offset(0) {
}

OffsetAttributeImpl::OffsetAttributeImpl(const OffsetAttributeImpl& other)
  : start_offset(other.start_offset),
    end_offset(other.end_offset) {
}

OffsetAttributeImpl::~OffsetAttributeImpl() {
}

uint32_t OffsetAttributeImpl::StartOffset() {
  return start_offset;
}

void OffsetAttributeImpl::SetOffset(const uint32_t new_start_offset,
                                    const uint32_t new_end_offset) {
  start_offset = new_start_offset;
  end_offset = new_end_offset;
}

uint32_t OffsetAttributeImpl::EndOffset() {
  return end_offset;
}

void OffsetAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("OffsetAttribute", "startOffset", std::to_string(start_offset));
  reflector("OffsetAttribute", "endOffset", std::to_string(end_offset));
}

void OffsetAttributeImpl::Clear() {
  start_offset = end_offset = 0;
}

bool OffsetAttributeImpl::operator==(const OffsetAttributeImpl& other) const {
  return (start_offset == other.start_offset && end_offset == other.end_offset);
}

std::vector<std::type_index> OffsetAttributeImpl::Attributes() {
  return {Attribute::TypeId<OffsetAttribute>()};
}

void OffsetAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    OffsetAttributeImpl& other = dynamic_cast<OffsetAttributeImpl&>(attr_impl);
    other.start_offset = start_offset;
    other.end_offset = end_offset;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

OffsetAttributeImpl&
OffsetAttributeImpl::operator=(const AttributeImpl& other) {
  const OffsetAttributeImpl& other_impl
    = dynamic_cast<const OffsetAttributeImpl&>(other);
  start_offset = other_impl.start_offset;
  end_offset = other_impl.end_offset;

  return *this;
}

OffsetAttributeImpl&
OffsetAttributeImpl::operator=(const OffsetAttributeImpl& other) {
  return
    OffsetAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
}

AttributeImpl* OffsetAttributeImpl::Clone() {
  return new OffsetAttributeImpl(*this);
}

/**
 * PayloadAttributeImpl
 */

PayloadAttributeImpl::PayloadAttributeImpl()
  : payload() {
}

PayloadAttributeImpl::PayloadAttributeImpl(const PayloadAttributeImpl& other)
  : payload(other.payload) {
}

PayloadAttributeImpl::~PayloadAttributeImpl() {
}

void PayloadAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("PayloadAttribute", "payload", payload.UTF8ToString());
}

void PayloadAttributeImpl::Clear() {
  payload = BytesRef();
}

bool PayloadAttributeImpl::operator==(const PayloadAttributeImpl& other) const {
  if (this == &other) {
    return true;
  }

  return (payload == other.payload);
}

BytesRef& PayloadAttributeImpl::GetPayload() {
  return payload;
}

void PayloadAttributeImpl::SetPayload(BytesRef& new_payload) {
  payload = new_payload;
}

std::vector<std::type_index> PayloadAttributeImpl::Attributes() {
  return {Attribute::TypeId<PayloadAttribute>()};
}

void PayloadAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PayloadAttributeImpl& other =
      dynamic_cast<PayloadAttributeImpl&>(attr_impl);
    payload.ShallowCopyTo(other.payload);
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

PayloadAttributeImpl&
PayloadAttributeImpl::operator=(const AttributeImpl& other) {
  const PayloadAttributeImpl& other_impl
    = dynamic_cast<const PayloadAttributeImpl&>(other);
  payload = other_impl.payload;

  return *this;
}

PayloadAttributeImpl&
PayloadAttributeImpl::operator=(const PayloadAttributeImpl& other) {
  return
    PayloadAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
}

AttributeImpl* PayloadAttributeImpl::Clone() {
  return new PayloadAttributeImpl(*this);
}

/**
 * PositionIncrementAttributeImpl
 */

PositionIncrementAttributeImpl::PositionIncrementAttributeImpl()
  : position_increment(1) {
}

PositionIncrementAttributeImpl::PositionIncrementAttributeImpl(
                                  const PositionIncrementAttributeImpl& other)
  : position_increment(other.position_increment) {
}

PositionIncrementAttributeImpl::~PositionIncrementAttributeImpl() {
}

void PositionIncrementAttributeImpl::SetPositionIncrement(
                                    const uint32_t new_position_increment) {
  position_increment = new_position_increment;
}

uint32_t PositionIncrementAttributeImpl::GetPositionIncrement() {
  return position_increment;
}

void PositionIncrementAttributeImpl::ReflectWith(
                                    AttributeReflector& reflector) {
  reflector("PositionIncrementAttribute",
            "positionIncrement",
            std::to_string(position_increment));
}

void PositionIncrementAttributeImpl::Clear() {
  position_increment = 1;
}

void PositionIncrementAttributeImpl::End() {
  position_increment = 0;
}

bool PositionIncrementAttributeImpl::operator==(
                          const PositionIncrementAttributeImpl& other) const {
  if (this == &other) {
    return true;
  }

  return (position_increment == other.position_increment);
}

std::vector<std::type_index> PositionIncrementAttributeImpl::Attributes() {
  return {Attribute::TypeId<PositionIncrementAttribute>()};
}

void PositionIncrementAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PositionIncrementAttributeImpl& other =
      dynamic_cast<PositionIncrementAttributeImpl&>(attr_impl);
    other.position_increment = position_increment;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

PositionIncrementAttributeImpl&
PositionIncrementAttributeImpl::operator=(const AttributeImpl& other) {
  const PositionIncrementAttributeImpl& other_impl
    = dynamic_cast<const PositionIncrementAttributeImpl&>(other);
  position_increment = other_impl.position_increment;

  return *this;
}

PositionIncrementAttributeImpl& PositionIncrementAttributeImpl::operator=(
                                const PositionIncrementAttributeImpl& other) {
  return PositionIncrementAttributeImpl::operator=(
                                  static_cast<const AttributeImpl&>(other));
}

AttributeImpl* PositionIncrementAttributeImpl::Clone() {
  return new PositionIncrementAttributeImpl(*this);
}

/**
 * PositionLengthAttributeImpl
 */
PositionLengthAttributeImpl::PositionLengthAttributeImpl()
  : position_length(1) {
}

PositionLengthAttributeImpl::PositionLengthAttributeImpl(
                                      const PositionLengthAttributeImpl& other)
  : position_length(other.position_length) {
}

PositionLengthAttributeImpl::~PositionLengthAttributeImpl() {
}

void PositionLengthAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("PositionLengthAttribute",
            "positionLength",
            std::to_string(position_length));
}

void PositionLengthAttributeImpl::Clear() {
  position_length = 1;
}

bool PositionLengthAttributeImpl::operator==(
                              const PositionLengthAttributeImpl& other) const {
  if (this == &other) {
    return true;
  }

  return (position_length == other.position_length);
}

void
PositionLengthAttributeImpl::SetPositionLength(uint32_t new_position_length) {
  position_length = new_position_length;
}

uint32_t PositionLengthAttributeImpl::GetPositionLength() {
  return position_length;
}

std::vector<std::type_index> PositionLengthAttributeImpl::Attributes() {
  return {Attribute::TypeId<PositionLengthAttribute>()};
}

void PositionLengthAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PositionLengthAttributeImpl& other =
      dynamic_cast<PositionLengthAttributeImpl&>(attr_impl);
    other.position_length = position_length;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

PositionLengthAttributeImpl&
PositionLengthAttributeImpl::operator=(const AttributeImpl& other) {
  const PositionLengthAttributeImpl& other_impl
    = dynamic_cast<const PositionLengthAttributeImpl&>(other);
  position_length = other_impl.position_length;

  return *this;
}

PositionLengthAttributeImpl& PositionLengthAttributeImpl::operator=(
                                    const PositionLengthAttributeImpl& other) {
  return PositionLengthAttributeImpl::operator=(
                                    static_cast<const AttributeImpl&>(other));
}

AttributeImpl* PositionLengthAttributeImpl::Clone() {
  return new PositionLengthAttributeImpl(*this);
}

/**
 * TermFrequencyAttributeImpl
 */

TermFrequencyAttributeImpl::TermFrequencyAttributeImpl()
  : term_frequency(1) {
}

TermFrequencyAttributeImpl::TermFrequencyAttributeImpl(
                                        const TermFrequencyAttributeImpl& other)
  : term_frequency(other.term_frequency) {
}

TermFrequencyAttributeImpl::~TermFrequencyAttributeImpl() {
}

void TermFrequencyAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("TermFrequencyAttribute",
            "termFrequency",
            std::to_string(term_frequency));
}
void TermFrequencyAttributeImpl::Clear() {
  term_frequency = 1;
}

bool TermFrequencyAttributeImpl::operator==(
                                const TermFrequencyAttributeImpl& other) const {
  if (this == &other) {
    return true;
  }

  return (term_frequency == other.term_frequency);
}

void TermFrequencyAttributeImpl::SetTermFrequency(uint32_t new_term_frequency) {
  term_frequency = new_term_frequency;
}

uint32_t TermFrequencyAttributeImpl::GetTermFrequency() {
  return term_frequency;
}

std::vector<std::type_index> TermFrequencyAttributeImpl::Attributes() {
  return {Attribute::TypeId<TermFrequencyAttribute>()};
}

void TermFrequencyAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    TermFrequencyAttributeImpl& other =
      dynamic_cast<TermFrequencyAttributeImpl&>(attr_impl);
    other.term_frequency = term_frequency;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

TermFrequencyAttributeImpl&
TermFrequencyAttributeImpl::operator=(const AttributeImpl& other) {
  const TermFrequencyAttributeImpl& other_impl
    = dynamic_cast<const TermFrequencyAttributeImpl&>(other);
  term_frequency = other_impl.term_frequency;

  return *this;
}

TermFrequencyAttributeImpl&
TermFrequencyAttributeImpl::operator=(const TermFrequencyAttributeImpl& other) {
  return TermFrequencyAttributeImpl::operator=(
                                      static_cast<const AttributeImpl&>(other));
}

AttributeImpl* TermFrequencyAttributeImpl::Clone() {
  return new TermFrequencyAttributeImpl(*this);
}

/**
 * TypeAttributeImpl
 */
TypeAttributeImpl::TypeAttributeImpl()
  : type() {
}

TypeAttributeImpl::TypeAttributeImpl(const TypeAttributeImpl& other)
  : type(other.type) {
}

TypeAttributeImpl::~TypeAttributeImpl() {
}

void TypeAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("TypeAttribute", "type", type);
}

void TypeAttributeImpl::Clear() {
  type = TypeAttribute::DEFAULT_TYPE;
}

bool TypeAttributeImpl::operator==(const TypeAttributeImpl& other) const {
  if (this == &other) {
    return true;
  }

  return (type == other.type);
}

std::string& TypeAttributeImpl::Type() {
  return type;
}

void TypeAttributeImpl::SetType(const std::string& new_type) {
  type = new_type;
}

std::vector<std::type_index> TypeAttributeImpl::Attributes() {
  return {Attribute::TypeId<TypeAttribute>()};
}

void TypeAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    TypeAttributeImpl& other = dynamic_cast<TypeAttributeImpl&>(attr_impl);
    // It's acutally a deep copy.
    other.type = type;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

TypeAttributeImpl& TypeAttributeImpl::operator=(const AttributeImpl& other) {
  const TypeAttributeImpl& other_impl =
    dynamic_cast<const TypeAttributeImpl&>(other);
  type = other_impl.type;

  return *this;
}

TypeAttributeImpl&
TypeAttributeImpl::operator=(const TypeAttributeImpl& other) {
  return TypeAttributeImpl::operator=(static_cast<const AttributeImpl&>(other));
}

AttributeImpl* TypeAttributeImpl::Clone() {
  return new TypeAttributeImpl(*this);
}

/**
 * CharTermAttributeImpl
 */

CharTermAttributeImpl::CharTermAttributeImpl()
  : term_buffer(
      new char[CharTermAttributeImpl::CHAR_TERM_ATTRIBUTE_IMPL_MIN_BUFFER_SIZE],
      std::default_delete<char[]>()),
    term_capacity(
      CharTermAttributeImpl::CHAR_TERM_ATTRIBUTE_IMPL_MIN_BUFFER_SIZE),
    term_length(0),
    builder() {
}

CharTermAttributeImpl::CharTermAttributeImpl(const CharTermAttributeImpl& other)
  : term_buffer(CopyOfRange(other.term_buffer.get(),
                                       0,
                                       other.term_capacity),
                std::default_delete<char>()),
    term_capacity(other.term_capacity),
    term_length(other.term_length),
    builder() {
}

CharTermAttributeImpl::~CharTermAttributeImpl() { }

void CharTermAttributeImpl::GrowTermBuffer(const uint32_t new_size) {
  if (term_capacity < new_size) {
      // Not big enough; create a new array with slight
      // Over allocation:
      term_capacity = Oversize<char>(new_size);
      term_buffer.reset(new char[term_capacity]);
  }
}

BytesRef& CharTermAttributeImpl::GetBytesRef() {
  builder.CopyBytes(term_buffer.get(), 0, term_length);
  return builder.Get();
}

void CharTermAttributeImpl::Clear() {
  term_length = 0;
}

void CharTermAttributeImpl::CopyBuffer(const char* buffer,
                                       const uint32_t offset,
                                       const uint32_t length) {
  GrowTermBuffer(length);
  std::memcpy(term_buffer.get(), buffer + offset, length);
  term_length = length;
}

char* CharTermAttributeImpl::Buffer() const {
  return term_buffer.get();
}

char* CharTermAttributeImpl::ResizeBuffer(const uint32_t new_capacity) {
  if (term_capacity < new_capacity) {
    std::pair<char*, uint32_t> new_term_buffer_info
      = Grow(term_buffer.get(), term_capacity, new_capacity);
    if (new_term_buffer_info.first != term_buffer.get()) {
      term_buffer.reset(new_term_buffer_info.first);
      term_capacity = new_term_buffer_info.second;
    }
  }

  return term_buffer.get();
}

uint32_t CharTermAttributeImpl::Length() const {
  return term_length;
}

std::string CharTermAttributeImpl::SubSequence(const uint32_t inclusive_start,
                                               const uint32_t exclusive_end) {
  CheckFromToIndex(inclusive_start, exclusive_end, term_length);
  return std::string(term_buffer.get(),
                     inclusive_start,
                     exclusive_end - inclusive_start);
}

CharTermAttributeImpl& CharTermAttributeImpl::SetLength(const uint32_t length) {
  CheckFromIndexSize(0, length, term_capacity);
  term_length = length;
  return *this;
}

CharTermAttributeImpl& CharTermAttributeImpl::SetEmpty() {
  term_length = 0;
  return *this;
}

CharTermAttribute& CharTermAttributeImpl::Append(const std::string& csq) {
  return Append(csq, 0, csq.size());
}

CharTermAttribute& CharTermAttributeImpl::Append(const std::string& csq,
                                                 const uint32_t inclusive_start,
                                                 const uint32_t exclusive_end) {
  CheckFromToIndex(inclusive_start, exclusive_end, csq.size());
  uint32_t len = (exclusive_end - inclusive_start);
  if (len == 0) return *this;
  ResizeBuffer(term_length + len);

  std::memcpy(term_buffer.get() + term_length,
              csq.c_str() + inclusive_start,
              len);
  term_length += len;
  return *this;
}

CharTermAttribute& CharTermAttributeImpl::Append(const char c) {
  ResizeBuffer(term_length + 1)[term_length++] = c;
  return *this;
}

CharTermAttribute&
CharTermAttributeImpl::Append(const CharTermAttribute& term_att) {
  const int32_t len = term_att.Length();
  std::memcpy(ResizeBuffer(term_length + len) + term_length,
              term_att.Buffer(),
              len);
  term_length += len;
  return *this;
}

void CharTermAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  reflector("CharTermAttribute",
            "term",
            std::string(term_buffer.get(), 0, term_length));
  reflector("TermToBytesRefAttribute", "bytes", GetBytesRef().UTF8ToString());
}

char& CharTermAttributeImpl::operator[](const uint32_t index) {
  CheckIndex(index, term_length);
  return term_buffer.get()[index];
}

bool
CharTermAttributeImpl::operator==(const CharTermAttributeImpl& other) const {
  if (term_length == other.term_length) {
    return
    (std::memcmp(term_buffer.get(), other.term_buffer.get(), term_length) == 0);
  }

  return false;
}

std::vector<std::type_index> CharTermAttributeImpl::Attributes() {
  return {Attribute::TypeId<CharTermAttribute>(),
          Attribute::TypeId<TermToBytesRefAttribute>()};
}

void CharTermAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    CharTermAttributeImpl& other =
      dynamic_cast<CharTermAttributeImpl&>(attr_impl);
    other.term_buffer = term_buffer;
    other.term_capacity = term_capacity;
    other.term_length = term_length;
    builder.Get().ShallowCopyTo(other.builder.Get());
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

CharTermAttributeImpl&
CharTermAttributeImpl::operator=(const AttributeImpl& other) {
  const CharTermAttributeImpl& other_impl
    = dynamic_cast<const CharTermAttributeImpl&>(other);

  CopyBuffer(other_impl.term_buffer.get(), 0, other_impl.term_capacity);
  term_length = other_impl.term_length;
  builder.Get() = const_cast<CharTermAttributeImpl&>(other_impl).builder.Get();

  return *this;
}

CharTermAttributeImpl&
CharTermAttributeImpl::operator=(const CharTermAttributeImpl& other) {
  return CharTermAttributeImpl::operator=(
                                      static_cast<const AttributeImpl&>(other));
}

AttributeImpl* CharTermAttributeImpl::Clone() {
  return new CharTermAttributeImpl(*this);
}

/**
 *  PackedTokenAttributeImpl
 */

PackedTokenAttributeImpl::PackedTokenAttributeImpl()
  : CharTermAttributeImpl(),
    start_offset(0),
    end_offset(0),
    type(TypeAttribute::DEFAULT_TYPE),
    position_increment(0),
    position_length(0),
    term_frequency(0) {
}

PackedTokenAttributeImpl::PackedTokenAttributeImpl(
                                          const PackedTokenAttributeImpl& other)
  : CharTermAttributeImpl(other),
    start_offset(other.start_offset),
    end_offset(other.end_offset),
    type(other.type),
    position_increment(other.position_increment),
    position_length(other.position_length),
    term_frequency(other.term_frequency) {
}

PackedTokenAttributeImpl::~PackedTokenAttributeImpl() {
}

std::string& PackedTokenAttributeImpl::Type() {
  return type;
}

void PackedTokenAttributeImpl::SetType(const std::string& new_type) {
  type = new_type;
}

void PackedTokenAttributeImpl::SetPositionIncrement(
                                        const uint32_t new_position_increment) {
  position_increment = new_position_increment;
}

uint32_t PackedTokenAttributeImpl::GetPositionIncrement() {
  return position_increment;
}

void PackedTokenAttributeImpl::SetPositionLength(uint32_t new_position_length) {
  position_length = new_position_length;
}

uint32_t PackedTokenAttributeImpl::GetPositionLength() {
  return position_length;
}

uint32_t PackedTokenAttributeImpl::StartOffset() {
  return start_offset;
}

void PackedTokenAttributeImpl::SetOffset(const uint32_t new_start_offset,
                                         const uint32_t new_end_offset) {
  start_offset = new_start_offset;
  end_offset = new_end_offset;
}

uint32_t PackedTokenAttributeImpl::EndOffset() {
  return end_offset;
}

void PackedTokenAttributeImpl::SetTermFrequency(uint32_t new_term_frequency) {
  term_frequency = new_term_frequency;
}

uint32_t PackedTokenAttributeImpl::GetTermFrequency() {
  return term_frequency;
}

PackedTokenAttributeImpl&
PackedTokenAttributeImpl::operator=(const AttributeImpl& other) {
  const PackedTokenAttributeImpl& other_impl
    = dynamic_cast<const PackedTokenAttributeImpl&>(other);

  CharTermAttributeImpl::operator=(
                        static_cast<const CharTermAttributeImpl&>(other_impl));
  start_offset = other_impl.start_offset;
  end_offset = other_impl.end_offset;
  type = other_impl.type;
  position_increment = other_impl.position_increment;
  position_length = other_impl.position_length;
  term_frequency = other_impl.term_frequency;

  return *this;
}

PackedTokenAttributeImpl&
PackedTokenAttributeImpl::operator=(const PackedTokenAttributeImpl& other) {
  return PackedTokenAttributeImpl::operator=(
                                      static_cast<const AttributeImpl&>(other));
}

std::vector<std::type_index> PackedTokenAttributeImpl::Attributes() {
  return {
    Attribute::TypeId<CharTermAttribute>(),
    Attribute::TypeId<TermToBytesRefAttribute>(),
    Attribute::TypeId<TypeAttribute>(),
    Attribute::TypeId<PositionIncrementAttribute>(),
    Attribute::TypeId<PositionLengthAttribute>(),
    Attribute::TypeId<OffsetAttribute>(),
    Attribute::TypeId<TermFrequencyAttribute>()
  };
}

void PackedTokenAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PackedTokenAttributeImpl& other =
      dynamic_cast<PackedTokenAttributeImpl&>(attr_impl);
    CharTermAttributeImpl::ShallowCopyTo(attr_impl);
    other.start_offset = start_offset;
    other.end_offset = end_offset;
    other.type = type;
    other.position_increment = position_increment;
    other.position_length = position_length;
    other.term_frequency = term_frequency;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. "
                                + std::string(typeid(*this).name())
                                + " -> " + typeid(attr_impl).name());
  }
}

AttributeImpl* PackedTokenAttributeImpl::Clone() {
  return new PackedTokenAttributeImpl(*this);
}
