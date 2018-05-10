#ifndef LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_
#define LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_

#include <Util/Attribute.h>

namespace lucene { namespace core { namespace analysis {

class TokenStream: public lucene::core::util::AttributeSource {
  protected:
    TokenStream();
    TokenStream(const lucene::core::util::AttributeSource& input);
    TokenStream(lucene::core::util::AttributeFactory* factory);

  public:
    virtual ~TokenStream();
    virtual bool IncrementToken() = 0;
    virtual void End();
    virtual void Reset() = 0;
};

class TokenFilter: public TokenStream {
  protected:
    TokenStream& input;

  protected:
    TokenFilter(TokenStream& input);

  public:
    virtual ~TokenFilter();
    void End() override;
    void Reset() override;
};

}}} // End of namespace

#endif
