#ifndef LUCENE_CORE_ANALYSIS_CHARACTERUTILS
#define LUCENE_CORE_ANALYSIS_CHARACTERUTILS

#include <Analysis/Reader.h>

namespace lucene { namespace core { namespace analysis { namespace characterutil {

void ToLowerCase(char* buffer, const uint32_t offset, const uint32_t limit);

class CharFilter: public Reader {
};

}}}}

#endif
