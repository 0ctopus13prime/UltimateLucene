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

#ifndef SRC_DOCUMENT_INDEXABLEFIELDTYPE_H_
#define SRC_DOCUMENT_INDEXABLEFIELDTYPE_H_

namespace lucene {
namespace core {
namespace document {

class IndexableFieldType {
 public:
  virtual bool Stored() = 0;
  virtual bool Tokenized() = 0;
  virtual bool StoreTermVectors() = 0;
  virtual bool StoreTermVectorOffsets() = 0;
  virtual bool StoreTermVectorPositions() = 0;
  virtual bool StoreTermVectorPayloads() = 0;
  virtual bool OmitNorms() = 0;
  virtual IndexOptions& IndexOptions() = 0;
  virtual DocValuesType& DocValuesType() = 0;
  virtual int PointDimensionCount() = 0;
  virtual unsigned int PointNumBytes() = 0;
};

}  // document
}  // core
}  // lucene

#endif  // SRC_DOCUMENT_INDEXABLEFIELDTYPE_H_
