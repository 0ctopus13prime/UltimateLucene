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

#include <
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

class Checksum {
 public:
  virtual ~Checksum() { }
  virtual Update(const char b);
  virtual Update(char bytes[], const int32_t off, const int32_t len);
  virtual int64_t GetValue();
  virtual void Reset();
};

class Crc32: public Checksum {

};

}  // namespace etc
}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_ETC_H_
