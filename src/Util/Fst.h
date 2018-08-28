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

#ifndef SRC_UTIL_FST_H_
#define SRC_UTIL_FST_H_

#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <cstring>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>

namespace lucene {
namespace core {
namespace util {

class BytesStore : public lucene::core::store::DataOutput {
 private:
  std::vector<std::pair<std::unique_ptr<char[]>, uint32_t>> blocks; 
  uint32_t block_size;
  uint32_t block_bits;
  uint32_t block_mask;
  char* current;
  uint32_t next_write;

 public:
  explicit BytesStore(const uint32_t new_block_bits)
    : blocks(),
      block_size(1 << new_block_bits),
      block_bits(new_block_bits),
      block_mask(block_size - 1),
      current(nullptr),
      next_write(block_size) {
  }

  BytesStore(std::unique_ptr<lucene::core::store::DataInput>&& in,
             const uint64_t num_bytes,
             const uint32_t max_block_size) {
    uint32_t tmp_block_size = 2;
    uint32_t tmp_block_bits = 1;

    while (tmp_block_size < num_bytes && tmp_block_size < max_block_size) {
      tmp_block_size *= 2;
      ++tmp_block_bits;
    }

    block_bits = tmp_block_bits;
    block_size = tmp_block_size;
    block_mask = tmp_block_size - 1;
    uint64_t left = num_bytes;
    while (left > 0) {
      const uint32_t chunk = std::min(block_size, static_cast<uint32_t>(left));
      std::unique_ptr<char[]> block(new char[chunk]);
      in->ReadBytes(block.get(), 0, chunk);
      blocks.push_back(std::pair<std::unique_ptr<char[]>, uint32_t>(
        std::move(block), chunk));
      left -= chunk;
    }

    next_write = blocks[blocks.size() - 1].second;
    current = nullptr;
  }

  void WriteByte(const uint32_t dest, const char b) {
    const uint32_t block_index = (dest >> block_bits);
    std::pair<std::unique_ptr<char[]>, uint32_t>& block_pair =
      blocks[block_index];
    block_pair.first[dest & block_mask] = b;
  }

  void WriteByte(const char b) {
    if (next_write == block_size) {
      std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
        new char[block_size], block_size);
      blocks.push_back(std::move(block_pair));
      next_write = 0;
      current = block_pair.first.get();
    }

    current[next_write++] = b;
  }

  void WriteBytes(const char bytes[], const uint32_t offset, const uint32_t len) {
    uint32_t offset_until = offset;
    uint32_t len_until = len;

    while (len > 0) {
      const uint32_t chunk = (block_size - next_write);
      if (len <= chunk) {
        std::memcpy(current + next_write, bytes + offset_until, len);
        next_write += len;
        break;
      } else {
        if (chunk > 0) {
          std::memcpy(current + next_write, bytes + offset_until, chunk);
          offset_until += chunk;
          len_until -= chunk;
        }

        std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
          new char[block_size], block_size);
        blocks.push_back(std::move(block_pair));
        next_write = 0;
        current = block_pair.first.get();
      }
    }
  }
};

}  // util
}  // core
}  // lucene


#endif  // SRC_UTIL_FST_H_
