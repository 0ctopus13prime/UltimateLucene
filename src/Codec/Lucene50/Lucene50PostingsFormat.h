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

#ifndef SRC_CODEC_LUCENE50_LUCENE50POSTINGSFORMAT_H_
#define SRC_CODEC_LUCENE50_LUCENE50POSTINGSFORMAT_H_

#include <Store/DataOutput.h>
#include <Codec/Posting.h>
#include <Codec/Lucene50/ForUtil.h>
#include <memory>
#include <vector>

namespace lucene {
namespace core {
namespace codec {

class Lucene50SkipWriter {
 private:
  uint32_t number_of_skip_levels;
  uint32_t skip_interval;
  uint32_t skip_multiplier;
  GrowableByteBucketOutputStream* skip_buffer; 
  std::vector<int32_t> last_skip_doc;
  std::vector<int32_t> last_skip_doc_pointer;
  std::vector<int32_t> last_skip_pos_pointer;
  std::vector<int32_t> last_payload_byte_upto;

  IndexOutput* doc_out;
  IndexOutput* pos_out;
  IndexOutput* pay_out;

  uint32_t cur_doc;
  uint32_t cur_pos_buffer_upto;
  uint32_t cur_payload_byte_upto;
  uint64_t cur_doc_pointer;
  uint64_t cur_pos_pointer;
  uint64_t cur_pay_pointer;
  bool field_has_positions;
  bool field_has_offsets;
  bool field_has_payloads;

  bool initialized;
  uint64_t last_doc_fp;
  uint64_t last_pos_fp;
  uint64_t last_pay_fp;

 private:
  void ResetSkip() {
    last_doc_fp = doc_out->GetFilePointer();
    if (field_has_positions) {
      last_pos_fp = pos_out->GetFilePointer();
      if (field_has_offsets || field_has_payloads) {
        last_pay_fp = pay_out->GetFilePointer();
      }
    }

    initialized = false;
  }

  void WriteSkipData(const uint32_t level, IndexOutput* skip_buffer) {
    const uint32_t delta = (cur_doc - last_skip_doc[level]);

    skip_buffer->WriteVInt32(delta);
    last_skip_doc[level] = cur_doc;

    skip_buffer->WriteVInt64(cur_doc_pointer - last_skip_doc_pointer[level]);
    last_skip_doc_pointer[level] = cur_doc_pointer;

    if (field_has_positions) {
      skip_buffer->WriteVInt64(cur_pos_pointer - last_skip_pos_pointer[level]);
      last_skip_pos_pointer[level] = cur_pos_pointer;
      skip_buffer->WriteVInt32(cur_pos_buffer_upto);

      if (field_has_payloads) {
        skip_buffer->WriteVInt32(cur_payload_byte_upto);
      }

      if (field_has_offsets || field_has_payloads) {
        skip_buffer->WriteVInt64(cur_pay_pointer -
                                 last_skip_pay_pointer[level]);
        last_skip_pay_pointer[level] = cur_pay_pointer;
      }
    }  // End-if
  }

  void SetUp() {
    // TODO(Kdy): IT
  }

 public:
  void BufferSkip(const uint32_t doc,
                  const uint32_t num_docs,
                  const uint64_t pos_fp,
                  const uint64_t pay_fp,
                  const uint32_t pos_buffer_upto,
                  const uint32_t payload_byte_upto) {
    InitSkip();
    cur_doc = doc; 
    cur_doc_pointer = doc_out->GetFilePointer();
    cur_pos_pointer = pos_fp;
    cur_pay_pointer = pay_fp;
    cur_pos_buffer_upto = pos_buffer_upto;
    cur_payload_byte_upto = payload_byte_upto;
    BufferSkip(num_docs);
  }

