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
    virtual int GetFlags() = 0;
    virtual void SetFlags(int flags) = 0;
};

class KeywordAttribute: public Attribute {
  public:
    virtual ~KeywordAttribute() { }
    virtual bool IsKeyword() = 0;
    virtual void SetKeyword(bool is_keyword) = 0;
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
    virtual void SetPositionIncrement(int position_increment) = 0;
    virtual unsigned int GetPositionIncrement() = 0;
};

class PositionLengthAttribute: public Attribute {
  public:
    virtual ~PositionLengthAttribute() { }
    virtual void SetPositionLength(unsigned int position_length) = 0;
    virtual unsigned int GetPositionLength() = 0;
};

class TermFrequencyAttribute: public Attribute {
  public:
    virtual ~TermFrequencyAttribute() { }
    virtual void SetTermFrequency(unsigned int term_frequency) = 0;
    virtual unsigned int GetTermFrequency() = 0;
};

class TypeAttribute: public Attribute {
  public:
    static const char* DEFAULT_TYPE;

  public:
    virtual ~TypeAttribute() { }
    virtual std::string& Type() = 0;
    virtual void SetType(std::string& type) = 0;
};


class CharTermAttribute: public Attribute {
  public:
    virtual ~CharTermAttribute() { }
    virtual void CopyBuffer(char* buffer, int offset, int length) = 0;
    virtual char* Buffer() = 0;
    virtual char* ResizeBuffer(int new_size) = 0;
    virtual int Length() = 0;
    virtual char operator[](const int idx) = 0;
    virtual std::string SubSequence(int start, int end) = 0;
    virtual CharTermAttribute& SetLength(int length) = 0;
    virtual CharTermAttribute& SetEmpty() = 0;
    virtual CharTermAttribute& Append(const std::string& csq) = 0;
    virtual CharTermAttribute& Append(const std::string& csq, const unsigned int start, const unsigned int end) = 0;
    virtual CharTermAttribute& Append(const char c) = 0;
    virtual CharTermAttribute& Append(const std::stringbuf& buf) = 0;
    virtual CharTermAttribute& Append(const CharTermAttribute& term_att) = 0;
};

class OffsetAttribute: public Attribute {
  public:
    virtual ~OffsetAttribute() { }
    virtual int StartOffset() = 0;
    virtual void SetOffset(const int start_offset, const int end_offset) = 0;
    virtual int EndOffset() = 0;
};

}}}} // End of namespace

#endif
