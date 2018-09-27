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

#ifndef SRC_ANALYSIS_ATTRIBUTEIMPL_H_
#define SRC_ANALYSIS_ATTRIBUTEIMPL_H_

#include <Analysis/Attribute.h>
#include <Util/Attribute.h>
#include <Util/Ref.h>
#include <memory>
#include <string>
#include <vector>

namespace lucene {
namespace core {
namespace analysis {
namespace tokenattributes {

class BytesTermAttributeImpl: public lucene::core::util::AttributeImpl,
                              public BytesTermAttribute {
 private:
  lucene::core::util::BytesRef bytes;

 public:
  BytesTermAttributeImpl();
  BytesTermAttributeImpl(const BytesTermAttributeImpl& other);
  lucene::core::util::BytesRef& GetBytesRef() override;
  virtual ~BytesTermAttributeImpl();
  void SetBytesRef(lucene::core::util::BytesRef& bytes) override;
  void Clear() override;
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  bool operator==(const BytesTermAttributeImpl& other) const;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  BytesTermAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  BytesTermAttributeImpl& operator=(const BytesTermAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class CharTermAttributeImpl: public lucene::core::util::AttributeImpl,
                             public CharTermAttribute,
                             public TermToBytesRefAttribute {
 private:
  static const uint32_t CHAR_TERM_ATTRIBUTE_IMPL_MIN_BUFFER_SIZE = 10;

  std::shared_ptr<char> term_buffer;
  uint32_t term_capacity;
  uint32_t term_length;
  lucene::core::util::BytesRefBuilder builder;

 private:
  void GrowTermBuffer(const uint32_t new_size);

 public:
  CharTermAttributeImpl();
  CharTermAttributeImpl(const CharTermAttributeImpl& other);
  virtual ~CharTermAttributeImpl();
  lucene::core::util::BytesRef& GetBytesRef() override;
  void Clear() override;
  void CopyBuffer(const char* buffer,
                  const uint32_t offset,
                  const uint32_t length) override;
  char* Buffer() const override;
  char* ResizeBuffer(const uint32_t new_capacity) override;
  uint32_t Length() const override;
  std::string SubSequence(const uint32_t inclusive_start,
                          const uint32_t exclusive_end) override;
  CharTermAttributeImpl& SetLength(const uint32_t length) override;
  CharTermAttributeImpl& SetEmpty() override;
  CharTermAttribute& Append(const std::string& csq) override;
  CharTermAttribute& Append(const std::string& csq,
                            const uint32_t inclusive_start,
                            const uint32_t exclusive_end) override;
  CharTermAttribute& Append(const char c) override;
  CharTermAttribute& Append(const CharTermAttribute& term_att) override;
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  char& operator[](const uint32_t idx) override;
  bool operator==(const CharTermAttributeImpl& other) const;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  CharTermAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  CharTermAttributeImpl& operator=(const CharTermAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class FlagsAttributeImpl: public lucene::core::util::AttributeImpl,
                          public FlagsAttribute {
 private:
  int32_t flags;

 public:
  FlagsAttributeImpl();
  FlagsAttributeImpl(const FlagsAttributeImpl& other);
  virtual ~FlagsAttributeImpl();
  int32_t GetFlags() override;
  void SetFlags(const int32_t flags) override;
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const FlagsAttributeImpl& other) const;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  FlagsAttributeImpl& operator=(const lucene::core::util::AttributeImpl& other);
  FlagsAttributeImpl& operator=(const FlagsAttributeImpl& other);
  AttributeImpl* Clone() override;
};

class KeywordAttributeImpl: public lucene::core::util::AttributeImpl,
                            public KeywordAttribute {
 private:
  bool keyword;

 public:
  KeywordAttributeImpl();
  KeywordAttributeImpl(const KeywordAttributeImpl& other);
  virtual ~KeywordAttributeImpl();
  bool IsKeyword() override;
  void SetKeyword(const bool is_keyword) override;
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const KeywordAttributeImpl& other) const;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  KeywordAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  KeywordAttributeImpl& operator=(const KeywordAttributeImpl& other);
  AttributeImpl* Clone() override;
};

class OffsetAttributeImpl: public lucene::core::util::AttributeImpl,
                           public OffsetAttribute {
 private:
  uint32_t start_offset;
  uint32_t end_offset;

 public:
  OffsetAttributeImpl();
  OffsetAttributeImpl(const OffsetAttributeImpl& other);
  virtual ~OffsetAttributeImpl();
  uint32_t StartOffset() override;
  void SetOffset(const uint32_t start_offset,
                 const uint32_t end_offset) override;
  uint32_t EndOffset() override;
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const OffsetAttributeImpl& other) const;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  OffsetAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  OffsetAttributeImpl& operator=(const OffsetAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class PackedTokenAttributeImpl: public CharTermAttributeImpl,
                                public TypeAttribute,
                                public PositionIncrementAttribute,
                                public PositionLengthAttribute,
                                public OffsetAttribute,
                                public TermFrequencyAttribute {
 private:
  uint32_t start_offset;
  uint32_t end_offset;
  std::string type;
  uint32_t position_increment;
  uint32_t position_length;
  uint32_t term_frequency;

 public:
  PackedTokenAttributeImpl();
  PackedTokenAttributeImpl(const PackedTokenAttributeImpl& other);
  virtual ~PackedTokenAttributeImpl();
  std::string& Type() override;
  void SetType(const std::string& new_type) override;
  void SetPositionIncrement(const uint32_t new_position_increment) override;
  uint32_t GetPositionIncrement() override;
  void SetPositionLength(const uint32_t new_position_length) override;
  uint32_t GetPositionLength() override;
  uint32_t StartOffset() override;
  void SetOffset(const uint32_t new_start_offset,
                 const uint32_t new_end_offset) override;
  uint32_t EndOffset() override;
  void SetTermFrequency(const uint32_t new_term_frequency) override;
  uint32_t GetTermFrequency() override;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  PackedTokenAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  PackedTokenAttributeImpl& operator=(const PackedTokenAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class PayloadAttributeImpl: public lucene::core::util::AttributeImpl,
                            public PayloadAttribute {
 private:
  lucene::core::util::BytesRef payload;

 public:
  PayloadAttributeImpl();
  PayloadAttributeImpl(const PayloadAttributeImpl& other);
  virtual ~PayloadAttributeImpl();
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const PayloadAttributeImpl& other) const;
  lucene::core::util::BytesRef& GetPayload() override;
  void SetPayload(lucene::core::util::BytesRef& payload) override;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  PayloadAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  PayloadAttributeImpl& operator=(const PayloadAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class PositionIncrementAttributeImpl: public lucene::core::util::AttributeImpl,
                                      public PositionIncrementAttribute {
 private:
  uint32_t position_increment;

 public:
  PositionIncrementAttributeImpl();
  PositionIncrementAttributeImpl(const PositionIncrementAttributeImpl& other);
  virtual ~PositionIncrementAttributeImpl();
  void SetPositionIncrement(const uint32_t position_increment) override;
  uint32_t GetPositionIncrement() override;
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void End() override;
  void Clear() override;
  bool operator==(const PositionIncrementAttributeImpl& other) const;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  PositionIncrementAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  PositionIncrementAttributeImpl&
    operator=(const PositionIncrementAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class PositionLengthAttributeImpl: public lucene::core::util::AttributeImpl,
                                   public PositionLengthAttribute {
 private:
  uint32_t position_length;

 public:
  PositionLengthAttributeImpl();
  PositionLengthAttributeImpl(const PositionLengthAttributeImpl& other);
  virtual ~PositionLengthAttributeImpl();
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const PositionLengthAttributeImpl& other) const;
  void SetPositionLength(const uint32_t position_length) override;
  uint32_t GetPositionLength() override;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  PositionLengthAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  PositionLengthAttributeImpl&
    operator=(const PositionLengthAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class TermFrequencyAttributeImpl: public lucene::core::util::AttributeImpl,
                                  public TermFrequencyAttribute {
 private:
  uint32_t term_frequency;

 public:
  TermFrequencyAttributeImpl();
  TermFrequencyAttributeImpl(const TermFrequencyAttributeImpl& other);
  virtual ~TermFrequencyAttributeImpl();
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const TermFrequencyAttributeImpl& other) const;
  void SetTermFrequency(const uint32_t term_frequency) override;
  uint32_t GetTermFrequency() override;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  TermFrequencyAttributeImpl&
    operator=(const lucene::core::util::AttributeImpl& other);
  TermFrequencyAttributeImpl&
    operator=(const TermFrequencyAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

class TypeAttributeImpl: public lucene::core::util::AttributeImpl,
                         public TypeAttribute {
 private:
  std::string type;

 public:
  TypeAttributeImpl();
  TypeAttributeImpl(const TypeAttributeImpl& other);
  virtual ~TypeAttributeImpl();
  void ReflectWith(lucene::core::util::AttributeReflector& reflector) override;
  void Clear() override;
  bool operator==(const TypeAttributeImpl& other) const;
  std::string& Type() override;
  void SetType(const std::string& type) override;
  std::vector<std::type_index> Attributes() override;
  void ShallowCopyTo(lucene::core::util::AttributeImpl& attr_impl) override;
  TypeAttributeImpl& operator=(const lucene::core::util::AttributeImpl& other);
  TypeAttributeImpl& operator=(const TypeAttributeImpl& other);
  lucene::core::util::AttributeImpl* Clone() override;
};

}  // namespace tokenattributes
}  // namespace analysis
}  // namespace core
}  // namespace lucene

#endif  // SRC_ANALYSIS_ATTRIBUTEIMPL_H_
