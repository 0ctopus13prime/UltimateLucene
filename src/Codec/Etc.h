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

#ifndef SRC_CODEC_ETC_H_
#define SRC_CODEC_ETC_H_

namespace lucene {
namespace core {
namespace codec {

class BlockTermState {
 public:
  int32_t doc_freq;
  int32_t term_block_ord;
  int64_t total_term_freq;
  uint64_t block_file_pointer;
  uint64_t ord;

 protected:
  BlockTermState()
    : doc_freq(0),
      term_block_ord(0),
      total_term_freq(0),
      block_file_pointer(0),
      ord(0) {
  }

 public:
  BlockTermState(const BlockTermState& other)
    : doc_freq(other.doc_freq),
      term_block_ord(other.term_block_ord),
      total_term_freq(other.total_term_freq),
      block_file_pointer(other.block_file_pointer),
      ord(other.ord) {
  };

  BlockTermState& operator=(const BlockTermState& other) {
    if (this != &other) {
      doc_freq = other.doc_freq;
      term_block_ord = other.term_block_ord;
      total_term_freq = other.total_term_freq;
      block_file_pointer = other.block_file_pointer;
      ord = other.ord;
    }

    return *this;
  }
};  // BlockTermState

}  // namespace codec
}  // namespace core
}  // namespace lucene

#endif  // SRC_CODEC_ETC_H_
