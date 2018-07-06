#include <thread>
#include <iostream>
#include <memory>
#include <string>
#include <Analysis/TokenStream.h>
#include <Analysis/Attribute.h>
#include <Analysis/Analyzer.h>
#include <gtest/gtest.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis;
using namespace lucene::core::analysis::tokenattributes;

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

class DummyAnalyzer: public Analyzer {
  protected:
    TokenStreamComponents* CreateComponents(const std::string& field_name) {
      std::shared_ptr<Tokenizer> tnz = std::make_shared<DummyTokenizer>();
      std::shared_ptr<TokenFilter> tf = std::make_shared<DummyTokenFilter>(tnz);
      return new TokenStreamComponents(tnz, tf);
    }
};

TEST(ANALYZER__TESTS, StringTokenStream) {
  std::string value("Wow! A miracle is happend in the World Cup!!");
  StringTokenStream sts(*TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY, value, value.size());
  sts.Reset();
  EXPECT_TRUE(sts.IncrementToken());
  std::shared_ptr<CharTermAttribute> term_attr = sts.AddAttribute<CharTermAttribute>();
  std::shared_ptr<OffsetAttribute> offset_attr = sts.AddAttribute<OffsetAttribute>();

  EXPECT_EQ(value, std::string(term_attr->Buffer(), 0, term_attr->Length()));
  EXPECT_EQ(0, offset_attr->StartOffset());
  EXPECT_EQ(value.size(), offset_attr->EndOffset());

  sts.End();
  sts.Close();
  EXPECT_EQ(value.size(), offset_attr->StartOffset());
  EXPECT_EQ(value.size(), offset_attr->EndOffset());
}


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

TEST(ANALYZER__TESTS, GLOBAL_REUSE_STRATEGY) {
  // global_reuse_strategy always returns same TokenStreamComponents even if different fields were passed.
  DummyAnalyzer dummy_analyzer;
  GlobalReuseStrategy& global_reuse_strategy = Singleton<GlobalReuseStrategy>::GetInstance();
  std::string field1("field1");
  std::shared_ptr<Tokenizer> tnz = std::make_shared<DummyTokenizer>();
  std::shared_ptr<TokenFilter> tf = std::make_shared<DummyTokenFilter>(tnz);
  TokenStreamComponents* ts_components = global_reuse_strategy.GetReusableComponents(dummy_analyzer, field1);
  EXPECT_EQ(nullptr, ts_components);
  global_reuse_strategy.SetReusableComponents(dummy_analyzer, field1, new TokenStreamComponents(tnz, tf));

  std::string field2("field2");
  ts_components = global_reuse_strategy.GetReusableComponents(dummy_analyzer, field2);
  EXPECT_NE(nullptr, ts_components);

  std::thread another_thread([&dummy_analyzer, &field1](){
    GlobalReuseStrategy& global_reuse_strategy = Singleton<GlobalReuseStrategy>::GetInstance();
    // `ts_components` that is outside thread should be invisible at different thread
    TokenStreamComponents* ts_components = global_reuse_strategy.GetReusableComponents(dummy_analyzer, field1);
    EXPECT_EQ(nullptr, ts_components);
  });

  another_thread.join();
}

TEST(ANALYZER__TESTS, PER_FIELD_REUSE_STRATEGY) {
  DummyAnalyzer dummy_analyzer;
  std::string field1("field1");
  std::string field2("field2");

  PerFieldReuseStrategy& per_field_reuse_strategy = Singleton<PerFieldReuseStrategy>::GetInstance();
  TokenStreamComponents* field1_ts = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field1);
  EXPECT_EQ(nullptr, field1_ts);
  TokenStreamComponents* field2_ts = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field2);
  EXPECT_EQ(nullptr, field2_ts);

  // Set TokenStreamComponents with field1
  {
    std::shared_ptr<Tokenizer> tnz = std::make_shared<DummyTokenizer>();
    std::shared_ptr<TokenFilter> tf = std::make_shared<DummyTokenFilter>(tnz);
    per_field_reuse_strategy.SetReusableComponents(dummy_analyzer, field1, new TokenStreamComponents(tnz, tf));
  }
  field1_ts = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field1);
  EXPECT_NE(nullptr, field1_ts);
  field2_ts = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field2);
  EXPECT_EQ(nullptr, field2_ts);

  // Set TokenStreamComponents with field2
  {
    std::shared_ptr<Tokenizer> tnz = std::make_shared<DummyTokenizer>();
    std::shared_ptr<TokenFilter> tf = std::make_shared<DummyTokenFilter>(tnz);
    per_field_reuse_strategy.SetReusableComponents(dummy_analyzer, field2, new TokenStreamComponents(tnz, tf));
  }
  field2_ts = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field2);
  EXPECT_NE(nullptr, field2_ts);

  std::thread another_thread([&dummy_analyzer, &field1, &field2](){
    PerFieldReuseStrategy& per_field_reuse_strategy = Singleton<PerFieldReuseStrategy>::GetInstance();
    // `ts_components` that is in outside thread should be invisible at different thread
    TokenStreamComponents* ts1 = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field1);
    TokenStreamComponents* ts2 = per_field_reuse_strategy.GetReusableComponents(dummy_analyzer, field2);
    EXPECT_EQ(nullptr, ts1);
    EXPECT_EQ(nullptr, ts2);
  });

  another_thread.join();
}

TEST(ANALYZER__TESTS, OTHER__TESTS) {
  // For a reader
  {
    DummyAnalyzer analyzer;
    std::string field1("field1");
    std::string contents1("a b cde f g ");
    StringReader str_reader;
    str_reader.SetValue(contents1);

    TokenStream& token_stream = analyzer.GetTokenStream(field1, str_reader);
    token_stream.Reset();
    EXPECT_TRUE(token_stream.IncrementToken());
    std::shared_ptr<CharTermAttribute> term_attr = token_stream.AddAttribute<CharTermAttribute>();
    EXPECT_EQ(contents1, std::string(term_attr->Buffer(), 0, term_attr->Length()));
    token_stream.End();

    token_stream.Reset();
    EXPECT_FALSE(token_stream.IncrementToken());
    token_stream.End();

    BytesRef normalized = analyzer.Normalize(field1, contents1);
    uint32_t pos_incr_gap = analyzer.GetPositionIncrementGap(field1);
    EXPECT_EQ(0, pos_incr_gap);
    uint32_t offset_gap = analyzer.GetOffsetGap(field1);
    EXPECT_EQ(1, offset_gap);
  }

  // For a string
  {
    DummyAnalyzer analyzer;
    std::string field1("field1");
    std::string contents1("a b cde fg h i");

    TokenStream& token_stream = analyzer.GetTokenStream(field1, contents1);
    token_stream.Reset();
    EXPECT_TRUE(token_stream.IncrementToken());
    std::shared_ptr<CharTermAttribute> term_attr = token_stream.AddAttribute<CharTermAttribute>();
    EXPECT_EQ(contents1, std::string(term_attr->Buffer(), 0, term_attr->Length()));
    token_stream.End();

    token_stream.Reset();
    EXPECT_FALSE(token_stream.IncrementToken());
    token_stream.End();

    BytesRef normalized = analyzer.Normalize(field1, contents1);
    uint32_t pos_incr_gap = analyzer.GetPositionIncrementGap(field1);
    EXPECT_EQ(0, pos_incr_gap);
    uint32_t offset_gap = analyzer.GetOffsetGap(field1);
    EXPECT_EQ(1, offset_gap);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
