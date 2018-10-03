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

#include <Util/Pack/Packed64.h>
#include <gtest/gtest.h>

using lucene::core::util::Packed64;

TEST(PACKED__64, BASIC__TEST) {
  const uint32_t value_count = 1000;
  const uint32_t bits = 9;

  Packed64 p64(value_count, bits);

  // Set ${value_count} values
  for (uint32_t i = 0 ; i < value_count ; ++i) {
    p64.Set(i, i & ((1 << bits) - 1));
  }
  
  // Get ${value_count} values
  for (uint32_t i = 0 ; i < value_count ; ++i) {
    ASSERT_EQ(i & ((1 << bits) - 1), p64.Get(i));
  }

  // Bulk set
  int64_t buf[value_count];
  for (uint32_t i = 0 ; i < value_count ; ++i) {
    buf[i] = (i & ((1 << bits) - 1));
  }

  p64.Set(0, buf, 0, value_count);

  // Get bulk values
  for (uint32_t i = 0 ; i < value_count ; ++i) {
    ASSERT_EQ(i & ((1 << bits) - 1), buf[i]);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
