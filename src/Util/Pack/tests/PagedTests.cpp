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

#include <Util/Pack/Paged.h>
#include <Util/Pack/Direct.h>
#include <gtest/gtest.h>

using lucene::core::util::Direct8;
using lucene::core::util::Direct16;
using lucene::core::util::Direct32;
using lucene::core::util::Direct64;
using lucene::core::util::GrowableWriter;
using lucene::core::util::PackedInts;
using lucene::core::util::PagedGrowableWriter;

TEST(PAGED__GROWABLE__WRITER, BASIC__TEST) {
  PagedGrowableWriter table(16, 1 << 27, 8, PackedInts::COMPACT);
}

TEST(DIRECT__8__TESTS, BASIC__TEST) {
  uint32_t value_count = 16;
  Direct8 d8(value_count);

  for (uint32_t i = 0 ; i < value_count ; ++i) {
    d8.Set(i, i & 0xFF);
  }

  for (uint32_t i = 0 ; i < value_count ; ++i) {
    ASSERT_EQ(i, d8.Get(i)); 
  }

  int64_t buffer[value_count];
  const uint32_t got = d8.Get(0, buffer, 0, 2 * value_count);
  ASSERT_EQ(got, value_count);

  for (uint32_t i = 0 ; i < value_count ; ++i) {
    ASSERT_EQ(i, buffer[i]); 
  }

  d8.Clear();
  d8.Get(0, buffer, 0, 2 * value_count);
  for (uint32_t i = 0 ; i < value_count ; ++i) {
    ASSERT_EQ(0, buffer[i]); 
  }

  // Fill 3th - 5th by `13`
  d8.Fill(3, 6, 13);
  d8.Get(0, buffer, 0, 2 * value_count);
  ASSERT_EQ(13, d8.Get(3));
  ASSERT_EQ(13, d8.Get(4));
  ASSERT_EQ(13, d8.Get(5));

  for (uint32_t i = 0 ; i < value_count ; ++i) {
    if (i < 3 || i > 5) {
      ASSERT_EQ(0, d8.Get(i)); 
    } else {
      ASSERT_EQ(13, d8.Get(i)); 
    }
  }
}

TEST(GROWABLE__WRITER, BASIC__TEST) {
  const uint32_t value_count = 16;
  const uint32_t init_bits = 8;

  // No extra overhead is allowed
  GrowableWriter gw(init_bits, value_count, 0);

  // 8-bit width, 1000 values
  for (uint32_t i = 0 ; i < value_count ; ++i) {
    gw.Set(i, i & 0xFF);
  }

  // Now, bits expand from 8-bit to 64-bit
  for (uint32_t j = 0, i = 8 ; i <= 64; ++i, ++j) {
    if (i != 64) {
      gw.Set(j % value_count, ((1 << i) - 1));
    } else {
      gw.Set(j % value_count, -1L);
    }
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
