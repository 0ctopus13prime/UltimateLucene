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
#include <Util/Bytes.h>
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

class FSTBytesReader: public lucene::core::store::DataInput {
 public:
  virtual ~FSTBytesReader() = default;

  virtual uint64_t GetPosition() const noexcept = 0;

  virtual void SetPosition(const uint64_t pos) noexcept = 0;

  virtual bool Reversed() const noexcept = 0;
};  // FSTBytesReader

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
};  // ForwardFSTBytesReader

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
};  // ReverseFSTBytesReader

class BytesStoreReverseFSTBytesReader;
class BytesStoreForwardFSTBytesReader;

class BytesStore: public lucene::core::store::DataOutput {
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

  BytesStore& operator=(BytesStore&& other) {
    if (this != &other) {
      blocks = std::move(other.blocks);
      block_bits = other.block_bits;
      block_size = other.block_size;
      block_mask = other.block_mask;
      next_write = other.next_write;
      current = other.current;
    }

    return *this;
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

  std::unique_ptr<FSTBytesReader> GetForwardReader();

  std::unique_ptr<FSTBytesReader> GetReverseReader(const bool allow_single = true);
};  // BytesStore

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
};  // BytesStoreForwardFSTBytesReader

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
};  // BytesStoreReverseFSTBytesReader

template<typename T>
class Outputs {
 public:
  virtual ~Outputs() = default;

  virtual T Common(T& output1, T& output2) = 0;

  virtual T Subtract(T& output, T& inc) = 0;

  virtual T Add(T& prefix, T& output) = 0;

  virtual void Write(T& output, lucene::core::store::DataOutput* out) = 0;

