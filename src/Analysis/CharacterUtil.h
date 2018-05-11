#ifndef LUCENE_CORE_ANALYSIS_CHARACTERUTILS
#define LUCENE_CORE_ANALYSIS_CHARACTERUTILS

#include <Analysis/Reader.h>

namespace lucene { namespace core { namespace analysis { namespace characterutil {

void ToLowerCase(char* buffer, const unsigned int offset, const unsigned int limit);

class CharFilter: public Reader {

};

}}}}

#endif
