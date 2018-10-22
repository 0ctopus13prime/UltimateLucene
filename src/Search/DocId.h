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

#ifndef SRC_SEARCH_DOCID_H_
#define SRC_SEARCH_DOCID_H_

namespace lucene {
namespace core {
namespace search {

class DocIdSetIterator {
 public:
  static const uint32_t NO_MORE_DOCS = std::numeric_limits<int32_t>::max();

  virtual ~DocIdSetIterator() = 0;

  virtual uint32_t DocId() = 0;

  virtual uint32_t NextDoc() = 0;

  virtual uint32_t Advance(uint32_t target) = 0;

  virtual uint64_t Cost() = 0;

 protected:
  uint32_t SlowAdvance(uint32_t target) {
    assert(DocId() < target); 

    uint32_t doc = NextDoc();
    while (doc < target) {
      doc = NextDoc();
    }

    return doc;
  }
};  // DocIdSetIterator

}  // namespace search
}  // namespace core
}  // namespace lucene

#endif  // SRC_SEARCH_DOCID_H_
