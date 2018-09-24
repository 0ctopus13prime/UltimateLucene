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

#include <Codec/CodecUtil.h>
#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Util/ArrayUtil.h>
#include <Util/Etc.h>
#include <Util/Exception.h>
#include <Util/Pack/Paged.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

namespace lucene {
namespace core {
namespace util {

class BytesStore;

class FSTBytesReader: public lucene::core::store::DataInput {
 public:
  virtual ~FSTBytesReader() = default;

  virtual uint64_t GetPosition() const noexcept = 0;

  virtual void SetPosition(const uint64_t pos) noexcept = 0;

  virtual bool Reversed() const noexcept = 0;
};

class ForwardFSTBytesReader: public FSTBytesReader {
 private:
  char* bytes;
  uint32_t pos;

 public:
  ForwardFSTBytesReader(char* bytes)
    : bytes(bytes),
      pos(0) {
  }

  char ReadByte() {
    return bytes[pos++];
  }

  void ReadBytes(char dest[], const uint32_t offset, const uint32_t len) {
    std::memcpy(dest + offset, bytes + pos, len);
    pos += len;
  }

  void SkipBytes(const uint64_t count) {
    pos += count;
  }

  uint64_t GetPosition() const noexcept {
    return pos;
  }

  void SetPosition(const uint64_t new_pos) noexcept {
    pos = new_pos; 
  }

  bool Reversed() const noexcept {
    return false;
  }
};

class ReverseFSTBytesReader: public FSTBytesReader {
 private:
  const char* bytes;
  uint32_t pos;

 public:
  ReverseFSTBytesReader(const char* bytes)
    : bytes(bytes),
      pos(0) {
  }

  char ReadByte() {
    return bytes[pos--];
  }

  void ReadBytes(char bytes[],
                 const uint32_t offset,
                 const uint32_t len) {
    char* base = bytes + offset;
    for (uint32_t i = 0 ; i < len ; ++i) {
      base[i] = bytes[pos--];
    }
  }

  void SkipBytes(const uint64_t count) {
    pos -= count;
  }

  uint64_t GetPosition() const noexcept {
    return pos;
  }

  void SetPosition(const uint64_t new_pos) noexcept {
    pos = static_cast<uint32_t>(new_pos);
  }

  bool Reversed() const noexcept {
    return true;
  }
};

class BytesStoreForwardFSTBytesReader: public FSTBytesReader {
 private:
  BytesStore* store;
  char* current;
  uint32_t next_buffer;
  uint32_t next_read;

 public:
  BytesStoreForwardFSTBytesReader(BytesStore* store);

  char ReadByte();

  void ReadBytes(char bytes[], const uint32_t offset, const uint32_t len);

  void SkipBytes(const uint64_t count);

  uint64_t GetPosition() const noexcept;

  void SetPosition(const uint64_t pos) noexcept;

  bool Reversed() const noexcept;
};

class BytesStoreReverseFSTBytesReader: public FSTBytesReader {
 private:
  BytesStore* store; 
  char* current;
  uint32_t next_buffer;
  uint32_t next_read;

 public:
  explicit BytesStoreReverseFSTBytesReader(BytesStore* store);

  char ReadByte();

  void ReadBytes(char bytes[], const uint32_t offset, const uint32_t len);

  void SkipBytes(const uint64_t count);

  uint64_t GetPosition() const noexcept;

  void SetPosition(const uint64_t new_pos) noexcept;

  bool Reversed() const noexcept;
};

class BytesStore : public lucene::core::store::DataOutput {
 private:
  friend class BytesStoreReverseFSTBytesReader;
  friend class BytesStoreForwardFSTBytesReader;

  std::vector<std::pair<std::unique_ptr<char[]>, uint32_t>> blocks;
  uint32_t block_bits;
  uint32_t block_size;
  uint32_t block_mask;
  uint32_t next_write;
  char* current;

 public:
  BytesStore(const BytesStore& other) = delete;

  BytesStore(BytesStore&& other)
    : blocks(std::move(other.blocks)),
      block_bits(other.block_bits),
      block_size(other.block_size),
      block_mask(other.block_mask),
      next_write(other.next_write),
      current(other.current) {
  }

  explicit BytesStore(const uint32_t block_bits)
    : blocks(),
      block_bits(block_bits),
      block_size(1 << block_bits),
      block_mask(block_size - 1),
      next_write(block_size),
      current(nullptr) {
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
      current = block_pair.first.get();
      blocks.push_back(std::move(block_pair));
      next_write = 0;
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
    assert(dest + len <= GetPosition());
    const uint64_t end = (dest + len);
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
        break;
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

  using lucene::core::store::DataOutput::CopyBytes;
  void CopyBytes(const uint64_t src, const uint64_t dest, const uint32_t len) {
    assert(src < dest);
    const uint64_t end = (src + len);
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
        WriteBytes(dest + len_tmp, block, 0, down_to);
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
    assert(src_pos < dest_pos);
    assert(dest_pos < GetPosition());
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

      if (dest != 0) {
        --dest;
      } else {
        dest_block_index--;
        dest_block = blocks[dest_block_index].first.get();
        dest = block_size - 1;
      }
    }
  }

  void SkipBytes(const uint32_t len) {
    uint32_t len_tmp = len;

    while (len_tmp > 0) {
      const uint32_t chunk = (block_size - next_write);
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
    assert(new_len <= GetPosition());
    assert(new_len >= 0);
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

    assert(new_len == GetPosition());
  }

  void Finish() {
    if (current != nullptr) {
      std::unique_ptr<char[]> last_buffer(new char[next_write]);
      std::memcpy(last_buffer.get(), current, next_write);
      std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
        std::move(last_buffer), block_size);
      blocks[blocks.size() - 1] = std::move(block_pair);
      current = nullptr;
    }
  }

  void WriteTo(lucene::core::store::DataOutput* out) {
    for (auto& block_pair : blocks) {
      out->WriteBytes(block_pair.first.get(), 0, block_pair.second);
    }
  }

