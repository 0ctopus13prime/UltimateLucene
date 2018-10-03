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

#include <Util/Numeric.h>
#include <gtest/gtest.h>
#include <iostream>

using lucene::core::util::Float;
using lucene::core::util::FloatConsts;
using lucene::core::util::Double;
using lucene::core::util::DoubleConsts;
using lucene::core::util::Number;
using lucene::core::util::NumericUtils;

TEST(NUMERIC__TESTS, BITS__BASIC) {
  for (uint32_t i = 0 ; i <= 64 ; ++i) {
    if (i != 64) {
      ASSERT_EQ(0, NumericUtils::NumberOfLeadingZerosInt32(~0L));
    } else {
      ASSERT_EQ(64 - i, NumericUtils::NumberOfLeadingZerosInt32((1 << i) - 1));
    }
  }
}

TEST(NUMERIC__TESTS, FLOAT__TESTS) {
  float nan = 0.0F / 0.0F;
  ASSERT_TRUE(Float::IsNaN(nan));
  ASSERT_EQ(0x7fc00000, Float::FloatToIntBits(nan));

  float num1 = 12345.67891f;
  int32_t rawint1 = Float::FloatToRawIntBits(num1);
  float rawfloat1 = Float::IntBitsToFloat(rawint1);
  ASSERT_EQ(num1, rawfloat1);
}

TEST(NUMERIC__TESTS, DOUBLE__TESTS) {
  double nan = 0.0 / 0.0;
  ASSERT_TRUE(Double::IsNaN(nan));
  ASSERT_EQ(0x7ff8000000000000L, Double::DoubleToLongBits(nan));

  double num1 = 12345.67891;
  int64_t rawint1 = Double::DoubleToRawLongBits(num1);
  double rawdouble1 = Double::LongBitsToDouble(rawint1);
  ASSERT_EQ(num1, rawdouble1);
}

TEST(NUMERIC_TESTS, NUMBER__TESTS) {
  {
    Number num(13);
    int8_t byte_value = num.ByteValue();
    ASSERT_EQ(13, byte_value);
    int16_t short_num = num.ShortValue();
    ASSERT_EQ(13, short_num);
    int32_t int_num = num.IntValue();
    ASSERT_EQ(13, int_num);
    int64_t long_num = num.LongValue();
    ASSERT_EQ(13, long_num);
    float float_num = num.FloatValue();
    ASSERT_EQ(13, float_num);
    double double_num = num.DoubleValue();
    ASSERT_EQ(13, double_num);
  }

  {
    Number num(13.12345f);
    int8_t byte_value = num.ByteValue();
    ASSERT_EQ(13, byte_value);
    int16_t short_num = num.ShortValue();
    ASSERT_EQ(13, short_num);
    int32_t int_num = num.IntValue();
    ASSERT_EQ(13, int_num);
    int64_t long_num = num.LongValue();
    ASSERT_EQ(13, long_num);
    float float_num = num.FloatValue();
    ASSERT_EQ(13.12345f, float_num);
    double double_num = num.DoubleValue();
    ASSERT_EQ(13.12345f, double_num);
  }
}

TEST(NUMERIC_TESTS, NUMERIC___UTILS__TESTS1) {
  {
    float fvalue1 = 123.123f;
    int32_t ivalue1 = NumericUtils::FloatToSortableInt(fvalue1);
    float newfvalue1 = NumericUtils::SortableIntToFloat(ivalue1);
    ASSERT_EQ(fvalue1, newfvalue1);

    float fvalue2 = 234.567f;
    int32_t ivalue2 = NumericUtils::FloatToSortableInt(fvalue2);
    ASSERT_LT(ivalue1, ivalue2);
  }

  {
    float fvalue1 = -123.123f;
    int32_t ivalue1 = NumericUtils::FloatToSortableInt(fvalue1);
    float newfvalue1 = NumericUtils::SortableIntToFloat(ivalue1);
    ASSERT_EQ(fvalue1, newfvalue1);

    float fvalue2 = -234.567f;
    int32_t ivalue2 = NumericUtils::FloatToSortableInt(fvalue2);
    ASSERT_GT(ivalue1, ivalue2);
  }
}

