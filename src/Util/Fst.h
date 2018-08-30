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
#include <Util/Etc.h>
#include <Util/Exception.h>
#include <Util/Packed.h>
#include <cstring>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace lucene {
namespace core {
namespace util {

class FSTBytesReader: public lucene::core::store::DataInput {

};

class ForwardFSTBytesReader: public FSTBytesReader {

};

class ReverseFSTBytesReader: public FSTBytesReader {

};

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

  void WriteBytes(const char bytes[],
                  const uint32_t offset,
                  const uint32_t len) {
    uint32_t offset_until = offset;
    uint32_t len_until = len;

    while (len_until > 0) {
      const uint32_t chunk = (block_size - next_write);
      if (len_until <= chunk) {
        std::memcpy(current + next_write, bytes + offset_until, len_until);
        next_write += len_until;
        break;
      } else {
        if (chunk > 0) {
          std::memcpy(current + next_write, bytes + offset_until, chunk);
          offset_until += chunk;
          len_until -= chunk;
        }

        std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
          new char[block_size], block_size);
        current = block_pair.first.get();
        blocks.push_back(std::move(block_pair));
        next_write = 0;
      }
    }
  }

  uint32_t GetBlockBits() const noexcept {
    return block_bits;
  }

  void WriteBytes(const uint64_t dest,
                  const char bytes[],
                  const uint32_t offset,
                  const uint32_t len) {
    const uint64_t end = dest + len;
    uint32_t block_index = static_cast<uint32_t>(end >> block_bits);
    uint32_t down_to = static_cast<uint32_t>(end & block_mask);

    if (down_to == 0) {
      block_index--;
      down_to = block_size;
    }

    std::pair<std::unique_ptr<char[]>, uint32_t>& block_pair =
      blocks[block_index];
    char* block = block_pair.first.get();

    uint32_t len_tmp = len;
    while (len_tmp > 0) {
      if (len_tmp <= down_to) {
        std::memcpy(block + (down_to - len), bytes + offset, len);
        --block_index;
        std::pair<std::unique_ptr<char[]>, uint32_t>& block_pair_tmp =
          blocks[block_index];
        block = block_pair_tmp.first.get();
        down_to = block_size;
      } else {
        len_tmp -= down_to;
        std::memcpy(block, bytes + offset + len_tmp, down_to);
        --block_index;
        std::pair<std::unique_ptr<char[]>, uint32_t>& block_pair_tmp =
          blocks[block_index];
        block = block_pair_tmp.first.get();
        down_to = block_size;
      }
    }
  }

  void CopyBytes(const uint64_t src, const uint64_t dest, const uint32_t len) {
    const uint64_t end = src + len;
    uint32_t block_index = static_cast<uint32_t>(end >> block_bits);
    uint32_t down_to = static_cast<uint32_t>(end & block_mask);
    if (down_to == 0) {
      --block_index;
      down_to = block_size;
    }

    std::pair<std::unique_ptr<char[]>, uint32_t>& block_pair_tmp =
      blocks[block_index];
    char* block = block_pair_tmp.first.get();

    uint32_t len_tmp = len;
    while (len_tmp > 0) {
      if (len_tmp <= down_to) {
        WriteBytes(dest, block, down_to - len_tmp, len_tmp);
        break;
      } else {
        len_tmp -= down_to;
        WriteBytes(block, dest + len_tmp, down_to);
        block_index--;
        std::pair<std::unique_ptr<char[]>, uint32_t>& block_pair_tmp =
          blocks[block_index];
        block = block_pair_tmp.first.get();
        down_to = block_size;
      }
    }
  }

  void WriteInt(const uint64_t pos, const uint32_t value) {
    uint32_t block_index = static_cast<uint32_t>(pos >> block_bits);
    uint32_t upto = static_cast<uint32_t>(pos & block_mask);
    char* block = blocks[block_index].first.get();
    uint32_t shift = 24;
    for (int i = 0 ; i < 4 ; ++i) {
      block[upto++] = static_cast<char>(value >> shift);
      shift -= 8;
      if (upto == block_size) {
        upto = 0;
        ++block_index;
        block = blocks[block_index].first.get();
      }
    }
  }

