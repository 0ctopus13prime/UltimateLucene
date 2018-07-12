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

#include <Analysis/Standard.h>
#include <string>
#include <stdexcept>
#include <utility>

using lucene::core::analysis::TokenStream;
using lucene::core::analysis::TokenStreamComponents;
using lucene::core::analysis::characterutil::CharSet;
using lucene::core::analysis::delete_unique_ptr;
using lucene::core::analysis::standard::StandardAnalyzer;
using lucene::core::analysis::standard::StandardFilter;
using lucene::core::analysis::standard::StandardTokenizer;
using lucene::core::analysis::standard::StandardTokenizerImpl;
using lucene::core::util::AttributeFactory;

/*
 * StandardAnalyzer
 */
const CharSet StandardAnalyzer::STOP_WORDS_SET({
  "a", "an", "and", "are", "as"
  , "at", "be", "but", "by", "for"
  , "if", "in", "into", "is", "it"
  , "no", "not", "of", "on", "or"
  , "such", "that", "the", "their", "then"
  , "there", "these", "they", "this", "to"
  , "was", "will", "with"
}, false);

TokenStreamComponents*
StandardAnalyzer::CreateComponents(const std::string& field_name) {
  // TODO(0ctopus13prime) Implement it.
  return nullptr;
}

delete_unique_ptr<TokenStream>
StandardAnalyzer::Normalize(const std::string& field_name,
                            delete_unique_ptr<TokenStream> in) {
  // TODO(0ctopus13prime) Implement it.
  return delete_unique_ptr<TokenStream>();
}

StandardAnalyzer::StandardAnalyzer(const CharSet& stop_words)
  : StopwordAnalyzerBase(StandardAnalyzer::STOP_WORDS_SET),
    max_token_length(StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH) {
}

StandardAnalyzer::StandardAnalyzer()
  : StandardAnalyzer(StandardAnalyzer::STOP_WORDS_SET) {
}

StandardAnalyzer::StandardAnalyzer(CharSet&& stop_words)
  : StopwordAnalyzerBase(std::forward<CharSet>(stop_words)),
    max_token_length(StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH) {
}

StandardAnalyzer::~StandardAnalyzer() {
}

void StandardAnalyzer::SetMaxTokenLength(const uint32_t length) {
  max_token_length = length;
}

uint32_t StandardAnalyzer::GetMaxTokenLength() {
  return max_token_length;
}


/*
 * StandardFilter
 */
StandardFilter::StandardFilter(TokenStream* in)
  : TokenFilter(in) {
}

StandardFilter::~StandardFilter() {
}

bool StandardFilter::IncrementToken() {
  return input->IncrementToken();
}


/*
 * StandardTokenizerImpl
 */

const int32_t StandardTokenizerImpl::YYEOF = -1;

StandardTokenizerImpl::StandardTokenizerImpl(Reader* in) {
  // TODO(0ctopus13prime) Implement it.
}

StandardTokenizerImpl::~StandardTokenizerImpl() {
  // TODO(0ctopus13prime) Implement it.
}

void StandardTokenizerImpl::SetBufferSize(uint32_t length) {
  // TODO(0ctopus13prime) Implement it.
}

uint32_t StandardTokenizerImpl::GetNextToken() {
  // TODO(0ctopus13prime) Implement it.
  return 0;
}

void
StandardTokenizerImpl::GetText(tokenattributes::CharTermAttribute& term_att) {
  // TODO(0ctopus13prime) Implement it.
}

uint32_t StandardTokenizerImpl::YyLength() {
  // TODO(0ctopus13prime) Implement it.
  return 0;
}

uint32_t StandardTokenizerImpl::YyChar() {
  // TODO(0ctopus13prime) Implement it.
  return 0;
}

void StandardTokenizerImpl::YyReset(lucene::core::analysis::Reader* reader) {
  // TODO(0ctopus13prime) Implement it.
}


/*
 * StandardTokenizer
 */

