#ifndef LUCENE_CORE_ANALYSIS_STANDARD_H_
#define LUCENE_CORE_ANALYSIS_STANDARD_H_

#include <string>
#include <Analysis/Analyzer.h>
#include <Analysis/TokenStream.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/AttributeImpl.h>
#include <Analysis/Reader.h>
#include <Util/Attribute.h>

namespace lucene { namespace core { namespace analysis { namespace standard {

class StandardAnalyzer: public lucene::core::analysis::StopwordAnalyzerBase  {
  public:
    static const uint32_t DEFAULT_MAX_TOKEN_LENGTH = 255;
    static const lucene::core::analysis::characterutil::CharSet STOP_WORDS_SET;

  private:
    uint32_t max_token_length;

  protected:
    TokenStreamComponents* CreateComponents(const std::string& field_name) override;
    delete_unique_ptr<TokenStream> Normalize(const std::string& field_name, delete_unique_ptr<TokenStream> in);

  public:
    StandardAnalyzer();
    StandardAnalyzer(lucene::core::analysis::Reader&& stop_words);
    StandardAnalyzer(const lucene::core::analysis::characterutil::CharSet& stop_words);
    StandardAnalyzer(lucene::core::analysis::characterutil::CharSet&& stop_words);
    ~StandardAnalyzer();
    void SetMaxTokenLength(const uint32_t length);
    uint32_t GetMaxTokenLength();
};

class StandardFilter: public lucene::core::analysis::TokenFilter {
  public:
    StandardFilter(TokenStream* in);
    virtual ~StandardFilter();
    bool IncrementToken() override;
};

class StandardTokenizerImpl {
  public:
    static const int32_t YYEOF;

  public:
    StandardTokenizerImpl(lucene::core::analysis::Reader& in);
    ~StandardTokenizerImpl();
    void SetBufferSize(uint32_t length);
    uint32_t GetNextToken();
    void GetText(tokenattributes::CharTermAttribute& term_att);
    uint32_t YyLength();
    uint32_t YyChar();
    void YyReset(lucene::core::analysis::Reader& reader);
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
    static const std::string TOKEN_TYPES[];
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
    StandardTokenizer(lucene::core::util::AttributeFactory& factory);
    virtual ~StandardTokenizer();
    void SetMaxTokenLength(uint32_t length);
    uint32_t GetMaxTokenLength();
    bool IncrementToken() override;
    void End();
    void Close();
    void Reset();
};


}}}}

#endif
