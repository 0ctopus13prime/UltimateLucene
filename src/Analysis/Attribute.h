#ifndef LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_H
#define LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_H

#include <string>
#include <sstream>
#include <Util/BytesRef.h>

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
    BytesTermAttribute();
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
    virtual int GetPositionIncrement() = 0;
};

class PositionLengthAttribute: public Attribute {
  public:
    virtual ~PositionLengthAttribute() { }
    virtual void SetPositionLength(int position_length) = 0;
    virtual int GetPositionLength() = 0;
};

class TermFrequencyAttribute: public Attribute {
  public:
    virtual ~TermFrequencyAttribute() { }
    virtual void SetTermFrequency(int term_frequency) = 0;
    virtual int GetTermFrequency() = 0;
};

class TermToBytesRefAttribute: public Attribute {
  public:
    virtual ~TermToBytesRefAttribute() { }
    BytesRef& GetBytesRef();
};

class TypeAttribute: public Attribute {
  public:
    virtual ~TypeAttribute() { }
    virtual std::string& Type() = 0;
    virtual void SetType(std::string& type) = 0;
}

class CharTermAttribute: public Attribute {
  public:
    virtual ~CharTermAttribute() { }
    virtual void CopyBuffer(char[] buffer, int offset, int length) = 0;
    virtual char* Buffer() = 0;
    virtual char* ResizeBuffer(int new_size) = 0;
    virtual int Length() = 0;
    virtual char operator[](const int idx) = 0;
    virtual std::string SubSequence(int start, int end) = 0;
    virtual CharTermAttribute& SetLength(int length) = 0;
    virtual CharTermAttribute& SetEmpty() = 0;
    virtual CharTermAttribute& Append(std::string& csq) = 0;
    virtual CharTermAttribute& Append(std::string& csq, int start, int end) = 0;
    virtual CharTermAttribute& Append(char c) = 0;
    virtual CharTermAttribute& Append(std::string& s) = 0;
    virtual CharTermAttribute& Append(std::stringbuf& buf) = 0;
    virtual CharTermAttribute& Append(CharTermAttribute& term_att) = 0;
};

class OffsetAttribute: public Attribute {
  public:
    virtual ~OffsetAttribute() { }
    virtual int StartOffset() = 0;
    virtual void SetOffset(int start_offset, int end_offset) = 0;
    virtual int EndOffset() = 0;
};

}}}} // End of namespace

#endif
