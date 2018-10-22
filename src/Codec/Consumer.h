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

#ifndef SRC_CODEC_CONSUMER_H_
#define SRC_CODEC_CONSUMER_H_

#include <Index/Field.h>

namespace lucene {
namespace core {
namespace codec {

class FieldsConsumer {
 protected:
  FieldsConsumer() = default;

 public:
  virtual ~FieldsConsumer() = default;

  virtual void Write(lucene::core::index::Fields* fields) = 0;

  // TODO(0ctopus13prime): IT
  // void Merge(MergeState merge_state);
};  // FieldsConsumer

}  // codec
}  // core
}  // lucene


#endif  // SRC_CODEC_CONSUMER_H_
