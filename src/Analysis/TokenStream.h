#ifndef LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_
#define LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_

#include <functional>
#include <memory>
#include <Util/Attribute.h>
#include <Analysis/Attribute.h>
#include <Analysis/CharacterUtil.h>

namespace lucene { namespace core { namespace analysis {

template<typename T>
using delete_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

class TokenStream: public lucene::core::util::AttributeSource {
  protected:
    TokenStream();
    TokenStream(const lucene::core::util::AttributeSource& input);
    TokenStream(lucene::core::util::AttributeFactory* factory);

  public:
    static AttributeFactory* DEFAULT_TOKEN_ATTRIBUTE_FACTORY;

  public:
    virtual ~TokenStream();
    virtual bool IncrementToken() = 0;
    virtual void End();
    virtual void Reset() = 0;
};

class TokenFilter: public TokenStream {
  protected:
    std::shared_ptr<TokenStream> input;

  protected:
    TokenFilter(TokenStream* input);

  public:
    virtual ~TokenFilter();
    void End() override;
    void Reset() override;
};

class Tokenizer: public TokenStream {
  protected:
    delete_unique_ptr<Reader> input;
    delete_unique_ptr<Reader> input_pending;

  protected:
    Tokenizer();
    Tokenizer(lucene::core::util::AttributeFactory* factory);
    unsigned int CorrectOffset(const unsigned int current_off);

  public:
    virtual ~Tokenizer();
    void SetReader(delete_unique_ptr<Reader>& input);
    void Reset() override;
    void Close();

    // Only used for testing
    void SetReaderTestPoint();
};

class LowerCaseFilter: public TokenFilter {
  private:
    std::shared_ptr<tokenattributes::CharTermAttribute> term_att;

  public:
      LowerCaseFilter(TokenStream* in);
      virtual ~LowerCaseFilter();
      bool IncrementToken() override;
};

}}} // End of namespace

#endif
