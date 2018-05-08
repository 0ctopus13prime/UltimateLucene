#ifndef LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_
#define LUCENE_CORE_ANALYSIS_TOKEN_STREAM_H_

#include <Util/Attribute.h>

namespace lucene { namespace core { namespace analysis {

class TokenStream: public AttributeSource {
  public:
    TokenStream();
    ~TokenStream();
    TokenStream(const AttributeSource& input);
    TokenStream(AttributeFactory* factory);
    virtual bool IncrementToken() = 0;
    virtual void End();
    virtual void Reset() = 0;
};

}}} // End of namespace

#endif
