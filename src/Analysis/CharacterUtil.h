#ifndef LUCENE_CORE_ANALYSIS_CHARACTERUTILS
#define LUCENE_CORE_ANALYSIS_CHARACTERUTILS

#include <Analysis/Reader.h>
#include <memory>

namespace lucene { namespace core { namespace analysis { namespace characterutil {

void ToLowerCase(char* buffer, const uint32_t offset, const uint32_t limit);

class CharFilter: public Reader {
  private:
    std::unique_ptr<Reader> input;

  public:
    CharFilter(Reader* input);
    virtual ~CharFilter();
    void Close();
    virtual int32_t Correct(int32_t current_off) = 0;
    int32_t CorrectOffset(int32_t current_off);
};

}}}}

#endif