  void InitSkip() {
    if (!initialized) {
      SetUp();  
      std::fill(last_skip_doc.begin(), last_skip_doc.end(), 0);
      std::fill(last_skip_doc_pointer.begin(),
                last_skip_doc_pointer.end(),
                last_doc_fp);

      if (field_has_positions) {
        std::fill(last_skip_pos_pointer.begin(),
                  last_skip_pos_pointer.end(),
                  last_pos_fp); 

        if (field_has_payloads) {
          std::fill(last_payload_byte_upto.begin(),
                    last_payload_byte_upto.end(),
                    0);
        }

        if (field_has_offsets || field_has_payloads) {
          std::fill(last_skip_pay_pointer.begin(),
                    last_skip_pay_pointer.end(),
                    last_pay_fp);
        }
      }  // End-if

      initialized = true;
    }
  }

  void BufferSkip(const uint32_t df) {
    assert(df % skip_interval == 0);
    uint32_t num_levels = 1;
    df /= skip_interval;

    // Determine max level
    while ((df % skip_multiplier) == 0 && num_levels < number_of_skip_levels) {
      num_levels++;
      df /= skip_multiplier;
    }

    uint64_t child_pointer = 0;

    for (uint32_t level = 0 ; level < num_levels ; level++) {
      WriteSkipData(level, skip_buffer[level]);

      const uint64_t new_child_pointer = skip_buffer[level].GetFilePointer();

      if (level != 0) {
        // Store child pointers for all levels except the lowest
        skip_buffer[level]->WriteVInt64(child_pointer);
      }

      // Remember the child pointer for the next level
      child_pointer = new_child_pointer;
    }  // End-for
  }

  uint64_t WriteSkip(IndexOutput* output) {
    const uint64_t skip_pointer = output->GetFilePointer();
    if (skip_buffer.size() == 0) return skip_pointer;

    for (uint32_t level = number_of_skip_levels - 1 ; level > 0 ; --level) {
      const uint64_t length = skip_buffer[level]->GetFilePointer();
      if (length > 0) {
        output->WriteVInt64(length);
        skip_buffer[level]->WriteTo(output);
      }
    }  // End-for

    skip_buffer[0]->WriteTo(output);

    return skip_pointer;
  }

  void SetField(const bool new_field_has_positions,
                const bool new_field_has_offsets,
                const bool new_field_has_payloads) {
    field_has_positions = new_field_has_positions; 
    field_has_offsets = new_field_has_offsets;
    field_has_payloads = new_field_has_payloads;
  }

  void Init(const uint32_t new_max_skip_levels,
            const uint32_t block_size,
            const uint32_t doc_count,
            lucene::core::store::IndexOut* new_doc_out,
            lucene::core::store::IndexOut* new_pos_out,
            lucene::core::store::IndexOut* new_pay_out) {
    skip_interval = block_size;
    skip_multiplier = 8;
    max_skip_levels = new_max_skip_levels;
    df = doc_count;

    // Calculate the maximum number of skip levels for this document frequency
    if (df <= skip_interval) {
      number_of_skip_levels = 1;
    } else {
      number_of_skip_levels =
        (1 + MathUtil::Log(df / skip_interval, skip_multiplier));
    }

    // Make sure it does not exceed max_skip_levels 
    if (number_of_skip_levels > max_skip_levels) {
      number_of_skip_levels = max_skip_levels;
    }

    // Setting outs
    doc_out = new_doc_out;
    pos_out = new_pos_out;
    pay_out = new_pay_out;

    // Setting last variables
    last_skip_doc.reserve(max_skip_levels);
    last_skip_doc_pointer.reserve(max_skip_levels);
    if (pos_out != nullptr) {
      last_skip_pos_pointer.reserve(max_skip_levels);
      if (pay_out != nullptr) {
        last_skip_pay_pointer.reserve(max_skip_levels);
      }
      last_payload_byte_upto.reserve(max_skip_levels);
    }
  }
};  // Lucene50SkipWriter

class Lucene50PostingsWriter : public PushPostingsWriterBase {
 using IndexOutput = lucene::core::store::IndexOutput;

 private:
  static IntBlockTermState EMPTY_STATE;

  lucene::core::index::FieldInfo* field_info;
  lucene::core::index::IndexOptions index_options;

  // TODO(0ctopus13prime): Bit flag instead of individual bool?
  bool write_freqs;
  bool write_positions;
  bool write_payloads;
  bool write_offsets;

  uint32_t enum_flags;
  IntBlockTermState last_state;

