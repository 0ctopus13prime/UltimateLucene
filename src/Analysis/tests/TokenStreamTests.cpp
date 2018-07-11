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

#include <Analysis/Attribute.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/TokenStream.h>
#include <gtest/gtest.h>
#include <array>
#include <iostream>
#include <memory>

using lucene::core::analysis::LowerCaseFilter;
using lucene::core::analysis::StopFilter;
using lucene::core::analysis::StringReader;
using lucene::core::analysis::TokenFilter;
using lucene::core::analysis::TokenStream;
using lucene::core::analysis::Tokenizer;
using lucene::core::analysis::characterutil::CharSet;
using lucene::core::analysis::tokenattributes::CharTermAttribute;
using lucene::core::analysis::tokenattributes::OffsetAttribute;

class NaiveWhiteSpaceTokenizer: public Tokenizer {
 public:
  std::shared_ptr<CharTermAttribute> term_att;
  std::shared_ptr<OffsetAttribute> offset_att;
  uint32_t index;
  uint32_t start_offset;
  uint32_t end_offset;
  std::string line;

 public:
  NaiveWhiteSpaceTokenizer()
    : term_att(AddAttribute<CharTermAttribute>()),
      offset_att(AddAttribute<OffsetAttribute>()),
      index(0),
      start_offset(0),
      end_offset(0),
      line() {
  }

  void Reset() {
    Tokenizer::Reset();
    index = 0;
    start_offset = 0;
    end_offset = 0;
    line.clear();
  }

  void End() {
    Tokenizer::End();
    offset_att->SetOffset(index, index);
  }

  bool IncrementToken() {
    if (line.empty()) {
      input->ReadLine(line);
    }
    if (index >= line.size()) {
      return false;
    }

    term_att->SetEmpty();
    char* buffer = term_att->Buffer();
    uint32_t buf_idx = 0;
    const char* cstr = line.c_str();
    uint32_t length = line.size();
    uint32_t old_index = index;
    while (index < length && cstr[index] != ' ') {
      index++;
    }
    term_att->Append(line, old_index, index);
    offset_att->SetOffset(old_index, index);

    index++;

    return true;
  }
};

class HateThreeWordsTokenFilter: public TokenFilter {
 public:
  std::shared_ptr<CharTermAttribute> term_att;
  std::shared_ptr<OffsetAttribute> offset_att;

 public:
  explicit HateThreeWordsTokenFilter(TokenStream* in)
    : TokenFilter(in),
      term_att(AddAttribute<CharTermAttribute>()),
      offset_att(AddAttribute<OffsetAttribute>()) {
  }

  bool IncrementToken() override {
    while (input->IncrementToken()) {
      uint32_t len = term_att->Length();
      if (len != 3) {
        return true;
      }
    }

    return false;
  }
};

class TransparentTokenFilter: public TokenFilter {
 public:
  std::shared_ptr<CharTermAttribute> term_att;
  std::shared_ptr<OffsetAttribute> offset_att;

 public:
  explicit TransparentTokenFilter(TokenStream* in)
    : TokenFilter(in),
      term_att(AddAttribute<CharTermAttribute>()),
      offset_att(AddAttribute<OffsetAttribute>()) {
  }

  bool IncrementToken() override {
    return input->IncrementToken();
  }
};

TEST(TOKENIZER__TESTS, BASIC__TEST) {
  NaiveWhiteSpaceTokenizer* nws_tnz = new NaiveWhiteSpaceTokenizer();
  CharSet dont_care_case_set({"stop1", "stop2", "stop3", "stop4"}, true);
  TransparentTokenFilter top_tf(
    new LowerCaseFilter(
      new StopFilter(
        new HateThreeWordsTokenFilter(nws_tnz),
        std::move(dont_care_case_set) ) ) );

  StringReader reader;
  std::string str = "A bcd stop1 EFG stop2 hi stop3 Jk Lmn STOP4";
  reader.SetValue(str);
  nws_tnz->SetReader(reader);
  top_tf.Reset();

  EXPECT_TRUE(top_tf.IncrementToken());
  EXPECT_EQ("a",
    std::string(top_tf.term_att->Buffer(), 0, top_tf.term_att->Length()));
  EXPECT_TRUE(top_tf.IncrementToken());
  EXPECT_EQ("hi",
    std::string(top_tf.term_att->Buffer(), 0, top_tf.term_att->Length()));
  EXPECT_TRUE(top_tf.IncrementToken());
  EXPECT_EQ("jk",
    std::string(top_tf.term_att->Buffer(), 0, top_tf.term_att->Length()));
  EXPECT_FALSE(top_tf.IncrementToken());

  top_tf.End();
  top_tf.Close();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
