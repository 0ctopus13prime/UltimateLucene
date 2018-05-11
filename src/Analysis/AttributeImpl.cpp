#include <memory>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <Analysis/AttributeImpl.h>
#include <Util/ArrayUtil.h>
#include <Util/Attribute.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis::tokenattributes;

/**
 * BytesTermAttributeImpl
 */

BytesTermAttributeImpl::BytesTermAttributeImpl()
  : bytes() {
}

BytesTermAttributeImpl::BytesTermAttributeImpl(const BytesTermAttributeImpl& other)
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
  // TODO Implement it.
}

bool BytesTermAttributeImpl::operator==(BytesTermAttributeImpl& other) {
  if(this == &other) {
    return true;
  }

  return (bytes == other.bytes);
}

std::vector<size_t> BytesTermAttributeImpl::Attributes() {
  return {typeid(BytesTermAttribute).hash_code()};
}

void BytesTermAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    BytesTermAttributeImpl& other = dynamic_cast<BytesTermAttributeImpl&>(attr_impl);
    bytes.ShallowCopyTo(other.bytes);
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

BytesTermAttributeImpl& BytesTermAttributeImpl::operator=(const BytesTermAttributeImpl& other) {
  bytes = other.bytes;
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

int FlagsAttributeImpl::GetFlags() {
  return flags;
}

void FlagsAttributeImpl::SetFlags(const int new_flags) {
  flags = new_flags;
}

void FlagsAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  // TODO Implement it.
}

void FlagsAttributeImpl::Clear() {
  flags = 0;
}

bool FlagsAttributeImpl::operator==(const FlagsAttributeImpl& other) {
  return (flags = other.flags);
}

std::vector<size_t> FlagsAttributeImpl::Attributes() {
  return {typeid(FlagsAttribute).hash_code()};
}

void FlagsAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    FlagsAttributeImpl& other = dynamic_cast<FlagsAttributeImpl&>(attr_impl);
    other.flags = flags;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

FlagsAttributeImpl& FlagsAttributeImpl::operator=(const FlagsAttributeImpl& other) {
  flags = other.flags;
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
  // TODO Implement it
}

void KeywordAttributeImpl::Clear() {
  keyword = false;
}

bool KeywordAttributeImpl::operator==(const KeywordAttributeImpl& other) {
  return (keyword == other.keyword);
}

std::vector<size_t> KeywordAttributeImpl::Attributes() {
  return {typeid(KeywordAttribute).hash_code()};
}

void KeywordAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    KeywordAttributeImpl& other = dynamic_cast<KeywordAttributeImpl&>(attr_impl);
    other.keyword = keyword;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

KeywordAttributeImpl& KeywordAttributeImpl::operator=(const KeywordAttributeImpl& other) {
  keyword = other.keyword;
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

int OffsetAttributeImpl::StartOffset() {
  return start_offset;
}

void OffsetAttributeImpl::SetOffset(const unsigned int new_start_offset, const unsigned int new_end_offset) {
  start_offset = new_start_offset;
  end_offset = new_end_offset;
}

int OffsetAttributeImpl::EndOffset() {
  return end_offset;
}

void OffsetAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  // TODO Implement it.
}

void OffsetAttributeImpl::Clear() {
  start_offset = end_offset = 0;
}

bool OffsetAttributeImpl::operator==(const OffsetAttributeImpl& other) {
  return (start_offset == other.start_offset && end_offset == other.end_offset);
}

std::vector<size_t> OffsetAttributeImpl::Attributes() {
  return {typeid(OffsetAttribute).hash_code()};
}

void OffsetAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    OffsetAttributeImpl& other = dynamic_cast<OffsetAttributeImpl&>(attr_impl);
    other.start_offset = start_offset;
    other.end_offset = end_offset;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

OffsetAttributeImpl& OffsetAttributeImpl::operator=(const OffsetAttributeImpl& other) {
  start_offset = other.start_offset;
  end_offset = other.end_offset;
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
  // TODO Implement it.
}

void PayloadAttributeImpl::Clear() {
  payload = BytesRef();
}