  std::unique_ptr<IndexOutput> doc_out;
  std::unique_ptr<IndexOutput> pos_out;
  std::unique_ptr<IndexOutput> pay_out;

  // Holds starting file pointers for current term:
  int64_t doc_start_fp;
  int64_t pos_start_fp;
  int64_t pay_start_fp;

  std::vector<int32_t> doc_delta_buffer;
  std::vector<int32_t> freq_buffer;
  uint32_t doc_buffer_upto;

  std::vector<int32_t> pos_delta_buffer;
  std::vector<int32_t> payload_length_buffer;
  std::vector<int32_t> offset_start_delta_buffer;
  std::vector<int32_t> offset_length_buffer;
  uint32_t pos_buffer_upto;

  std::vector<uint8_t> payload_bytes;
  uint32_t payload_byte_upto;

  int32_t last_block_doc_id;
  int64_t last_block_pos_fp;
  int64_t last_block_pay_fp;
  int32_t last_block_pos_buffer_upto;
  int32_t last_block_payload_byte_upto;

  uint32_t last_doc_id;
  uint32_t last_position;
  uint32_t last_start_offset;
  uint32_t doc_count;

  std::vector<uint8_t> encoded;

  ForUtil for_util;
  Lucene50SkipWriter skip_writer;
  IntBlockTermState empty_state;

 public:
  Lucene50PostingsWriter(lucene::core::index::SegmentWriteState& state) {
    const flog acceptable_ovehead_ratio = PackedInts::COMPACT;

    // Doc file
    std::string doc_file_name =
      IndexFileNames::SegmentFileName(state.segment_info.name,
                                      state.segment_suffix,
                                      Lucene50PostingsFormat::DOC_EXTENSION);
    doc_out =
      std::move(state.directory.CreateOutput(doc_file_name, state.context));
    CodecUtil::WriteIndexHeader(doc_out, DOC_CODEC, VERSION_CURRENT,
                                state.segment_info.GetId(),
                                state.segment_suffix);

    for_util.Init(acceptable_overhead_ratio, doc_out.get());

    if (state.field_infos.HasProx()) {
      // Pos file
      pos_delta_buffer.reserve(MAX_DATA_SIZE); 
      std::string pos_file_name =
        IndexFileNames.SegmentFileName(state.segment_info.name,
                                       state.segment_suffix,
                                       Lucene50PostingsFormat::POS_EXTENSION);
      pos_out =
        std::move(state.directory.CreateOutput(pos_file_name, state.context));
      CodecUtil::WriteIndexHeader(pos_out.get(),
                                  POS_CODEC,
                                  VERSION_CURRENT,
                                  state.segment_info.GetId(),
                                  state.segment_suffix);

      // Setting payload buffers
      if (state.field_infos.HasPayloads()) {
        payload_bytes.reserve(128);   
        payload_length_buffer.reserve(128);
      }

      // Setting offset buffers
      if (state.field_infos.HasOffsets()) {
        offset_start_delta_buffer.reserve(MAX_DATA_SIZE); 
        offset_length_buffer.reserve(MAX_DATA_SIZE);
      }

      // Pay file
      if (state.field_infos.HasPayloads() ||
          state.field_infos.HasOffsets()) {
        std::string pay_file_name =
          IndexFileNames::SegmentFileName(state.segment_info.name,
                                        state.segment_suffix,
                                        Lucene50PostingsFormat::PAY_EXTENSION);
        pay_out = std::move(state.directory.create_output(pay_file_name,
                                                          state.context)); 
        CodecUtil::WriteIndexHeader(pay_out.get(), PAY_CODEC, VERSION_CURRENT,
                                      state.segment_info.GetId(),
                                      state.segment_suffix);
      }
    }  // End pos, pay file

    // Delta buffers + SkipWriter
    doc_delta_buffer.reserve(ForUtil::MAX_DATA_SIZE);
    freq_buffer.reserve(ForUtil::MAX_DATA_SIZE);
    skip_writer.Init(MAX_SKIP_LEVELS, BLOCK_SIZE, state.segment_info.MaxDoc(),
                     pos_out.get(), pay_out.get());
    encoded.reserve(MAX_ENCODED_SIZE);
  }

