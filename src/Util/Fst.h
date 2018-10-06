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

// TEST
#include <iostream>
// TEST

#include <Codec/CodecUtil.h>
#include <Store/DataInput.h>
#include <Store/DataOutput.h>
#include <Util/ArrayUtil.h>
#include <Util/Ref.h>
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

class FstBytesReader: public lucene::core::store::DataInput {
 public:
  virtual ~FstBytesReader() = default;

  virtual uint64_t GetPosition() const noexcept = 0;

  virtual void SetPosition(const uint64_t pos) noexcept = 0;

  virtual bool Reversed() const noexcept = 0;
};  // FstBytesReader

class ForwardFstBytesReader: public FstBytesReader {
 private:
  char* bytes;
  uint32_t pos;

 public:
  ForwardFstBytesReader(char* bytes)
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
};  // ForwardFstBytesReader

class ReverseFstBytesReader: public FstBytesReader {
 private:
  const char* bytes;
  uint32_t pos;

 public:
  ReverseFstBytesReader(const char* bytes)
    : bytes(bytes),
      pos(0) {
  }

  char ReadByte() {
    return bytes[pos--];
  }

  void ReadBytes(char b[],
                 const uint32_t offset,
                 const uint32_t len) {
    char* base = (b + offset);
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
};  // ReverseFstBytesReader

class BytesStoreReverseFstBytesReader;
class BytesStoreForwardFstBytesReader;

class BytesStore: public lucene::core::store::DataOutput {
 private:
  friend class BytesStoreReverseFstBytesReader;
  friend class BytesStoreForwardFstBytesReader;

  std::vector<std::pair<std::unique_ptr<char[]>, uint32_t>> blocks;
  uint32_t block_bits;
  uint32_t block_size;
  uint32_t block_mask;
  uint32_t next_write;
  char* current;

 private:
  void NewBlock() {
    std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
      std::make_unique<char[]>(block_size), block_size);
    current = block_pair.first.get();
    blocks.push_back(std::move(block_pair));
    next_write = 0;
  }

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

