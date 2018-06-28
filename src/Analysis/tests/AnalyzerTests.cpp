#include <iostream>
#include <memory>
#include <string>
#include <Analysis/TokenStream.h>
#include <Analysis/Attribute.h>
#include <Analysis/Analyzer.h>
#include <gtest/gtest.h>

using namespace lucene::core::analysis;
using namespace lucene::core::analysis::tokenattributes;

TEST(ANALYZER__TESTS, StringTokenStream) {
  std::string value("StringTokenStream is a cool stream");
  StringTokenStream sts(*TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY, value, value.size());
  std::shared_ptr<CharTermAttribute> char_term_attr = sts.AddAttribute<CharTermAttribute>();
  std::shared_ptr<OffsetAttribute> offset_attr = sts.AddAttribute<OffsetAttribute>();

  sts.Reset();
  sts.IncrementToken();

  EXPECT_EQ(value, std::string(char_term_attr->Buffer(), char_term_attr->Length()));
  EXPECT_EQ(0, offset_attr->StartOffset());
  EXPECT_EQ(value.size(), offset_attr->EndOffset());

  sts.End();
}

class DummyTokenizer: public Tokenizer {
  public:
    std::shared_ptr<CharTermAttribute> char_term_attr;
    bool read;

  public:
    DummyTokenizer()
      : Tokenizer(),
        char_term_attr(AddAttribute<CharTermAttribute>()),
        read(false) {
    }

    bool IncrementToken() {
      if(read) {
        return false;
      }

      std::string line;
      input->ReadLine(line);
      std::string tmp(line);
      char_term_attr->Append(tmp);
      return (read = true);
    }
};

class DummyTokenFilter: public TokenFilter {
  public:
    std::shared_ptr<CharTermAttribute> char_term_attr;

  public:
    DummyTokenFilter(std::shared_ptr<TokenStream> in)
      : TokenFilter(in),
        char_term_attr(AddAttribute<CharTermAttribute>()) {
    }

    DummyTokenFilter(TokenStream* in)
      : TokenFilter(in),
        char_term_attr(AddAttribute<CharTermAttribute>()) {
    }

    bool IncrementToken() {
      return input->IncrementToken();
    }
};

TEST(ANALYZER__TESTS, TokenStreamComponents) {
  std::shared_ptr<DummyTokenizer> tnz = std::make_shared<DummyTokenizer>();
  std::shared_ptr<DummyTokenFilter> tf = std::make_shared<DummyTokenFilter>(tnz);
  TokenStreamComponents ts_components(tnz, tf);

  EXPECT_EQ(tnz.get(), &(ts_components.GetTokenizer()));
  EXPECT_EQ(tf.get(), &(ts_components.GetTokenStream()));

  StringReader reader;
  std::string line("Wow! What a wonderful day!");
  reader.SetValue(line);
  ts_components.SetReader(reader);

  TokenStream& got_tf = ts_components.GetTokenStream();

  got_tf.Reset();
  got_tf.IncrementToken();

  EXPECT_EQ(line, std::string(tf->char_term_attr->Buffer(), 0, tf->char_term_attr->Length()));

  got_tf.End();
  got_tf.Close();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
