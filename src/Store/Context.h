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
#ifndef SRC_UTIL_CONTEXT_H_
#define SRC_UTIL_CONTEXT_H_

#include <Util/Etc.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

namespace lucene {
namespace core {
namespace store {

class MergeInfo {
 public:
  static const MergeInfo DEFAULT;

 public:
  const uint32_t total_max_doc;
  const uint64_t estimate_merge_bytes;
  const bool is_external;
  const uint32_t merge_max_num_segments;

  MergeInfo(const uint32_t total_max_doc,
            const uint64_t estimate_merge_bytes,
            const bool is_external,
            const uint32_t merge_max_num_segments)
    : total_max_doc(total_max_doc),
      estimate_merge_bytes(estimate_merge_bytes),
      is_external(is_external),
      merge_max_num_segments(merge_max_num_segments) {
  }

  bool operator!=(const MergeInfo& other) const {
    return !operator==(other);
  }

  bool operator==(const MergeInfo& other) const {
    return ((total_max_doc == other.total_max_doc) &&
           (estimate_merge_bytes == other.estimate_merge_bytes) &&
           (is_external == other.is_external) &&
           (merge_max_num_segments == other.merge_max_num_segments)); 
  }
};

class FlushInfo {
 public:
  static const FlushInfo DEFAULT;

 public:
  const uint32_t num_docs;
  const uint64_t estimated_segment_size;

  FlushInfo(const uint32_t num_docs, const uint64_t estimated_segment_size)
    : num_docs(num_docs),
      estimated_segment_size(estimated_segment_size) {
  }
};

class IOContext {
 public:
  static const IOContext DEFAULT;
  static const IOContext READONCE;
  static const IOContext READ;

 public:
  enum class Context {
    MERGE, READ, FLUSH, DEFAULT
  };

  const Context context;  
  const MergeInfo merge_info;
  const FlushInfo flush_info;
  const bool read_once;

 private:
  IOContext(const bool read_once)
    : context(Context::READ),
      merge_info(MergeInfo::DEFAULT),
      read_once(read_once),
      flush_info(FlushInfo::DEFAULT) {
  }

  IOContext(const Context context, const MergeInfo merge_info)
    : context(context),
      read_once(false),
      merge_info(merge_info),
      flush_info(FlushInfo::DEFAULT) {
    assert(context != Context::MERGE || merge_info != MergeInfo::DEFAULT); 
    assert(context != Context::FLUSH);
  }

 public:
  IOContext()
    : IOContext(false) {
  }

  IOContext(const FlushInfo& flush_info)
    : context(Context::FLUSH),
      merge_info(MergeInfo::DEFAULT),
      read_once(false),
      flush_info(flush_info) {
  }

  IOContext(const Context context)
    : IOContext(context, MergeInfo::DEFAULT) {
  }

  IOContext(const MergeInfo& merge_info)
    : IOContext(Context::MERGE, merge_info) {
  }

  IOContext(const IOContext& ctxt, const bool read_once)
    : context(ctxt.context),
      merge_info(ctxt.merge_info),
      flush_info(ctxt.flush_info),
      read_once(read_once) {
  }
};

class BufferedChecksum: public lucene::core::util::Checksum {
 public:
  const uint32_t DEFAULT_BUFFERSIZE = 256; 

 private:
  std::unique_ptr<lucene::core::util::Checksum> in; 
  std::unique_ptr<char[]> buffer;
  uint32_t buffer_size;
  uint32_t upto;

 private:
  void Flush() {
    if (upto > 0) {
      in->Update(buffer.get(), 0, upto);
    }
  }

 public:
  BufferedChecksum(std::unique_ptr<lucene::core::util::Checksum>&& in)
    : in(std::forward<std::unique_ptr<lucene::core::util::Checksum>>(in)),
      buffer(std::make_unique<char[]>(BufferedChecksum::DEFAULT_BUFFERSIZE)),
      buffer_size(BufferedChecksum::DEFAULT_BUFFERSIZE),
      upto(0) {
  }

  BufferedChecksum(std::unique_ptr<lucene::core::util::Checksum>&& in,
                   const uint32_t buffer_size)
    : in(std::forward<std::unique_ptr<lucene::core::util::Checksum>>(in)),
      buffer(std::make_unique<char[]>(buffer_size)),
      buffer_size(buffer_size),
      upto(0) {
  }

  void Update(const char b) {
    if (upto == buffer_size) {
      Flush();
    }

    buffer[upto++] = b;
  }

  void Update(char bytes[], const uint32_t off, const uint32_t len) {
    if (len >= buffer_size) {
      Flush();
      in->Update(bytes, off, len);
    } else {
      if (upto + len > buffer_size) {
        Flush();
      }

      std::memcpy(buffer.get() + upto, bytes + off, len);
      upto += len;
    }
  }

  int64_t GetValue() {
    Flush();
    return in->GetValue();
  }

  void Reset() {
    upto = 0;
    in->Reset();
  }
};

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_CONTEXT_H_