    // std::cout << "BytesStore()"
    //           << ", block_bits -> " << block_bits << std::endl;
  }

  BytesStore(std::unique_ptr<lucene::core::store::DataInput>&& in,
             const uint64_t num_bytes,
             const uint32_t max_block_size) {
    uint32_t tmp_block_size = 2;
    uint32_t tmp_block_bits = 1;

    while (tmp_block_size < num_bytes && tmp_block_size < max_block_size) {
      tmp_block_size <<= 1;
      ++tmp_block_bits;
    }

    block_bits = tmp_block_bits;
    block_size = tmp_block_size;
    block_mask = (tmp_block_size - 1);
    uint64_t left = num_bytes;
    while (left > 0) {
      const uint32_t chunk = std::min(block_size, static_cast<uint32_t>(left));
      std::unique_ptr<char[]> block(std::make_unique<char[]>(chunk));
      in->ReadBytes(block.get(), 0, chunk);
      blocks.push_back(std::pair<std::unique_ptr<char[]>, uint32_t>(
                       std::move(block), chunk));
      left -= chunk;
    }

    // std::cout << "BytesStore()"
    //           << ", num_bytes -> " << num_bytes
    //           << ", max_block_size -> " << max_block_size
    //           << std::endl;

    next_write = blocks.back().second;
    current = nullptr;
  }

  void WriteByte(const uint32_t dest, const char b) {
    // std::cout << "ByteStore, WriteByte, dest - " << dest
    //           << ", b - " << static_cast<int>(b) << std::endl;
    blocks[dest >> block_bits].first[dest & block_mask] = b;
  }

  void WriteByte(const char b) {
    // std::cout << "ByteStore, WriteByte, b - " << static_cast<int>(b) << std::endl;
    if (next_write == block_size) {
      NewBlock();
    }

    current[next_write++] = b;
  }

  void WriteBytes(const char bytes[],
                  const uint32_t offset,
                  const uint32_t len) {
    // std::cout << "ByteStore, WriteBytes, bytes - " << reinterpret_cast<long>(bytes)
    //           << ", offset - " << offset
    //           << ", len - " << len << std::endl;

    uint32_t offset_until = offset;
    uint32_t len_until = len;
    
    // Fill the last block in blocks
    const uint32_t last_block_to_fill = std::min(block_size - next_write, len);
    std::memcpy(current + next_write, bytes + offset, last_block_to_fill);
    offset_until += last_block_to_fill;
    len_until -= last_block_to_fill;
    next_write += last_block_to_fill;

    // Fill left bytes
    while (len_until > 0) {
      NewBlock();
      const uint32_t chunk = std::min(len_until, block_size);

      std::memcpy(current, bytes + offset_until, chunk);
      offset_until += chunk;
      len_until -= chunk;
      next_write += chunk;
    }
  }

  uint32_t GetBlockBits() const noexcept {
    return block_bits;
  }

  void WriteBytes(const uint64_t dest,
                  const char bytes[],
                  const uint32_t offset,
                  const uint32_t len) {
    // std::cout << "ByteStore, WriteBytes, dest - " << dest
    //           << ", bytes - " << reinterpret_cast<long>(bytes)
    //           << ", offset - " << offset
    //           << ", len - " << len << std::endl;
    assert(dest + len <= GetPosition());

    uint32_t offset_tmp = offset;
    uint32_t len_tmp = len;
    uint32_t block_index = static_cast<uint32_t>(dest >> block_bits);
    uint32_t upto = static_cast<uint32_t>(dest & block_mask);

    // Fill the last block
    const uint32_t last_block_to_fill = std::min(block_size - upto, len);
    char* block = blocks[block_index].first.get();
    std::memcpy(block + upto, bytes + offset, last_block_to_fill);
    offset_tmp += last_block_to_fill;
    len_tmp -= last_block_to_fill;

    // Fill left bytes
    while (len_tmp > 0) {
      block = blocks[++block_index].first.get();
      upto = 0;
      const uint32_t chunk = std::min(block_size, len_tmp);
      std::memcpy(block, bytes + offset_tmp, chunk);
      offset_tmp += chunk;
      len_tmp -= chunk;
      upto += chunk;
    }
  }

  using lucene::core::store::DataOutput::CopyBytes;
  void CopyBytes(const uint64_t src, const uint64_t dest, const uint32_t len) {
    // std::cout << "ByteStore, CopyBytes, src - " << src
    //           << ", dest - " << dest
    //           << ", len - " << len
    //           << std::endl;
    assert(src < dest);
    // We have to copy from rear to front to prevent data corruption
    // Because copy range would overlap occationally
    // For example, src = 0, dest 20, len 30
    // src <-------->
    //           <--------> end

    // Original comment
    // Note: weird: must go "backwards" because copyBytes
    // calls us with overlapping src/dest.  If we
    // go forwards then we overwrite bytes before we can
    // copy them:

    // Prepare
    const uint32_t BUF_SIZE = 2048;
    char buf[BUF_SIZE];
    const uint32_t src_end_pos = (src + len);
    uint32_t src_blk_idx = static_cast<uint32_t>(src_end_pos >> block_bits);
    const uint32_t src_end = static_cast<uint32_t>(src_end_pos & block_mask);
    char* src_block = blocks[src_blk_idx].first.get();

    // Copy residual bytes in last block
    // When len < src_end
    // Block <------------------------->
    //           ^          ^       ^
    //        src_start   c_idx  src_end
    //           <------------------>
    //                 first_drop
    //
    // OR
    // When len >= src_end
    // Block <------------------------->
    //        ^             ^       ^
    //    src_start       c_idx  src_end
    //        <--------------------->
    //                 first_drop

    const uint32_t first_drop = std::min(src_end, len);
    uint32_t dest_tmp = (dest + len);
    uint32_t len_tmp = (len - first_drop);
    for (uint32_t src_start = src_end - first_drop, cidx = src_end ; cidx > src_start ; ) {
      const uint32_t copy_len = std::min(cidx - src_start, BUF_SIZE);
      std::memcpy(buf, src_block + cidx - copy_len, copy_len);
      dest_tmp -= copy_len;
      WriteBytes(dest_tmp, buf, 0, copy_len);
      cidx -= copy_len;
    }

    // Copy left bytes
    while (len_tmp > 0) {
      src_block = blocks[--src_blk_idx].first.get();  
      const uint32_t chunk = std::min(block_size, len_tmp);
      // When len_tmp < block_size
      // Block <------------------------->
      //           ^          ^         ^
      //        src_start   c_idx   block_size 
      //           <------------------>
      //                   chunk
      //
      // OR
      //
      // When len_tmp >= block_size
      // Block <------------------------->
      //        ^             ^         ^
      //    src_start       c_idx   block_size
      //        <--------------------->
      //                 chunk

      dest_tmp = (dest + len_tmp);
      len_tmp -= chunk;
      for (uint32_t src_start = block_size - chunk, cidx = block_size ; cidx > src_start ; ) {
        const uint32_t copy_len = std::min(cidx - src_start, BUF_SIZE);
        std::memcpy(buf, src_block + cidx - copy_len, copy_len);
        dest_tmp -= copy_len;
        WriteBytes(dest_tmp, buf, 0, copy_len);
        cidx -= copy_len;
      }
    }

    /*
    uint64_t end = (src + len);
    uint32_t block_index = static_cast<uint32_t>(end >> block_bits);
    uint32_t down_to = static_cast<uint32_t>(end & block_mask);
    if (down_to == 0) {
      block_index--;
      down_to = block_size;
    }

    char* block = blocks[block_index].first.get();
    uint32_t len_tmp = len;

    while (len > 0) {
      if (len <= down_to) {
        WriteBytes(dest, block, down_to - len_tmp , len_tmp);
        break;
      } else {
        len_tmp -= down_to;
        WriteBytes(dest + len_tmp, block, 0, down_to);
        block_index--;
        block = blocks[block_index].first.get();
        down_to = block_size;
      }
    }
    */
  }

  void WriteInt32(const uint64_t pos, const uint32_t value) {
    // std::cout << "BytesStore, WriteInt32 pos - " << pos
    //           << ", value - " << value << std::endl;
    uint32_t block_index = static_cast<uint32_t>(pos >> block_bits);
    uint32_t upto = static_cast<uint32_t>(pos & block_mask);
    char* block = blocks[block_index].first.get();
    uint32_t shift = 24;
    for (int i = 0 ; i < 4 ; ++i) {
      block[upto++] = static_cast<char>(value >> shift);
      shift -= 8;
      if (upto == block_size) {
        upto = 0;
        block = blocks[++block_index].first.get();
      }
    }
  }

  void Reverse(const uint64_t src_pos, const uint64_t dest_pos) {
    // std::cout << "BytesStore, Reverse src_pos - " << src_pos
    //           << ", dest_pos - " << dest_pos << std::endl;
    assert(src_pos < dest_pos);
    assert(dest_pos < GetPosition());
    
    // Src prepare
    uint32_t src_block_index = static_cast<uint32_t>(src_pos >> block_bits);
    uint32_t src = static_cast<uint32_t>(src_pos & block_mask);
    char* src_block = blocks[src_block_index].first.get();

    // Dest prepare
    uint32_t dest_block_index = static_cast<uint32_t>(dest_pos >> block_bits);
    uint32_t dest = static_cast<uint32_t>(dest_pos & block_mask);
    char* dest_block = blocks[dest_block_index].first.get();

    // std::cout << "src - " << src << ", dest - " << dest
    //           << ", block_bits - " << block_bits << std::endl;

    // Reverse 
    uint32_t limit = static_cast<uint32_t>(dest_pos - src_pos + 1) >> 1;
    for (uint32_t i = 0 ; i < limit ; ++i) {
      // std::cout << "Swaping .. src - " << src
      //           << ", dest - " << dest << std::endl;
      std::swap(src_block[src], dest_block[dest]);
      if (++src == block_size) {
        src_block = blocks[++src_block_index].first.get();
        src = 0;
      }

      if (dest != 0) {
        --dest;
      } else {
        dest_block = blocks[--dest_block_index].first.get();
        dest = (block_size - 1);
      }
    }
  }

  void SkipBytes(const uint32_t len) {
    // std::cout << "BytesStore, SkipBytes len - " << len << std::endl;
    uint32_t len_tmp = len;
    
    // Skip the last block
    const uint32_t last_block_to_skip = std::min(block_size - next_write, len);
    len_tmp -= last_block_to_skip;
    next_write += last_block_to_skip;

    // Skip left parts
    if (len_tmp > 0) {
      const uint32_t num_blocks_to_need = ((len_tmp / block_size) + 1);
      for (uint32_t i = 0 ; i < num_blocks_to_need ; ++i) {
        NewBlock();
      }
      next_write = (len_tmp % block_size);
    }
  }

  uint64_t GetPosition() {
    // std::cout << "GetPosition, block's size -> " << blocks.size()
    //           << ", block_size -> " << block_size
    //           << ", next_write -> " << next_write << std::endl;
    return static_cast<uint64_t>((blocks.size() - 1) * block_size + next_write);
  }

  void Truncate(const uint64_t new_len) {
    // std::cout << "BytesStore, Truncat, new_len - " << new_len << std::endl;
    assert(new_len <= GetPosition());

    if (new_len != 0) {
      uint32_t block_index = static_cast<uint32_t>(new_len >> block_bits);
      next_write = static_cast<uint32_t>(new_len & block_mask);

      blocks.erase(blocks.begin() + block_index + 1, blocks.end());
      current = blocks[block_index].first.get();
    } else {
      current = nullptr;
      next_write = (block_size - 1);
      blocks.clear();
    }

    assert(new_len == GetPosition());
  }

  void Finish() {
    if (current != nullptr) {
      // Make current buffer compact so that no redundant space in final bytes
      std::unique_ptr<char[]> last_buffer(std::make_unique<char[]>(next_write));
      std::memcpy(last_buffer.get(), current, next_write);
      std::pair<std::unique_ptr<char[]>, uint32_t> block_pair(
        std::move(last_buffer), block_size);
      blocks.back() = std::move(block_pair);
      current = nullptr;
    }
  }

  void WriteTo(lucene::core::store::DataOutput* out) {
    // std::cout << "BytesStore, WriteTo out - " << reinterpret_cast<long>(out) << std::endl;
    for (auto& block_pair : blocks) {
      out->WriteBytes(block_pair.first.get(), 0, block_pair.second);
    }
  }

  std::unique_ptr<FstBytesReader> GetForwardReader();

  std::unique_ptr<FstBytesReader> GetReverseReader(const bool allow_single = true);
};  // BytesStore

