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

#ifndef SRC_INDEX_TEMP_H_
#define SRC_INDEX_TEMP_H_

#include <Index/Field.h>
#include <Index/Term.h>
#include <Index/Posting.h>

namespace lucene {
namespace core {
namespace index {

class StringVectorIterator: Iterator<std::string> {
 private:
  std::vector<string>& v;
  std::vector<string>::iterator it;


 public:
  explicit StringVectorIterator(std::vector<std::string>& v)
    : v(v),
      it(v.begin()) {
  }

  std::string* Next() {
    if (it != v.end()) {
      return &(*it);
    } else {
      return nullptr;
    }
  }
};  // StringVectorIterator

class TempPostingsEnum: public PostingsEnum {
 private:
  uint32_t df;
  uint32_t tf;
  uint32_t curr_doc;
  uint32_t curr_offset;

 public:
  TempPostingsEnum(const uint32_t df,
                   const uint32_t tf)
    : df(df),
      tf(tf),
      curr_doc(0),
      curr_offset(0) {
  }
 
  int32_t Freq() {
    return tf;
  }

  int32_t NextPosition() {
    return curr_offset++;    
  }

  int32_t StartOffset() {
    return 0;
  }

  int32_t EndOffset() {
    return freq;
  }

  uint32_t DocId() {
    return curr_doc;
  }

  uint32_t NextDoc() {
    if (curr_doc < doc_count) {
      return ++curr_doc;
    } else {
      return NO_MORE_DOCS;
    }
  }

  uint32_t Advance(uint32_t target) {
    uint32_t doc;
    while ((doc = NextDoc()) < target);
    return doc;
  }

  uint64_t Cost() {
    return 0;
  }
};  // PostingsEnum

class TempTermsEnum: public TermsEnum {
 private:
  std::vector<std::string> terms;
  lucene::core::util::BytesRef scratch;
  uint32_t df;
  uint32_t ord;
  TempPostingsEnum postings_enum;

 public:
  TempTermsEnum(std::vector<std::string>&& terms,
                const uint32_t df,
                const uint32_t tf)
    : terms(std::forward<std::string>(terms)),
      scratch(),
      freq(freq),
      ord(0),
      postings_enum(df, tf) {
  }

  lucene::core::util::BytesRef* Next() {
    if (ord == terms.size()) {
      return nullptr;
    }

    std::string& t = terms[ord++];
    scratch.SetBytes(t.c_str())
           .SetOffset(0)
           .SetLength(t.size());
    
    return &scratch;
  }

  lucene::core::util::BytesRef* Term() {
    return &scratch;
  }

  uint64_t Ord() {
    return ord;
  }

  uint32_t DocFreq() {
    return df;
  }

  uint64_t TotalTermFreq() {
    return (df * tf);
  }

  PostingsEnum* Postings() {
    return &postings_enum;
  }

  PostingsEnum* Postings(uint32_t flags) {
    return &postings_enum;
  }

  std::vector<std::string>& GetDummyTerms() const noexcept {
    return terms;
  }
};  // TempTermsEnum

class TempTerms: public Terms {
 private:
  TempTermsEnum terms_enum;
  uint32_t df;
  uint32_t tf;

 public:
  TempTerms(std::vector<string>&& dummy_terms,
            const uint32_t df,
            const uint32_t tf)
    : terms_enum(std::forward<std::vector<std::string>>(dummy_terms),
                 df,
                 tf),
      df(df),
      tf(tf) {
  }

  TermsEnum* Iterator() {
    return &terms_enum; 
  }

  int64_t Size() {
    return terms_enum.GetDummyTerms().size();
  }

  int64_t GetSumTotalTermFreq() {
    return (terms_enum.GetDummyTerms().size() * df * tf);
  }

  int64_t GetSumDocFreq() {
    return (terms_enum.GetDummyTerms().size() * df);
  }

  int32_t GetDocCount() {
    return df; 
  }

  bool HasFreqs() {
    return true;
  }

  bool HasOffsets() {
    return true;
  }

  bool HasPositions() {
    return true;
  }

  bool HasPayLoads() {
    return true;
  }
};  // TempTerms

class TempFields: public Fields {
 private:
  std::vector<std::string> dummy_fields;  
  TempTerms temp_terms;

 public:
  TempFields(const uint32_t field_count,
             const uint32_t term_count,
             const uint32_t df,
             const uint32_t tf)
    : temp_terms(std::vector<std::string>(), df, tf) {
    dummy_fields.reserve(field_count);

    for (uint32_t i = 0 ; i < field_count ; ++i) {
      dummy_fields.push_back("field-" + std:to_string(i));
    }

    std::vector<std::string> dummy_terms =
      temp_terms.GetDummyTerms();

    for (uint32_t i = 0 ; i < term_count ; ++i) {
      dummy_terms.push_back("term-" + std::to_string(i));
    }
  }

  lucene::core::util::Iterator<std::string> Iterator() {
    return StringVectorIterator(dummy_fields);
  }

  Terms* GetTerms(const std::string& field) {
    return &temp_terms;
  }

  int32_t Size() {
    return dummy_fields.size();
  }
};  // TempFields

}  // namespace index
}  // namespace core
}  // namespace lucene

#endif  // SRC_INDEX_TEMP_H_