  virtual void WriteFinalOutput(T& output,
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

  virtual uint32_t HashCode() {
    const uint32_t bottom =
      static_cast<uint32_t>(reinterpret_cast<uint64_t>(this));

    return bottom << 24 |
           (bottom << 8) & 0x00FF0000 |
           (bottom >> 8) & 0x0000FF00 |
           bottom >> 24;
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
      return NO_OUTPUT;
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

  IntsRef Subtract(IntsRef& output, IntsRef& inc) {
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

  void Write(IntsRef& prefix, lucene::core::store::DataOutput* out) {
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

class ByteSequenceOutputs: public Outputs<BytesRef> {
 private:
  static BytesRef NO_OUTPUT;

 public:
  BytesRef Common(BytesRef& output1, BytesRef& output2) {
    uint32_t pos1 = output1.offset;
    uint32_t pos2 = output2.offset;
    uint32_t stop_at_1 = (pos1 + std::min(output1.length, output2.length));

    while (pos1 < stop_at_1 && output1.bytes.get()[pos1] != output2.bytes.get()[pos2]) {
      pos1++;
      pos2++;
    }

    if (pos1 == output1.offset) {
      return NO_OUTPUT;
    } else if (pos1 == output1.offset + output1.length) {
      return output1;
    } else if (pos2 == output2.offset + output2.length) {
      return output2;
    } else {
      return BytesRef(output1.bytes.get(),
                      output1.offset,
                      pos1 - output1.offset);
    }
  }

  BytesRef Subtract(BytesRef& output, BytesRef& inc) {
    if (inc == NO_OUTPUT) {
      // No prefix removed
      return output;
    } else {
      if (inc.length == output.length) {
        // Entire output removed. Ex) output = ABC, inc = ABC
        return NO_OUTPUT;
      } else {
        return BytesRef(output.bytes.get(),
                        output.offset + inc.length,
                        output.length - inc.length);
      }
    }
  }

  BytesRef Add(BytesRef& prefix, BytesRef& output) {
    if (prefix == NO_OUTPUT) {
      return output;
    } else if (output == NO_OUTPUT) {
      return prefix;
    } else {
      BytesRef result(prefix.length + output.length);
      std::memcpy(result.bytes.get(), prefix.bytes.get() + prefix.offset, prefix.length);
      std::memcpy(result.bytes.get() + prefix.length,
                  output.bytes.get() + output.offset,
                  output.length);
      result.length = (prefix.length + output.length);
      return result;
    }
  }

  void Write(BytesRef& prefix, lucene::core::store::DataOutput* out) {
    out->WriteVInt32(prefix.length);
    out->WriteBytes(prefix.bytes.get(), prefix.offset, prefix.length);
  }

  BytesRef Read(lucene::core::store::DataInput* in) {
    const int32_t len = in->ReadVInt32();
    if (len != 0) {
      BytesRef output(len); 
      in->ReadBytes(output.bytes.get(), 0, len);
      output.length = len;
      return output;
    } else {
      return NO_OUTPUT; 
    }
  }

  void SkipOutput(lucene::core::store::DataInput* in) {
    const int32_t len = in->ReadVInt32();
    if (len != 0) {
      in->SkipBytes(len);
    }
  }

  BytesRef& GetNoOutput() {
    return NO_OUTPUT;
  }

  std::string OutputToString(BytesRef& output) {
    // TODO(0ctopus13prime): IT
    return std::string();
  }
};  // ByteSequenceOutputs

enum class FST_INPUT_TYPE {
  BYTE1, BYTE2, BYTE4
};

template <typename T>
class Builder;

template <typename T>
class NodeHash;

// template <typename T>
// class FST {
//  friend class Builder<T>;
//  friend class NodeHash<T>;
// 
//  public:
//   static const uint32_t BIT_FINAL_ARC = (1 << 0);
//   static const uint32_t BIT_LAST_ARC = (1 << 1);
//   static const uint32_t BIT_TARGET_NEXT = (1 << 2);
//   static const uint32_t BIT_STOP_NODE = (1 << 3);
//   static const uint32_t BIT_ARC_HAS_OUTPUT = (1 << 4);
//   static const uint32_t BIT_ARC_HAS_FINAL_OUTPUT = (1 << 5);
//   static const char ARCS_AS_FIXED_ARRAY = BIT_ARC_HAS_FINAL_OUTPUT;
//   static const uint32_t FIXED_ARRAY_SHALLOW_DISTANCE = 3;
//   static const uint32_t FIXED_ARRAY_NUM_ARCS_SHALLOW = 5;
//   static const uint32_t FIXED_ARRAY_NUM_ARCS_DEEP = 10;
//   static const uint32_t DEFAULT_MAX_BLOCK_BITS = 30;
// 
//  private:
//   static const std::string FILE_FORMAT_NAME;  // "FST"
//   static const uint32_t VERSION_START = 0;
//   static const uint32_t VERSION_INT_NUM_BYTES_PER_ARC = 1;
//   static const uint32_t VERSION_SHORT_BYTE2_LABELS = 2;
//   static const uint32_t VERSION_PACKED = 3;
//   static const uint32_t VERSION_VINT_TARGET = 4;
//   static const uint32_t VERSION_NO_NODE_ARC_COUNTS =5;
//   static const uint32_t VERSION_PACKED_REMOVED = 6;
//   static const uint32_t VERSION_CURRENT = VERSION_PACKED_REMOVED;
//   static const int64_t FINAL_END_NODE = -1;
//   static const int64_t NON_FINAL_END_NODE = 0;
//   static const int32_t END_LABEL = -1;
//   static const uint32_t MAX_BLOCK_BITS = 30;
// 
//  public:
//   class Arc;
// 
//   FST_INPUT_TYPE input_type;
//   T empty_output;
//   bool is_empty_output_empty;
//   BytesStore bytes;
//   bool is_bytes_empty;
//   std::unique_ptr<char[]> bytes_array;
//   uint32_t bytes_array_size;
//   int64_t start_node;
//   std::unique_ptr<Outputs<T>> outputs;
//   std::unordered_map<uint32_t, Arc> cached_root_arcs;
//   uint32_t version;
// 
//  private:
//   static bool Flag(const uint32_t flag, const uint32_t bit) noexcept {
//     return ((flag & bit) != 0);
//   }
// 
//  public:
//   FST(lucene::core::store::DataInput* in,
//       std::unique_ptr<Outputs<T>>&& outputs)
//     : FST(in, std::move(outputs), MAX_BLOCK_BITS) {
//   }
// 
//   FST(lucene::core::store::DataInput* in,
//       std::unique_ptr<Outputs<T>>&& outputs,
//       const uint32_t max_block_bits) {
//     is_empty_output_empty = true;
//     this->outputs = std::move(outputs);
// 
//     if (max_block_bits == 0 || max_block_bits > MAX_BLOCK_BITS) {
//       throw IllegalArgumentException("Max block bits should be 1 .. 30. Got " +
//                                      std::to_string(max_block_bits));
//     }
// 
//     version =
//       lucene::core::codec::CodecUtil::CheckHeader(in,
//                                                   FILE_FORMAT_NAME,
//                                                   VERSION_PACKED,
//                                                   VERSION_CURRENT);
//     if (version < VERSION_PACKED_REMOVED &&
//         in->ReadByte() == 1) {
//         throw lucene::core::index::CorruptIndexException(
//               "Cannot read packed FSTs anymore");
//     }
// 
//     if (in->ReadByte() == 1) {
//       // Accept empty string
//       // 1KB blocks
//       BytesStore empty_bytes(10);
//       const int32_t num_bytes = in->ReadVInt32();
//       empty_bytes.CopyBytes(in, num_bytes);
// 
//       // De-serialize empty-string output:
//       std::unique_ptr<FSTBytesReader> reader =
//         empty_bytes.GetReverseReader();
// 
//       // NoOutputs uses 0 bytes when writing its output,
//       // so we have to check here else BytesStore gets angry:
//       if (num_bytes > 0) {
//         reader->SetPosition(num_bytes - 1);
//       }
// 
//       empty_output = outputs->ReadFinalOutput(reader.get());
//       is_empty_output_empty = false;
//     }
// 
//     const char t = in->ReadByte();
//     switch (t) {
//       case 0:
//         input_type = FST_INPUT_TYPE::BYTE1;
//         break;
//       case 1:
//         input_type = FST_INPUT_TYPE::BYTE2;
//         break;
//       case 2:
//         input_type = FST_INPUT_TYPE::BYTE4;
//         break;
//       default:
//         throw IllegalStateException("Invalid input type " +
//                               std::to_string(static_cast<uint32_t>(t & 0xFF)));
//     }
// 
//     start_node = in->ReadVInt64();
//     if (version < VERSION_NO_NODE_ARC_COUNTS) {
//       in->ReadVInt64();
//       in->ReadVInt64();
//       in->ReadVInt64();
//     }
// 
//     const int64_t num_bytes = in->ReadVInt64();
//     is_bytes_empty = false;
//     if (num_bytes > (1 << max_block_bits)) {
//       // FST is big: We need to multiple pages
//       bytes = BytesStore(in, num_bytes, 1 << max_block_bits);
//       bytes_array.reset();
//       bytes_array_size = 0;
//     } else {
//       // FST file into a single block:
//       // Use ByteArrayBytesStoreReader for less overhead
//       bytes = BytesStore();
//       bytes_array_size = num_bytes;
//       bytes_array = std::make_unique<char[]>(bytes_array_size);
//       in->ReadBytes(bytes_array.get(), 0, bytes_array_size);
//     }
// 
//     CacheRootArcs();
//   }
// 
//   FST(const FST_INPUT_TYPE input_type, 
//       std::unique_ptr<Outputs<T>>&& outputs,
//       const uint32_t bytes_page_bits)
//     : input_type(input_type),
//       empty_output(),
//       is_empty_output_empty(true),
//       bytes(bytes_page_bits),
//       is_bytes_empty(false),
//       bytes_array(),
//       bytes_array_size(0),
//       start_node(-1),
//       outputs(std::move(outputs)),
//       cached_root_arcs(0x100),
//       version(VERSION_CURRENT) {
//   }
// 
//   FST_INPUT_TYPE GetInputType() const noexcept {
//     return input_type;
//   }
// 
//   T& GetEmptyOutput() const noexcept {
//     return empty_output;
//   }
// 
//   void Save(lucene::core::store::DataOutput* out) {
//     lucene::core::codec::CodecUtil::WriteHeader(out,
//                                                 FILE_FORMAT_NAME,
//                                                 VERSION_CURRENT);
//     if (!is_empty_output_empty) {
//       out->WriteByte(static_cast<char>(1));
//       // TODO(0ctopus13prime): Implement RAMOutputStream
//       /*
//       lucene::core::store::RAMOutputStream ros;
//       outputs->WriteFinalOutput(empty_output, &ros);
// 
//       const uint32_t empty_output_bytes_size = ros.GetFilePointer();
//       char empty_output_bytes[empty_output_bytes_size];
//       ros.WriteTo(empty_output_bytes, 0);
// 
//       // Reverse
//       const uint32_t stop_at = (empty_output_bytes_size >> 1);
//       for (uint32_t upto = 0 ; upto < stop_at ; ++upto) {
//         const char b = empty_output_bytes[upto];
//         empty_output_bytes[upto] =
//           empty_output_bytes[empty_output_bytes_size - upto - 1];
//         empty_output_bytes[empty_output_bytes_size - upto - 1] = b;
//       }
// 
//       out->WriteVInt32(empty_output_bytes_size);
//       out->WriteBytes(empty_output_bytes, 0, empty_output_bytes_size);
//       */
//     } else {
//       out->WriteByte(static_cast<char>(0));
//     }
// 
//     switch (input_type) {
//       case FST_INPUT_TYPE::BYTE1:
//         out->WriteByte(0);
//         break;
//       case FST_INPUT_TYPE::BYTE2:
//         out->WriteByte(1);
//         break;
//       default:
//         out->WriteByte(2);
//         break;
//     }
// 
//     out->WriteVInt64(start_node);
//     if (!is_bytes_empty) {
//       out->WriteVInt64(bytes.GetPosition());
//       bytes.WriteTo(out);
//     } else {
//       assert(bytes_array);
//       out->WriteVInt64(bytes_array_size);
//       out->WriteBytes(bytes_array.get(), 0, bytes_array_size);
//     }
//   }
// 
//   void Save(const std::string& path) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   void GetFirstArc(Arc& arc) {
//     T NO_OUTPUT = outputs->GetNoOutput(); 
// 
//     if (!is_empty_output_empty) {
//       arc.flags = (BIT_FINAL_ARC | BIT_LAST_ARC);
//       arc.next_final_output = empty_output;
//       if (&empty_output != &NO_OUTPUT) {
//         arc.flags |= BIT_ARC_HAS_FINAL_OUTPUT; 
//       }
//     } else {
//       arc.floags = BIT_LAST_ARC;
//       arc.next_final_output = NO_OUTPUT;
//     }
//     arc.output = NO_OUTPUT;
// 
//     arc.output = NO_OUTPUT;
// 
//     // If there are no nodes, ie, the FST only accepts the
//     // empty string, then startNode is 0
//     arc.target = start_node;
//   }
// 
//   void ReadLastTargetArc(Arc& follow,
//                          Arc& arc,
//                          FSTBytesReader& in) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   void ReadFirstTargetArc(Arc& follow, Arc& arc, FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   void ReadFirstRealTargetArc(const int64_t node,
//                               Arc& arc,
//                               FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//   }
//   
//   void ReadNextArc(Arc& arc, FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   int32_t ReadNextArcLabel(Arc& arc, FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//     return 0;
//   }
// 
//   void ReadNextRealArc(Arc& arc, FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   void FindTargetArc(const int32_t label_to_match,
//                      Arc& follow,
//                      Arc& arc,
//                      FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   std::unique_ptr<FSTBytesReader> GetBytesReader() {
//     // TODO(0ctopus13prime): IT
//     return {};
//   }
// 
//  public:
//   static FST<T> Read(const std::string& path,
//                      std::unique_ptr<Outputs<T>> outputs) {
//     // TODO(0ctopus13prime): IT
//     return FST<T>(nullptr, std::unique_ptr<Outputs<T>>());
//   }
// 
//   static bool TargetHasArcs(Arc& arc) {
//     // TODO(0ctopus13prime): IT
//     return true;
//   }
// 
//  private:
//   void FindTargetArc(const int32_t label_to_match,
//                      Arc& follow,
//                      Arc& arc,
//                      FSTBytesReader* in,
//                      const bool use_root_arc_cache) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   void SeekToNextNode(FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   bool ShouldExpand(Builder<T>& builder, typename Builder<T>::UnCompiledNode& node) {
//     // TODO(0ctopus13prime): IT
//     return true;
//   }
// 
//   bool AssertRootCachedArc(const int32_t label, Arc& cached_arc) {
//     // TODO(0ctopus13prime): IT
//     return true;
//   }
// 
//   bool IsExpandedTarget(Arc& follow, FSTBytesReader* in) {
//     // TODO(0ctopus13prime): IT
//     return true;
//   }
// 
//   int64_t ReadUnpackedNodeTarget(FSTBytesReader& in) {
//     // TODO(0ctopus13prime): IT
//     return 0L;
//   }
// 
//   int64_t AddNode(Builder<T>& builder, typename Builder<T>::UnCompiledNode node_in) {
//     // TODO(0ctopus13prime): IT
//     return 0L;
//   }
// 
//   void WriteLabel(lucene::core::store::DataOutput* out, const int32_t v) {
//     // TODO(0ctopus13prime): IT
//   }
// 
//   int32_t ReadLabel(lucene::core::store::DataInput* in) {
//     // TODO(0ctopus13prime): IT
//     return 0;
//   }
// 
//   void SetEmptyOutput(T& v) {
//     if (is_empty_output_empty) {
//       empty_output = v;
//     } else {
//       outputs->Merge(empty_output, v);
//     }
//   }
// 
//   void CacheRootArcs() {
//     Arc arc;
//     GetFirstArc(arc);
//     if (TargetHasArcs(arc)) {
//       std::unique_ptr<FSTBytesReader> in = GetBytesReader(); 
//       std::unordered_map<uint32_t, Arc> arcs(0x100);
//       ReadFirstRealTargetArc(arc.target, arc, in.get());
//       uint32_t count = 0;
//       cached_root_arcs.clear();
//       while (true) {
//         assert(arc.label != END_LABEL);
//         if (arc.label < 0x80) {
//           arcs[arc.label] = arc;
//         } else {
//           break;
//         }
// 
//         if (arc.IsLast()) {
//           break;
//         }
// 
//         ReadNextRealArc(arc, in.get());
//         count++;
//       }  // End while
// 
//       if (count >= FIXED_ARRAY_NUM_ARCS_SHALLOW) {
//         cached_root_arcs = std::move(arcs); 
//       }
//     }  // End if
//   }
// };  // FST

template <typename T>
class FST {
 friend class Builder<T>;
 friend class NodeHash<T>;

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
    : FST(in, std::forward<Outputs<T>>(outputs), MAX_BLOCK_BITS) {
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
      outputs(std::forward<std::unique_ptr<Outputs<T>>>(outputs)),
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

  bool AcceptEmptyOutput() {
    // TODO(0ctopus13prime): IT
    return true;
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

/**
 *  BytesStore
 */
std::unique_ptr<FSTBytesReader> BytesStore::GetForwardReader() {
  if (blocks.size() == 1) {
    return std::make_unique<ForwardFSTBytesReader>(blocks[0].first.get());
  } else {
    return std::make_unique<BytesStoreForwardFSTBytesReader>(this);
  }
}

std::unique_ptr<FSTBytesReader> BytesStore::GetReverseReader(const bool allow_single/*= true*/) {
  if (allow_single && block_size == 1) {
    return std::make_unique<ReverseFSTBytesReader>(blocks[0].first.get());
  } else {
    return std::make_unique<BytesStoreReverseFSTBytesReader>(this);
  }
}

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

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_FST_H_
