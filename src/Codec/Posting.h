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

#ifndef SRC_CODEC_POSTING_H_
#define SRC_CODEC_POSTING_H_

#include <Codec/Etc.h>
#include <Util/Ref.h>
#include <Util/Bit.h>
#include <Index/Field.h>
#include <Index/Posting.h>
#include <Index/Segment.h>
#include <Index/Term.h>
#include <cassert>

namespace lucene {
namespace core {
namespace codec {

class PushPostingsWriterBase {
 private:
  lucene::core::index::PostingsEnum* postings_enum; 
  uint32_t enum_flags;

 protected:
  lucene::core::index::FieldInfo* field_info;
  lucene::core::index::IndexOptions index_options;

  // TODO(0ctopus13prime): Bit flag instead of individual bool?
  bool write_freqs;
  bool write_positions;
  bool write_payloads;
  bool write_offsets;

 protected:
  PushPostingsWriterBase() = default; 

 public:
  virtual ~PushPostingsWriterBase() = default;

  virtual void Init(lucene::core::store::IndexOutput* terms_out,
                    lucene::core::index::SegmentWriteState& state) = 0

  virtual BlockTermState
  WriteTerm(lucene::core::util::BytesRef& term,
            lucene::core::index::TermsEnum& terms_enum,
            lucene::core::util::FixedBitSet& docs_seen) {
    StartTerm(); 
    postings_enum = terms_enum.Postings(postings_enum, enum_flags);
    assert(postings_enum != nullptr);

    int32_t doc_freq = 0;
    int64_t total_term_freq = 0;
  
    for (uint32_t doc_id = postings_enum->NextDoc() ; 
         doc_id != lucene::core::index::PostingsEnum::NO_MORE_DOCS ;
         doc_id = postings_enum->NextDoc()) {
      doc_freq++;
      docs_seen.set(doc_id);
      int32_t freq = -1;
      if (write_freqs) {
        freq = postings_enum->Freq();
        total_term_freq += freq;
      }

      StartDoc(doc_id, freq);

      if (write_positions) {
        for (uint32_t i = 0 ; i < freq ; ++i) {
          const int32_t pos = postings_enum->NextPosition();
          lucene::core::util::BytesRef* payload =
            (write_payloads ? postings_enum->GetPayload() : nullptr);
          int32_t start_offset = -1, end_offset = -1;
          if (write_offsets) {
            start_offset = postings_enum->StartOffset();
            end_offset = postings_enum->EndOffset();
          }

          AddPosition(pos, payload, start_offset, end_offset);
        }  // End for (pos loop)
      }  // End if (write_positions)

      FinishDoc();
    }  // End for (doc-id loop)

    if (doc_freq == 0) {
      return nullptr;
    } else {
      BlockTermState* state = NewTermState();
      state->doc_freq = doc_freq;
      state->total_term_freq = (write_flags ? total_term_freq : -1);
      FinishTerm(state);
      return state;
    }
  }

  virtual void EncodeTerm(uint64_t longs[],
                          lucene::core::store::DataOutput* out,
                          lucene::core::index::FiledInfo& field_info,
                          BlockTermState& state,
                          const bool absolute) = 0

  virtual void uint32_t
  SetField(lucene::core::index::FieldInfo* new_field_info) {
    using IndexOptions = lucene::core::index::IndexOptions;
    using PostingsEnum = lucene::core::index::PostingsEnum;

    field_info = new_field_info; 
    index_options = field_info->GetIndexOption();

    write_freqs =
      (index_options >= IndexOptions::DOCS_AND_FREQS);
    write_positions =
      (index_options >= IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
    write_offsets =
      (index_options >= IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    write_payloads = field_info.HasPayloads();

    if (!write_freqs) {
      enum_flags = 0;
    } else if (!write_positions) {
      enum_flags = PostingsEnum::FREQS;
    } else if (!write_offsets) {
      if (write_payloads) {
        enum_flags = PostingsEnum::PAYLOADS;
      } else {
        enum_flags = PostingsEnum::POSITIONS;
      }
    } else {
      if (write_payloads) {
        enum_flags = (PostingsEnum::PAYLOADS | PostingsEnum::OFFSETS);
      } else {
        enum_flags = PostingsEnum::OFFSETS;
      }
    }

    return 0;
  }

  virtual void Close() = 0;

  virtual BlockTermState NewTermState() = 0

  virtual void FinishiTerm(BlockTermState& state) = 0;

  virtual void StartDoc(const uint32_t doc_id, const uint32_t freq) = 0;

  virtual void AddPosition(const int32_t position,
                           lucene::core::util::BytesRef* payload,
                           const int32_t start_offset,
                           const int32_t end_offset) = 0;

  virtual void FinishDoc() = 0;
};  // PushPostingsWriterBase

}  // codec
}  // core
}  // lucene

#endif  // SRC_CODEC_POSTING_H_
