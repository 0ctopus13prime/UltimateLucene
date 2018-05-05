#include <Util/ArrayUtil.h>

using namespace lucene::core::util;

// TODO. I'm not sure this is a valid implementation. How can I limit maximum array size?
unsigned int arrayutil::Oversize(const unsigned int min_target_size, const unsigned int bytes_per_element) {
  unsigned int extra = (min_target_size >> 3);
  extra = (extra < 3 ? 3 : extra);

  unsigned int new_size = (min_target_size + extra);
  switch(bytes_per_element) {
    case 4:
      return (new_size + 1) & 0xfffffffe;
    case 2:
      return (new_size + 1) & 0xfffffffc;
    case 1:
      return (new_size + 1) & 0xfffffff8;
    case 8:
    default:
      return new_size;
  }
}
