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

#ifndef SRC_UTIL_NUMERIC_H_
#define SRC_UTIL_NUMERIC_H_

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace lucene {
namespace core {
namespace util {
namespace numeric { // TODO(0ctopus13prime): Remove this.

typedef union {
  int16_t int16;
  char bytes[2];
} Int16AndBytes;

typedef union {
  int32_t int32;
  char bytes[4];
} Int32AndBytes;

typedef union {
  int64_t int64;
  char bytes[8];
} Int64AndBytes;

class FloatConsts {
 public:
  static const float POSITIVE_INFINITY;  // 1.0F / 0.0
  static const float NEGATIVE_INFINITY;  // -1.0F / 0.0;
  static const float NaN;  // Quiet NaN
  static const float MAX_VALUE;  // 3.4028235E38F;
  static const float MIN_VALUE;  // 1.4E-45F;
  static const float MIN_NORMAL;  // 1.17549435E-38F;
  static const int32_t SIGNIFICAND_WIDTH;  // 24;
  static const int32_t MAX_EXPONENT;  // 127;
  static const int32_t MIN_EXPONENT;  // -126;
  static const int32_t MIN_SUB_EXPONENT;  // -149;
  static const int32_t EXP_BIAS;  // 127;
  static const int32_t SIGN_BIT_MASK;  // -2147483648;
  static const int32_t EXP_BIT_MASK;  // 2139095040;
  static const int32_t SIGNIF_BIT_MASK;  // 8388607;
};

class Float {
 public:
  static bool IsNaN(const float v) noexcept {
    return std::isnan(v);
  }

  static int32_t FloatToIntBits(const float value) noexcept {
    const int32_t result = Float::FloatToRawIntBits(value);
    // Check for NaN based on values of bit fields, maximum
    // exponent and nonzero significand.
    if (((result & FloatConsts::EXP_BIT_MASK) == FloatConsts::EXP_BIT_MASK)
        && (result & FloatConsts::SIGNIF_BIT_MASK) != 0) {
      return 0x7fc00000;
    }

    return result;
  }

  static int32_t FloatToRawIntBits(const float value) noexcept {
    // Burrowed from open jdk's implementation.
    // In jni.h jfloat is just float's alias. Same as jint
    union {
      int32_t i;
      float f;
    } u;

    u.f = value;
    return u.i;
  }

  static float IntBitsToFloat(const int32_t value) noexcept {
    union {
      int32_t i;
      float f;
    } u;

    u.i = (int64_t) value;
    return u.f;
  }
};

class DoubleConsts {
 public:
  static const double POSITIVE_INFINITY;  // 1.0D / 0.0;
  static const double NEGATIVE_INFINITY;  // -1.0D / 0.0;
  static const double NaN;  // Quiet NaN
  static const double MAX_VALUE;  // 1.7976931348623157E308D;
  static const double MIN_VALUE;  // 4.9E-324D;
  static const double MIN_NORMAL;  // 2.2250738585072014E-308D;
  static const int32_t SIGNIFICAND_WIDTH;  // 53;
  static const int32_t MAX_EXPONENT;  // 1023;
  static const int32_t MIN_EXPONENT;  // -1022;
  static const int32_t MIN_SUB_EXPONENT;  // -1074;
  static const int32_t EXP_BIAS;  // 1023;
  static const int64_t SIGN_BIT_MASK;  // -9223372036854775808L;
  static const int64_t EXP_BIT_MASK;  // 9218868437227405312L;
  static const int64_t SIGNIF_BIT_MASK;  // 4503599627370495L;
};

class Double {
 public:
  static bool IsNaN(const double v) noexcept {
    return (v != v);
  }

  static int64_t DoubleToRawLongBits(const double value) noexcept {
    // Burrowed from open jdk's implementation.
    union {
      int64_t l;
      double d;
    } u;

    u.d = value;
    return u.l;
  }

  static double LongBitsToDouble(const int64_t value) noexcept {
    // Burrowed from open jdk's implementation.
    union {
        int64_t l;
        double d;
    } u;

    u.l = value;
    return u.d;
  }

  static int64_t DoubleToLongBits(const double value) noexcept {
    // Check for NaN based on values of bit fields, maximum
    // exponent and nonzero significand.
    const int64_t result = Double::DoubleToRawLongBits(value);
    if (((result & DoubleConsts::EXP_BIT_MASK) == DoubleConsts::EXP_BIT_MASK)
         && (result & DoubleConsts::SIGNIF_BIT_MASK) != 0L) {
      return 0x7ff8000000000000L;
    }

    return result;
  }
};

class Number {
 private:
  union NumberRepresentation {
    int64_t int64_value;
    double double_value;
  } rprs;
  bool is_integral;

 public:
  Number() { }

  template <typename T>
  explicit Number(T value) {
    if (std::is_integral<T>()) {
      rprs.int64_value = value;
      is_integral = true;
    } else {
      rprs.double_value = value;
      is_integral = false;
    }
  }

  Number(const Number& other)
    : rprs(other.rprs),
      is_integral(other.is_integral) {
  }

  template <typename T>
  Number& operator=(T value) noexcept {
    if (std::is_integral<T>()) {
      rprs.int64_value = value;
      is_integral = true;
    } else {
      rprs.double_value = value;
      is_integral = false;
    }

    return *this;
  }

  Number& operator=(const Number& other) noexcept {
    rprs = other.rprs;
    is_integral = other.is_integral;

    return *this;
  }

  int8_t ByteValue() const noexcept {
    if (is_integral) {
      return static_cast<int8_t>(rprs.int64_value);
    } else {
      return static_cast<int8_t>(rprs.double_value);
    }
  }

  int16_t ShortValue() const noexcept {
    if (is_integral) {
      return static_cast<int16_t>(rprs.int64_value);
    } else {
      return static_cast<int16_t>(rprs.double_value);
    }
  }

  int32_t IntValue() const noexcept {
    if (is_integral) {
      return static_cast<int32_t>(rprs.int64_value);
    } else {
      return static_cast<int32_t>(rprs.double_value);
    }
  }

  int64_t LongValue() const noexcept {
    if (is_integral) {
      return rprs.int64_value;
    } else {
      return static_cast<int64_t>(rprs.double_value);
    }
  }

  float FloatValue() const noexcept {
    if (is_integral) {
      return static_cast<float>(rprs.int64_value);
    } else {
      return static_cast<float>(rprs.double_value);
    }
  }

  double DoubleValue() const noexcept {
    if (is_integral) {
      return static_cast<double>(rprs.int64_value);
    } else {
      return rprs.double_value;
    }
  }
};

class NumericUtils {
 private:
  NumericUtils() { }

 public:
  static int64_t DoubleToSortableLong(const double value) noexcept {
    return SortableDoubleBits(Double::DoubleToLongBits(value));
  }

  static double SortableLongToDouble(const int64_t encoded) noexcept {
    return Double::LongBitsToDouble(NumericUtils::SortableDoubleBits(encoded));
  }

  static int32_t FloatToSortableInt(const float value) noexcept {
    return NumericUtils::SortableFloatBits(Float::FloatToIntBits(value));
  }

  static float SortableIntToFloat(const int32_t encoded) noexcept {
    return Float::IntBitsToFloat(NumericUtils::SortableFloatBits(encoded));
  }

  static int64_t SortableDoubleBits(const int64_t bits) noexcept {
    return bits ^ ((bits >> 63) & 0x7FFFFFFFFFFFFFFFL);
  }

  static int32_t SortableFloatBits(const int32_t bits) noexcept {
    return bits ^ ((bits >> 31) & 0x7FFFFFFF);
  }

  static void Substract4Bytes(const uint32_t dim_idx,
                              const char* dimension1,
                              const char* dimension2,
                              char* result) {
    const int32_t start = dim_idx * 4;
    const int32_t end = start + 4;
    int32_t num1, num2;
    Int32AndBytes iab;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[0] = dimension1[start + 3];
    iab.bytes[1] = dimension1[start + 2];
    iab.bytes[2] = dimension1[start + 1];
    iab.bytes[3] = dimension1[start];
    num1 = iab.int32;

    iab.bytes[0] = dimension2[start + 3];
    iab.bytes[1] = dimension2[start + 2];
    iab.bytes[2] = dimension2[start + 1];
    iab.bytes[3] = dimension2[start];
    num2 = iab.int32;
#else
    iab.bytes[0] = dimension1[start];
    iab.bytes[1] = dimension1[start + 1];
    iab.bytes[2] = dimension1[start + 2];
    iab.bytes[3] = dimension1[start + 3];
    num1 = iab.int32;

    iab.bytes[0] = dimension2[start];
    iab.bytes[1] = dimension2[start + 1];
    iab.bytes[2] = dimension2[start + 2];
    iab.bytes[3] = dimension2[start + 3];
    num2 = iab.int32;
#endif

    if (num1 < num2) {
      throw std::invalid_argument("Substract error in NumericUtils. a < b");
    }

    iab.int32 = num1 - num2;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    result[0] = iab.bytes[3];
    result[1] = iab.bytes[2];
    result[2] = iab.bytes[1];
    result[3] = iab.bytes[0];
#else
    result[0] = iab.bytes[0];
    result[1] = iab.bytes[1];
    result[2] = iab.bytes[2];
    result[3] = iab.bytes[3];
#endif
  }

  static void Substract(const uint32_t bytes_per_dim,
                        const uint32_t dim_idx,
                        const char* dimensions1,
                        const char* dimensions2,
                        char* result) {
    const int32_t start = dim_idx * bytes_per_dim;
    const int32_t end = start + bytes_per_dim;
    int32_t borrow = 0;

    for (int32_t i = end - 1 ; i >= start ; --i) {
      int32_t diff = (dimensions1[i] & 0xFF) - (dimensions2[i] & 0xFF) - borrow;

      if (diff < 0) {
        diff += 256;
        borrow = 1;
      } else {
        borrow = 0;
      }

      result[i - start] = static_cast<char>(diff);
    }

    if (borrow != 0) {
      throw std::invalid_argument("Substract error in NumericUtils. a < b");
    }
  }

  static void Add(const uint32_t bytes_per_dim,
                  const uint32_t dim,
                  const char* a,
                  const char* b,
                  char* result) {
    const int32_t start = dim * bytes_per_dim;
    const int32_t end = start + bytes_per_dim;
    int32_t carry = 0;

    for (int32_t i = end - 1 ; i >= start ; --i) {
      int32_t digit_sum = (a[i] & 0xFF) + (b[i] & 0xFF) + carry;

      if (digit_sum > 255) {
        digit_sum -= 256;
        carry = 1;
      } else {
        carry = 0;
      }

      result[i - start] = static_cast<char>(digit_sum);
    }

    if (carry != 0) {
      throw std::invalid_argument("Add error in NumericUtils. "
                                  "a + b overflows bytes_per_dim = "
                                  + std::to_string(bytes_per_dim));
    }
  }

  static void IntToSortableBytes(const int32_t value,
                                 char* result,
                                 const int32_t offset) noexcept {
    // Flip the sign bit, so negative ints sort before positive ints correctly
    const int32_t fliped_value = value ^ 0x80000000;
    Int32AndBytes iab;
    iab.int32 = fliped_value;
    char* dest = result + offset;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    dest[0] = iab.bytes[3];
    dest[1] = iab.bytes[2];
    dest[2] = iab.bytes[1];
    dest[3] = iab.bytes[0];
#else
    dest[0] = iab.bytes[0];
    dest[1] = iab.bytes[1];
    dest[2] = iab.bytes[2];
    dest[3] = iab.bytes[3];
#endif
  }

  static int32_t SortableBytesToInt(const char* encoded,
                                    const int32_t offset) noexcept {
    Int32AndBytes iab;
    const char* source = encoded + offset;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[0] = source[3];
    iab.bytes[1] = source[2];
    iab.bytes[2] = source[1];
    iab.bytes[3] = source[0];
#else
    iab.bytes[0] = source[0];
    iab.bytes[1] = source[1];
    iab.bytes[2] = source[2];
    iab.bytes[3] = source[3];
#endif

    // Re-flip the sign bit to restore the original value
    return (iab.int32 ^ 0x80000000);
  }

  static void LongToSortableBytes(const int64_t value,
                                  char result[],
                                  const uint32_t offset) noexcept {
    // Flip the sign bit, so negative ints sort before positive ints correctly
    Int64AndBytes iab;
    iab.int64 = value ^ 0x8000000000000000L;
    char* dest = result + offset;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    dest[0] = iab.bytes[7];
    dest[1] = iab.bytes[6];
    dest[2] = iab.bytes[5];
    dest[3] = iab.bytes[4];
    dest[4] = iab.bytes[3];
    dest[5] = iab.bytes[2];
    dest[6] = iab.bytes[1];
    dest[7] = iab.bytes[0];
#else
    dest[0] = iab.bytes[0];
    dest[1] = iab.bytes[1];
    dest[2] = iab.bytes[2];
    dest[3] = iab.bytes[3];
    dest[4] = iab.bytes[4];
    dest[5] = iab.bytes[5];
    dest[6] = iab.bytes[6];
    dest[7] = iab.bytes[7];
#endif
  }

  static int64_t SortableBytesToLong(const char* encoded,
                                     const int32_t offset) noexcept {
    Int64AndBytes iab;
    const char* source = encoded + offset;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    iab.bytes[0] = source[7];
    iab.bytes[1] = source[6];
    iab.bytes[2] = source[5];
    iab.bytes[3] = source[4];
    iab.bytes[4] = source[3];
    iab.bytes[5] = source[2];
    iab.bytes[6] = source[1];
    iab.bytes[7] = source[0];
#else
    iab.bytes[0] = source[0];
    iab.bytes[1] = source[1];
    iab.bytes[2] = source[2];
    iab.bytes[3] = source[3];
    iab.bytes[4] = source[4];
    iab.bytes[5] = source[5];
    iab.bytes[6] = source[6];
    iab.bytes[7] = source[7];
#endif

    // Re-flip the sign bit to restore the original value
    return (iab.int64 ^ 0x8000000000000000L);
  }

  // TODO(0ctopus13prime): Implement below functions
  // public static void bigIntToSortableBytes(BigInteger bigInt, int bigIntSize,
  //                                          byte[] result, int offset) {
  // public static BigInteger sortableBytesToBigInt(byte[] encoded, int offset,
  //                                                int length) {

  static float FloatNextUp(const float f) noexcept {
    if (Float::IsNaN(f) || f == FloatConsts::POSITIVE_INFINITY) {
      return f;
    } else {
      float tmp = f + 0.0F;
      return Float::IntBitsToFloat(Float::FloatToRawIntBits(tmp)
             + (tmp >= 0.0F ? 1 : -1));
    }
  }

  static float FloatNextDown(const float f) noexcept {
    if (Float::IsNaN(f) || f == FloatConsts::NEGATIVE_INFINITY) {
      return f;
    } else {
      if (f == 0.0F) {
        return -FloatConsts::MIN_VALUE;
      } else {
        return Float::IntBitsToFloat(Float::FloatToRawIntBits(f)
               + (f > 0.0F ? -1 : 1));
      }
    }
  }

  static double DoubleNextUp(const double d) noexcept {
    if (Double::IsNaN(d) || d == DoubleConsts::POSITIVE_INFINITY) {
      return d;
    } else {
      double tmp = d + 0.0D;
      return Double::LongBitsToDouble(Double::DoubleToRawLongBits(tmp)
             + (tmp >= 0.0F ? 1 : -1));
    }
  }

  static float DoubleNextDown(const double d) noexcept {
    if (Double::IsNaN(d) || d == DoubleConsts::NEGATIVE_INFINITY) {
      return d;
    } else {
      if (d == 0.0D) {
        return -DoubleConsts::MIN_VALUE;
      } else {
        return Double::LongBitsToDouble(Double::DoubleToRawLongBits(d)
               + (d > 0.0D ? -1 : 1));
      }
    }
  }
};

}  // namespace numeric
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_NUMERIC_H_
