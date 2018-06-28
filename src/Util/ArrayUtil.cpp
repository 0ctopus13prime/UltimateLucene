#include <Util/ArrayUtil.h>

using namespace lucene::core::util;

// TODO. I'm not sure this is a valid implementation. How can I limit maximum array size like Java does?
uint32_t arrayutil::Oversize(const uint32_t min_target_size, const uint32_t bytes_per_element) {
  uint32_t extra = (min_target_size >> 3); // extra <- min_target_size / 8
  if(extra < 3) {
    extra = 3;
  }

  uint32_t new_size = (min_target_size + extra);
  return new_size;
}
