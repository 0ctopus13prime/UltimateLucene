#include <Analysis/CharacterUtil.h>
#include <ctype.h>

using namespace lucene::core::analysis::characterutil;

void lucene::core::analysis::characterutil::ToLowerCase(char* buffer, const uint32_t offset, const uint32_t limit) {
  for(uint32_t i = offset ; i < offset + limit ; ++i) {
    buffer[i] = tolower(buffer[i]);
  }
}

/**
 * CharFilter
 */
CharFilter::CharFilter(Reader* input)
  : input(input) {
}

CharFilter::~CharFilter() {
  Close();
}

void CharFilter::Close() {
  input->Close();
}

int32_t CharFilter::CorrectOffset(int32_t current_off) {
  const int32_t corrected = Correct(current_off);

  if(CharFilter* char_filter = dynamic_cast<CharFilter*>(input.get())) {
    return char_filter->CorrectOffset(corrected);
  } else {
    return corrected;
  }
}
