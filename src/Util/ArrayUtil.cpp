#include <Util/ArrayUtil.h>

using namespace lucene::core::util;

// TODO. I'm not sure this is a valid implementation. How can I limit maximum array size?
uint32_t arrayutil::Oversize(const uint32_t min_target_size, const uint32_t bytes_per_element) {
  uint32_t extra = (min_target_size >> 3); // extra <- min_target_size / 8
  if(extra < 3) {
    extra = 3;
  }

  uint32_t new_size = (min_target_size + extra);
  switch(bytes_per_element) {
    case 4:
      return (new_size + 1) & 0xfffffffe; // 11111111111111111111111111111110
    case 2:
      return (new_size + 1) & 0xfffffffc; // 11111111111111111111111111111100
    case 1:
      return (new_size + 1) & 0xfffffff8; // 11111111111111111111111111111000
    case 8:
    default:
      return new_size;
  }
}
