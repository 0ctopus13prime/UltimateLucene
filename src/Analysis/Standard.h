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

#ifndef SRC_ANALYSIS_STANDARD_H_
#define SRC_ANALYSIS_STANDARD_H_

#include <Analysis/Analyzer.h>
#include <Analysis/AttributeImpl.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/Reader.h>
#include <Analysis/TokenStream.h>
#include <Util/Attribute.h>
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace analysis {
namespace standard {

class StandardAnalyzer: public lucene::core::analysis::StopwordAnalyzerBase  {
 public:
  static const uint32_t DEFAULT_MAX_TOKEN_LENGTH = 255;
  static const lucene::core::analysis::characterutil::CharSet STOP_WORDS_SET;

 private:
  uint32_t max_token_length;

 protected:
  TokenStreamComponents*
    CreateComponents(const std::string& field_name) override;
  delete_unique_ptr<TokenStream>
    Normalize(const std::string& field_name, delete_unique_ptr<TokenStream> in);

 public:
  StandardAnalyzer();
  explicit StandardAnalyzer(lucene::core::analysis::Reader&& stop_words);
  StandardAnalyzer(const lucene::core::analysis::characterutil::CharSet&
                     stop_words);
  explicit StandardAnalyzer(lucene::core::analysis::characterutil::CharSet&&
                              stop_words);
  ~StandardAnalyzer();
  void SetMaxTokenLength(const uint32_t length);
  uint32_t GetMaxTokenLength();
};

class StandardFilter: public lucene::core::analysis::TokenFilter {
 public:
  explicit StandardFilter(TokenStream* in);
  virtual ~StandardFilter();
  bool IncrementToken() override;
};

class StandardTokenizerImpl {
 public:
  static const int32_t YYEOF;

 public:
  explicit StandardTokenizerImpl(lucene::core::analysis::Reader* in);
  ~StandardTokenizerImpl();
  void SetBufferSize(uint32_t length);
  uint32_t GetNextToken();
  void GetText(tokenattributes::CharTermAttribute& term_att);
  uint32_t YyLength();
  uint32_t YyChar();
  void YyReset(lucene::core::analysis::Reader* reader);
};

class StandardTokenizer: public lucene::core::analysis::Tokenizer {
 public:
  static const uint32_t ALPHANUM;
  static const uint32_t NUM;
  static const uint32_t SOUTHEAST_ASIAN;
  static const uint32_t IDEOGRAPHIC;
  static const uint32_t HIRAGANA;
  static const uint32_t KATAKANA;
  static const uint32_t HANGUL;
  static const char* TOKEN_TYPES[];
  static const uint32_t MAX_TOKEN_LENGTH_LIMIT;

 private:
  int32_t skipped_positions;
  int32_t max_token_length/* = StandardAnalyzer.DEFAULT_MAX_TOKEN_LENGTH*/;
  std::shared_ptr<tokenattributes::CharTermAttribute> term_att;
  std::shared_ptr<tokenattributes::OffsetAttribute> offset_att;
  std::shared_ptr<tokenattributes::PositionIncrementAttribute> pos_incr_att;
  std::shared_ptr<tokenattributes::TypeAttribute> type_att;
  StandardTokenizerImpl scanner;

 public:
  StandardTokenizer();
  explicit StandardTokenizer(lucene::core::util::AttributeFactory& factory);
  virtual ~StandardTokenizer();
  void SetMaxTokenLength(uint32_t length);
  uint32_t GetMaxTokenLength();
  bool IncrementToken() override;
  void End();
  void Close();
  void Reset();
};


}  // namespace standard
}  // namespace analysis
}  // namespace core
}  // namespace lucene

#endif  // SRC_ANALYSIS_STANDARD_H_
