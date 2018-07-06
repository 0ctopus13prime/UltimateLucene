#ifndef LUCENE_CORE_ANALYSIS_ANALYZER_H_
#define LUCENE_CORE_ANALYSIS_ANALYZER_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <Util/Attribute.h>
#include <Util/Etc.h>
#include <Util/Concurrency.h>
#include <Analysis/AttributeImpl.h>
#include <Analysis/TokenStream.h>
#include <Analysis/Reader.h>
#include <Analysis/CharacterUtil.h>

namespace lucene { namespace core { namespace analysis {

class TokenStreamComponents {
  protected:
    // This is shared between TokenFilter and TokenStreamComponents
    std::shared_ptr<Tokenizer> source;
    // It's shared_ptr in case souce and sink are equivalent
    std::shared_ptr<TokenStream> sink;
    StringReader reusable_string_reader;

  public:
    /**
     * TokenStreamComponents constructor.
     * Both source, result instance owned by this instance.
     */
    TokenStreamComponents(std::shared_ptr<Tokenizer> source, std::shared_ptr<TokenStream> result);
    TokenStreamComponents(std::shared_ptr<Tokenizer> source);
    ~TokenStreamComponents();
    TokenStream& GetTokenStream();
    Tokenizer& GetTokenizer();
    StringReader& GetReusableStringReader();
    virtual void SetReader(Reader& reader);
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

class Analyzer {
  public:
    class ReuseStrategy {
      public:
        ReuseStrategy();
        virtual ~ReuseStrategy();
        virtual TokenStreamComponents* GetReusableComponents(Analyzer& analyzer, const std::string& field_name) = 0;
        virtual void SetReusableComponents(Analyzer& analyzer, const std::string& field_name, TokenStreamComponents* components) = 0;
    };

  private:
    bool closed;
    std::unique_ptr<ReuseStrategy> reuse_strategy;
    lucene::core::util::etc::Version version;

  protected:
    virtual TokenStreamComponents* CreateComponents(const std::string& field_name) = 0;
    virtual Reader& InitReader(const std::string& field_name, Reader& reader);
    TokenStream& Normalize(const std::string& field_name, TokenStream& in);
    Reader& InitReaderForNormalization(const std::string& field_name, Reader& reader);
    AttributeFactory& GetAttributeFactory(const std::string& field_name);

  public:
    Analyzer();
    Analyzer(ReuseStrategy* reuse_strategy);
    virtual ~Analyzer();

    // Create a new TokenStream with given reader.
    // Parameter reader will be destructed once it is given to this instance.
    // Analyzer have a full ownership of reader.
    TokenStream& GetTokenStream(const std::string& field_name, Reader& reader);
    TokenStream& GetTokenStream(const std::string& field_name, const std::string& text);
    BytesRef Normalize(const std::string& field_name, const std::string& text);
    uint32_t GetPositionIncrementGap(const std::string& field_name) {
      return 0;
    }
    uint32_t GetOffsetGap(const std::string& field_name) {
      return 1;
    }
    ReuseStrategy& GetReuseStrategy();
    void SetVersion(lucene::core::util::etc::Version& v);
    //const lucene::core::etc::Version& GetVersion()

    void Close() noexcept {
      closed = true;
    }

    bool IsClosed() const noexcept {
      return closed;
    }
};

class GlobalReuseStrategy: public Analyzer::ReuseStrategy {
  private:
    lucene::core::util::CloseableThreadLocal<GlobalReuseStrategy, std::unique_ptr<TokenStreamComponents>> stored_value;

  public:
    GlobalReuseStrategy();
    virtual ~GlobalReuseStrategy();
    TokenStreamComponents* GetReusableComponents(Analyzer& analyzer, const std::string& field_name) override;
    void SetReusableComponents(Analyzer& analyzer,
                              const std::string& field_name,
                              TokenStreamComponents* components) override;
};

class PerFieldReuseStrategy: public Analyzer::ReuseStrategy {
  private:
    lucene::core::util::CloseableThreadLocal<PerFieldReuseStrategy, std::unordered_map<std::string, std::unique_ptr<TokenStreamComponents>>> stored_value;

  public:
    PerFieldReuseStrategy();
    virtual ~PerFieldReuseStrategy();
    TokenStreamComponents* GetReusableComponents(Analyzer& analyzer, const std::string& field_name) override;
    void SetReusableComponents(Analyzer& analyzer,
                              const std::string& field_name,
                              TokenStreamComponents* components) override;
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