  void Reverse(const uint64_t src_pos, const uint64_t dest_pos) {
    uint32_t src_block_index = static_cast<uint32_t>(src_pos >> block_bits);
    uint32_t src = static_cast<uint32_t>(src_pos & block_mask);
    char* src_block = blocks[src_block_index].first.get();

    uint32_t dest_block_index = static_cast<uint32_t>(dest_pos >> block_bits);
    uint32_t dest = static_cast<uint32_t>(dest_pos & block_bits);
    char* dest_block = blocks[dest_block_index].first.get();

    uint32_t limit = static_cast<uint32_t>(dest_pos - src_pos + 1) >> 1;
    for (uint32_t i = 0 ; i < limit ; ++i) {
      std::swap(src_block[src], dest_block[dest]);
      src++;
      if (src == block_size) {
        src_block_index++;
        src_block = blocks[src_block_index].first.get();
        src = 0;
      }

      dest--;
      if (dest == -1) {
        dest_block_index--;
        dest_block = blocks[dest_block_index].first.get();
        dest = block_size - 1;
      }
    }
  }

  void SkipBytes(const uint32_t len) {
    uint32_t len_tmp = len;

    while (len_tmp > 0) {
      const uint32_t chunk = block_size - next_write;
      if (len_tmp <= chunk) {
        next_write += len_tmp;
        break;
      } else {
        len_tmp -= chunk;
        std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
          new char[block_size], block_size);
        current = block_pair.first.get();
        blocks.push_back(std::move(block_pair));
        next_write = 0;
      }
    }
  }

  uint64_t GetPosition() {
    return static_cast<uint64_t>(blocks.size() - 1) * block_size + next_write;
  }

  void Truncate(const uint64_t new_len) {
    uint32_t block_index = static_cast<uint32_t>(new_len >> block_bits);
    next_write = static_cast<uint32_t>(new_len & block_mask);
    if (next_write == 0) {
      --block_index;
      next_write = block_size;
    }

    blocks.erase(blocks.begin() + block_index + 1);
    if (new_len == 0) {
      current = nullptr;
    } else {
      current = blocks[block_index].first.get();
    }
  }

  void Finish() {
    if (current != nullptr) {
      std::unique_ptr<char[]> last_buffer(new char[next_write]);
      std::memcpy(last_buffer.get(), current, next_write);
      std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
        new char[block_size], block_size);
      current = block_pair.first.get();
      blocks[blocks.size() - 1] = std::move(block_pair);
      current = nullptr;
    }
  }

  void WriteTo(lucene::core::store::DataOutput* out) {
    for (auto& block_pair : blocks) {
      out->WriteBytes(block_pair.first.get(), 0, block_pair.second);
    }
  }
};

template<typename T>
class Outputs {
 public:
  Outputs() = default;

  virtual ~Outputs() = default;

  virtual T Common(T& output1, T& output2) = 0;

  virtual T Substract(T& output, T& inc) = 0;

  virtual T Add(T& prefix, T& output) = 0;

  virtual T Write(T& output, lucene::core::store::DataOutput* out) = 0;

  virtual T WriteFinalOutput(T& output,
                             lucene::core::store::DataOutput* out) = 0;

  virtual T Read(lucene::core::store::DataInput* in) = 0;

  virtual void SkipOutput(lucene::core::store::DataInput* in) = 0;

  virtual T ReadFinalOutput(lucene::core::store::DataInput* in) = 0;

  virtual void SkipFinalOutput(lucene::core::store::DataInput* in) = 0;

  virtual T GetNoOutput() = 0;

  virtual std::string OutputToString(T& output) = 0;

  virtual T Merge(T& first, T& second) {
    throw lucene::core::util::UnsupportedOperationException();
  }
};