  std::unique_ptr<FSTBytesReader> GetForwardReader() {
    if (blocks.size() == 1) {
      return std::make_unique<ForwardFSTBytesReader>(blocks[0].first.get());
    } else {
      return std::make_unique<BytesStoreForwardFSTBytesReader>(this);
    }
  }

  std::unique_ptr<FSTBytesReader>
  GetReverseReader(const bool allow_single = true) {
    if (allow_single && block_size == 1) {
      return std::make_unique<ReverseFSTBytesReader>(blocks[0].first.get());
    } else {
      return std::make_unique<BytesStoreReverseFSTBytesReader>(this);
    }
  }
};  // BytesStore

template<typename T>
class Outputs {
 public:
  virtual ~Outputs() = default;

  virtual T Common(T& output1, T& output2) = 0;

  virtual T Substract(T& output, T& inc) = 0;

  virtual T Add(T& prefix, T& output) = 0;

  virtual T Write(T& output, lucene::core::store::DataOutput* out) = 0;

  virtual T WriteFinalOutput(T& output,
                             lucene::core::store::DataOutput* out) {
    Write(output, out);
  }

  virtual T Read(lucene::core::store::DataInput* in) = 0;

  virtual void SkipOutput(lucene::core::store::DataInput* in) {
    Read(in);
  }

  virtual T ReadFinalOutput(lucene::core::store::DataInput* in) {
    return Read(in);
  }

  virtual void SkipFinalOutput(lucene::core::store::DataInput* in) {
    SkipOutput(in);
  }

  virtual T& GetNoOutput() = 0;

  virtual std::string OutputToString(T& output) = 0;

  virtual T Merge(T& first, T& second) {
    throw lucene::core::util::UnsupportedOperationException();
  }
};  // Outputs<T>

class IntSequenceOutputs: public Outputs<IntsRef> {
 private:
  static IntsRef NO_OUTPUT;

 public:
  IntsRef Common(IntsRef& output1, IntsRef& output2) {
    uint32_t pos1 = output1.offset;  
    uint32_t pos2 = output2.offset;
    uint32_t stop_at_1 = (pos1 + std::min(output1.length, output2.length));
    while ((output1.ints[pos1++] == output2.ints[pos2++]) &&
           (pos1 < stop_at_1));
    
    if (pos1 == output1.offset) {
      // No common prefix
      return IntSequenceOutputs::NO_OUTPUT;
    } else if (pos1 == output1.offset + output1.length) {
      // Output1 is a prefix of output2
      return output1;
    } else if (pos2 == output2.offset + output2.length) {
      // Output2 is a prefix of output1
      return output2;
    } else {
      // Return a common prefix between output1 and output2
      return IntsRef(output1.ints,
                     output1.capacity,
                     output1.offset,
                     pos1 - output1.offset);
    }
  }

  IntsRef Substract(IntsRef& output, IntsRef& inc) {
    if (inc == NO_OUTPUT) {
      // No prefix removed
      return output;
    } else if (inc.length == output.length) {
      // Entire output removed
      return NO_OUTPUT;
    } else {
      assert(inc.length > 0 && inc.length < output.length);
      return IntsRef(output.ints,
                     output.capacity,
                     output.offset + inc.length,
                     output.length - inc.length);
    }
  }

  IntsRef Add(IntsRef& prefix, IntsRef& output) {
    if (prefix == NO_OUTPUT) {
      return output;
    } else if (output == NO_OUTPUT) {
      return prefix;
    } else {
      assert(prefix.length > 0 && output.length > 0);
      IntsRef result(prefix.length + output.length);
      std::memcpy(result.ints,
                  prefix.ints + prefix.offset,
                  prefix.length);
      std::memcpy(result.ints + prefix.length,
                  output.ints + output.offset,
                  output.length);
      result.length = (prefix.length + output.length);
      return result;
    }
  }

  IntsRef Write(IntsRef& prefix, lucene::core::store::DataOutput* out) {
    out->WriteVInt32(prefix.length);
    for (uint32_t idx = 0 ; idx < prefix.length ; ++idx) {
      out->WriteVInt32(prefix.ints[prefix.offset + idx]);
    }
  }

  IntsRef Read(lucene::core::store::DataInput* in) {
    const int32_t len = in->ReadVInt32();
    if (len == 0) {
      return NO_OUTPUT;
    } else {
      IntsRef output(len);
      for (uint32_t idx = 0 ; idx < len ; ++idx) {
        output.ints[idx] = in->ReadVInt32();
      }
      output.length = len;
      return output;
    }
  }

  void SkipOutput(lucene::core::store::DataInput* in) {
    const int32_t len = in->ReadVInt32();
    if (len == 0) {
      return;
    }

    for (uint32_t idx = 0 ; idx < len ; ++idx) {
      in->ReadVInt32();
    }
  }

  IntsRef& GetNoOutput() {
    return NO_OUTPUT;
  }

  std::string OutputToString(IntsRef& output) {
    // TODO(0ctopus13prime): IT
    return std::string();
  }
};  // IntSequenceOutputs

enum class FST_INPUT_TYPE {
  BYTE1, BYTE2, BYTE4
};

template<typename T>
class FST;

template<typename T>
class NodeHash;

template<typename T>
class Builder {
 public:
  class Node;
  class UnCompiledNode;
  class CompiledNode;
  class Arc;

 private:
  std::unique_ptr<NodeHash<T>> dedup_hash;
  FST<T> fst;
  T NO_OUTPUT;
  uint32_t min_suffix_count1;
  uint32_t min_suffix_count2;
  BytesStore& bytes;
  uint32_t reused_byte_per_arc[4];
  uint32_t share_max_tail_length;
  lucene::core::util::IntsRefBuilder last_input;
  std::vector<UnCompiledNode> frontier;
  uint64_t last_frozen_node;
  uint64_t arc_count;
  uint64_t node_count;
  bool do_share_non_singleton_nodes;
  bool allow_array_arcs;

