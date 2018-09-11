#!/usr/bin/python

# Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


HEADER_FILE = "Packed64SingleBlock.h"
LICENSE = """
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
""".lstrip()

HEAD = """
// This file has been automatically generated, DO NOT EDIT

#ifndef SRC_UTIL_PACK_PACKED64_H_
#define SRC_UTIL_PACK_PACKED64_H_

namespace lucene {
namespace core {
namespace util {
"""

TAIL = """
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_PACK_PACKED64_H_
"""

f = None

def l(line):
  global f 
  f.write(line)

def ln(line):
  global f 
  l(line + "\n")

def start():
  global f 
  f = open(HEADER_FILE, "w")
  l(LICENSE)
  l(HEAD)

def end():
  global f
  l(TAIL)
  f.close()

def gen_pack64():
  pass
if __name__ == "__main__":
  start()
  gen_pack64()
  end()