const uint32_t StandardTokenizer::ALPHANUM = 0;
const uint32_t StandardTokenizer::NUM = 1;
const uint32_t StandardTokenizer::SOUTHEAST_ASIAN = 2;
const uint32_t StandardTokenizer::IDEOGRAPHIC = 3;
const uint32_t StandardTokenizer::HIRAGANA = 4;
const uint32_t StandardTokenizer::KATAKANA = 5;
const uint32_t StandardTokenizer::HANGUL = 6;
const uint32_t StandardTokenizer::MAX_TOKEN_LENGTH_LIMIT = 1024 * 1024;
const char* StandardTokenizer::TOKEN_TYPES[] = {
  "<ALPHANUM>",
  "<NUM>",
  "<SOUTHEAST_ASIAN>",
  "<IDEOGRAPHIC>",
  "<HIRAGANA>",
  "<KATAKANA>",
  "<HANGUL>"
};

StandardTokenizer::StandardTokenizer()
  : Tokenizer(),
    skipped_positions(),
    max_token_length(),
    term_att(AddAttribute<tokenattributes::CharTermAttribute>()),
    offset_att(AddAttribute<tokenattributes::OffsetAttribute>()),
    pos_incr_att(AddAttribute<tokenattributes::PositionIncrementAttribute>()),
    type_att(AddAttribute<tokenattributes::TypeAttribute>()),
    scanner(input) {
}

StandardTokenizer::StandardTokenizer(AttributeFactory& factory)
  : Tokenizer(factory),
    skipped_positions(),
    max_token_length(),
    term_att(AddAttribute<tokenattributes::CharTermAttribute>()),
    offset_att(AddAttribute<tokenattributes::OffsetAttribute>()),
    pos_incr_att(AddAttribute<tokenattributes::PositionIncrementAttribute>()),
    type_att(AddAttribute<tokenattributes::TypeAttribute>()),
    scanner(input) {
}

StandardTokenizer::~StandardTokenizer() {
}

void StandardTokenizer::SetMaxTokenLength(uint32_t length) {
  if (length > MAX_TOKEN_LENGTH_LIMIT) {
    throw std::invalid_argument("Max token length may not exceed "
                                + std::to_string(MAX_TOKEN_LENGTH_LIMIT));
  }

  max_token_length = length;
  scanner.SetBufferSize(length);
}

uint32_t StandardTokenizer::GetMaxTokenLength() {
  return max_token_length;
}

bool StandardTokenizer::IncrementToken() {
  ClearAttributes();
  skipped_positions = 0;

  while (true) {
    uint32_t token_type = scanner.GetNextToken();
    if (token_type == StandardTokenizerImpl::YYEOF) {
      return false;
    }

    if (scanner.YyLength() <= max_token_length) {
      pos_incr_att->SetPositionIncrement(skipped_positions + 1);
      scanner.GetText(*term_att.get());
      const uint32_t start = scanner.YyChar();
      offset_att->SetOffset(CorrectOffset(start),
                            CorrectOffset(start + term_att->Length()));
      type_att->SetType(StandardTokenizer::TOKEN_TYPES[token_type]);
      return true;
    } else {
      // When we skip a too-long term, we still increment the
      // position increment
      skipped_positions++;
    }
  }

  return false;
}

void StandardTokenizer::End() {
  Tokenizer::End();
  // Set final offset
  uint32_t final_offset = CorrectOffset(scanner.YyChar() + scanner.YyLength());
  offset_att->SetOffset(final_offset, final_offset);

  // Adjust any skipped tokens
  pos_incr_att->SetPositionIncrement(pos_incr_att->GetPositionIncrement()
                                     + skipped_positions);
}

void StandardTokenizer::Close() {
  Tokenizer::Close();
  scanner.YyReset(input);
}

void StandardTokenizer::Reset() {
  Tokenizer::Reset();
  scanner.YyReset(input);
  skipped_positions = 0;
}
