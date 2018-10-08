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

#ifndef SRC_UTIL_FSTUTIL_H_
#define SRC_UTIL_FSTUTIL_H_

// TEST
#include <iostream>

#include <Util/Ref.h>
#include <Util/Fst.h>
#include <memory>

namespace lucene {
namespace core {
namespace util {

class FstUtil {
 private:
  FstUtil() = default;

 public:
  template <typename T>
  static void Get(Fst<T>& fst, IntsRef& input, T& scratch) {
    typename Fst<T>::Arc arc;
    fst.GetFirstArc(arc);
    // TODO(0ctopus13prime): Every time new? Really?
    // There must some ways to around this. Ex: Reuse reader
    // If get nothing from `FindTargetArc`, this newed reader
    // is a totally useless garbage.
    std::unique_ptr<FstBytesReader> fst_reader =
      fst.GetBytesReader();

    // Accumulate output as we go
    fst.outputs->MakeNoOutput(scratch);
    const int32_t* base = (input.Ints() + input.Offset());
    for (uint32_t i = 0 ; i < input.Length() ; ++i) {
      std::cout << "xxxxxxx " << i
                << ", output length - " << scratch.Length() << std::endl;
      fst.FindTargetArc(base[i],
                        arc,
                        arc,
                        fst_reader.get());
      std::cout << "jjjjjjjj " << std::endl;
      if (arc.IsEmptyArc()) {
        return;
      }
      std::cout << "kkkkkkkkkk " << std::endl;
      fst.outputs->Append(scratch, arc.output);
      std::cout << std::endl;
    }

    std::cout << "zzzzzzzzzz " << std::endl;
    if (arc.IsFinal()) {
      fst.outputs->Append(scratch, arc.next_final_output);
    } else {
      fst.outputs->MakeNoOutput(scratch);
    }
  }
};

}  // util
}  // core
}  // lucene

#endif  // SRC_UTIL_FSTUTIL_H_
