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

#include <cassert>
#include <cstdint>

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

}  // namespace store
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_CONTEXT_H_
