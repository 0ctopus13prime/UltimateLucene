#include <Store/DataInput.h>

using lucene::core::store::DataInput;
using lucene::core::store::IndexInput;
using lucene::core::store::BufferedIndexInput;

const uint32_t DataInput::SKIP_BUFFER_SIZE = 1024;

std::unique_ptr<BufferedIndexInput>
BufferedIndexInput::Wrap(const std::string& slice_desc,
                         IndexInput* other,
                         const uint64_t offset,
                         const uint64_t length) {
  return
  std::make_unique<BufferedIndexInput::SlicedIndexInput>(slice_desc,
                                                         other,
                                                         offset,
                                                         length);
}
