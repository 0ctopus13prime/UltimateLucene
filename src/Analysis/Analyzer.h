#ifndef LUCENE_CORE_ANALYSIS_ANALYZER_H_
#define LUCENE_CORE_ANALYSIS_ANALYZER_H_

#include <vector>
#include <string>
#include <sstream>
#include <Util/Attribute.h>
#include <Util/Etc.h>
#include <Analysis/AttributeImpl.h>
#include <Analysis/TokenStream.h>
#include <Analysis/Reader.h>
#include <Analysis/CharacterUtil.h>

namespace lucene { namespace core { namespace analysis {

class Analyzer;

class TokenStreamComponents {
  protected:
    std::unique_ptr<Tokenizer> source;
    std::unique_ptr<TokenStream> sink;
    StringReader reusable_string_reader;

  public:
    /**
     * TokenStreamComponents constructor.
     * Both source, result instance owned by this instance.
     */
    TokenStreamComponents(Tokenizer* source, TokenStream* result);
    TokenStreamComponents(Tokenizer* source);
    TokenStream& GetTokenStream();
    Tokenizer& GetTokenizer();
    StringReader& GetReusableStringReader();
    virtual void SetReader(Reader& reader);
};

class ReuseStrategy {
  protected:
    template<typename T>
    T& GetStoredValue(Analyzer& analyzer) {
      // TODO. Implement it.
    }

    template<typename T>
    void SetStoredValue(Analyzer& analyzer, T& stored_value) {
      // TODO. Implement it.
    }

  public:
    ReuseStrategy();
    virtual ~ReuseStrategy();
    virtual TokenStreamComponents* GetReusableComponents(Analyzer& analyzer, const std::string& field_name) = 0;
    virtual void SetReusableComponents(Analyzer& analyzer, const std::string& field_name, TokenStreamComponents* components) = 0;
};

class PreDefinedReuseStrategy: public ReuseStrategy {
  public:
    PreDefinedReuseStrategy();
    virtual ~PreDefinedReuseStrategy();
    TokenStreamComponents* GetReusableComponents(Analyzer& analyzer, const std::string& field_name) override;
    void SetReusableComponents(Analyzer& analyzer,
                              const std::string& field_name,
                              TokenStreamComponents* components) override;
};

class StringTokenStream: public TokenStream {
  private:
    std::string& value;
    size_t length;
    bool used;
    std::shared_ptr<tokenattributes::CharTermAttribute> term_attribute;
    std::shared_ptr<tokenattributes::OffsetAttribute> offset_attribute;

  public:
    StringTokenStream(lucene::core::util::AttributeFactory& input, std::string& value, size_t length);
    void Reset() override;
    bool IncrementToken() override;
    void End() override;
};

static PreDefinedReuseStrategy PER_FIELD_REUSE_STRATEGY;

class Analyzer {
  private:
    ReuseStrategy& reuse_strategy;
    lucene::core::util::etc::Version version;

  protected:
    virtual TokenStreamComponents* CreateComponents(const std::string& field_name) = 0;
    virtual Reader& InitReader(const std::string& field_name, Reader& reader);
    delete_unique_ptr<TokenStream>&& Normalize(const std::string& field_name, delete_unique_ptr<TokenStream>& in);
    delete_unique_ptr<TokenStream>&& Normalize(const std::string& field_name, delete_unique_ptr<TokenStream>&& in);
    delete_unique_ptr<Reader>&& InitReaderForNormalization(const std::string& field_name, delete_unique_ptr<Reader>& reader);
    delete_unique_ptr<Reader>&& InitReaderForNormalization(const std::string& field_name, delete_unique_ptr<Reader>&& reader);
    AttributeFactory& GetAttributeFactory(const std::string& field_name);

  public:
    Analyzer();
    Analyzer(ReuseStrategy& reuse_strategy);
    ~Analyzer();

    // Create a new TokenStream with given reader.
    // Parameter reader will be destructed once it is given to this instance.
    // Analyzer have a full ownership of reader.
    TokenStream& GetTokenStream(const std::string& field_name, Reader& reader);
    TokenStream& GetTokenStream(const std::string& field_name, const std::string& text);
    BytesRef Normalize(const std::string& field_name, const std::string& text);
    uint32_t GetPositionIncrementGap(const std::string& field_name);
    uint32_t GetOffsetGap(const std::string& field_name);
    ReuseStrategy& GetReuseStrategy();
    void SetVersion(lucene::core::util::etc::Version& v);
};

class StopwordAnalyzerBase: public Analyzer {
  protected:
    characterutil::CharSet stop_words;

  protected:
    StopwordAnalyzerBase(const characterutil::CharSet& stop_words);
    StopwordAnalyzerBase(characterutil::CharSet&& stop_words);
    StopwordAnalyzerBase();
    virtual ~StopwordAnalyzerBase();

    static characterutil::CharSet LoadStopWordSet(const std::string& path);
    static characterutil::CharSet LoadStopWordSet(const Reader& reader);

  public:
    characterutil::CharSet& GetStopWords();
};

class WordlistLoader {
  private:
    static const uint32_t INITIAL_CAPACITY = 16;

  private:
    WordlistLoader();

  public:
    ~WordlistLoader();

    static void GetWordSet(Reader& reader, characterutil::CharSet& result);
    static characterutil::CharSet GetWordSet(Reader& reader);
    static characterutil::CharSet GetWordSet(Reader& reader, std::string& comment);
    static void GetWordSet(Reader& reader, std::string& comment, characterutil::CharSet& result);
    static void GetSnowballWordSet(Reader& reader, characterutil::CharSet& result);
    static characterutil::CharSet GetSnowballWordSet(Reader& reader);
    static characterutil::CharMap<std::string> GetStemDict(Reader& reader, characterutil::CharMap<std::string>& result);
    static std::vector<std::string> GetLines(Reader& reader);
};

}}} // End of namespace
#endif
