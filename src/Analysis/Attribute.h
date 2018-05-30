#ifndef LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_H
#define LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_H

#include <string>
#include <sstream>
#include <Util/Bytes.h>
#include <Util/Attribute.h>

using namespace lucene::core::util;

namespace lucene { namespace core { namespace analysis { namespace tokenattributes {

class TermToBytesRefAttribute: public Attribute {
  public:
    virtual ~TermToBytesRefAttribute() { }
    virtual BytesRef& GetBytesRef() = 0;
};

class BytesTermAttribute: public TermToBytesRefAttribute {
  private:
    BytesRef bytes;

  public:
    BytesTermAttribute(): bytes() { }
    virtual ~BytesTermAttribute() { }
    virtual void SetBytesRef(BytesRef& bytes) = 0;
};

class FlagsAttribute: public Attribute {
  public:
    virtual ~FlagsAttribute() { }
    virtual int32_t GetFlags() = 0;
    virtual void SetFlags(int32_t flags) = 0;
};

class KeywordAttribute: public Attribute {
  public:
    virtual ~KeywordAttribute() { }
    virtual bool IsKeyword() = 0;
    virtual void SetKeyword(const bool is_keyword) = 0;
};

class PayloadAttribute: public Attribute {
  public:
    virtual ~PayloadAttribute() { }
    virtual BytesRef& GetPayload() = 0;
    virtual void SetPayload(BytesRef& payload) = 0;
};

class PositionIncrementAttribute: public Attribute {
  public:
    virtual ~PositionIncrementAttribute() { }
    virtual void SetPositionIncrement(const uint32_t position_increment) = 0;
    virtual uint32_t GetPositionIncrement() = 0;
};

class PositionLengthAttribute: public Attribute {
  public:
    virtual ~PositionLengthAttribute() { }
    virtual void SetPositionLength(const uint32_t position_length) = 0;
    virtual uint32_t GetPositionLength() = 0;
};

class TermFrequencyAttribute: public Attribute {
  public:
    virtual ~TermFrequencyAttribute() { }
    virtual void SetTermFrequency(const uint32_t term_frequency) = 0;
    virtual uint32_t GetTermFrequency() = 0;
};

class TypeAttribute: public Attribute {
  public:
    static const char* DEFAULT_TYPE;

  public:
    virtual ~TypeAttribute() { }
    virtual std::string& Type() = 0;
    virtual void SetType(const std::string& type) = 0;
};


class CharTermAttribute: public Attribute {
  public:
    virtual ~CharTermAttribute() { }
    virtual void CopyBuffer(const char* buffer, const uint32_t offset, const uint32_t length) = 0;
    virtual char* Buffer() const = 0;
    virtual char* ResizeBuffer(const uint32_t new_size) = 0;
    virtual uint32_t Length() const = 0;
    virtual char& operator[](const uint32_t idx) = 0;
    virtual std::string SubSequence(const uint32_t start, const uint32_t end) = 0;
    virtual CharTermAttribute& SetLength(const uint32_t length) = 0;
    virtual CharTermAttribute& SetEmpty() = 0;
    virtual CharTermAttribute& Append(const std::string& csq) = 0;
    virtual CharTermAttribute& Append(const std::string& csq, const uint32_t start, const uint32_t end) = 0;
    virtual CharTermAttribute& Append(const char c) = 0;
    virtual CharTermAttribute& Append(const CharTermAttribute& term_att) = 0;
};

class OffsetAttribute: public Attribute {
  public:
    virtual ~OffsetAttribute() { }
    virtual uint32_t StartOffset() = 0;
    virtual void SetOffset(const uint32_t start_offset, const uint32_t end_offset) = 0;
    virtual uint32_t EndOffset() = 0;
};

}}}} // End of namespace

#endif
