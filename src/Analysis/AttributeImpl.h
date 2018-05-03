#ifndef LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_IMPL_H
#define LUCENE_CORE_ANALYSIS_TOKEN_ATTRIBUTES_ATTRIBUTES_IMPL_H

#include <string>
#include <Analysis/Attribute.h>
#include <Util/Bytes.h>
#include <Util/Attribute.h>

using namespace lucene::core::util;

namespace lucene { namespace core { namespace analysis { namespace tokenattributes {

class BytesTermAttributeImpl: public AttributeImpl, public BytesTermAttribute {
  private:
    BytesRef bytes;

  public:
    BytesTermAttributeImpl();
    BytesTermAttributeImpl(const BytesTermAttributeImpl& other);
    BytesRef& GetBytesRef() override;
    void SetBytesRef(BytesRef& bytes) override;
    void Clear() override;
    void CopyTo(AttributeImpl& target) override;
    void ReflectWith(AttributeReflector& reflector) override;
    bool operator==(BytesTermAttributeImpl& other);
};

class CharTermAttributeImpl: public AttributeImpl, public CharTermAttribute, public TermToBytesRefAttribute {
  private:
    void GrowTermBuffer(int new_size);

  public:
    CharTermAttributeImpl();
    CharTermAttributeImpl(const CharTermAttributeImpl& other);
    virtual ~CharTermAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    BytesRef& GetBytesRef() override;
    void CopyBuffer(char* buffer, const int offset, const int length) override;
    char* Buffer() override;
    char* ResizeBuffer(const int new_size) override;
    int Length() override;
    std::string SubSequence(const int start, const int end) override;
    CharTermAttributeImpl& SetLength(const int length) override;
    CharTermAttributeImpl& SetEmpty() override;
    CharTermAttributeImpl& Append(std::string& csq) override;
    CharTermAttributeImpl& Append(std::string& csq, const int start, const int end) override;
    CharTermAttributeImpl& Append(char c) override;
    CharTermAttributeImpl& Append(std::stringbuf& buf) override;
    CharTermAttributeImpl& Append(CharTermAttribute& term_att) override;
    char operator[](const int idx) override;
    bool operator==(CharTermAttributeImpl& other);
};

class FlagsAttributeImpl: public AttributeImpl, public FlagsAttribute {
  public:
    FlagsAttributeImpl();
    FlagsAttributeImpl(const FlagsAttributeImpl& other);
    virtual ~FlagsAttributeImpl();
    int GetFlags() override;
    void SetFlags(int flags) override;
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(FlagsAttributeImpl& other);
};

class KeywordAttributeImpl: public AttributeImpl, public KeywordAttribute {
  private:
    bool keyword;

  public:
    KeywordAttributeImpl();
    KeywordAttributeImpl(const KeywordAttributeImpl& other);
    virtual ~KeywordAttributeImpl();
    bool IsKeyword() override;
    void SetKeyword(bool is_keyword) override;
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(FlagsAttributeImpl& other);
};

class OffsetAttributeImpl: public AttributeImpl, public OffsetAttribute {
  public:
    OffsetAttributeImpl();
    OffsetAttributeImpl(const OffsetAttributeImpl& other);
    virtual ~OffsetAttributeImpl();
    int StartOffset() override;
    void SetOffset(const int start_offset, const int end_offset) override;
    int EndOffset() override;
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(OffsetAttributeImpl& other);
};

class PackedTokenAttributeImpl {
  // TODO Implement it
};

class PayloadAttributeImpl: public AttributeImpl, public PayloadAttribute {
  private:
    BytesRef payload;

  public:
    PayloadAttributeImpl();
    PayloadAttributeImpl(const PayloadAttributeImpl& other);
    virtual ~PayloadAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(PayloadAttributeImpl& other);
    BytesRef& GetPayload() override;
    void SetPayload(BytesRef& payload) override;
};

class PositionIncrementAttributeImpl: public AttributeImpl, public PositionIncrementAttribute {
  private:
    int position_increment = 1;

  public:
    PositionIncrementAttributeImpl();
    PositionIncrementAttributeImpl(const PositionIncrementAttributeImpl& other);
    virtual ~PositionIncrementAttributeImpl();
    void SetPositionIncrement(int position_increment) override;
    int GetPositionIncrement() override;
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(PositionIncrementAttributeImpl& other);
};

class PositionLengthAttributeImpl: public AttributeImpl, public PositionLengthAttribute {
  private:
    int position_length = 1;

  public:
    PositionLengthAttributeImpl();
    PositionLengthAttributeImpl(const PositionLengthAttributeImpl& other);
    virtual ~PositionLengthAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(PositionLengthAttributeImpl& other);
    void SetPositionLength(int position_length) override;
    int GetPositionLength() override;
};

class TermFrequencyAttributeImpl: public AttributeImpl, public TermFrequencyAttribute {
  private:
    int term_frequency = 1;

  public:
    TermFrequencyAttributeImpl();
    TermFrequencyAttributeImpl(const TermFrequencyAttributeImpl& other);
    virtual ~TermFrequencyAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(TermFrequencyAttributeImpl& other);
    void SetTermFrequency(int term_frequency) override;
    int GetTermFrequency() override;
};

class TypeAttributeImpl: public AttributeImpl, public TypeAttribute {
  private:
    std::string type;

  public:
    TypeAttributeImpl();
    TypeAttributeImpl(const TypeAttributeImpl& other);
    virtual ~TypeAttributeImpl();
    void ReflectWith(AttributeReflector& reflector) override;
    void CopyTo(AttributeImpl& target) override;
    void Clear() override;
    bool operator==(TypeAttributeImpl& other);
    std::string& Type() override;
    void SetType(std::string& type) override;
};

}}}} // End of namespace

#endif
