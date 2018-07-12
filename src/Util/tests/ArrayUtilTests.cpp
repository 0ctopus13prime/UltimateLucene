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

#include <assert.h>
#include <gtest/gtest.h>
#include <Util/ArrayUtil.h>
#include <memory>
#include <iostream>

using lucene::core::util::arrayutil::CopyOf;
using lucene::core::util::arrayutil::Grow;
using lucene::core::util::arrayutil::CopyOfRange;

TEST(ARRAY__UTIL__TEST, COPE__OF) {
  size_t length = 10;
  int* arr = new int[length];
  std::unique_ptr<int[]> guard1(arr);

  size_t half_length = length / 2;
  int* copy_arr1 = CopyOf<int>(arr, length, half_length);
  std::unique_ptr<int[]> guard2(copy_arr1);
  for (size_t i = 0 ; i < half_length ; ++i) {
    EXPECT_EQ(arr[i], copy_arr1[i]);
  }

  size_t twice_length = length * 2;
  int* copy_arr2 = CopyOf<int>(arr, length, twice_length);
  std::unique_ptr<int[]> guard3(copy_arr2);
  for (size_t i = 0 ; i < length ; ++i) {
    EXPECT_EQ(arr[i], copy_arr2[i]);
  }
}

TEST(ARRAY__UTIL__TEST, GROW) {
  size_t length = 50;
  size_t min_length = 2 * length;
  int* arr = new int[length];
  std::unique_ptr<int[]> guard1(arr);

  std::pair<int*, uint32_t> p1 = Grow(arr, length, min_length);
  EXPECT_NE(p1.first, nullptr);
  EXPECT_LE(min_length, p1.second);
  std::unique_ptr<int[]> guard2(p1.first);

  min_length = length - 1;  // smaller than length
  std::pair<int*, uint32_t> p2 = Grow(arr, length, min_length);
  EXPECT_EQ(p2.first, nullptr);
  EXPECT_EQ(p2.second, 0);
}

TEST(ARRAY__UTIL, COPY__OF__RANGE) {
  size_t length = 50;
  int* arr = new int[length];
  std::unique_ptr<int[]> guard1(arr);
  for (size_t i = 0 ; i < length ; ++i) {
    arr[i] = i;
  }

  size_t s = 13;
  size_t e = 37;
  int* new_arr = CopyOfRange(arr, s, e);
  std::unique_ptr<int[]> guard2(new_arr);
  for (size_t i = s ; i < e ; ++i) {
    EXPECT_EQ(arr[i], new_arr[i - s]);
  }

  s = 37; e = 13;
  try {
    CopyOfRange(arr, s, e);
    FAIL();
  } catch(std::invalid_argument& e) {
    // ignore
  }

  s = 13; e = 13;
  try {
    CopyOfRange(arr, s, e);
    FAIL();
  } catch(std::invalid_argument& e) {
    // ignore
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