TEST(NUMERIC_TESTS, NUMERIC___UTILS__TESTS2) {
  {
    int32_t num1 = 0x12345678;
    char arr1[6];
    NumericUtils::IntToSortableBytes(num1, arr1, 2);
    int32_t gotnum1 = NumericUtils::SortableBytesToInt(arr1, 2);
    ASSERT_EQ(num1, gotnum1);

    int32_t num2 = -0x12345678;
    char arr2[6];
    NumericUtils::IntToSortableBytes(num2, arr2, 2);
    int32_t gotnum2 = NumericUtils::SortableBytesToInt(arr2, 2);
    ASSERT_EQ(num2, gotnum2);
  }

  {
    int64_t num1 = 0x1234567878563412;
    char arr1[10];
    NumericUtils::LongToSortableBytes(num1, arr1, 2);
    int64_t gotnum1 = NumericUtils::SortableBytesToLong(arr1, 2);
    ASSERT_EQ(num1, gotnum1);

    int64_t num2 = -0x1234567878563412;
    char arr2[10];
    NumericUtils::LongToSortableBytes(num2, arr2, 2);
    int64_t gotnum2 = NumericUtils::SortableBytesToLong(arr2, 2);
    ASSERT_EQ(num2, gotnum2);
  }
}

TEST(NUMERIC_TESTS, NUMERIC___UTILS__TESTS3) {
  {
    char dimension1[] = {
      0x78, 0x2A, 0x7C, 0x12, 0x34, 0x56
    };
    char dimension2[] = {
      0x3A, 0x28, 0x1C, 0x1, 0x32, 0x27
    };
    char result[3];

    NumericUtils::Substract(3, 1, dimension1, dimension2, result);
    ASSERT_EQ(result[0], 0x11);
    ASSERT_EQ(result[1], 0x2);
    ASSERT_EQ(result[2], 0x2F);

    // dimension3 = dimension1 - dimension2
    char dimension3[] = {
      0, 0, 0, result[0], result[1], result[2]
    };

    // dimension2 + dimension3 = dimension1
    NumericUtils::Add(3, 1, dimension2, dimension3, result);
    ASSERT_EQ(result[0], dimension1[3]);
    ASSERT_EQ(result[1], dimension1[4]);
    ASSERT_EQ(result[2], dimension1[5]);

    try {
      NumericUtils::Substract(3, 1, dimension2, dimension1, result);
      FAIL();
    } catch(std::invalid_argument&) {
    }
  }

  {
    char dimension1[] = {
      0x78, 0x2A, 0x7C, 0x12, 0x34, 0x56, 0x01, 0x45
    };
    char dimension2[] = {
      0x3A, 0x28, 0x1C, 0x1, 0x32, 0x73, 0, 0x32
    };
    char result1[4];
    char result2[4];

    NumericUtils::Substract(4, 1, dimension1, dimension2, result1);
    ASSERT_EQ(result1[0], '\x1');
    ASSERT_EQ(result1[1], '\xE3');
    ASSERT_EQ(result1[2], '\x1');
    ASSERT_EQ(result1[3], '\x13');

    NumericUtils::Substract4Bytes(1, dimension1, dimension2, result2);
    ASSERT_EQ(result1[0], result2[0]);
    ASSERT_EQ(result1[1], result2[1]);
    ASSERT_EQ(result1[2], result2[2]);
    ASSERT_EQ(result1[3], result2[3]);
  }

  {
    char dimension1[] = {
      -1/*255*/, 0x7F, 0x7C, 0x12, 0x34, 0x56
    };
    char dimension2[] = {
      -1/*255*/, 0x12, 0x1C, 0x1, 0x32, 0x27
    };
    char result[3];

    try {
      NumericUtils::Add(3, 0, dimension1, dimension2, result);
      FAIL();
    } catch(std::invalid_argument&) {
    }
  }
}

TEST(NUMERIC_TESTS, NUMERIC___UTILS__TESTS4) {
  {
    float num = 0;
    float next_num = NumericUtils::FloatNextUp(num);
    ASSERT_GT(next_num, num);
  }

  {
    float num = 1;
    float next_num = NumericUtils::FloatNextDown(num);
    ASSERT_LT(next_num, num);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
