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

#ifndef SRC_CODEC_WRITER_H_
#define SRC_CODEC_WRITER_H_

#include <Util/Bits.h>
#include <Store/DataOutput.h>
#include <Index/Field.h>
#include <Index/Segment.h>
#include <Codec/Etc.h>

namespace lucene {
namespace core {
namespace codec {

class PostingsWriterBase {
 protected:
  PostingsWriterBase() = default;

 public:
  virtual void Init(lucene::core::store::IndexOutput* terms_out,
                    lucene::core::index::SegmentWriteState state) = 0;

  virtual BlockTermState WriteTerm(lucene::core::util::BytesRef& term,
                               lucene::core::index::TermsEnum& terms_enum,
                               lucene::core::util::FixedBitSet& docs_seen) = 0;

  virtual void EncodeTerm(uint64_t longs[],
                          const uint32_t longs_size,
                          lucene::core::index::FieldInfo& field_info,
                          BlockTermState& state,
                          bool absolute) = 0;

  virtual int32_t SetField(lucene::core::index::FieldInfo& field_info) = 0;

  virtual void Close() = 0;
};  // PostingsWriterBase

}  // namespace codec
}  // core
}  // lucene

#endif  // SRC_CODEC_WRITER_H_
