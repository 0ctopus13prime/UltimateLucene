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
#include <stdexcept>
#include <string>

namespace lucene {
namespace core {
namespace util {
namespace numeric {

class FloatConsts {
 public:
  static const float POSITIVE_INFINITY; // 1.0F / 0.0
  static const float NEGATIVE_INFINITY; // -1.0F / 0.0;
  static const float NaN; // 0.0F / 0.0;
  static const float MAX_VALUE; // 3.4028235E38F;
  static const float MIN_VALUE; // 1.4E-45F;
  static const float MIN_NORMAL; // 1.17549435E-38F;
  static const int32_t SIGNIFICAND_WIDTH; // 24;
  static const int32_t MAX_EXPONENT; // 127;
  static const int32_t MIN_EXPONENT; // -126;
  static const int32_t MIN_SUB_EXPONENT; // -149;
  static const int32_t EXP_BIAS; // 127;
  static const int32_t SIGN_BIT_MASK; // -2147483648;
  static const int32_t EXP_BIT_MASK; // 2139095040;
  static const int32_t SIGNIF_BIT_MASK; // 8388607;  
};

class Float {
 public:
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
  static const double POSITIVE_INFINITY; // 1.0D / 0.0;
  static const double NEGATIVE_INFINITY; // -1.0D / 0.0;
  static const double NaN; // 0.0D / 0.0;
  static const double MAX_VALUE; // 1.7976931348623157E308D;
  static const double MIN_VALUE; // 4.9E-324D;
  static const double MIN_NORMAL; // 2.2250738585072014E-308D;
  static const int32_t SIGNIFICAND_WIDTH; // 53;
  static const int32_t MAX_EXPONENT; // 1023;
  static const int32_t MIN_EXPONENT; // -1022;
  static const int32_t MIN_SUB_EXPONENT; // -1074;
  static const int32_t EXP_BIAS; // 1023;
  static const int64_t SIGN_BIT_MASK; // -9223372036854775808L;
  static const int64_t EXP_BIT_MASK; // 9218868437227405312L;
  static const int64_t SIGNIF_BIT_MASK; // 4503599627370495L;
};

class Double {
 public:
  static bool IsNaN(const double v) noexcept {
    return (v != v);
  }

  static bool IsInfinite(const double v) noexcept {
    return (v == DoubleConsts::POSITIVE_INFINITY)
           || (v == DoubleConsts::NEGATIVE_INFINITY);
  }

  static bool IsFinite(const double v) noexcept {
    return std::abs(v) <= DoubleConsts::MAX_VALUE;
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
    return bits ^ ((bits >> 63) & 0x7FFFFFFFFFFFFFFF);
  }

  static int32_t SortableFloatBits(const int32_t bits) noexcept {
    return bits ^ ((bits >> 31) & 0x7FFFFFFF);
  }

  static void Subtract(const int32_t bytes_per_dim,
                       const int32_t dim,
                       const char* a,
                       const char* b,
                       char* result) {
    const int32_t start = dim * bytes_per_dim;
    const int32_t end = start + bytes_per_dim;
    int32_t borrow = 0;

    for (int32_t i = end - 1 ; i >= start ; --i) {
      int32_t diff = (a[i] & 0xFF) - (b[i] & 0xFF) - borrow;

      if (diff < 0) {
        diff += 256;
        borrow = 1;
      } else {
        borrow = 0;
      }

      result[i - start] = (char) diff;
    }

    if (borrow != 0) {
      throw std::invalid_argument("Substract error in NumericUtils. a < b");
    }
  }

  static void Add(const int32_t bytes_per_dim,
                  const int32_t dim,
                  const char* a,
                  const char* b,
                  char* result) {
    const int32_t start = dim * bytes_per_dim;
    const int32_t end = start + bytes_per_dim;
    int32_t carry = 0;

    for (int32_t i = end - 1 ; i > start ; --i) {
      int32_t digit_sum = (a[i] & 0xFF) + (b[i] & 0xFF) + carry;

      if (digit_sum > 255) {
        digit_sum -= 256;
        carry = 1;
      } else {
        carry = 0;
      }

      result[i - start] = (char) digit_sum;
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
    // TODO(0ctopus13prime): Can we make this simpler?
    // Without considering an endian issue, this can be done easily
    // just mapping char array into int64_t. Ex) *((int64_t*)(encoded))

    // Flip the sign bit, so negative ints sort before positive ints correctly 
    const int32_t fliped_value = value ^ 0x80000000;
    result[offset] = (char) (fliped_value >> 24);
    result[offset + 1] = (char) (fliped_value >> 16);
    result[offset + 2] = (char) (fliped_value >> 8);
    result[offset + 3] = (char) fliped_value;
  }

  static int32_t SortableBytesToInt(const char* encoded,
                                    const int32_t offset) noexcept {
    // TODO(0ctopus13prime): Can we make this simpler?
    // Without considering an endian issue, this can be done easily
    // just mapping char array into int64_t. Ex) *((int64_t*)(encoded))

    const int32_t x = ((encoded[offset] & 0xFF) << 24) | 
                      ((encoded[offset + 1] & 0xFF) << 16) |
                      ((encoded[offset + 2] & 0xFF) <<  8) | 
                      (encoded[offset + 3] & 0xFF);

    // Re-flip the sign bit to restore the original value
    return (x ^ 0x80000000);
  }

  static void LongToSortableBytes(const int64_t value,
                                  char* result,
                                  const int32_t offset) noexcept {
    // TODO(0ctopus13prime): Can we make this simpler?
    // Without considering an endian issue, this can be done easily
    // just mapping char array into int64_t. Ex) *((int64_t*)(encoded))

    // Flip the sign bit so negative longs sort before positive longs:
    const int64_t fliped_value = value ^ 0x8000000000000000L;
    result[offset] =   (char) (fliped_value >> 56);
    result[offset + 1] = (char) (fliped_value >> 48);
    result[offset + 2] = (char) (fliped_value >> 40);
    result[offset + 3] = (char) (fliped_value >> 32);
    result[offset + 4] = (char) (fliped_value >> 24);
    result[offset + 5] = (char) (fliped_value >> 16);
    result[offset + 6] = (char) (fliped_value >> 8);
    result[offset + 7] = (char) fliped_value;
  }

  // public static long sortableBytesToLong(byte[] encoded, int offset) {
  static int64_t SortableBytesToLong(const char* encoded,
                                     const int32_t offset) noexcept {
    // TODO(0ctopus13prime): Can we make this simpler?
    // Without considering an endian issue, this can be done easily
    // just mapping char array into int64_t. Ex) *((int64_t*)(encoded))
    const int64_t v = ((encoded[offset] & 0xFFL) << 56)   |
                      ((encoded[offset + 1] & 0xFFL) << 48) |
                      ((encoded[offset + 2] & 0xFFL) << 40) |
                      ((encoded[offset + 3] & 0xFFL) << 32) |
                      ((encoded[offset + 4] & 0xFFL) << 24) |
                      ((encoded[offset + 5] & 0xFFL) << 16) |
                      ((encoded[offset + 6] & 0xFFL) << 8)  |
                      (encoded[offset + 7] & 0xFFL);
    // Flip the sign bit back
    return (v ^ 0x8000000000000000L);
  }

  // TODO(0ctopus13prime): Implement below functions
  // public static void bigIntToSortableBytes(BigInteger bigInt, int bigIntSize,
  //                                          byte[] result, int offset) {
  // public static BigInteger sortableBytesToBigInt(byte[] encoded, int offset,
  //                                                int length) {
};

}  // namespace numeric 
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_NUMERIC_H_
