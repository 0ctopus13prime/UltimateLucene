#include <string>
#include <stdexcept>
#include <Analysis/AttributeImpl.h>

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

/*
CharTermAttributeImpl::CharTermAttributeImpl();
CharTermAttributeImpl(const CharTermAttributeImpl& other);
virtual ~CharTermAttributeImpl();
void ReflectWith(AttributeReflector& reflector) override;
BytesRef& GetBytesRef() override;
void CopyBuffer(char* buffer, const int offset, const int length) override;
char* Buffer() override;
char* ResizeBuffer(const int new_size) override;
int Length() override;
std::string SubSequence(const int start, const int end) override;
CharTermAttributeImpl& SetLength(const int length) override;
CharTermAttributeImpl& SetEmpty() override;
CharTermAttribute& operator+=(const std::string& csq) override;
CharTermAttribute& operator+=(const std::string& csq, const unsigned int start, const unsigned int end) override;
CharTermAttribute& operator+=(const char c) override;
CharTermAttribute& operator+=(const std::stringbuf& buf) override;
CharTermAttribute& operator+=(const CharTermAttribute& term_att) override;
char operator[](const int idx) override;
bool operator==(CharTermAttributeImpl& other);
*/