 private:
  CompiledNode CompileNode(UnCompiledNode& node_in,
                           const uint32_t tail_length) {
    int64_t node;
    const uint64_t bytes_pos_start = bytes.GetPosition();

    if (dedup_hash &&
        (do_share_non_singleton_nodes || node_in.num_arcs <= 1) &&
        tail_length <= share_max_tail_length) {
      if (node_in.num_arcs == 0) {
        node = fst.AddNode(this, node_in); 
        last_frozen_node = node;
      } else {
        node = dedup_hash.Add(this, node_in);
      }
    } else {
      node = fst.AddNode(this, node_in);
    }

    const uint64_t bytes_pos_end = bytes.GetPosition();
    if (bytes_pos_end != bytes_pos_start) {
      last_frozen_node = node;
    }

    node_in.Clear();

    CompiledNode fn;
    fn.node = node;
    return fn;
  }
  
  void FreezeTail(const uint32_t prefix_len_plus1) {
    const uint32_t down_to = (1 > prefix_len_plus1 ? 1 : prefix_len_plus1);

    for (uint32_t idx = last_input.Length() ; idx >= down_to ; --idx) {
      bool do_prune = false;
      bool do_compile = false;

      UnCompiledNode& node = frontier[idx];
      UnCompiledNode& parent = frontier[idx - 1]; 

      if (node.input_count < min_suffix_count1) {
        do_prune = true;
        do_compile = true;
      } else if(idx > prefix_len_plus1) {
        if (parent.input_count < min_suffix_count2 ||
            (min_suffix_count2 == 1 && parent.input_count == 1 && idx > 1)) {
          do_prune = true;
        } else {
          do_prune = false;
        }
        do_compile = true;
      } else {
        do_compile = (min_suffix_count2 == 0);
      }

      if (node.input_count < min_suffix_count2 ||
          (min_suffix_count2 == 1 && node.input_count == 1 && idx > 1)) {
        for (uint32_t arc_idx = 0 ; arc_idx < node.num_arcs ; ++arc_idx) {
          UnCompiledNode& target =
          dynamic_cast<UnCompiledNode&>(*node.arcs[arc_idx].target); 
          target.Clear();
        }

        node.num_arcs = 0;
      }

      if (do_prune) {
        node.Clear();
        parent.DeleteLast(last_input.IntAt(idx - 1), &node);
      } else {
        if (min_suffix_count2 != 0) {
          CompileAllTargets(node, last_input.Length() - idx);
        }
        T& next_final_output = node.output;
        const bool is_final = (node.is_final || node.num_arcs == 0);

        if (do_compile) {
          CompiledNode compiled_node =
            CompileNode(node, 1 + last_input.Length() - idx); 
          parent.ReplaceLast(last_input.IntAt(idx - 1),
                             &compiled_node,
                             std::move(next_final_output),
                             is_final);
        } else {
          parent.ReplaceLast(last_input.IntAt(idx - 1),
                             &node,
                             std::move(next_final_output),
                             is_final);
          frontier[idx] = UnCompiledNode(this, idx);
        }
      }
    }
  }

  bool ValidOutput(T& output) const {
    return (&output == &NO_OUTPUT || output != NO_OUTPUT);
  }

  void CompileAllTargets(UnCompiledNode& node, const uint32_t tail_length) {
    for (uint32_t arc_idx = 0 ; arc_idx < node.num_arcs ; arc_idx++) {
      Arc& arc = node.arcs[arc_idx];
      if (!arc.target->IsCompiled()) {
        UnCompiledNode& n = dynamic_cast<UnCompiledNode&>(*(arc.target));
        if (n.num_arcs == 0) {
          arc.is_final = n.is_final = true;
        }

        arc.target = CompileNode(n, tail_length - 1);
      }
    }
  }

 public:
  Builder(const FST_INPUT_TYPE input_type,
          std::unique_ptr<Outputs<T>>&& outputs)
    : Builder(input_type,
              0,
              0,
              true,
              true,
              std::numeric_limits<int32_t>::max(),
              std::move(outputs),
              true,
              15) {
  }

  Builder(const FST_INPUT_TYPE input_type,
          const uint32_t min_suffix_count1,
          const uint32_t max_suffix_count2,
          const bool do_share_suffix,
          const bool do_share_non_singleton_nodes,
          const uint32_t share_max_tail_length,
          std::unique_ptr<Outputs<T>>&& outputs,
          const bool allow_array_arcs,
          const uint32_t bytes_page_bits)
    : dedup_hash(),
      fst(input_type, std::move(outputs), bytes_page_bits),
      NO_OUTPUT(outputs->GetNoOutput()),
      min_suffix_count1(min_suffix_count1),
      min_suffix_count2(min_suffix_count2),
      bytes(fst.bytes),
      reused_byte_per_arc{0},
      share_max_tail_length(0),
      last_input(),
      frontier(),
      last_frozen_node(0),
      arc_count(0),
      node_count(0),
      do_share_non_singleton_nodes(do_share_non_singleton_nodes),
      allow_array_arcs(allow_array_arcs) {
    if (do_share_suffix) {
      dedup_hash =
        std::make_unique<NodeHash<T>>(&fst, bytes.GetReverseReader(false));
    }

    frontier.reserve(10);
    for (uint32_t idx = 0 ; idx < 10 ; ++idx) {
      frontier.push_back(UnCompiledNode(this, idx));
    }
  }

  uint64_t GetTermCount() const noexcept {
    return frontier[0].input_count; 
  }

  uint64_t GetNodeCount() const noexcept {
    return (1 + node_count);
  }

  uint64_t GetArcCount() const noexcept {
    return arc_count;
  }

  uint64_t GetMappedStateCount() const noexcept {
    return (dedup_hash ? node_count : 0L);
  }