class BytesStoreForwardFstBytesReader: public FstBytesReader {
 private:
  BytesStore* store;
  char* current;
  uint32_t next_buffer;
  uint32_t next_read;

 private:
  void NextBlock();

 public:
  BytesStoreForwardFstBytesReader(BytesStore* store);

  char ReadByte();

  void ReadBytes(char bytes[], const uint32_t offset, const uint32_t len);

  void SkipBytes(const uint64_t count);

  uint64_t GetPosition() const noexcept;

  void SetPosition(const uint64_t pos) noexcept;

  bool Reversed() const noexcept;
};  // BytesStoreForwardFstBytesReader

class BytesStoreReverseFstBytesReader: public FstBytesReader {
 private:
  BytesStore* store; 
  char* current;
  int32_t next_buffer;
  int32_t next_read;

 public:
  explicit BytesStoreReverseFstBytesReader(BytesStore* store);

  char ReadByte();

  void ReadBytes(char bytes[], const uint32_t offset, const uint32_t len);

  void SkipBytes(const uint64_t count);

  uint64_t GetPosition() const noexcept;

  void SetPosition(const uint64_t new_pos) noexcept;

  bool Reversed() const noexcept;
};  // BytesStoreReverseFstBytesReader

template<typename T>
class Outputs {
 public:
  virtual ~Outputs() = default;

  virtual bool IsNoOutput(const T& output) = 0;

  virtual void MakeNoOutput(T& output) = 0;

  virtual uint32_t PrefixLen(const T& output1, const T& output2) = 0;

  virtual void Prepend(const T& prefix, T& output) = 0;

  virtual void ShiftLeftSuffix(T& output, const uint32_t prefix_len) = 0;

  virtual void DropSuffix(T& output, const uint32_t prefix_len) = 0;

  virtual T PrefixReference(T& output, const uint32_t prefix_len) = 0;

  virtual T SuffixReference(T& output, const uint32_t prefix_len) = 0;

  virtual void Write(const T& output, lucene::core::store::DataOutput* out) = 0;

  virtual void WriteFinalOutput(const T& output,
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

  virtual std::string OutputToString(const T& output) = 0;

  virtual T Merge(const T& first, const T& second) {
    throw lucene::core::util::UnsupportedOperationException();
  }
};  // Outputs<T>

class IntSequenceOutputs: public Outputs<IntsRef> {
 public:
  bool IsNoOutput(const IntsRef& output) {
    return (output.Ints() == nullptr);
  }

  void MakeNoOutput(IntsRef& output) {
    output.SafeDeleteInts();
  }

  uint32_t PrefixLen(const IntsRef& output1, const IntsRef& output2) {
    const int32_t* pos1 = (output1.Ints() + output1.Offset()); 
    const int32_t* pos2 = (output2.Ints() + output2.Offset()); 
    uint32_t cmp_len = std::min(output1.Length(), output2.Length());
    uint32_t common = 0;
    
    for ( ; (common < cmp_len && *pos1++ == *pos2++) ; ++common);

    return common;
  }

