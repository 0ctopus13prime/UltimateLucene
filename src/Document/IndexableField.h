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

#ifndef SRC_DOCUMENT_INDEXABLEFIELD_H_
#define SRC_DOCUMENT_INDEXABLEFIELD_H_

#include <string>

namespace lucene {
namespace core {
namespace document {

class IndexableField {
 public:
  virtual const string& name() = 0;
  virtual IndexableFieldType& fieldType() = 0;
  virtual TokenStream& tokenStream(Analyzer& analyzer, TokenStream& reuse) = 0;
  virtual BytesRef& binaryValue() = 0;
  virtual string& stringValue() = 0;
  virtual Reader& readerValue() = 0;
  virtual Number numericValue() = 0;
};

}  // namespace document
}  // namespace core
}  // namespace lucene

#endif  // SRC_DOCUMENT_INDEXABLEFIELD_H_
