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

#ifndef SRC_INDEX_TERM_H_
#define SRC_INDEX_TERM_H_

#include <Index/Posting.h>
#include <Util/Etc.h>
#include <Util/Exception.h>

namespace lucene {
namespace core {
namespace index {

class TermsEnum: public lucene::core::util::Iterator<
                          lucene::core::util::BytesRef> {
 public:
  class EmptyTermsEnum;

  enum class SeekStatus {
    END,
    FOUND,
    NOT_FOUND
  };

 protected:
  TermsEnum() = default;

 public:
  virtual ~TermsEnum() = default;

  virtual PostingsEnum* Postings(uint32_t flags) = 0;

  virtual lucene::core::util::BytesRef* Term() = 0;

  virtual uint64_t Ord() = 0;

  virtual uint32_t DocFreq() = 0;

  virtual uint64_t TotalTermFreq() = 0;

  // TODO(0ctopus13prime): IT
  // AttributeSource attributes()
  // bool SeekExact(BytesRef text)
  // SeekStatus SeekCeil(BytesRef text)
  // void SeekExact(long ord)
  // void SeekExact(BytesRef term, TermState state)
  // TermState TermState()
};  // TermsEnum

class TermsEnum::EmptyTermsEnum: public TermsEnum {
  lucene::core::util::BytesRef* Term() {
    throw lucene::core::util::UnsupportedOperationException();
  }

  uint64_t Ord() {
    throw lucene::core::util::UnsupportedOperationException();
  }

  uint32_t DocFreq() {
    throw lucene::core::util::UnsupportedOperationException();
  }

  uint64_t TotalTermFreq() {
    throw lucene::core::util::UnsupportedOperationException();
  }

  lucene::core::util::BytesRef* Next() {
    return nullptr;
  }

  virtual PostingsEnum* Postings(PostingsEnum* reuse, const uint32_t flags) = 0;

  PostingsEnum* Postings(PostingsEnum* reuse) {
    return Postings(reuse, PostingsEnum::FREQS);
  }

  // TODO(0ctopus13prime): IT
  // AttributeSource attributes()
  // bool SeekExact(BytesRef text)
  // SeekStatus SeekCeil(BytesRef text)
  // void SeekExact(long ord)
  // void SeekExact(BytesRef term, TermState state)
  // PostingsEnum Postings(PostingsEnum reuse)
  // PostingsEnum Postings(PostingsEnum reuse)
  // PostingsEnum Postings(PostingsEnum reuse, int flags)
  // TermState TermState()
};  // TermsEnum::EmptyTermsEnum

class Terms {
 protected:
  Terms() = default;

 public:
  virtual ~Terms() = default;

  virtual TermsEnum* Iterator() = 0;

  // TODO(0ctopus13prime): IT
  // TermsEnum Intersect(CompiledAutomaton compiled, BytesRef& startTerm);

  virtual int64_t Size() = 0;

  virtual int64_t GetSumTotalTermFreq() = 0;

  virtual int64_t GetSumDocFreq() = 0;

  virtual int32_t GetDocCount() = 0;

  virtual bool HasFreqs() = 0;

  virtual bool HasOffsets() = 0;

  virtual bool HasPositions() = 0;

  virtual bool HasPayLoads() = 0;

  // TODO(0ctopus13prime): IT
  // lucene::core::util::BytsRef* GetMin();

  // TODO(0ctopus13prime): IT
  // lucene::core::util::BytesRef* GetMax(); 
};  // Terms

}  // namespace index
}  // namespace core
}  // namespace lucene

 #endif  // SRC_INDEX_TERM_H_
