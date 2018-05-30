#include <Analysis/CharacterUtil.h>
#include <ctype.h>

void lucene::core::analysis::characterutil::ToLowerCase(char* buffer, const uint32_t offset, const uint32_t limit) {
  for(uint32_t i = offset ; i < offset + limit ; ++i) {
    buffer[i] = tolower(buffer[i]);
  }
}
