#ifndef LUCENE_CORE_ANALYSIS_STANDARD_H_
#define LUCENE_CORE_ANALYSIS_STANDARD_H_

#include <string>
#include <Analyzer/Analyzer.h>
#include <Analyzer/TokenStream.h>
#include <Analyzer/CharacterUtil.h>

namespace lucene { namespace core { namespace analysis { namespace standard {

template<typename T>
using delete_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

class StandardAnalyzer: public lucene::core::analysis::StopwordAnalyzerBase  {
  public:
    static const uint32_t DEFAULT_MAX_TOKEN_LENGTH = 255;
    static lucene::core::analysis::characterutil::CharSet STOP_WORDS_SET;

  private:
    uint32_t max_token_length;

  protected:
    TokenStreamComponents* CreateComponents(const std::string& field_name) override;
    delete_unique_ptr<TokenStream> Normalize(const std::string& field_name, delete_unique_ptr<TokenStream> in);

  public:
    StandardAnalyzer();
    StandardAnalyzer(lucene::core::analysis::characterutil::CharSet& stop_words);
    StandardAnalyzer(lucene::core::analysis::characterutil::CharSet&& stop_words);
    void SetMaxTokenLength(uint32_t length);
    uint32_t GetMaxTokenLength();
};

class StandardFilter: public lucene::core::analysis::TokenFilter {
  publiic:
    StandardFilter(TokenStream* in);
    bool IncrementToken();
};

class StandardTokenizerImpl {

};

class StandardTokenizer: public lucene::core::analysis::Tokenizer {
  private:
    StandardTokenizerImpl scanner;

  public:
    static const uint32_t ALPHANUM = 0;
    static const uint32_t NUM = 1;
    static const uint32_t SOUTHEAST_ASIAN = 2;
    static const uint32_t IDEOGRAPHIC = 3;
    static const uint32_t HIRAGANA = 4;
    static const uint32_t KATAKANA = 5;
    static const uint32_t HANGUL = 6;
    /*
    "<ALPHANUM>",
    "<NUM>",
    "<SOUTHEAST_ASIAN>",
    "<IDEOGRAPHIC>",
    "<HIRAGANA>",
    "<KATAKANA>",
    "<HANGUL>"
    */
    static const std::string TOKEN_TYPES[];
    static const uint32_t MAX_TOKEN_LENGTH_LIMIT = 1024 * 1024;
  private:
    int32_t skipped_positions;
    int32_t max_token_length/* = StandardAnalyzer.DEFAULT_MAX_TOKEN_LENGTH*/;
};


}}}}

#endif
