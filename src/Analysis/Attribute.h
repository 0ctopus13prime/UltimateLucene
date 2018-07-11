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

#ifndef SRC_ANALYSIS_ATTRIBUTE_H_
#define SRC_ANALYSIS_ATTRIBUTE_H_

#include <Util/Attribute.h>
#include <Util/Bytes.h>
#include <sstream>
#include <string>

namespace lucene {
namespace core {
namespace analysis {
namespace tokenattributes {

class TermToBytesRefAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~TermToBytesRefAttribute() { }
  virtual lucene::core::util::BytesRef& GetBytesRef() = 0;
};

class BytesTermAttribute: public TermToBytesRefAttribute {
 private:
  lucene::core::util::BytesRef bytes;

 public:
  BytesTermAttribute(): bytes() { }
  virtual ~BytesTermAttribute() { }
  virtual void SetBytesRef(lucene::core::util::BytesRef& bytes) = 0;
};

class FlagsAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~FlagsAttribute() { }
  virtual int32_t GetFlags() = 0;
  virtual void SetFlags(int32_t flags) = 0;
};

class KeywordAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~KeywordAttribute() { }
  virtual bool IsKeyword() = 0;
  virtual void SetKeyword(const bool is_keyword) = 0;
};

class PayloadAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~PayloadAttribute() { }
  virtual lucene::core::util::BytesRef& GetPayload() = 0;
  virtual void SetPayload(lucene::core::util::BytesRef& payload) = 0;
};

class PositionIncrementAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~PositionIncrementAttribute() { }
  virtual void SetPositionIncrement(const uint32_t position_increment) = 0;
  virtual uint32_t GetPositionIncrement() = 0;
};

class PositionLengthAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~PositionLengthAttribute() { }
  virtual void SetPositionLength(const uint32_t position_length) = 0;
  virtual uint32_t GetPositionLength() = 0;
};

class TermFrequencyAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~TermFrequencyAttribute() { }
  virtual void SetTermFrequency(const uint32_t term_frequency) = 0;
  virtual uint32_t GetTermFrequency() = 0;
};

class TypeAttribute: public lucene::core::util::Attribute {
 public:
  static const char* DEFAULT_TYPE;

 public:
  virtual ~TypeAttribute() { }
  virtual std::string& Type() = 0;
    virtual void SetType(const std::string& type) = 0;
};


class CharTermAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~CharTermAttribute() { }
  virtual void CopyBuffer(const char* buffer,
                          const uint32_t offset,
                          const uint32_t length) = 0;
  virtual char* Buffer() const = 0;
  virtual char* ResizeBuffer(const uint32_t new_size) = 0;
  virtual uint32_t Length() const = 0;
  virtual char& operator[](const uint32_t idx) = 0;
  virtual std::string SubSequence(const uint32_t start, const uint32_t end) = 0;
  virtual CharTermAttribute& SetLength(const uint32_t length) = 0;
  virtual CharTermAttribute& SetEmpty() = 0;
  virtual CharTermAttribute& Append(const std::string& csq) = 0;
  virtual CharTermAttribute& Append(const std::string& csq,
                                    const uint32_t start,
                                    const uint32_t end) = 0;
  virtual CharTermAttribute& Append(const char c) = 0;
  virtual CharTermAttribute& Append(const CharTermAttribute& term_att) = 0;
};

class OffsetAttribute: public lucene::core::util::Attribute {
 public:
  virtual ~OffsetAttribute() { }
  virtual uint32_t StartOffset() = 0;
  virtual void SetOffset(const uint32_t start_offset,
                         const uint32_t end_offset) = 0;
  virtual uint32_t EndOffset() = 0;
};

}  // namespace tokenattributes
}  // namespace analysis
}  // namespace core
}  // namespace lucene

#endif  // SRC_ANALYSIS_ATTRIBUTE_H_