  void Init(lucene::core::store::IndexOutput* terms_out,
            lucene::core::index::SegmentWriteState& state) {
    CodecUtil::WriteIndexHeader(terms_out, TERMS_CODEC, VERSION_CURRENT,
                            state.segment_info.GetId(), state.segment_suffix);
    terms_out->WriteVInt(BLOCK_SIZE);
  }

  template<class TermsEnum>
  IntBlockTermState WriteTerm(lucene::core::util::BytesRef& term,
                              TermsEnum& terms_enum,
                              lucene::core::util::FixedBitSet& docs_seen) {
    StartTerm(); 
    auto postings_enum = terms_enum.Postings(enum_flags);
    assert(postings_enum != nullptr);

    int32_t doc_freq = 0;
    int64_t total_term_freq = 0;
  
    for (uint32_t doc_id : postings_enum) {
      doc_freq++;
      docs_seen.Set(doc_id);
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
      IntBlockTermState state;
      state.doc_freq = doc_freq;
      state.total_term_freq = (write_freqs ? total_term_freq : -1);
      FinishTerm(state);
      return state;
    }
  }

  void EncodeTerm(uint64_t longs[],
                  lucene::core::store::DataOutput* out,
                  lucene::core::index::FieldInfo& field_info,
                  IntBlockTermState& state,
                  const bool absolute) {
    if (absolute) {
      last_state = empty_state;
    }

    // Doc fp delta
    longs[0] = (state.doc_start_fp - last_state.doc_start_fp);

    if (write_position) {
      // Pos fp delta
      longs[1] = (state.pos_start_fp - last_state.pos_start_fp);
      if (write_payloads || write_offsets) {
        // Pay fp delta
        longs[2] = (state.pay_start_fp - last_state.pay_start_fp);
      }
    }

    if (state.singleton_doc_id != -1) {
      out->WriteVInt32(state.singleton_doc_id);
    }

    if (write_positions && state.last_pos_block_offset != -1) {
      out->WriteVInt32(state.last_pos_block_offset); 
    }

    if (state.skip_offset != -1) {
      out->WriteVInt64(state.skip_offset);
    }

    last_state = state;
  }