template<typename T>
class FST {
 public:
  static const uint32_t BIT_FINAL_ARC = (1 << 0);
  static const uint32_t BIT_LAST_ARC = (1 << 1);
  static const uint32_t BIT_TARGET_NEXT = (1 << 2);
  static const uint32_t BIT_STOP_NODE = (1 << 3);
  static const uint32_t BIT_ARC_HAS_OUTPUT = (1 << 4);
  static const uint32_t BIT_ARC_HAS_FINAL_OUTPUT = (1 << 5);
  static const char ARCS_AS_FIXED_ARRAY = BIT_ARC_HAS_FINAL_OUTPUT;
  static const uint32_t FIXED_ARRAY_SHALLOW_DISTANCE = 3;
  static const uint32_t FIXED_ARRAY_NUM_ARCS_SHALLOW = 5;
  static const uint32_t FIXED_ARRAY_NUM_ARCS_DEEP = 10;
  static const uint32_t DEFAULT_MAX_BLOCK_BITS = 30;

 private:
  static const std::string FILE_FORMAT_NAME;  // "FST"
  static const uint32_t VERSION_START = 0;
  static const uint32_t VERSION_INT_NUM_BYTES_PER_ARC = 1;
  static const uint32_t VERSION_SHORT_BYTE2_LABELS = 2;
  static const uint32_t VERSION_PACKED = 3;
  static const uint32_t VERSION_VINT_TARGET = 4;
  static const uint32_t VERSION_NO_NODE_ARC_COUNTS =5;
  static const uint32_t VERSION_PACKED_REMOVED = 6;
  static const uint32_t VERSION_CURRENT = VERSION_PACKED_REMOVED;
  static const int64_t FINAL_END_NODE = -1;
  static const int64_t NON_FINAL_END_NODE = 0;
  static const int32_t END_LABEL = -1;

 public:
  class Arc;
  enum class INPUT_TYPE { BYTE1, BYTE2, BYTE4 };

 public:
  INPUT_TYPE input_type;
  T empty_output;
  BytesStore bytes;
  char* byte_array;
  int64_t start_node;
  Outputs<T> outputs;
  Arc* cached_root_arcs;
  uint32_t version;
};

template<typename T>
class FST<T>::Arc {
 public:
  uint32_t label;
  T output;
  T next_final_output;
  uint64_t target;
  uint64_t next_arc;
  uint64_t pos_arcs_start;
  uint32_t bytes_per_arc;
  uint32_t arc_idx;
  uint32_t num_arcs;
  char flags;
};

template<typename T>
class Builder;

template<typename T>
class NodeHash {
 private:
  lucene::core::util::PagedGrowableWriter table;
  uint64_t count;
  uint64_t mask;
  FST<T> fst;
  typename FST<T>::Arc scratch_arc;
  std::unique_ptr<FSTBytesReader> in;
};

template<typename T>
class Builder {
 public:
  class Node;
  class UnCompiledNode;
  class CompiledNode;
  class Arc;

 private:
  NodeHash<T> dedup_hash;
  FST<T> fst;
  T NO_OUTPUT;
  uint32_t min_suffix_count1;
  uint32_t min_suffix_count2;
  bool do_share_non_singleton_nodes;
  uint32_t share_max_tail_length;
  lucene::core::util::IntsRefBuilder last_input;
  UnCompiledNode* frontier;
  uint64_t last_frozen_node;
  uint32_t reused_byte_per_arc[4];
  uint64_t arc_count;
  uint64_t node_count;
  bool allow_array_arcs;
  BytesStore bytes;
};

template<typename T>
class Builder<T>::Node {
 public:
  Node() = default;

  virtual ~Node() = default;

  virtual bool IsCompiled() const noexcept = 0;
};

template<typename T>
class Builder<T>::Arc {
 public:
  uint32_t label;
  Node* target;
  bool is_final;
  T output;
  T next_final_output;
};

template<typename T>
class Builder<T>::UnCompiledNode : public Builder<T>::Node {
 private:
  Builder<T>* owner;

 public:
  uint32_t num_arcs;
  Arc* arcs; 
  T output;
  bool is_final;
  uint64_t input_count;
  uint32_t depth;
};

template<typename T>
class Builder<T>::CompiledNode: public Builder<T>::Node {
 private:
  uint64_t node;

  bool IsCompiled() const noexcept {
    return true;
  }
};

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_FST_H_