  void Prepend(const IntsRef& prefix, IntsRef& output) {
    if (prefix.Length() > 0) {
      // Grow first
      std::pair<int32_t*, uint32_t> pair =
        ArrayUtil::Grow(output.Ints(),
                        output.Capacity(), 
                        output.Offset() + output.Length() + prefix.Length());
      if (pair.first != output.Ints()) {
        if (output.Ints() != nullptr) {
          delete[] output.Ints();
        }
        output.SetInts(pair.first)
              .SetCapacity(pair.second);
      }

      // Shift back
      int32_t* base = (output.Ints() + output.Offset());
      int32_t* ptr = (base + output.Length());

      for ( ; ptr != base ; --ptr) {
        ptr[prefix.Length() - 1] = ptr[-1];
      }

      // Prepend
      ptr = (prefix.Ints() + prefix.Offset());
      for (uint32_t i = 0 ; i < prefix.Length() ; ++i) {
        base[i] = ptr[i];
      }

      output.SetLength(output.Length() + prefix.Length());
    }
  }

  void ShiftLeftSuffix(IntsRef& output, const uint32_t prefix_len) {
    if (prefix_len > 0) {
      if (prefix_len >= output.Length()) {
        MakeNoOutput(output); 
      } else {
        int32_t* ptr = (output.Ints() + output.Offset() + prefix_len);
        const uint32_t new_length = (output.Length() - prefix_len);
        
        for (uint32_t i = 0 ; i < new_length ; ++i) {
          *(ptr - prefix_len) = *ptr;
          ptr++;
        }

        output.SetLength(new_length);
      }
    }
  }

  IntsRef PrefixReference(IntsRef& output, const uint32_t prefix_len) {
    return IntsRef(output.Ints(),
                   output.Offset(),
                   std::min(output.Length(), prefix_len),
                   output.Capacity());
  }

  IntsRef SuffixReference(IntsRef& output, const uint32_t prefix_len) {
    if (output.Length() > prefix_len) {
      return IntsRef(output.Ints(),
                     output.Offset() + prefix_len,
                     output.Length() - prefix_len,
                     output.Capacity());
    } else {
      return IntsRef(output.Ints(),
                     output.Offset() + prefix_len,
                     0,
                     output.Capacity());
    }
  }

  void DropSuffix(IntsRef& output, const uint32_t prefix_len) {
    if (prefix_len == 0) {
      MakeNoOutput(output);
    } else {
      output.SetLength(std::min(output.Length(), prefix_len));
    }
  }

  void Write(const IntsRef& prefix, lucene::core::store::DataOutput* out) {
    assert(!IsNoOutput(prefix));
    out->WriteVInt32(prefix.Length());
    for (uint32_t idx = 0 ; idx < prefix.Length() ; ++idx) {
      out->WriteVInt32(prefix.Ints()[prefix.Offset() + idx]);
    }
  }