  void Add(lucene::core::util::IntsRef& input, T& output) {
    if (output == NO_OUTPUT) {
      output = NO_OUTPUT;
    }

    assert(last_input.Length() == 0 || !(input < last_input.Get()));
    assert(ValidOutput(output));

    if (input.length == 0) {
      frontier[0].input_count++;
      frontier[0].is_final = true;
      fst.SetEmptyOutput(output);
      return;
    }

    uint32_t pos1 = 0;
    uint32_t pos2 = input.offset;
    uint32_t pos1_stop = std::min(last_input.Length(), input.length);
    while (true) {
      frontier[pos1].input_count++;
      if (pos1 >= pos1_stop ||
          last_input.IntAt(pos1) != input.ints[pos2]) {
        break;
      }

      pos1++;
      pos2++;
    }

    const uint32_t prefix_len_plus1 = (pos1 + 1);

    // Minimize / Compile states from previous input's
    // orphan'd suffix
    FreezeTail(prefix_len_plus1);

    for (uint32_t idx = prefix_len_plus1 ; idx <= input.length ; ++idx) {
      frontier[idx - 1].AddArc(input.ints[input.offset + idx - 1],
                               frontier[idx]);
      frontier[idx].input_count++;
    }

    UnCompiledNode& last_node = frontier[input.length];
    if (last_input.Length() != input.length ||
        prefix_len_plus1 != input.length + 1) {
      last_node.is_final = true;
      last_node.output = NO_OUTPUT;
    }

    // Push conflicting outputs forward, only as far as needed
    for (uint32_t idx = 1 ; idx < prefix_len_plus1 ; ++idx) {
      UnCompiledNode& node = frontier[idx];
      UnCompiledNode& parent_node = frontier[idx - 1];

      T& last_output =
      parent_node.GetLastOutput(input.ints[input.offset + idx - 1]);
      assert(ValidOutput(last_output));

      T common_output_prefix;
      T word_suffix;

      if (&last_output != &NO_OUTPUT) {
        common_output_prefix = fst.outputs->Common(output, last_output);
        assert(ValidOutput(common_output_prefix));
        word_suffix = fst.outputs->Substract(last_output, common_output_prefix);
        assert(ValidOutput(word_suffix));
        parent_node.SetLastOutput(input.ints[input.offset + idx - 1],
                                  common_output_prefix);
        node.PrependOutput(word_suffix);
      } else {
        common_output_prefix = word_suffix = NO_OUTPUT;
      }

      output = fst.outputs->Substract(output, common_output_prefix);
    }

    if (last_input.Length() == input.length &&
        prefix_len_plus1 == (1 + input.length)) {
      // Same input more than 1 time in a row, mapping to
      // multiple outputs
      last_node.output = fst.outputs->Merge(last_node.output, output);
    } else {
      // This new arc is private to this new input; 
      // set its arc output to the leftover output:
      frontier[prefix_len_plus1 - 1].SetLastOutput(
      input.ints[input.offset + prefix_len_plus1 - 1], output);
    }

    // Save last input
    last_input.CopyInts(input);
  }

  FST<T>* Finish() {
    UnCompiledNode& root = *(frontier[0]);
    FreezeTail(0);
    if (root.input_count < min_suffix_count1 ||
        root.input_count < min_suffix_count2 ||
        root.num_arcs == 0) {
      if (!fst.AcceptEmptyOutput() ||
          (min_suffix_count1 > 0 || min_suffix_count2 > 0)) {
        return nullptr;
      }
    } else {
      if (min_suffix_count2 != 0) {
        CompileAllTargets(root, last_input.Length());
      }
    }
    fst.Finish(CompileNode(root, last_input.Length()).node);

    return &fst;
  }
};  // Builder<T>

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

  Arc() = default;

  Arc(const Arc& other)
    : label(other.label),
      target(other.target),
      is_final(other.is_final),
      output(other.output),
      next_final_output(other.next_final_output) {
  }

  Arc(Arc&& other)
    : label(other.label),
      target(other.target),
      is_final(other.is_final),
      output(std::move(other.output)),
      next_final_output(std::move(other.next_final_output)) {
  }
};

template<typename T>
class Builder<T>::UnCompiledNode : public Builder<T>::Node {
 private:
  Builder<T>* owner;

 public:
  std::unique_ptr<Arc[]> arcs; 
  T output;
  uint64_t input_count;
  uint32_t num_arcs;
  uint32_t arcs_size;
  uint32_t depth;
  bool is_final;

 private:
  void GrowIf() {
    if (num_arcs == arcs_size) {
      std::pair<Arc*, uint32_t> pair =
      lucene::core::util::arrayutil::Grow(arcs, arcs_size + 1);
      
      arcs.reset(pair.first);
      arcs_size = pair.second;
    }
  }

 public:
  UnCompiledNode(Builder<T>* owner, const uint32_t depth)
    : owner(owner),
      arcs(new Arc[1]),
      output(owner->NO_OUTPUT),
      input_count(0),
      num_arcs(0),
      arcs_size(1),
      depth(depth),
      is_final(false) {
  }

  UnCompiledNode(const UnCompiledNode& other) = delete;

  UnCompiledNode& operator=(const UnCompiledNode& other) = delete;

  UnCompiledNode(UnCompiledNode&& other)
    : owner(other.owner),
      arcs(std::move(other.arcs)),
      output(other.output),
      input_count(other.input_count),
      num_arcs(other.num_arcs),
      arcs_size(other.arcs_size),
      depth(other.depth),
      is_final(other.is_final) {
  }

  UnCompiledNode& operator=(UnCompiledNode&& other) {
    owner = other.owner;
    arcs = std::move(other.arcs);
    output = other.output;
    input_count = other.input_count;
    num_arcs = other.num_arcs;
    arcs_size = other.arcs_size;
    depth = other.depth;
    is_final = other.is_final;
  }

  bool IsCompiled() const noexcept {
    return false;
  }

  void Clear() noexcept {
    num_arcs = 0;
    is_final = false;
    output = owner->NO_OUTPUT;
    input_count = 0;
  }

  T& GetLastOutput(const uint32_t label_to_match) const noexcept {
    return arcs[num_arcs - 1].output;
  }

  void AddArc(const uint32_t label, Node& target) {
    GrowIf();
    Arc& arc = arcs[num_arcs++]; 
    arc.label = label;
    arc.target = &target;
    arc.output = arc.next_final_output = owner->NO_OUTPUT;
    arc.is_final = false;
  }

