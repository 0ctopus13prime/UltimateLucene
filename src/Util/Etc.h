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

#ifndef SRC_UTIL_ETC_H_
#define SRC_UTIL_ETC_H_

#include <string>

namespace lucene {
namespace core {
namespace util {
namespace etc {

class Version {
 public:
  static Version LATEST;

 private:
  uint8_t major;
  uint8_t minor;
  uint8_t bugfix;
  uint8_t prerelease;
  uint32_t encoded_value;

 private:
  Version(const uint8_t major, const uint8_t minor, const uint8_t bugfix);
  Version(const uint8_t major,
          const uint8_t minor,
          const uint8_t bugfix,
          const uint8_t prerelease);
  bool EncodedIsValid() const;

 public:
  Version(const Version& other);
  Version(Version&& other);
  bool OnOrAfter(const Version& other) const;
  bool OnOrAfter(Version&& other) const;
  std::string ToString() const;
  Version& operator=(const Version& other);
  Version& operator=(Version&& other);
  bool operator==(const Version& other) const;
  bool operator==(Version&& other) const;
  uint8_t GetMajor() const {
    return major;
  }
  uint8_t GetMinor() const {
    return minor;
  }
  uint8_t GetBugfix() const {
    return bugfix;
  }
  uint8_t GetPreRelease() const {
    return prerelease;
  }

 public:
  static Version Parse(const std::string& version);
  static Version Parse(std::string&& version);
  static Version ParseLeniently(const std::string& version);
  static Version ParseLeniently(std::string&& version);
  static Version FromBits(const uint8_t major,
                          const uint8_t minor,
                          const uint8_t bugfix);
};

class Float {
 public:
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
};

class Double {
 public:
  static int64_t DoubleToRawLongBits(const double value) noexcept {
    // Burrowed from open jdk's implementation.
    union {
      int64_t l;
      double d;
    } u;

    u.d = value;
    return u.l;
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

}  // namespace etc
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_ETC_H_
