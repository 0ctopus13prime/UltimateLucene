#include <memory>
#include <string>
#include <stdexcept>
#include <Analysis/AttributeImpl.h>
#include <Util/ArrayUtil.h>

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

void BytesTermAttributeImpl::SetBytesRef(BytesRef& other_bytes) {
  bytes = other_bytes;
}

void BytesTermAttributeImpl::Clear() {
  BytesRef empty_bytes_ref;
  bytes = empty_bytes_ref;
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

void OffsetAttributeImpl::SetOffset(const int new_start_offset, const int new_end_offset) {
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

void PositionIncrementAttributeImpl::SetPositionIncrement(int new_position_increment) {
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

void TypeAttributeImpl::SetType(std::string& new_type) {
  type = new_type;
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

void CharTermAttributeImpl::CopyBuffer(char* buffer, const int offset, const int length) {
  GrowTermBuffer(length);
  std::memcpy(term_buffer, buffer + offset, length);
  term_length = length;
}

char* CharTermAttributeImpl::Buffer() const {
  return term_buffer;
}

char* CharTermAttributeImpl::ResizeBuffer(const int new_size) {
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

std::string CharTermAttributeImpl::SubSequence(const int start, const int end) {
  arrayutil::CheckFromToIndex(start, end, term_length);
  return std::string(term_buffer, start, end - start);
}

CharTermAttributeImpl& CharTermAttributeImpl::SetLength(const int length) {
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

char& CharTermAttributeImpl::operator[](const int index) {
  arrayutil::CheckIndex(index, term_length);
  return term_buffer[index];
}

bool CharTermAttributeImpl::operator==(CharTermAttributeImpl& other) {
  if(term_length == other.term_length) {
    return (std::memcmp(term_buffer, other.term_buffer, term_length) == 0);
  }

  return false;
}