  void ReplaceLast(const uint32_t last_to_match,
                   Node* target,
                   T&& next_final_output,
                   const bool is_final) {
    Arc& arc = arcs[num_arcs - 1];
    arc.target = target;
    arc.next_final_output = std::forward<T>(next_final_output);
    arc.is_final = is_final;
  }

  void DeleteLast(const uint32_t label, Node* target) noexcept {
    --num_arcs;
  }

  void SetLastOutput(const uint32_t label_to_match, T& new_output) {
    Arc& arc = arcs[num_arcs - 1];   
    arc.output = new_output;
  }

  void PrependOutput(const T& output_prefix) {
    for (uint32_t arc_idx = 0 ; arc_idx < num_arcs ; ++arc_idx) {
      arcs[arc_idx].output =
      owner->fst.outputs->Add(output_prefix, arcs[arc_idx].output);
    }

    if (is_final) {
      output = owner->fst.outputs->Add(output_prefix, output);
    }
  }
};  // Builder<T>::UnCompiledNode

template<typename T>
class Builder<T>::CompiledNode: public Builder<T>::Node {
 private:
  uint64_t node;

  bool IsCompiled() const noexcept {
    return true;
  }
};  // Builder<T>::CompiledNode

template<typename T>
class FST {
 friend class Builder<T>;

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
  static const uint32_t MAX_BLOCK_BITS = 30;

 public:
  class Arc;

  FST_INPUT_TYPE input_type;
  T empty_output;
  bool is_empty_output_empty;
  BytesStore bytes;
  bool is_bytes_empty;
  std::unique_ptr<char[]> bytes_array;
  uint32_t bytes_array_size;
  int64_t start_node;
  std::unique_ptr<Outputs<T>> outputs;
  std::unordered_map<uint32_t, Arc> cached_root_arcs;
  uint32_t version;

 private:
  static bool Flag(const uint32_t flag, const uint32_t bit) noexcept {
    return ((flag & bit) != 0);
  }

 public:
  FST(lucene::core::store::DataInput* in,
      std::unique_ptr<Outputs<T>>&& outputs)
    : FST(in, std::move(outputs), MAX_BLOCK_BITS) {
  }

  FST(lucene::core::store::DataInput* in,
      std::unique_ptr<Outputs<T>>&& outputs,
      const uint32_t max_block_bits) {
    is_empty_output_empty = true;
    this->outputs = std::move(outputs);

    if (max_block_bits == 0 || max_block_bits > MAX_BLOCK_BITS) {
      throw IllegalArgumentException("Max block bits should be 1 .. 30. Got " +
                                     std::to_string(max_block_bits));
    }

    version =
      lucene::core::codec::CodecUtil::CheckHeader(in,
                                                  FILE_FORMAT_NAME,
                                                  VERSION_PACKED,
                                                  VERSION_CURRENT);
    if (version < VERSION_PACKED_REMOVED &&
        in->ReadByte() == 1) {
        throw lucene::core::index::CorruptIndexException(
              "Cannot read packed FSTs anymore");
    }

    if (in->ReadByte() == 1) {
      // Accept empty string
      // 1KB blocks
      BytesStore empty_bytes(10);
      const int32_t num_bytes = in->ReadVInt32();
      empty_bytes.CopyBytes(in, num_bytes);

      // De-serialize empty-string output:
      std::unique_ptr<FSTBytesReader> reader =
        empty_bytes.GetReverseReader();

      // NoOutputs uses 0 bytes when writing its output,
      // so we have to check here else BytesStore gets angry:
      if (num_bytes > 0) {
        reader->SetPosition(num_bytes - 1);
      }

      empty_output = outputs->ReadFinalOutput(reader.get());
      is_empty_output_empty = false;
    }

    const char t = in->ReadByte();
    switch (t) {
      case 0:
        input_type = FST_INPUT_TYPE::BYTE1;
        break;
      case 1:
        input_type = FST_INPUT_TYPE::BYTE2;
        break;
      case 2:
        input_type = FST_INPUT_TYPE::BYTE4;
        break;
      default:
        throw IllegalStateException("Invalid input type " +
                              std::to_string(static_cast<uint32_t>(t & 0xFF)));
    }

    start_node = in->ReadVInt64();
    if (version < VERSION_NO_NODE_ARC_COUNTS) {
      in->ReadVInt64();
      in->ReadVInt64();
      in->ReadVInt64();
    }

    const int64_t num_bytes = in->ReadVInt64();
    is_bytes_empty = false;
    if (num_bytes > (1 << max_block_bits)) {
      // FST is big: We need to multiple pages
      bytes = BytesStore(in, num_bytes, 1 << max_block_bits);
      bytes_array.reset();
      bytes_array_size = 0;
    } else {
      // FST file into a single block:
      // Use ByteArrayBytesStoreReader for less overhead
      bytes = BytesStore();
      bytes_array_size = num_bytes;
      bytes_array = std::make_unique<char[]>(bytes_array_size);
      in->ReadBytes(bytes_array.get(), 0, bytes_array_size);
    }

    CacheRootArcs();
  }

  FST(const FST_INPUT_TYPE input_type, 
      std::unique_ptr<Outputs<T>>&& outputs,
      const uint32_t bytes_page_bits)
    : input_type(input_type),
      empty_output(),
      is_empty_output_empty(true),
      bytes(bytes_page_bits),
      is_bytes_empty(false),
      bytes_array(),
      bytes_array_size(0),
      start_node(-1),
      outputs(std::move(outputs)),
      cached_root_arcs(0x100),
      version(VERSION_CURRENT) {
  }

  FST_INPUT_TYPE GetInputType() const noexcept {
    return input_type;
  }

  T& GetEmptyOutput() const noexcept {
    return empty_output;
  }