bool PayloadAttributeImpl::operator==(PayloadAttributeImpl& other) {
  if(this == &other) {
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

std::vector<size_t> PayloadAttributeImpl::Attributes() {
  return {typeid(PayloadAttribute).hash_code()};
}

void PayloadAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PayloadAttributeImpl& other = dynamic_cast<PayloadAttributeImpl&>(attr_impl);
    payload.ShallowCopyTo(other.payload);
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

PayloadAttributeImpl& PayloadAttributeImpl::operator=(const PayloadAttributeImpl& other) {
  payload = other.payload;
}

/**
 * PositionIncrementAttributeImpl
 */

PositionIncrementAttributeImpl::PositionIncrementAttributeImpl()
  : position_increment(1) {
}

PositionIncrementAttributeImpl::PositionIncrementAttributeImpl(const PositionIncrementAttributeImpl& other)
  : position_increment(other.position_increment) {
}

PositionIncrementAttributeImpl::~PositionIncrementAttributeImpl() {
}

void PositionIncrementAttributeImpl::SetPositionIncrement(const unsigned int new_position_increment) {
  position_increment = new_position_increment;
}

unsigned int PositionIncrementAttributeImpl::GetPositionIncrement() {
  return position_increment;
}

void PositionIncrementAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  // TODO Implement it.
}

void PositionIncrementAttributeImpl::Clear() {
  position_increment = 1;
}

void PositionIncrementAttributeImpl::End() {
  position_increment = 0;
}

bool PositionIncrementAttributeImpl::operator==(PositionIncrementAttributeImpl& other) {
  if(this == &other) {
    return true;
  }

  return (position_increment == other.position_increment);
}

std::vector<size_t> PositionIncrementAttributeImpl::Attributes() {
  return {typeid(PositionIncrementAttribute).hash_code()};
}

void PositionIncrementAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PositionIncrementAttributeImpl& other = dynamic_cast<PositionIncrementAttributeImpl&>(attr_impl);
    other.position_increment = position_increment;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

PositionIncrementAttributeImpl& PositionIncrementAttributeImpl::operator=(const PositionIncrementAttributeImpl& other) {
  position_increment = other.position_increment;
}

/**
 * PositionLengthAttributeImpl
 */
PositionLengthAttributeImpl::PositionLengthAttributeImpl()
  : position_length(1) {
}

PositionLengthAttributeImpl::PositionLengthAttributeImpl(const PositionLengthAttributeImpl& other)
  : position_length(other.position_length) {
}

PositionLengthAttributeImpl::~PositionLengthAttributeImpl() {
}

void PositionLengthAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  // TODO Implement it
}

void PositionLengthAttributeImpl::Clear() {
  position_length = 1;
}

bool PositionLengthAttributeImpl::operator==(PositionLengthAttributeImpl& other) {
  if(this == &other) {
    return true;
  }

  return (position_length == other.position_length);
}

void PositionLengthAttributeImpl::SetPositionLength(unsigned int new_position_length) {
  position_length = new_position_length;
}

unsigned int PositionLengthAttributeImpl::GetPositionLength() {
  return position_length;
}

std::vector<size_t> PositionLengthAttributeImpl::Attributes() {
  return {typeid(PositionLengthAttribute).hash_code()};
}

void PositionLengthAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PositionLengthAttributeImpl& other = dynamic_cast<PositionLengthAttributeImpl&>(attr_impl);
    other.position_length = position_length;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

PositionLengthAttributeImpl& PositionLengthAttributeImpl::operator=(const PositionLengthAttributeImpl& other) {
  position_length = other.position_length;
}

/**
 * TermFrequencyAttributeImpl
 */

TermFrequencyAttributeImpl::TermFrequencyAttributeImpl()
  : term_frequency(1) {
}

TermFrequencyAttributeImpl::TermFrequencyAttributeImpl(const TermFrequencyAttributeImpl& other)
  : term_frequency(other.term_frequency) {
}

TermFrequencyAttributeImpl::~TermFrequencyAttributeImpl() {
}

void TermFrequencyAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  // TODO Implement it
}
void TermFrequencyAttributeImpl::Clear() {
  term_frequency = 1;
}

bool TermFrequencyAttributeImpl::operator==(TermFrequencyAttributeImpl& other) {
  if(this == &other) {
    return true;
  }

  return (term_frequency == other.term_frequency);
}

void TermFrequencyAttributeImpl::SetTermFrequency(unsigned int new_term_frequency) {
  term_frequency = new_term_frequency;
}

unsigned int TermFrequencyAttributeImpl::GetTermFrequency() {
  return term_frequency;
}

std::vector<size_t> TermFrequencyAttributeImpl::Attributes() {
  return {typeid(TermFrequencyAttribute).hash_code()};
}

void TermFrequencyAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    TermFrequencyAttributeImpl& other = dynamic_cast<TermFrequencyAttributeImpl&>(attr_impl);
    other.term_frequency = term_frequency;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