  uint32_t SetField(lucene::core::index::FieldInfo* new_field_info) {
    using IndexOptions = lucene::core::index::IndexOptions;
    using PostingsEnum = lucene::core::index::PostingsEnum;

    field_info = new_field_info; 
    index_options = field_info->GetIndexOptions();

    write_freqs =
      (index_options >= IndexOptions::DOCS_AND_FREQS);
    write_positions =
      (index_options >= IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
    write_offsets =
      (index_options >= IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    write_payloads = field_info->HasPayloads();

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

  void Close() {
    if (doc_out) {
      CodecUtil::WriteFooter(doc_out.get());
      doc_out.reset();
    }

    if (pos_out) {
      CodecUtil::WriteFooter(pos_out.get());
      pos_out.reset();
    }

    if (pay_out) {
      CodecUtil::WriteFooter(pay_out.get());
      pay_out.reset();
    }
  }

  void StartTerm() {
    doc_start_fp = doc_out->GetFilePointer();
    if (write_positions) {
      if (write_payloads || write_offsets) {
        pay_start_fp = pay_out->GetFilePointer();
      }
    }

    last_doc_id = 0;
    last_block_doc_id = -1;
    skip_writer.ResetSkip();
  }

  void FinishTerm(IntBlockTermState& state) {
    // docFreq == 1, don't write the single docid/freq to
    // a separate file along with a pointer to it
    int32_t singleton_doc_id = -1;
    if (state.doc_freq == 1) {
      singleton_doc_id = doc_delta_buffer[0]; 
    } else {
      for (uint32_t i = 0 ; i < doc_buffer_upto ; i++) {
        const uint32_t doc_delta = doc_delta_buffer[i];
        const uint32_t freq = freq_buffer[i];
        if (!write_freqs) {
          doc_out->WriteVInt32(doc_delta);
        } else if (freq_buffer[i] == 1) {
          doc_out->WriteVInt32((doc_delta << 1) | 1);
        } else {
          doc_out->WriteVInt32(doc_delta << 1);
          doc_out->WriteVInt32(freq);
        }
      }
    }  // End if

    // Handling position
    int64_t last_pos_block_offset = -1;

    if (write_positions) {
      // Total_term_freq is just total 
      // number of positions(or payloads, or offsets)
      // associated with current term.
      assert(state.total_term_freq != -1);
      if (state.total_term_freq > BLOCK_SIZE) {
        // Record file offset for last pos in last block 
        last_pos_block_offset = (pos_out->GetFilePointer() - post_start_fp);   
      }

      if (pos_buffer_upto > 0) {
        // vInt encode the remaining positions/payloads/offsets: 
        // force first payload length to be written
        int32_t last_payload_length = -1;
        // force first offset length to be written
        int32_t last_offset_length = -1;
        int32_t payload_bytes_read_upto = 0;

        for (uint32_t i = 0 ; i < pos_buffer_upto ; i++) {
          const uint32_t pos_delta = pos_delta_buffer[i];
          if (write_payloads) {
            const uint32_t payload_length = payload_length_buffer[i]; 
            if (payload_length != last_payload_length) {
              last_payload_length = payload_length;  
              pos_out->WriteVInt32((pos_delta << 1) | 1);
              pos_out->WriteVInt32(payload_length);
            } else {
              pos_out->WriteVInt(pos_delta << 1);
            }

            if (payload_length != 0) {
              pos_out->WriteBytes(payload_bytes,
                                  payload_bytes_read_upto,
                                  payload_length);
              payload_bytes_read_upto += payload_length;
            }
          } else {
            pos_out->WriteVInt32(pos_delta);
          }

          if (write_offsets) {
            const uint32_t delta = offset_start_delta_buffer[i];
            const uint32_t length = offset_length_buffer[i];
            if (length == last_offset_length) {
              pos_out->WriteVInt32(delta << 1);
            } else {
              pos_out->WriteVInt(delta << 1 | 1);
              pos_out->WriteVInt(length);
              last_offset_length = length;
            }
          }
        }  // End-for

        if (write_payloads) {
          assert(payload_bytes_read_upto == payload_byte_upto);
          payload_byte_upto = 0;
        }
      }
    }  // End-if

    // Skip offset
    int64_t skip_offset = -1;
    if (doc_count > BLOCK_SIZE) {
      skip_offset = skip_writer.WriteSkip(doc_out.get()) - doc_start_fp;
    }

    // Setting state values
    state.doc_start_fp = doc_start_fp;
    state.pos_start_fp = pos_start_fp;
    state.pay_start_fp = pay_start_fp;
    state.singleton_doc_id = singleton_doc_id;
    state.skip_offset = skip_offset;
    state.last_pos_block_offset = last_pos_block_offset;
    doc_buffer_upto = pos_buffer_upto = last_doc_id = doc_count = 0;
  }

  void StartDoc(const uint32_t doc_id, const uint32_t freq) {
    // Have collected a block of docs, and get a new doc. 
    // Should write skip data as well as postings list for
    // current block.
    if (last_block_doc_id != -1 && doc_buffer_upto == 0) {
      skip_writer.BufferSkip(last_block_doc_id,
                             doc_count,
                             last_block_pos_fp,
                             last_block_pay_fp,
                             last_block_pos_buffer_upto,
                             last_block_payload_byte_upto);
    }

    const uint32_t doc_delta = (doc_id - last_doc_id);
    if (doc_id < last_doc_id) {
      throw CorruptIndexException();
    }

    doc_delta_buffer[doc_buffer_upto] = doc_delta;
    if (write_freqs) {
      freq_buffer[doc_buffer_upto] = term_doc_freq;
    }

    doc_buffer_upto++;
    doc_count++;

    if (doc_buffer_upto = BLOCK_SIZE) {
      for_util.WriteBlock(doc_delta_buffer, encoded, doc_out.get());
      for_util.WriteBlock(freq_buffer, encoded, doc_out.get());

      // NOTE: don't set docBufferUpto back to 0 here;
      // finishDoc will do so (because it needs to see that
      // the block was filled so it can save skip data)
    }

    last_doc_id = doc_id;
    last_position = 0;
    last_start_offset = 0;
  }

  void FinishDoc() {
    // Since we don't know df for current term, we had to buffer
    // those skip data for each block, and when a new doc comes, 
    // write them to skip file.
    if (doc_buffer_upto == BLOCK_SIZE) {
      last_block_doc_id = last_doc_id;
      if (pos_out) {
        if (pay_out) {
          last_block_pay_fp = pay_out->GetFilePointer();
        }
        last_block_pos_fp = pos_out->GetFilePointer();
        last_block_pos_buffer_upto = pos_buffer_upto;
        last_block_payload_byte_upto = payload_byte_upto;
      }

      doc_buffer_upto = 0;
    }
  }

  void AddPosition(const int32_t position,
                   lucene::core::util::BytesRef* payload,
                   const int32_t start_offset,
                   const int32_t end_offset) {
    if (position > IndexWriter::MAX_POSITION) {
      throw CorruptIndexException("Position = " + std::to_string(position) +
                                " is to large (> IndexWrite::MAX_POSITION = " +
                                std::to_string(IndexWriter::MAX_POSITION) +
                                ')');
    }

    if (position < 0) {
      throw CorruptIndexException("Position = " + std::to_string(position) +
                                  " is < 0");
    }

    pos_delta_buffer[pos_buffer_upto] = (position - last_position);
    if (write_payloads) {

    }  // End-if

    if (write_offsets) {

    }

    pos_buffer_upto++;
    last_position = position;
    if (pos_buffer_upto == BLOCK_SIZE) {
      for_util.WriteBlock(pos_delta_buffer, encoded, pos_out.get());

      if (write_payloads) {
        for_util.WriteBlock(payload_length_buffer, encoded, pay_out.get());
        pay_out->WriteVInt32(payload_byte_upto);
        pay_out->WriteBytes(payload_bytes, 0, payload_byte_upto);
        payload_byte_upto = 0;
      }

      if (write_offsets) {
        for_util.WriteBlock(offset_start_delta_buffer, encoded, pay_out.get()); 
        for_util.WriteBlock(offset_length_buffer, encoded, pay_out.get());
      }

      pos_buffer_upto = 0;
    }  // End-if
  }
};  // Lucene50PostingsWriter

class Lucene50PostingsFormat: public PostingsFormat {
 public:
  static const std::string DOC_EXTENSION;  // = "doc";
  static const std::string POS_EXTENSION;  // = "pos";
  static const std::string PAY_EXTENSION;  // = "pay";

 private:
  static const std::string TERMS_CODEC;  // = "Lucene50PostingsWriterTerms";
  static const std::string DOC_CODEC;  // = "Lucene50PostingsWriterDoc";
  static const std::string POS_CODEC;  // = "Lucene50PostingsWriterPos";
  static const std::string PAY_CODEC;  // = "Lucene50PostingsWriterPay";
  static const uint32_t MAX_SKIP_LEVELS = 10;
  static const uint32_t VERSION_START = 0;
  static const uint32_t VERSION_CURRENT = VERSION_START;
  static const uint32_t BLOCK_SIZE = 128;
  
  uint32_t minTermBlockSize;
  uint32_t maxTermBlockSize;

 public:
  Lucene50PostingsFormat()
    : Lucene50PostingsFormat(BlockTreeTermWriter::DEFAULT_MIN_BLOCK_SIZE,
                             BlockTreeTermWriter::DEFAULT_MAX_BLOCK_SIZE) {
  }

  Lucene50PostingsFormat(const uint32_t min_term_block_size,
                         const uint32_t max_term_block_size)
    : PostingsFormat(std::string("Lucene50")) {
  }

  FieldsConsumer* FieldConsumer(SegmentWriteState& state) {
    
  }

  FieldsProducer* FieldsProducer(SegmentWriteState& state) {
  
  }
};  // Lucene50PostingsFormat

}  // codec
}  // core
}  // lucene

#endif  // SRC_CODEC_LUCENE50_LUCENE50POSTINGSFORMAT_H_
