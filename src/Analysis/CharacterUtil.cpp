#include <Analysis/CharacterUtil.h>
#include <ctype.h>

void lucene::core::analysis::characterutil::ToLowerCase(char* buffer, const unsigned int offset, const unsigned int limit) {
  for(int i = offset ; i < offset + limit ; ++i) {
    buffer[i] = tolower(buffer[i]);
  }
}