TermFrequencyAttributeImpl& TermFrequencyAttributeImpl::operator=(const TermFrequencyAttributeImpl& other) {
  term_frequency = other.term_frequency;
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
  // TODO Implement it
}

void TypeAttributeImpl::Clear() {
  type = TypeAttribute::DEFAULT_TYPE;
}

bool TypeAttributeImpl::operator==(TypeAttributeImpl& other) {
  if(this == &other) {
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

std::vector<size_t> TypeAttributeImpl::Attributes() {
  return {typeid(TypeAttribute).hash_code()};
}

void TypeAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    TypeAttributeImpl& other = dynamic_cast<TypeAttributeImpl&>(attr_impl);
    // It's acutally a deep copy.
    other.type = type;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

TypeAttributeImpl& TypeAttributeImpl::operator=(const TypeAttributeImpl& other) {
  type = other.type;
}

/**
 * CharTermAttributeImpl
 */

CharTermAttributeImpl::CharTermAttributeImpl()
  : term_buffer(new char[CHAR_TERM_ATTRIBUTE_IMPL_MIN_BUFFER_SIZE]),
    term_capacity(CHAR_TERM_ATTRIBUTE_IMPL_MIN_BUFFER_SIZE),
    term_length(0),
    builder() {
}

CharTermAttributeImpl::CharTermAttributeImpl(const CharTermAttributeImpl& other)
  : term_buffer(arrayutil::CopyOfRange(other.term_buffer, 0, other.term_capacity)),
    term_capacity(other.term_capacity),
    term_length(other.term_capacity),
    builder() {
}

CharTermAttributeImpl::~CharTermAttributeImpl() {
  if(term_buffer != nullptr) {
    delete[] term_buffer;
  }
}

void CharTermAttributeImpl::GrowTermBuffer(const unsigned int new_size) {
  if(term_capacity < new_size) {
      // Not big enough; create a new array with slight
      // Over allocation:
      term_capacity = arrayutil::Oversize(new_size, sizeof(char));
      delete[] term_buffer;
      term_buffer = new char[term_capacity];
  }
}

BytesRef& CharTermAttributeImpl::GetBytesRef() {
  builder.CopyBytes(term_buffer, 0, term_length);
  return builder.Get();
}

void CharTermAttributeImpl::Clear() {
  term_length = 0;
}

void CharTermAttributeImpl::CopyBuffer(const char* buffer, const unsigned int offset, const unsigned int length) {
  GrowTermBuffer(length);
  std::memcpy(term_buffer, buffer + offset, length);
  term_length = length;
}

char* CharTermAttributeImpl::Buffer() const {
  return term_buffer;
}

char* CharTermAttributeImpl::ResizeBuffer(const unsigned int new_size) {
  if(term_capacity < new_size) {
    std::pair<char*, unsigned int> new_term_buffer_info = arrayutil::Grow(term_buffer, term_capacity, new_size);
    if(new_term_buffer_info.first) {
      delete[] term_buffer;
      term_buffer = new_term_buffer_info.first;
      term_capacity = new_term_buffer_info.second;
    }
  }

  return term_buffer;
}

int CharTermAttributeImpl::Length() const {
  return term_length;
}

std::string CharTermAttributeImpl::SubSequence(const unsigned int start, const unsigned int end) {
  arrayutil::CheckFromToIndex(start, end, term_length);
  return std::string(term_buffer, start, end - start);
}

CharTermAttributeImpl& CharTermAttributeImpl::SetLength(const unsigned int length) {
  arrayutil::CheckFromIndexSize(0, length, term_capacity);
}

CharTermAttributeImpl& CharTermAttributeImpl::SetEmpty() {
  term_length = 0;
  return *this;
}

CharTermAttribute& CharTermAttributeImpl::Append(const std::string& csq) {
  Append(csq, 0, csq.size());
}

CharTermAttribute& CharTermAttributeImpl::Append(const std::string& csq, const unsigned int start, const unsigned int end) {
  arrayutil::CheckFromToIndex(start, end, csq.size());
  unsigned int len = (end - start);
  if(len == 0) return *this;
  ResizeBuffer(term_length + len);

  std::memcpy(term_buffer + term_length, csq.c_str() + start, len);
  return *this;
}

CharTermAttribute& CharTermAttributeImpl::Append(const char c) {
  ResizeBuffer(term_length + 1)[term_length++] = c;
}

CharTermAttribute& CharTermAttributeImpl::Append(const CharTermAttribute& term_att) {
  const int len = term_att.Length();
  std::memcpy(ResizeBuffer(term_length + len) + term_length, term_att.Buffer(), len);
  term_length += len;
  return *this;
}

void CharTermAttributeImpl::ReflectWith(AttributeReflector& reflector) {
  // TODO Implement it.
}

char& CharTermAttributeImpl::operator[](const unsigned int index) {
  arrayutil::CheckIndex(index, term_length);
  return term_buffer[index];
}

bool CharTermAttributeImpl::operator==(CharTermAttributeImpl& other) {
  if(term_length == other.term_length) {
    return (std::memcmp(term_buffer, other.term_buffer, term_length) == 0);
  }

  return false;
}

std::vector<size_t> CharTermAttributeImpl::Attributes() {
  return {typeid(CharTermAttribute).hash_code(), typeid(TermToBytesRefAttribute).hash_code()};
}

void CharTermAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    CharTermAttributeImpl& other = dynamic_cast<CharTermAttributeImpl&>(attr_impl);
    other.term_buffer = term_buffer;
    other.term_capacity = term_capacity;
    other.term_length = term_length;
    builder.Get().ShallowCopyTo(other.builder.Get());
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}

CharTermAttributeImpl& CharTermAttributeImpl::operator=(const CharTermAttributeImpl& other) {
  term_buffer = arrayutil::CopyOfRange(other.term_buffer, 0, other.term_capacity);
  term_capacity = other.term_capacity;
  term_length = other.term_length;
  builder.Get() = const_cast<CharTermAttributeImpl&>(other).builder.Get();
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

PackedTokenAttributeImpl::PackedTokenAttributeImpl(const PackedTokenAttributeImpl& other)
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

void PackedTokenAttributeImpl::SetPositionIncrement(const unsigned int new_position_increment) {
  position_increment = new_position_increment;
}

unsigned int PackedTokenAttributeImpl::GetPositionIncrement() {
  return position_increment;
}

void PackedTokenAttributeImpl::SetPositionLength(unsigned int new_position_length) {
  position_length = new_position_length;
}

unsigned int PackedTokenAttributeImpl::GetPositionLength() {
  return position_length;
}

int PackedTokenAttributeImpl::StartOffset() {
  return start_offset;
}

void PackedTokenAttributeImpl::SetOffset(const unsigned int new_start_offset, const unsigned int new_end_offset) {
  start_offset = new_start_offset;
  end_offset = new_end_offset;
}

int PackedTokenAttributeImpl::EndOffset() {
  return end_offset;
}

void PackedTokenAttributeImpl::SetTermFrequency(unsigned int new_term_frequency) {
  term_frequency = term_frequency;
}

unsigned int PackedTokenAttributeImpl::GetTermFrequency() {
  return term_frequency;
}

PackedTokenAttributeImpl& PackedTokenAttributeImpl::operator=(const PackedTokenAttributeImpl& other) {
  CharTermAttributeImpl::operator=(dynamic_cast<const CharTermAttributeImpl&>(other));
  start_offset = other.start_offset;
  end_offset = other.end_offset;
  type = other.type;
  position_increment = other.position_increment;
  position_length = other.position_length;
  term_frequency = other.term_frequency;
}

std::vector<size_t> PackedTokenAttributeImpl::Attributes() {
  return {
    typeid(CharTermAttribute).hash_code(),
    typeid(TermToBytesRefAttribute).hash_code(),
    typeid(TypeAttribute).hash_code(),
    typeid(PositionIncrementAttribute).hash_code(),
    typeid(PositionLengthAttribute).hash_code(),
    typeid(OffsetAttribute).hash_code(),
    typeid(TermFrequencyAttribute).hash_code()
  };
}

void PackedTokenAttributeImpl::ShallowCopyTo(AttributeImpl& attr_impl) {
  try {
    PackedTokenAttributeImpl& other = dynamic_cast<PackedTokenAttributeImpl&>(attr_impl);
    CharTermAttributeImpl::ShallowCopyTo(attr_impl);
    other.start_offset = start_offset;
    other.end_offset = end_offset;
    other.type = type;
    other.position_increment = position_increment;
    other.position_length = position_length;
    other.term_frequency = term_frequency;
  } catch(std::bad_cast& e) {
    throw std::invalid_argument("Shallow copy failed. " + std::string(typeid(*this).name()) + " -> " + typeid(attr_impl).name());
  }
}