  IntsRef Read(lucene::core::store::DataInput* in) {
    const int32_t len = in->ReadVInt32();
    if (len == 0) {
      return IntsRef();
    } else {
      // std::cout << "IntsSequenceOutputs, Reading len - " << len << std::endl;
      IntsRef output(IntsRef::MakeOwner(len));
      for (uint32_t idx = 0 ; idx < len ; ++idx) {
        // std::cout << "Reading idx - " << idx << std::endl;
        output.Ints()[idx] = in->ReadVInt32();
      }
      output.SetLength(len);
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

  std::string OutputToString(const IntsRef& output) {
    // TODO(0ctopus13prime): IT
    return std::string();
  }
};  // IntSequenceOutputs

class ByteSequenceOutputs: public Outputs<BytesRef> {
 private:
  static BytesRef NO_OUTPUT;

 public:
  BytesRef Common(const BytesRef& output1, const BytesRef& output2) {
  /*
    uint32_t pos1 = output1.offset;
    uint32_t pos2 = output2.offset;
    uint32_t stop_at_1 = (pos1 + std::min(output1.length, output2.length));

    while (pos1 < stop_at_1 &&
           output1.bytes.get()[pos1] != output2.bytes.get()[pos2]) {
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
  */
    // TODO(0ctopus13prime): IT
    return BytesRef();
  }

  BytesRef Subtract(const BytesRef& output, const BytesRef& inc) {
  /*
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
  */
    // TODO(0ctopus13prime): IT
    return BytesRef();
  }

  BytesRef Add(const BytesRef& prefix, const BytesRef& output) {
  /*
    if (prefix == NO_OUTPUT) {
      return NO_OUTPUT;
    } else if (output == NO_OUTPUT) {
      return NO_OUTPUT;
    } else {
      BytesRef result(prefix.length + output.length);
      std::memcpy(result.bytes.get(),
                  prefix.bytes.get() + prefix.offset,
                  prefix.length);
      std::memcpy(result.bytes.get() + prefix.length,
                  output.bytes.get() + output.offset,
                  output.length);
      result.length = (prefix.length + output.length);
      return result;
    }
  */
    // TODO(0ctopus13prime): IT
    return BytesRef();
  }

  void Write(const BytesRef& prefix, lucene::core::store::DataOutput* out) {
  /*
    out->WriteVInt32(prefix.length);
    out->WriteBytes(prefix.bytes.get(), prefix.offset, prefix.length);
  */
    // TODO(0ctopus13prime): IT
    return;
  }

  BytesRef Read(lucene::core::store::DataInput* in) {
  /*
    const int32_t len = in->ReadVInt32();
    if (len != 0) {
      BytesRef output(len); 
      in->ReadBytes(output.bytes.get(), 0, len);
      output.length = len;
      return output;
    } else {
      return NO_OUTPUT;
    }
  */
    // TODO(0ctopus13prime): IT
    return BytesRef();
  }

  void SkipOutput(lucene::core::store::DataInput* in) {
  /*
    const int32_t len = in->ReadVInt32();
    if (len != 0) {
      in->SkipBytes(len);
    }
  */
    // TODO(0ctopus13prime): IT
    return;
  }

  std::string OutputToString(const BytesRef& output) {
    // TODO(0ctopus13prime): IT
    return std::string();
  }
};  // ByteSequenceOutputs

enum class FST_INPUT_TYPE {
  BYTE1, BYTE2, BYTE4
};

template <typename T>
class FstBuilder;

template <typename T>
class NodeHash;

template <typename T>
class Fst {
 friend class FstBuilder<T>;
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
  Fst(const Fst&) = delete;
  Fst& operator=(const Fst&) = delete;

  Fst(Fst&& other)
    : input_type(other.input_type),
      empty_output(other.empty_output),
      is_empty_output_empty(other.is_empty_output_empty),
      bytes(std::move(other.bytes)),
      is_bytes_empty(other.is_bytes_empty),
      bytes_array(std::move(other.bytes_array)),
      bytes_array_size(other.bytes_array_size),
      start_node(other.start_node),
      outputs(std::move(other.outputs)),
      cached_root_arcs(std::move(other.cached_root_arcs)),
      version(other.version) {
  }

  Fst& operator=(Fst&& other) {
      if (this != &other) {
        input_type = other.input_type;
        empty_output = other.empty_output;
        is_empty_output_empty = other.is_empty_output_empty;
        bytes = std::move(other.bytes);
        is_bytes_empty = other.is_bytes_empty;
        bytes_array = std::move(other.bytes_array);
        bytes_array_size = other.bytes_array_size;
        start_node = other.start_node;
        outputs = std::move(outputs);
        cached_root_arcs = std::move(other.cached_root_arcs);
        version = other.version;
      }

      return *this;
  }

  Fst(lucene::core::store::DataInput* in,
      std::unique_ptr<Outputs<T>>&& outputs)
    : Fst(in, std::forward<Outputs<T>>(outputs), MAX_BLOCK_BITS) {
  }

  Fst(lucene::core::store::DataInput* in,
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
      std::unique_ptr<FstBytesReader> reader =
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

  Fst(const FST_INPUT_TYPE input_type, 
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
    // Pad: Ensure no node gets address 0 which is reserved to mean
    // the stop state w/ no arcs
    bytes.WriteByte(0);
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
      // Accept empty string
      out->WriteByte(static_cast<char>(1));

      // Serialize empty-string output
      lucene::core::store::GrowableByteArrayOutputStream bos;
      outputs->WriteFinalOutput(empty_output, &bos);

      const uint32_t empty_output_bytes_size = bos.Size();
      // Avoid overflow stack limit
      std::unique_ptr<char[]> empty_output_bytes = 
        std::make_unique<char[]>(empty_output_bytes_size);

      bos.WriteTo(empty_output_bytes.get());

      // Free space
      bos.~GrowableByteArrayOutputStream();

      // Reverse
      const uint32_t stop_at = (empty_output_bytes_size >> 1);
      for (uint32_t upto = 0 ; upto < stop_at ; ++upto) {
        const char b = empty_output_bytes[upto];
        empty_output_bytes[upto] =
          empty_output_bytes[empty_output_bytes_size - upto - 1];
        empty_output_bytes[empty_output_bytes_size - upto - 1] = b;
      }

      out->WriteVInt32(empty_output_bytes_size);
      out->WriteBytes(empty_output_bytes.get(), 0, empty_output_bytes_size);
    } else {
      out->WriteByte(0);
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
    lucene::core::store::BufferedFileOutputStream out(path);
    Save(&out);
  }

  Arc GetFirstArc(Arc& arc) {
    // std::cout << "GetFirstArc" << std::endl;
    if (!is_empty_output_empty) {
      arc.flags = (BIT_FINAL_ARC | BIT_LAST_ARC);
      arc.next_final_output = empty_output;
      if (!outputs.IsNoOutput(empty_output)) {
        arc.flags |= BIT_ARC_HAS_FINAL_OUTPUT; 
      }
    } else {
      arc.flags = BIT_LAST_ARC;
      outputs.MakeNoOutput(arc.next_final_output);
    }
    outputs.MakeNoOutput(arc.output);

    // If there are no nodes, ie, the FST only accepts the
    // empty string, then startNode is 0
    arc.target = start_node;
  }

  Arc ReadLastTargetArc(Arc& follow,
                        Arc& arc,
                        FstBytesReader* in) {
    if (!TargetHasArcs(follow)) {
      assert(follow.IsFinal());
      arc.label = END_LABEL;
      arc.target = FINAL_END_NODE;
      arc.output = follow.next_final_output;
      arc.flags = BIT_LAST_ARC;
    } else {
      in->SetPosition(follow.target);
      const char b = in->ReadByte();

      if (b == ARCS_AS_FIXED_ARRAY) {
        // Array: jump straight to end
        arc.num_arcs = in->ReadVInt32();
        if (version >= VERSION_VINT_TARGET) {
          arc.bytes_per_arc = in->ReadVInt32();
        } else {
          arc.bytes_per_arc = in->ReadInt32();
        }
        arc.pos_arc_start = in->GetPosition();
        arc.arc_idx = (arc.num_arcs - 2);
      } else {
        arc.flags = b;
        // Non-array: Linear scan
        arc.byts_per_arc = 0;

        while (!arc.IsLast()) {
          // Skip this arc:
          ReadLabel(in);
          if (arc.Flag(BIT_ARC_HAS_OUTPUT)) {
            outputs->SkipOutput(in);
          }
          if (arc.Flag(BIT_ARC_HAS_FINAL_OUTPUT)) {
            outputs->SkipFinalOutput(in);
          }
          if (!arc.Flag(BIT_STOP_NODE) && !arc.Flag(BIT_TARGET_NEXT)) {
            ReadUnpackedNodeTarget(in);
          }

          arc.flags = in->ReadByte();
        }

        // Undo the byte flags we read:
        in->SkipBytes(-1);
        arc.next_arc = in->GetPosition();
      }

      ReadNextRealArc(arc, in);
      assert(arc.IsLast());
      return arc;
    }
  }

  Arc ReadFirstTargetArc(Arc& follow, Arc& arc, FstBytesReader* in) {
    if (follow.IsFinal()) {
      arc.label = END_LABEL;
      arc.output = follow.next_final_output;
      arc.flags = BIT_FINAL_ARC;
      if (follow.target <= 0) {
        arc.flags |= BIT_LAST_ARC;
      } else {
        arc.next_arc = follow.target;
      }

      arc.target = FINAL_END_NODE;
      return arc;
    } else {
      return ReadFirstRealTargetArc(follow.target, arc, in);
    }
  }

  Arc ReadFirstRealTargetArc(const int64_t node,
                             Arc& arc,
                             FstBytesReader* in) {
    const int64_t address = node;
    // std::cout << "ReadFirstRealTargetArc, set position - " << address << std::endl;
    in->SetPosition(address);

    if (in->ReadByte() == ARCS_AS_FIXED_ARRAY) {
      // std::cout << "ReadFirstRealTargetArc, It's a fixed array" << std::endl;
      // This is first arc in a fixed-array
      arc.num_arcs = in->ReadVInt32();
      // std::cout << "Num arcs -> " << arc.num_arcs << std::endl;
      if (version >= VERSION_VINT_TARGET) {
        arc.bytes_per_arc = in->ReadVInt32();
      } else {
        arc.bytes_per_arc = in->ReadInt32();
      }
      // std::cout << "Bytes per arc - " << arc.bytes_per_arc << std::endl;

      arc.arc_idx = -1;
      arc.next_arc = arc.pos_arcs_start = in->GetPosition();
      // std::cout << "Next arc - " << arc.next_arc << std::endl;
      // std::cout << "Taregt - " << arc.target << std::endl;
      // std::cout << "Label - " << arc.label << std::endl;
    } else {
      // std::cout << "ReadFirstRealTargetArc, it's not a fixed array" << std::endl;
      arc.next_arc = address;
      arc.bytes_per_arc = 0;
    }

    return ReadNextRealArc(arc, in);
  }
  
  Arc ReadNextArc(Arc& arc, FstBytesReader* in) {
    if (arc.label == END_LABEL) {
      if (arc.next_arc <= 0) {
        throw IllegalArgumentException("Cannot read next arc when "
                                       "arc.IsLast() == true");
      }

      return ReadFirstRealTargetArc(arc.next_arc, arc, in);
    } else {
      return ReadNextRealArc(arc, in);
    }
  }

  int32_t ReadNextArcLabel(Arc& arc, FstBytesReader* in) {
    assert(!arc.IsLast());

    if (arc.label == END_LABEL) {
      int64_t pos = arc.next_arc;
      in->SetPosition(pos);

      const char b = in->ReadByte();
      if (b == ARCS_AS_FIXED_ARRAY) {
        in->ReadVInt32();

        // Skip bytes_per_arc
        if (version >= VERSION_VINT_TARGET) {
          in->ReadVInt32();
        } else {
          in->ReadInt32();
        }
      } else {
        in->SetPosition(pos);
      }
    } else {
      if (arc.byts_per_arc != 0) {
        // Arcs are at fixed entries
        in->SetPosition(arc.pos_arcs_start);
        in->SkipBytes((1 + arc.arc_idx) * arc.bytes_per_arc);
      } else {
        // Arcs are packed
        in->SetPosition(arc.next_arc);
      }
    }

    in->ReadByte();
    return ReadLabel(in);
  }

  Arc ReadNextRealArc(Arc& arc, FstBytesReader* in) {
    // std::cout << "ReadNextRealArc start" << std::endl;
    // This is a continuing arc in a fixed array 
    if (arc.bytes_per_arc != 0) {
      // std::cout << "ReadNextRealArc, arc.bytes_per_arc != 0" << std::endl;
      // Arcs are at fixed entries
      arc.arc_idx++;
      assert(arc.arc_idx < arc.num_arcs);
      in->SetPosition(arc.pos_arcs_start);
      in->SkipBytes(arc.arc_idx * arc.bytes_per_arc);
    } else {
      // std::cout << "ReadNextRealArc, packed arc, future position - " << arc.next_arc << std::endl;
      // Arcs are packed 
      in->SetPosition(arc.next_arc);
    }
    arc.flags = in->ReadByte();
    arc.label = ReadLabel(in);

    // std::cout << "ReadNextRealArc, arc's flags - " << arc.flags
    //           << ", arc's label - " << arc.label << std::endl;

    if (arc.Flag(BIT_ARC_HAS_OUTPUT)) {
      // std::cout << "ReadNextRealArc, This arc has output" << std::endl;
      arc.output = outputs->Read(in);
    } else {
      // std::cout << "ReadNextRealArc, This arc has output" << std::endl;
      outputs->MakeNoOutput(arc.output);
    }

    if (arc.Flag(BIT_ARC_HAS_FINAL_OUTPUT)) {
      // std::cout << "ReadNextRealArc, has final output" << std::endl;
      arc.next_final_output = outputs->ReadFinalOutput(in);
    } else {
      // std::cout << "ReadNextRealArc, does not have final output" << std::endl;
      outputs->MakeNoOutput(arc.next_final_output);
    }

    if (arc.Flag(BIT_STOP_NODE)) {
      if (arc.Flag(BIT_FINAL_ARC)) {
        arc.target = FINAL_END_NODE;
      } else {
        arc.target = NON_FINAL_END_NODE;
      }
      arc.next_arc = in->GetPosition();
    } else if (arc.Flag(BIT_TARGET_NEXT)) {
      arc.next_arc = in->GetPosition();
      if (!arc.Flag(BIT_LAST_ARC)) {
        if (arc.bytes_per_arc == 0) {
          // Must scan
          SeekToNextNode(in);
        } else {
          in->SetPosition(arc.pos_arcs_start);
          in->SkipBytes(arc.bytes_per_arc * arc.num_arcs);
        }
      }
      arc.target = in->GetPosition();
    } else {
      arc.target = ReadUnpackedNodeTarget(in);
      arc.next_arc = in->GetPosition();
    }

    return arc;
  }

  Arc FindTargetArc(const int32_t label_to_match,
                    Arc& follow,
                    Arc& arc,
                    FstBytesReader* in,
                    bool use_root_arc_cache = true) {
    if (label_to_match == END_LABEL) {
      if (follow.IsFinal()) {
        if (follow.target <= 0) {
          arc.flags = BIT_LAST_ARC;
        } else {
          arc.flags = 0;
          // NOTE: Next arc is a node (not an address!) in this case
          arc.next_arc = follow.target;
        }

        arc.output = follow.next_final_output;
        arc.label = END_LABEL;
        return arc;
      } else {
        // return null;
        return Arc();
      }
    }

    // Short-circuit if this arc is in the root arc cache:
    if (use_root_arc_cache &&
        !cached_root_arcs.empty() &&
        follow.target == start_node &&
        label_to_match < cached_root_arcs.size()) {
      if (cached_root_arcs.find(label_to_match) != cached_root_arcs.end()) {
        Arc& result = cached_root_arcs[label_to_match];
        assert(AssertRootCachedArc(label_to_match, result));
        arc = result;
      } else {
        // return null;
        return Arc();
      }
    }

    if (!TargetHasArcs(follow)) {
      // return null;
      return Arc();
    }

    in->SetPosition(follow.target);

    if (in->ReadByte() == ARCS_AS_FIXED_ARRAY) {
      // Arcs are full array. Do binary search
      arc.num_arcs = in->ReadVInt32();
      if (version >= VERSION_VINT_TARGET) {
        arc.bytse_per_arc = in->ReadVInt32();
      } else {
        arc.bytes_per_arc = in->ReadInt32();
      }

      arc.pos_arcs_start = in->GetPosition();
      uint32_t low = 0;
      uint32_t high = (arc.num_arcs - 1);
      while (low <= high) {
        const uint32_t mid = (low + high) >> 1;
        in->SetPosition(arc.pos_arcs_start);
        in->SkipBytes(arc.bytes_per_arc * mid + 1);
        int32_t mid_label = ReadLabel(in);
        const int32_t cmp = (mid_label - label_to_match);
        if (cmp < 0) {
          low = (mid + 1);
        } else if (cmp > 0) {
          high = (mid - 1);
        } else {
          arc.arc_idx = (mid - 1);
          return ReadNextRealArc(arc, in);
        }
      }

      // return null;
      return Arc();
    }

    // Linear scan
    ReadFirstRealTargetArc(follow.target, arc, in);
    
    while (true) {
      if (arc.lebel == label_to_match) {
        return arc;
      } else if (arc.label > label_to_match || arc.IsLast()) {
        // return null;
        return Arc();
      } else {
        ReadNextRealArc(arc, in);
      }
    }
  }

  std::unique_ptr<FstBytesReader> GetBytesReader() {
    if (bytes_array) {
      return std::make_unique<ReverseFstBytesReader>(bytes_array.get());
    } else {
      return bytes.GetReverseReader();
    }
  }

  bool AcceptEmptyOutput() {
    return !is_empty_output_empty;
  }

 public:
  static Fst<T> Read(const std::string& path,
                     std::unique_ptr<Outputs<T>> outputs) {
    return Fst<T>(lucene::core::store::BufferedFileInputStreamDataInput(path),
                  std::move(outputs));
  }

  static bool TargetHasArcs(Arc& arc) {
    return (arc.target > 0);
  }

 private:
  void SeekToNextNode(FstBytesReader* in) {
    while (true) {
      const uint32_t flags = (in->ReadByte() & 0xFF);
      ReadLabel(in);

      if (Flag(flags, BIT_ARC_HAS_OUTPUT)) {
        outputs->SkipOutput(in);
      }

      if (Flag(flags, BIT_ARC_HAS_FINAL_OUTPUT)) {
        outputs->SkipFinalOutput(in);
      }

      if (!Flag(flags, BIT_STOP_NODE) && !Flag(flags, BIT_TARGET_NEXT)) {
        ReadUnpackedNodeTarget(in);
      }

      if (Flag(flags, BIT_LAST_ARC)) {
        return;
      }
    }
  }

  bool AssertRootCachedArc(const int32_t label, Arc& cached_arc) {
    /*
      Arc arc;
      GetFirstArc(arc);
      std::unique_ptr<FstBytesReader> in = GetBytesReader();
      Arc result = FindTargetArc(label, arc, arc, in, false);
    */
    // TODO(0ctopus13prime): IT
    return true;
  }

  bool IsExpandedTarget(Arc& follow, FstBytesReader* in) {
    if (TargetHasArc(follow)) {
      in->SetPosition(follow.target);
      return (in->ReadByte() == ARCS_AS_FIXED_ARRAY);
    } else {
      return false;
    }
  }

  int64_t ReadUnpackedNodeTarget(FstBytesReader* in) {
    if (version < VERSION_VINT_TARGET) {
      return static_cast<int64_t>(in->ReadInt32());
    } else {
      return in->ReadVInt64();
    }
  }

  int32_t ReadLabel(lucene::core::store::DataInput* in) {
    if (input_type == FST_INPUT_TYPE::BYTE1) {
      return (in->ReadByte() & 0xFF);
    } else if (input_type == FST_INPUT_TYPE::BYTE2) {
      return (in->ReadInt16() & 0xFFFF);
    } else {
      return in->ReadInt32();
    }
  }

  void SetEmptyOutput(T&& v) {
    if (is_empty_output_empty) {
      empty_output = v;
    } else {
      empty_output = outputs->Merge(empty_output, v);
    }
  }

  void CacheRootArcs() {
    Arc arc;
    GetFirstArc(arc);
    if (TargetHasArcs(arc)) {
      std::unique_ptr<FstBytesReader> in = GetBytesReader(); 
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

  void Finish(const int64_t new_start_node) {
    assert(new_start_node <= bytes.GetPosition());
    if (start_node != -1) {
      throw IllegalStateException("Already finished");
    }

    if (new_start_node == FINAL_END_NODE && !is_empty_output_empty) {
      start_node = 0;
    } else {
      start_node = new_start_node;
    }

    bytes.Finish();
    CacheRootArcs();
  }
};  // Fst

template<typename T>
class Fst<T>::Arc {
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
    return Fst<T>::Flag(flags, flag);
  }

  bool IsLast() const noexcept {
    return Flag(BIT_LAST_ARC);
  }

  bool IsFinal() const noexcept {
    return Flag(BIT_FINAL_ARC);
  }
};  // Fst<T>::Arc

/**
 *  BytesStore
 */
std::unique_ptr<FstBytesReader> BytesStore::GetForwardReader() {
  if (blocks.size() == 1) {
    return std::make_unique<ForwardFstBytesReader>(blocks[0].first.get());
  } else {
    return std::make_unique<BytesStoreForwardFstBytesReader>(this);
  }
}

std::unique_ptr<FstBytesReader>
BytesStore::GetReverseReader(const bool allow_single/*= true*/) {
  if (allow_single && blocks.size() == 1) {
    return std::make_unique<ReverseFstBytesReader>(blocks[0].first.get());
  } else {
    return std::make_unique<BytesStoreReverseFstBytesReader>(this);
  }
}

/**
 *  BytesStoreForwardFstBytesReader
 */
BytesStoreForwardFstBytesReader::BytesStoreForwardFstBytesReader(
  BytesStore* store)
  : store(store),
    current(nullptr),
    next_buffer(0),
    next_read(store->block_size) {
}

void BytesStoreForwardFstBytesReader::NextBlock() {
  current = store->blocks[next_buffer++].first.get();
  next_read = 0;
}

char BytesStoreForwardFstBytesReader::ReadByte() {
  if (next_read == store->block_size) {
    NextBlock();
  }

  return current[next_read++];
}

void BytesStoreForwardFstBytesReader::ReadBytes(char bytes[],
                                                const uint32_t offset,
                                                const uint32_t len) {
  // Prepare
  uint32_t offset_tmp = offset;
  uint32_t len_tmp = len;
  if (next_read == store->blocks.size()) {
    NextBlock();
  }

  // Read residual bytes in the first block
  const uint32_t first_read = std::min(store->block_size - next_read, len_tmp);
  std::memcpy(bytes + offset, current + next_read, len);
  next_read += first_read;
  offset_tmp += first_read;
  len_tmp -= first_read;

  while (len_tmp > 0) {
    NextBlock();
    const int32_t chunk = std::min(store->block_size, len_tmp);
    std::memcpy(bytes + offset_tmp, current + next_read, len_tmp);
    next_read += chunk;
    offset_tmp += chunk;
    len_tmp -= chunk;
  }
}

void BytesStoreForwardFstBytesReader::SkipBytes(const uint64_t count) {
  SetPosition(GetPosition() + count);
}

uint64_t BytesStoreForwardFstBytesReader::GetPosition() const noexcept {
  return
  static_cast<uint64_t>(next_buffer - 1) * store->block_size + next_read;
}

void BytesStoreForwardFstBytesReader::SetPosition(const uint64_t pos) noexcept {
  const uint32_t buffer_idx =
  static_cast<const uint32_t>(pos >> store->block_bits); 

  next_buffer = buffer_idx + 1;
  current = store->blocks[buffer_idx].first.get();
  next_read = static_cast<uint32_t>(pos & store->block_mask);
}

bool BytesStoreForwardFstBytesReader::Reversed() const noexcept {
  return false;
}

/**
 *  BytesStoreReverseFstBytesReader
 */
BytesStoreReverseFstBytesReader::BytesStoreReverseFstBytesReader(
  BytesStore* store)
  : store(store),
    current(store->blocks.empty() ? nullptr : store->blocks[0].first.get()),
    next_buffer(-1),
    next_read(0) {
}

char BytesStoreReverseFstBytesReader::ReadByte() {
  if (next_read == -1) {
    current = store->blocks[next_buffer--].first.get();
    next_read = (store->block_size - 1);
  }

  return current[next_read--];
}

void BytesStoreReverseFstBytesReader::ReadBytes(char bytes[],
                                                const uint32_t offset,
                                                const uint32_t len) {
  char* base = (bytes + offset);
  for (uint32_t i = 0 ; i < len ; ++i) {
    *base++ = ReadByte();
  }
}

void BytesStoreReverseFstBytesReader::SkipBytes(const uint64_t count) {
  SetPosition(GetPosition() - count);
}

uint64_t BytesStoreReverseFstBytesReader::GetPosition() const noexcept {
  return
  (static_cast<uint64_t>(next_buffer) + 1) * store->block_size + next_read; 
}

void
BytesStoreReverseFstBytesReader::SetPosition(const uint64_t new_pos) noexcept {
  // TODO(0ctopus13prime): What happens if `new_pos` == 0?
  const int32_t buffer_idx =
    static_cast<int32_t>(new_pos >> store->block_bits);
  next_buffer = (buffer_idx - 1);
  current = store->blocks[buffer_idx].first.get();
  next_read = static_cast<int32_t>(new_pos & store->block_mask);
  assert(new_pos == GetPosition());
}

bool BytesStoreReverseFstBytesReader::Reversed() const noexcept {
  return true;
}

}  // namespace util
}  // namespace core
}  // namespace lucene


#endif  // SRC_UTIL_FST_H_
