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

#include <Analysis/Reader.h>
#include <Document/Field.h>
#include <Util/Bytes.h>
#include <Util/Numeric.h>
#include <gtest/gtest.h>
#include <memory>
#include <iostream>
#include <string>

using lucene::core::analysis::StringReader;
using lucene::core::util::BytesRef;
using lucene::core::util::numeric::Number;
using lucene::core::document::Field;
using lucene::core::document::FieldType;
using lucene::core::document::FieldTypeBuilder;
using lucene::core::document::TextField;
using lucene::core::document::StringField;
using lucene::core::document::BinaryPoint;
using lucene::core::document::DoublePoint;
using lucene::core::document::DoubleRange;
using lucene::core::document::FloatPoint;
using lucene::core::document::FloatRange;
using lucene::core::document::IntPoint;
using lucene::core::document::IntRange;
using lucene::core::document::LongPoint;
using lucene::core::document::LongRange;

TEST(FIELD__TESTS, BASIC__TEST) {
  {
    StringReader* reader = new StringReader();

    std::string value("field value");
    reader->SetValue(value);
    std::string name("field_name");
    FieldType type = FieldTypeBuilder().Build();

    Field field(name, reader, type);
  }

  {
    std::string name("field_name");
    FieldType type = FieldTypeBuilder().Build();
    Field field(name, BytesRef("abcdef", 6), type);
  }

  {
    std::string name("field_name");
    std::string value("field_value");
    std::string value_copy(value);
    FieldType type = FieldTypeBuilder().Build();

    Field field1(name, value, type);
    auto opt1 = field1.StringValue();
    ASSERT_TRUE(opt1);
    std::string& got1 = *opt1;
    ASSERT_EQ(got1, value);

    Field field2(name, std::move(value), type);
    auto opt2 = field2.StringValue();
    ASSERT_TRUE(opt2);
    std::string& got2 = *opt2;
    ASSERT_EQ(got2, value_copy);
  }

  {
    std::string name("field_name");
    std::string value("field_value");
    FieldType type = FieldTypeBuilder().Build();
    Field field(name, value, type);
    int64_t num = 13l;
    try {
      field.SetLongValue(num);
      FAIL();
    } catch(std::runtime_error&) {
    }
  }

  {
    std::string name("field_name");
    FieldType type = FieldTypeBuilder().Build();
    const char* cstr = "Hello Doochi! :)";
    Field field(name, cstr, 16, type);
    BytesRef bytesref(cstr, 16);

    auto opt = field.BinaryValue();
    if (opt) {
      BytesRef& got_bytesref = *opt;
      ASSERT_EQ(bytesref, got_bytesref);
    } else {
      FAIL();
    }
  }
}

TEST(FIELD__TESTS, BINARY__POINT__TEST) {
  std::string name("field_name");
  const char points[3][2] = {
    {0x01, 0x12},
    {0x23, 0x34},
    {0x01, 0x12}
  };

  BinaryPoint bp(name, &points[0][0], 3, 2);
  auto opt = bp.BinaryValue();
  if (opt) {
    BytesRef& got = *opt;
    BytesRef org(&points[0][0], 6);
    ASSERT_EQ(org, got);
  } else {
    FAIL();
  }
}

TEST(FIELD__TESTS, DOUBLE__POINT__TEST) {
  {
    std::string name("field_name");
    const double d = 123.123;
    DoublePoint dp(name, {d});
    auto opt = dp.NumericValue();
    if (opt) {
      ASSERT_EQ((*opt).DoubleValue(), d);
    } else {
      FAIL();
    }
  }

  {
    std::string name("field_name");
    const double points[] = {11.11D, 12.12D, 13.13D};
    DoublePoint dp(name, {points[0], points[1], points[2]});
    auto opt = dp.BinaryValue();
    if (opt) {
      BytesRef& bytes_ref = *opt;
      ASSERT_EQ(3 * sizeof(double), bytes_ref.length);

      double got = DoublePoint::DecodeDimension(bytes_ref.bytes.get(), 0);
      ASSERT_EQ(11.11D, got);
    } else {
      FAIL();
    }
  }
}

TEST(FIELD__TESTS, DOUBLE__RANGE__TEST) {
  {
    std::string name("field_name");
    double min[4] = { 12.12, 13.13, 14.14, 15.15 };
    double max[4] = { 45.12, 23.12, 56.45, 19.14 };
    DoubleRange dr(name, min, max, 4);

    for (uint32_t i = 0 ; i < 4 ; ++i) {
      ASSERT_EQ(min[i], dr.GetMin(i));
      ASSERT_EQ(max[i], dr.GetMax(i));
    }
  }
}

