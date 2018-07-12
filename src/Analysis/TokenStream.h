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

#ifndef SRC_ANALYSIS_TOKENSTREAM_H_
#define SRC_ANALYSIS_TOKENSTREAM_H_

#include <Analysis/Attribute.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/Reader.h>
#include <Util/Attribute.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace lucene {
namespace core {
namespace analysis {

template<typename T>
using delete_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

class TokenStream: public lucene::core::util::AttributeSource {
 protected:
  TokenStream();
  explicit TokenStream(const lucene::core::util::AttributeSource& input);
  explicit TokenStream(lucene::core::util::AttributeFactory& factory);

 public:
  static lucene::core::util::AttributeFactory* DEFAULT_TOKEN_ATTRIBUTE_FACTORY;

 public:
  virtual ~TokenStream();
  virtual bool IncrementToken() = 0;
  virtual void End();
  virtual void Reset() = 0;
  virtual void Close();
};

class TokenFilter: public TokenStream {
 protected:
  std::shared_ptr<TokenStream> input;

 protected:
  explicit TokenFilter(std::shared_ptr<TokenStream> input);
  explicit TokenFilter(TokenStream* input);

 public:
  virtual ~TokenFilter();
  void End() override;
  void Reset() override;
  void Close() override;

  template <typename ATTR>
  std::shared_ptr<ATTR> AddAttribute() {
    std::shared_ptr<ATTR> attr = input->AddAttribute<ATTR>();
    input->ShallowCopyTo(*this);  // For sharing attributes

    return attr;
  }
};


class Tokenizer: public TokenStream {
 private:
  static lucene::core::analysis::IllegalStateReader ILLEGAL_STATE_READER;

 protected:
  Reader* input;  // Read onlly
  Reader* input_pending;  // Read only

 protected:
  Tokenizer();
  explicit Tokenizer(lucene::core::util::AttributeFactory& factory);
  uint32_t CorrectOffset(const uint32_t current_off);

 public:
  virtual ~Tokenizer();
  void SetReader(Reader& new_input);
  void Reset() override;
  void Close();

  // Only used for testing
  void SetReaderTestPoint();
};

class LowerCaseFilter: public TokenFilter {
 private:
  std::shared_ptr<tokenattributes::CharTermAttribute> term_att;

 public:
    explicit LowerCaseFilter(std::shared_ptr<TokenStream> in);
    explicit LowerCaseFilter(TokenStream* in);
    virtual ~LowerCaseFilter();
    bool IncrementToken() override;
};

class CachingTokenFilter: public TokenFilter {
 private:
  std::vector<std::unique_ptr<lucene::core::util::AttributeSource::State>>
    cache;
  std::vector<std::unique_ptr<lucene::core::util::AttributeSource::State>>
    ::iterator iterator;
  std::unique_ptr<lucene::core::util::AttributeSource::State> final_state;
  bool first_time;

 private:
  void FillCache();
  bool IsCached();

 public:
  explicit CachingTokenFilter(std::shared_ptr<TokenStream> in);
  explicit CachingTokenFilter(TokenStream* in);
  virtual ~CachingTokenFilter();
  void End() override;
  void Reset() override;
  bool IncrementToken() override;
};

class FilteringTokenFilter: public TokenFilter {
 private:
  std::shared_ptr<tokenattributes::PositionIncrementAttribute> pos_incr_attr;
  int32_t skipped_positions;

 protected:
  virtual bool Accept() = 0;

 public:
  explicit FilteringTokenFilter(std::shared_ptr<TokenStream> in);
  explicit FilteringTokenFilter(TokenStream* in);
  virtual ~FilteringTokenFilter();
  bool IncrementToken() override;
  void Reset() override;
  void End() override;
};

class StopFilter: public FilteringTokenFilter {
 private:
  characterutil::CharSet stop_words;
  std::shared_ptr<tokenattributes::CharTermAttribute> term_att;

 protected:
  bool Accept() override;

 public:
  StopFilter(TokenStream* in, characterutil::CharSet& stop_words);
  StopFilter(TokenStream* in, characterutil::CharSet&& stop_words);
  StopFilter(std::shared_ptr<TokenStream> in,
             characterutil::CharSet& stop_words);
  StopFilter(std::shared_ptr<TokenStream> in,
             characterutil::CharSet&& stop_words);
  virtual ~StopFilter();

  static characterutil::CharSet
  MakeStopSet(std::vector<std::string>& stop_words, bool ignore_case = false);
};

}  // namespace analysis
}  // namespace core
}  // namespace lucene

#endif  // SRC_ANALYSIS_TOKENSTREAM_H_
