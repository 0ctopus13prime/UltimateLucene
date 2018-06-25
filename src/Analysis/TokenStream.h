#ifndef LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_
#define LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <Util/Attribute.h>
#include <Analysis/Attribute.h>
#include <Analysis/CharacterUtil.h>
#include <Analysis/Reader.h>

namespace lucene { namespace core { namespace analysis {

template<typename T>
using delete_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

class TokenStream: public lucene::core::util::AttributeSource {
  protected:
    TokenStream();
    TokenStream(const lucene::core::util::AttributeSource& input);
    TokenStream(lucene::core::util::AttributeFactory& factory);

  public:
    static AttributeFactory* DEFAULT_TOKEN_ATTRIBUTE_FACTORY;

  public:
    virtual ~TokenStream();
    virtual bool IncrementToken() = 0;
    virtual void End();
    virtual void Reset() = 0;
    virtual void Close();
};

class TokenFilter: public TokenStream {
  protected:
    std::unique_ptr<TokenStream> input;

  protected:
    TokenFilter(std::unique_ptr<TokenStream>&& input);
    TokenFilter(TokenStream* input);

  public:
    virtual ~TokenFilter();
    void End() override;
    void Reset() override;
    void Close() override;

    template <typename ATTR>
    std::shared_ptr<ATTR> AddAttribute() {
      std::shared_ptr<ATTR> attr = input->AddAttribute<ATTR>();
      input->ShallowCopyTo(*this); // For sharing attributes

      return attr;
    }
};


class Tokenizer: public TokenStream {
  private:
    static lucene::core::analysis::IllegalStateReader ILLEGAL_STATE_READER;

  protected:
    Reader* input; // Read onlly
    Reader* input_pending; // Read only

  protected:
    Tokenizer();
    Tokenizer(lucene::core::util::AttributeFactory& factory);
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
      LowerCaseFilter(std::shared_ptr<TokenStream> in);
      LowerCaseFilter(TokenStream* in);
      virtual ~LowerCaseFilter();
      bool IncrementToken() override;
};

class CachingTokenFilter: public TokenFilter {
  private:
    std::vector<std::unique_ptr<lucene::core::util::AttributeSource::State>> cache;
    std::vector<std::unique_ptr<lucene::core::util::AttributeSource::State>>::iterator iterator;
    std::unique_ptr<lucene::core::util::AttributeSource::State> final_state;
    bool first_time;

  private:
    void FillCache();
    bool IsCached();

  public:
    CachingTokenFilter(std::shared_ptr<TokenStream> in);
    CachingTokenFilter(TokenStream* in);
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
    FilteringTokenFilter(std::shared_ptr<TokenStream> in);
    FilteringTokenFilter(TokenStream* in);
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
    StopFilter(std::shared_ptr<TokenStream> in, characterutil::CharSet& stop_words);
    StopFilter(std::shared_ptr<TokenStream> in, characterutil::CharSet&& stop_words);
    virtual ~StopFilter();

    static characterutil::CharSet MakeStopSet(std::vector<std::string>& stop_words, bool ignore_case = false);
};

}}} // End of namespace

#endif