TEST(FIELD__TESTS, FLOAT__POINT__TEST) {
  std::string name("field_name");

  {
    float f = 123.456F;
    FloatPoint fp(name, &f, 1);

    auto nv_opt = fp.NumericValue();
    if (nv_opt) {
      Number& number = *nv_opt;
      ASSERT_EQ(f, number.FloatValue());
    } else {
      FAIL();
    }
  }

  {
    float points[3] = { 12.3F, 45.123F, 101.12F };
    FloatPoint fp(name, points, 3);
    auto opt = fp.BinaryValue();
    if (opt) {
      BytesRef& bytes_ref = *opt;
      ASSERT_EQ(3 * sizeof(float), bytes_ref.length);

      float got = FloatPoint::DecodeDimension(bytes_ref.bytes.get(),
                                              1 * sizeof(float));
      ASSERT_EQ(45.123F, got);
    } else {
      FAIL();
    }
  }
}

TEST(FIELD__TESTS, FLOAT__RANGE__TEST) {
  {
    std::string name("field_name");
    float min[4] = { 12.12F, 13.13F, 14.14F, 15.15F };
    float max[4] = { 45.12F, 23.12F, 56.45F, 19.14F };
    FloatRange fr(name, min, max, 4);

    for (uint32_t i = 0 ; i < 4 ; ++i) {
      ASSERT_EQ(min[i], fr.GetMin(i));
      ASSERT_EQ(max[i], fr.GetMax(i));
    }
  }
}

TEST(FIELD__TESTS, INT__POINT__TEST) {
  std::string name("field_name");
  {
    int32_t points[3] = { 12, 34, 5678910 };
    IntPoint ip(name, points, 3);
    auto opt = ip.BinaryValue();
    if (opt) {
      BytesRef& bytes_ref = *opt;
      ASSERT_EQ(bytes_ref.length, 3 * sizeof(int32_t));

      for (uint32_t i = 0 ; i < 3 ; ++i) {
        int32_t point =
        IntPoint::DecodeDimension(bytes_ref.bytes.get(), i * sizeof(int32_t));
        ASSERT_EQ(points[i], point);
      }
    } else {
      FAIL();
    }
  }

  {
    int32_t point = 147;
    IntPoint ip(name, &point, 1);
    auto opt = ip.NumericValue();
    if (opt) {
      Number& num = *opt;
      ASSERT_EQ(point, num.IntValue());
    } else {
      FAIL();
    }
  }
}

TEST(FIELD__TESTS, INT__RANGE__TEST) {
  std::string name("field_name");

  {
    int32_t min[3] = {
      13, 45, 66
    };

    int32_t max[3] = {
      123123, 123141, 44949
    };

    IntRange ir(name, min, max, 3);
    for (uint32_t i = 0 ; i < 3 ; ++i) {
      ASSERT_EQ(min[i], ir.GetMin(i));
      ASSERT_EQ(max[i], ir.GetMax(i));
    }
  }
}

TEST(FIELD__TESTS, LONG__POINT__TEST) {
  std::string name("field_name");
  {
    int64_t points[3] = { 12L, 34L, 5678910L };
    LongPoint lp(name, points, 3);
    auto opt = lp.BinaryValue();
    if (opt) {
      BytesRef& bytes_ref = *opt;
      ASSERT_EQ(bytes_ref.length, 3 * sizeof(int64_t));

      for (uint32_t i = 0 ; i < 3 ; ++i) {
        int64_t point =
        LongPoint::DecodeDimension(bytes_ref.bytes.get(), i * sizeof(int64_t));
        ASSERT_EQ(points[i], point);
      }
    } else {
      FAIL();
    }
  }

  {
    int64_t point = 147L;
    LongPoint lp(name, &point, 1);
    auto opt = lp.NumericValue();
    if (opt) {
      Number& num = *opt;
      ASSERT_EQ(point, num.LongValue());
    } else {
      FAIL();
    }
  }
}

TEST(FIELD__TESTS, LONG__RANGE__TEST) {
  std::string name("field_name");

  {
    int64_t min[3] = {
      13L, 45L, 66L
    };

    int64_t max[3] = {
      123123L, 123141L, 44949L
    };

    LongRange lr(name, min, max, 3);
    for (uint32_t i = 0 ; i < 3 ; ++i) {
      ASSERT_EQ(min[i], lr.GetMin(i));
      ASSERT_EQ(max[i], lr.GetMax(i));
    }
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
