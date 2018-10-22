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

#ifndef SRC_CODEC_ETC_H_
#define SRC_CODEC_ETC_H_

namespace lucene {
namespace core {
namespace codec {

class BlockTermState {
 protected:
  BlockTermState() = default;

 public:
  BlockTermState(const BlockTermState& other) {
    // TODO(0ctopus13prime): IT
  };

  BlockTermState& operator=(const BlockTermState& other) {
    // TODO(0ctopus13prime): IT
  }

  BlockTermState(BlockTermState&& other) {
    // TODO(0ctopus13prime): IT
  };

  BlockTermState& operator=(BlockTermState&& other) {
    // TODO(0ctopus13prime): IT
  }
};  // BlockTermState

}  // namespace codec
}  // namespace core
}  // namespace lucene

#endif  // SRC_CODEC_ETC_H_
