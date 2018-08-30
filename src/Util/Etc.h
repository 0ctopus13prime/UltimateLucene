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

#include <Util/ZlibCrc32.h>
#include <string>

namespace lucene {
namespace core {
namespace util {

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

class Checksum {
 public:
  Checksum() { }
  virtual ~Checksum() { }
  virtual void Update(const char b) = 0;
  virtual void Update(const char bytes[],
                      const uint32_t off,
                      const uint32_t len) = 0;
  virtual int64_t GetValue() = 0;
  virtual void Reset() = 0;
};

class Crc32: public Checksum {
 private:
  int32_t crc;

 public:
  Crc32()
    : Checksum(),
      crc(0) {
  }

  ~Crc32() { }

  void Update(const char b) {
    Update(&b, 0, 1);
  }

  void Update(const char bytes[], const uint32_t off, const uint32_t len) {
    crc = _l_crc32(crc,
                  reinterpret_cast<const unsigned char*>(bytes + off),
                  len);
  }

  int64_t GetValue() {
    return (static_cast<int64_t>(crc)) & 0xFFFFFFFFL;
  }

  void Reset() {
    crc = 0;
  }
};

class IntsRefBuilder {

};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_ETC_H_
