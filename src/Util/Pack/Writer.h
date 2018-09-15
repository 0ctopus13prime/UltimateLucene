/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SRC_UTIL_PACK_WRITER_H_
#define SRC_UTIL_PACK_WRITER_H_

#include <Store/DataOutput.h>
#include <Util/Exception.h>
#include <Util/Pack/PackedInts.h>
#include <Util/Pack/BulkOperation.h>
#include <cassert>
#include <cstring>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class PackedWriter: public PackedInts::Writer {
 private:
  PackedInts::Format format;
  BulkOperation* encoder;
  std::unique_ptr<char[]> next_blocks;
  std::unique_ptr<int64_t[]> next_values;
  uint32_t iterations;
  uint32_t off;
  uint32_t written;
  uint32_t next_blocks_size;
  uint32_t next_values_size;
  bool finished;

 private:
  void Flush() {
    encoder->Encode(next_values.get(), 0, next_blocks.get(), 0, iterations);
    const uint32_t block_count =
      static_cast<const uint32_t>(format.ByteCount(PackedInts::VERSION_CURRENT,
                                                   off,
                                                   bits_per_value));
      out->WriteBytes(next_blocks.get(), block_count); 
      std::memset(next_values.get(), 0, sizeof(int64_t) * next_values_size);
      off = 0;
  }

 public:
  PackedWriter(PackedInts::Format format,
               lucene::core::store::DataOutput* out,
               const uint32_t value_count,
               const uint32_t bits_per_value,
               const uint32_t mem)
    : PackedInts::Writer(out, value_count, bits_per_value),
      format(format),
      encoder(BulkOperation::Of(format, bits_per_value)),
      next_blocks(),
      next_values(),
      iterations(encoder->ComputeIterations(value_count, mem)),
      off(0),
      written(0),
      finished(false) {
    next_blocks_size = (iterations * encoder->ByteBlockCount());
    next_blocks =
     std::make_unique<char[]>(next_blocks_size);

    next_values_size = (iterations * encoder->ByteValueCount());
    next_values =
      std::make_unique<int64_t[]>(next_values_size);
  }

  PackedInts::Format GetFormat() {
    return format;
  }

  void Add(int64_t v) {
    assert(PackedInts::UnsignedBitsRequired(v) <= bits_per_value); 
    assert(!finished);

    if (written >= value_count) {
      throw EOFException("Writing past end of stream");
    }

    next_values[off++] = v;
    if (off == next_values_size) {
      Flush();
    }

    ++written;
  }

  void Finish() {
    assert(!finished);

    while (written < value_count) {
      Add(0);
    }

    Flush();
    finished = true;
  }

  uint32_t Ord() {
    return written - 1;
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_WRITER_H_
