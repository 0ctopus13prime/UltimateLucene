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

#ifndef SRC_INDEX_POSTING_H_
#define SRC_INDEX_POSTING_H_

#include <Search/DocId.h>
#include <cassert>
#include <limits>

namespace lucene {
namespace core {
namespace index {

class PostingsEnum: public lucene::core::search::DocIdSetIterator {
 public:
  static const uint16_t NONE = 0;
  static const uint16_t FREQS = (1 << 3);
  static const uint16_t POSITIONS = (FREQS | 1 << 4);
  static const uint16_t OFFSETS = (POSITIONS | 1 << 5);
  static const uint16_t PAYLOADS = (POSITIONS | 1 << 6);
  static const uint16_t ALL = (OFFSETS | PAYLOADS);

  static bool FeatureRequested(const uint32_t flags, const uint16_t feature) {
    return ((flags & feature) == feature);
  }

 private:
  // TODO(0ctopus13prime): IT
  // AttributeSource atts = null;

 protected:
  PostingsEnum() = default; 

 public:
  virtual int32_t Freq() = 0;

  // TODO(0ctopus13prime): IT
  // AttributeSource atts = null;
  // AttributeSource attributes() {
  //   if (atts == null) atts = new AttributeSource();
  //     return atts;
  // }

  virtual int32_t NextPosition() = 0;

  virtual int32_t StartOffset() = 0;

  virtual int32_t EndOffset() = 0;

  virtual lucene::core::util::BytesRef* GetPayload() = 0;
};  // PostingsEnum

}  // namespace index
}  // namespace core
}  // namespace lucene

#endif  // SRC_INDEX_POSTING_H_
