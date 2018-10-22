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

#ifndef SRC_CODEC_BLOCKTREE_WRITER_H_
#define SRC_CODEC_BLOCKTREE_WRITER_H_

#include <Codec/Consumer.h>
#include <Codec/Etc.h>
#include <Codec/Writer.h>
#include <Index/Field.h>
#include <Store/DataOutput.h>
#include <cstring>
#include <memory>
#include <vector>

namespace lucene {
namespace core {
namespace codec {

class BlockTreeTermsWriter: public FieldsConsumer {
 public:
  static const uint32_t DEFAULT_MIN_BLOCK_SIZE = 25;
  static const uint32_t DEFAULT_MAX_BLOCK_SIZE = 48;

  class FieldMetaData {
   public:
    lucene::core::index::FieldInfo field_info;
    lucene::core::util::BytesRef root_code;
    uint64_t num_terms;
    uint64_t index_start_fp;
    uint64_t sum_total_term_freq;
    uint64_t sum_doc_freq;
    uint32_t doc_count;
    uint64_t longs_size;
    lucene::core::util::BytesRef min_term;
    lucene::core::util::BytesRef max_term;

    FieldMetaData(lucene::core::index::FieldInfo&& field_info,
                  lucene::core::util::BytesRef&& root_code,
                  uint64_t num_terms,
                  uint64_t index_start_fp,
                  uint64_t sum_total_term_freq,
                  uint64_t sum_doc_freq,
                  uint32_t doc_count,
                  uint32_t longs_size,
                  lucene::core::util::BytesRef&& min_term,
                  lucene::core::util::BytesRef&& max_term)
      : field_info(std::forward<lucene::core::index::FieldInfo>(field_info)),
        root_code(std::forward<lucene::core::util::BytesRef>(root_code)),
        num_terms(num_terms),
        index_start_fp(index_start_fp),
        sum_total_term_freq(sum_total_term_freq),
        sum_doc_freq(sum_doc_freq),
        doc_count(doc_count),
        longs_size(longs_size),
        min_term(std::forward<lucene::core::util::BytesRef>(min_term)),
        max_term(std::forward<lucene::core::util::BytesRef>(max_term)) {
    }
  };  // FieldMetaData


  // TODO(0ctopus13prime): I don't understand.
  // Just a single bool field, do we really need a hierarchy structure here?
  class PendingEntry {
   public:
    bool is_term; 

    PendingEntry(bool is_term)
      : is_term(is_term) {
    }
  };  // PendingEntry

  class PendingTerm: public PendingEntry {
   public:
    std::unique_ptr<char[]> term_bytes; 
    uint32_t term_bytes_size;
    BlockTermState state;

    PendingTerm(const lucene::core::util::BytesRef& term,
                BlockTermState&& state)
      : PendingEntry(true),
        term_bytes(new char[term.Length()]),
        term_bytes_size(term.Length()),
        state(std::forward<BlockTermState>(state)) {
      std::memcpy(term_bytes.get(),
                  term.Bytes() + term.Offset(),
                  term.Length());
    }
  };  // PendingTerm

  class PendingBlock: public PendingEntry {
    // TODO(0ctopus13prime): IT
  };  // PendingBlock

  class TermWriter {
    // TODO(0ctopus13prime): IT
  };  // TermWriter

 private:
  std::unique_ptr<lucene::core::store::IndexOutput> terms_out;
  std::unique_ptr<lucene::core::store::IndexOutput> index_out;
  uint32_t max_doc; 
  uint32_t min_items_in_block;
  uint32_t max_items_in_block;

  std::unique_ptr<PostingsWriterBase> posting_writer;
  lucene::core::index::FieldInfos field_infos;

  std::vector<FieldMetaData> fields; 

  lucene::core::store::GrowableByteBucketOutputStream scratch_bytes;
  lucene::core::util::IntsRefBuilder scratch_ints_ref;

  bool closed;

 private:
  static uint64_t EncodeOutput(const uint64_t fp,
                               const bool has_terms,
                               const bool is_floor) {
    // TODO(0ctopus13prime): IT
  }

  static void WriteBytesRef(lucene::core::store::IndexOutput* out,
                            lucene::core::util::BytesRef& bytes) {
    out->WriteVInt32(bytes.Length());
    out->WriteBytes(bytes.Bytes(), bytes.Offset(), bytes.Length());
  }

  void WriteTrailer(lucene::core::store::IndexOutput* out, const uint64_t dir_start) {
    // TODO(0ctopus13prime): IT
  }

  void WriteIndexTrailer(lucene::core::store::IndexOutput* index_out,
                         const uint64_t dir_start) {
    // TODO(0ctopus13prime): IT
  }

 public:
  static void ValidateSettings(const uint32_t min_items_in_block,
                               const uint32_t max_items_in_block) {

  }
    
  void Write(lucene::core::index::Fields* fields) {
    // TODO(0ctopus13prime): IT
  }

  void Close() {
    // TODO(0ctopus13prime): IT
  }
};  // BlockTreeTermsWriter

}  // codec
}  // core
}  // lucene


#endif  // SRC_CODEC_BLOCKTREE_WRITER_H_