  void Save(lucene::core::store::DataOutput* out) {
    lucene::core::codec::CodecUtil::WriteHeader(out,
                                                FILE_FORMAT_NAME,
                                                VERSION_CURRENT);
    if (!is_empty_output_empty) {
      out->WriteByte(static_cast<char>(1));
      // TODO(0ctopus13prime): Implement RAMOutputStream
      /*
      lucene::core::store::RAMOutputStream ros;
      outputs->WriteFinalOutput(empty_output, &ros);

      const uint32_t empty_output_bytes_size = ros.GetFilePointer();
      char empty_output_bytes[empty_output_bytes_size];
      ros.WriteTo(empty_output_bytes, 0);

      // Reverse
      const uint32_t stop_at = (empty_output_bytes_size >> 1);
      for (uint32_t upto = 0 ; upto < stop_at ; ++upto) {
        const char b = empty_output_bytes[upto];
        empty_output_bytes[upto] =
          empty_output_bytes[empty_output_bytes_size - upto - 1];
        empty_output_bytes[empty_output_bytes_size - upto - 1] = b;
      }

      out->WriteVInt32(empty_output_bytes_size);
      out->WriteBytes(empty_output_bytes, 0, empty_output_bytes_size);
      */
    } else {
      out->WriteByte(static_cast<char>(0));
    }

    switch (input_type) {
      case FST_INPUT_TYPE::BYTE1:
        out->WriteByte(0);
        break;
      case FST_INPUT_TYPE::BYTE2:
        out->WriteByte(1);
        break;
      default:
        out->WriteByte(2);
        break;
    }

    out->WriteVInt64(start_node);
    if (!is_bytes_empty) {
      out->WriteVInt64(bytes.GetPosition());
      bytes.WriteTo(out);
    } else {
      assert(bytes_array);
      out->WriteVInt64(bytes_array_size);
      out->WriteBytes(bytes_array.get(), 0, bytes_array_size);
    }
  }

  void Save(const std::string& path) {
    // TODO(0ctopus13prime): IT
  }

  void GetFirstArc(Arc& arc) {
    T NO_OUTPUT = outputs->GetNoOutput(); 

    if (!is_empty_output_empty) {
      arc.flags = (BIT_FINAL_ARC | BIT_LAST_ARC);
      arc.next_final_output = empty_output;
      if (&empty_output != &NO_OUTPUT) {
        arc.flags |= BIT_ARC_HAS_FINAL_OUTPUT; 
      }
    } else {
      arc.floags = BIT_LAST_ARC;
      arc.next_final_output = NO_OUTPUT;
    }
    arc.output = NO_OUTPUT;

    arc.output = NO_OUTPUT;

    // If there are no nodes, ie, the FST only accepts the
    // empty string, then startNode is 0
    arc.target = start_node;
  }

  void ReadLastTargetArc(Arc& follow,
                         Arc& arc,
                         FSTBytesReader& in) {
    // TODO(0ctopus13prime): IT
  }

  void ReadFirstTargetArc(Arc& follow, Arc& arc, FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
  }

  void ReadFirstRealTargetArc(const int64_t node,
                              Arc& arc,
                              FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
  }
  
  void ReadNextArc(Arc& arc, FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
  }

  int32_t ReadNextArcLabel(Arc& arc, FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
    return 0;
  }

  void ReadNextRealArc(Arc& arc, FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
  }

  void FindTargetArc(const int32_t label_to_match,
                     Arc& follow,
                     Arc& arc,
                     FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
  }

  std::unique_ptr<FSTBytesReader> GetBytesReader() {
    // TODO(0ctopus13prime): IT
    return {};
  }

 public:
  static FST<T> Read(const std::string& path,
                     std::unique_ptr<Outputs<T>> outputs) {
    // TODO(0ctopus13prime): IT
    return FST<T>(nullptr, std::unique_ptr<Outputs<T>>());
  }

  static bool TargetHasArcs(Arc& arc) {
    // TODO(0ctopus13prime): IT
    return true;
  }

 private:
  void FindTargetArc(const int32_t label_to_match,
                     Arc& follow,
                     Arc& arc,
                     FSTBytesReader* in,
                     const bool use_root_arc_cache) {
    // TODO(0ctopus13prime): IT
  }

  void SeekToNextNode(FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
  }

  bool ShouldExpand(Builder<T>& builder, typename Builder<T>::UnCompiledNode& node) {
    // TODO(0ctopus13prime): IT
    return true;
  }

  bool AssertRootCachedArc(const int32_t label, Arc& cached_arc) {
    // TODO(0ctopus13prime): IT
    return true;
  }

  bool IsExpandedTarget(Arc& follow, FSTBytesReader* in) {
    // TODO(0ctopus13prime): IT
    return true;
  }

  int64_t ReadUnpackedNodeTarget(FSTBytesReader& in) {
    // TODO(0ctopus13prime): IT
    return 0L;
  }

  int64_t AddNode(Builder<T>& builder, typename Builder<T>::UnCompiledNode node_in) {
    // TODO(0ctopus13prime): IT
    return 0L;
  }

  void WriteLabel(lucene::core::store::DataOutput* out, const int32_t v) {
    // TODO(0ctopus13prime): IT
  }

  int32_t ReadLabel(lucene::core::store::DataInput* in) {
    // TODO(0ctopus13prime): IT
    return 0;
  }

  void SetEmptyOutput(T& v) {
    if (is_empty_output_empty) {
      empty_output = v;
    } else {
      outputs->Merge(empty_output, v);
    }
  }

  void CacheRootArcs() {
    Arc arc;
    GetFirstArc(arc);
    if (TargetHasArcs(arc)) {
      std::unique_ptr<FSTBytesReader> in = GetBytesReader(); 
      std::unordered_map<uint32_t, Arc> arcs(0x100);
      ReadFirstRealTargetArc(arc.target, arc, in.get());
      uint32_t count = 0;
      cached_root_arcs.clear();
      while (true) {
        assert(arc.label != END_LABEL);
        if (arc.label < 0x80) {
          arcs[arc.label] = arc;
        } else {
          break;
        }

        if (arc.IsLast()) {
          break;
        }

        ReadNextRealArc(arc, in.get());
        count++;
      }  // End while

      if (count >= FIXED_ARRAY_NUM_ARCS_SHALLOW) {
        cached_root_arcs = std::move(arcs); 
      }
    }  // End if
  }
};  // FST

template<typename T>
class FST<T>::Arc {
 public:
  T output;
  T next_final_output;
  uint64_t target;
  uint64_t next_arc;
  uint64_t pos_arcs_start;
  int32_t label;
  int32_t bytes_per_arc;
  uint32_t arc_idx;
  uint32_t num_arcs;
  uint8_t flags;

  Arc()
    : output(),
      next_final_output(),
      target(0),
      next_arc(0),
      pos_arcs_start(0),
      label(0),
      bytes_per_arc(0),
      arc_idx(0),
      num_arcs(0),
      flags(0) {
  }

  Arc(const Arc& other)
    : output(other.output),
      next_final_output(other.next_final_output),
      target(other.target),
      next_arc(other.next_arc),
      pos_arcs_start(other.pos_arcs_start),
      label(other.label),
      bytes_per_arc(other.bytes_per_arc),
      arc_idx(other.arc_idx),
      num_arcs(other.num_arcs),
      flags(other.flags) {
  }

  Arc& operator=(const Arc& other) {
    if (this != &other) {
      output = other.output;
      next_final_output = other.next_final_output;
      target = other.target;
      next_arc = other.next_arc;
      pos_arcs_start = other.pos_arcs_start;
      label = other.label;
      bytes_per_arc = other.bytes_per_arc;
      arc_idx = other.arc_idx;
      num_arcs = other.num_arcs;
      flags = other.flags;
    }

    return *this;
  }

  Arc(Arc&& other)
    : output(std::move(other.output)),
      next_final_output(std::move(other.next_final_output)),
      target(other.target),
      next_arc(other.next_arc),
      pos_arcs_start(other.pos_arcs_start),
      label(other.label),
      bytes_per_arc(other.bytes_per_arc),
      arc_idx(other.arc_idx),
      num_arcs(other.num_arcs),
      flags(other.flags) {
  }

  Arc& operator=(Arc&& other) {
    if (this != &other) {
      output = std::move(other.output);
      next_final_output = std::move(other.next_final_output);
      target = other.target;
      next_arc = other.next_arc;
      pos_arcs_start = other.pos_arcs_start;
      label = other.label;
      bytes_per_arc = other.bytes_per_arc;
      arc_idx = other.arc_idx;
      num_arcs = other.num_arcs;
      flags = other.flags;
    }

    return *this;
  }

  bool Flag(const uint8_t flag) const noexcept {
    return FST<T>::Flag(flags, flag);
  }

  bool IsLast() const noexcept {
    return Flag(BIT_LAST_ARC);
  }

  bool IsFinal() const noexcept {
    return Flag(BIT_FINAL_ARC);
  }
};  // FST<T>::Arc

template<typename T>
class NodeHash {
 private:
  PagedGrowableWriter table;
  uint64_t count;
  uint64_t mask;
  FST<T>* fst;
  typename FST<T>::Arc scratch_arc;
  std::unique_ptr<FSTBytesReader> in;

 private:
  // Only for rehash
  NodeHash(PagedGrowableWriter&& table, NodeHash<T>&& other)
    : table(std::forward<PagedGrowableWriter>(table)),
      count(other.count),
      mask(this->table.Size() - 1),
      fst(other.fst),
      scratch_arc(std::move(other.scratch_arc)),
      in(std::move(other.in)) {
  }

  bool NodesEqual(typename Builder<T>::UnCompiledNode& node,
                  const uint64_t address) {
    fst->ReadFirstRealTargetArc(address, scratch_arc, in.get());
    if (scratch_arc.bytes_per_arc != 0 &&
        node.num_arc != scratch_arc.num_arcs) {
      return false; 
    }

    for (uint32_t arc_upto = 0 ; arc_upto < node.num_arcs ; ++arc_upto) {
      typename Builder<T>::Arc& arc = node.arcs[arc_upto];
      if (arc.label != scratch_arc.label ||
          arc.output != scratch_arc.output ||
          dynamic_cast<typename Builder<T>::CompiledNode*>(arc.target)->node
            != scratch_arc.target ||
          arc.next_final_output != scratch_arc.next_final_output ||
          arc.is_final != scratch_arc.IsFinal()) {
        return false;
      } else if (scratch_arc.IsLast()) {
        return (arc_upto == node.num_arcs - 1);
      }

      fst->ReadNextRealArc(scratch_arc, in.get());
    }

    return false;
  }

  int64_t Hash(typename Builder<T>::UnCompiledNode& node);

  int64_t Hash(const int64_t node) {
    const uint32_t PRIME = 31;
    int64_t h = 0;
    fst->ReadFirstRealTargetArc(node, scratch_arc, in.get());
    while (true) {
      h = PRIME * h + scratch_arc.label;
      h = PRIME * h +
          static_cast<int32_t>(scratch_arc.target ^ (scratch_arc.target >> 32));
      h = PRIME * h + scratch_arc.output.HashCode();
      h = PRIME * h + scratch_arc.next_final_output.HashCode();
      if (scratch_arc.IsFinal()) {
        h += 17;
      }

      if (scratch_arc.IsLast()) {
        break;
      }

      fst->ReadNextRealArc(scratch_arc, in.get());
    }

    return (h & std::numeric_limits<int64_t>::max());
  }

  int64_t Add(Builder<T>& builder,
              typename Builder<T>::UnCompiledNode& node_in) {
    const int64_t h = Hash(node_in);
    int64_t pos = (h & mask);
    int32_t c = 0;

    while (true) {
      const int64_t v = table.Get(pos);
      if (v == 0) {
        // Freeze & Add
        const int64_t node = fst->AddNode(builder, node_in);
        assert(Hash(node) == h);
        count++;
        table.Set(pos, node);
        if (count > (2 * table.Size()) / 3) {
          Rehash();
        }
      } else if (NodesEqual(node_in, v)) {
        return v;
      }

      pos = ((pos + (++c)) & mask);
    }
  }

  void AddNew(const uint64_t address) {
    int64_t pos = (Hash(address) & mask);
    int32_t c = 0;

    while (table.Get(pos)) {
      pos = ((pos + (++c)) & mask);
    }

    table.Set(pos, address);
  }

  void Rehash() {
    NodeHash<T> new_one(PagedGrowableWriter(
                          2 * table.Size(),
                          1 << 30,
                          PackedInts::BitsRequired(count),
                          PackedInts::COMPACT),
                        std::move(*this));
    
    for (uint32_t idx = 0 ; idx < table.Size() ; ++idx) {
      const int64_t address = table.Get(idx);

      if (address != 0) {
        new_one.AddNew(address);  
      }
    }

    operator=(std::move(new_one));
  }

 public:
  NodeHash(FST<T>* fst, std::unique_ptr<FSTBytesReader>&& in)
    : table(16, 1 << 27, 8, PackedInts::COMPACT),
      count(0),
      mask(16),
      fst(fst),
      scratch_arc(),
      in(std::forward<std::unique_ptr<FSTBytesReader>>(in)) {
  }

  NodeHash(const NodeHash<T>& other) = delete;

  NodeHash<T>& operator=(const NodeHash<T>& other) = delete;

  NodeHash(NodeHash<T>&& other)
    : table(std::move(other.table)),
      mask(other.mask),
      fst(other.fst),
      scratch_arc(other.scratch_arc),
      in(std::move(other.in)) {
  }

  NodeHash<T>& operator=(NodeHash<T>&& other) {
    table = std::move(other.table);
    count = other.count;
    mask = other.mask;
    fst = other.fst;
    scratch_arc = other.scratch_arc;
    in = std::move(other.in);
  }
};

/**
 *  BytesStoreForwardFSTBytesReader
 */
BytesStoreForwardFSTBytesReader::BytesStoreForwardFSTBytesReader(
  BytesStore* store)
  : store(store),
    current(nullptr),
    next_buffer(0),
    next_read(store->block_size) {
}

char BytesStoreForwardFSTBytesReader::ReadByte() {
  if (next_read == store->block_size) {
    current = store->blocks[next_buffer++].first.get();
    next_read = 0;
  }

  return current[next_read++];
}

void BytesStoreForwardFSTBytesReader::ReadBytes(char bytes[],
                                                const uint32_t offset,
                                                const uint32_t len) {
  uint32_t len_tmp = len;
  uint32_t offset_tmp = offset;

  while (len_tmp > 0) {
    const int32_t chunk_left = store->block_size - next_read; 
    if (len_tmp <= chunk_left) {
      std::memcpy(bytes + offset, current + next_read, len);
      break;
    } else {
      if (chunk_left > 0) {
        std::memcpy(bytes + offset, current + next_read, chunk_left);
        offset_tmp += chunk_left;
        len_tmp -= chunk_left;
      }

      current = store->blocks[next_buffer++].first.get();
      next_read = 0;
    }
  }
}

void BytesStoreForwardFSTBytesReader::SkipBytes(const uint64_t count) {
  SetPosition(GetPosition() + count);
}

uint64_t BytesStoreForwardFSTBytesReader::GetPosition() const noexcept {
  return
  static_cast<uint64_t>(next_buffer - 1) * store->block_size + next_read;
}

void BytesStoreForwardFSTBytesReader::SetPosition(const uint64_t pos) noexcept {
  const uint32_t buffer_idx =
  static_cast<const uint32_t>(pos >> store->block_bits); 

  next_buffer = buffer_idx + 1;
  current = store->blocks[buffer_idx].first.get();
  next_read = static_cast<uint32_t>(pos & store->block_mask);
}

bool BytesStoreForwardFSTBytesReader::Reversed() const noexcept {
  return false;
}

/**
 *  BytesStoreReverseFSTBytesReader
 */
BytesStoreReverseFSTBytesReader::BytesStoreReverseFSTBytesReader(
  BytesStore* store)
  : store(store),
    current(store->blocks.size() == 0 ? nullptr : store->blocks[0].first.get()),
    next_buffer(0),
    next_read(0) {
}

char BytesStoreReverseFSTBytesReader::ReadByte() {
  char ret = current[next_read--];
  if (next_read == 0) {
    current = store->blocks[next_buffer--].first.get();
    next_read = store->block_size - 1;
  }

  return ret;
}

void BytesStoreReverseFSTBytesReader::ReadBytes(char bytes[],
                                                const uint32_t offset,
                                                const uint32_t len) {
  char* base = bytes + offset;
  for (uint32_t i = 0 ; i < len ; ++i) {
    base[i] = ReadByte();
  }
}

void BytesStoreReverseFSTBytesReader::SkipBytes(const uint64_t count) {
  SetPosition(GetPosition() - count);
}

uint64_t BytesStoreReverseFSTBytesReader::GetPosition() const noexcept {
  return
  static_cast<uint64_t>(next_buffer + 1) * store->block_size + next_read; 
}

void
BytesStoreReverseFSTBytesReader::SetPosition(const uint64_t new_pos) noexcept {
  const uint32_t buffer_idx =
  static_cast<uint32_t>(new_pos >> store->block_bits);
  next_buffer = buffer_idx - 1;
  current = store->blocks[buffer_idx].first.get();
  next_read = static_cast<uint32_t>(new_pos & store->block_mask);
}

bool BytesStoreReverseFSTBytesReader::Reversed() const noexcept {
  return true;
}

/*
 *  NodeHash<T>
 */
template<typename T>
int64_t NodeHash<T>::Hash(typename Builder<T>::UnCompiledNode& node) {
  const uint32_t PRIME = 31;  
  int64_t h = 0;

  for (uint32_t arc_idx = 0 ; arc_idx < node.num_arcs ; ++arc_idx) {
    typename Builder<T>::Arc& arc = node.arcs[arc_idx];
    h = PRIME * h + arc.label;
    const int64_t n =
      dynamic_cast<typename Builder<T>::CompiledNode*>(arc.target)->node;
    h = PRIME * h + static_cast<int32_t>(n ^ (n >> 32));
    h = PRIME * h + arc.output.HashCode();
    h = PRIME * h + arc.next_final_output.HashCode();
    if (arc.is_final) {
      h += 17;
    }
  }

  return (h & std::numeric_limits<int64_t>::max());
}

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_FST_H_
