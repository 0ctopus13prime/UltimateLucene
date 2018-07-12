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

#include <Util/Etc.h>
#include <cassert>
#include <locale>
#include <regex>
#include <stdexcept>
#include <vector>

/**
 * The problem is that g++ adds -D_GNU_SOURCE and major() is a macro
 * in _GNU_SOURCE (or _BSD_SOURCE or when no feature set is requested).
 * So we can't compile source unless we don't undef major and minor before
 * the use. Please refer https://bugzilla.redhat.com/show_bug.cgi?id=130601
 */
#undef major
#undef minor

using lucene::core::util::etc::Version;

/**
 *  Version
 */
Version Version::LATEST(7, 3, 0);

Version::Version(const Version& other)
  : Version(other.major, other.minor, other.bugfix, other.prerelease) {
}

Version::Version(Version&& other)
  : Version(other.major, other.minor, other.bugfix, other.prerelease) {
}

Version::Version(const uint8_t major, const uint8_t minor, const uint8_t bugfix)
  : Version(major, minor, bugfix, 0) {
}

Version::Version(const uint8_t major,
                 const uint8_t minor,
                 const uint8_t bugfix,
                 const uint8_t prerelease)
  : major(major),
    minor(minor),
    bugfix(bugfix),
    prerelease(prerelease),
    encoded_value((major << 18) | (minor << 10) | (bugfix << 2) | prerelease) {
  if (prerelease > 2) {
    throw std::invalid_argument("Illegal prerelease version: "
                                + std::to_string(prerelease));
  }
  if (prerelease != 0 && (minor != 0 || bugfix != 0)) {
    throw std::invalid_argument("Prerelease version only supported with major"
                                "release got prerelease: "
                                + std::to_string(prerelease)
                                + ", minor: " + std::to_string(minor)
                                + ", minor: " + std::to_string(minor)
                                + ", bugfix: " + std::to_string(bugfix) + ")");
  }

  assert(EncodedIsValid());
}

bool Version::EncodedIsValid() const {
  assert(major == ((encoded_value >> 18) & 0xFF));
  assert(minor == ((encoded_value >> 10) & 0xFF));
  assert(bugfix == ((encoded_value >> 2) & 0xFF));
  assert(prerelease == (encoded_value & 0x03));

  return true;
}

bool Version::OnOrAfter(Version&& other) const {
  return encoded_value >= other.encoded_value;
}

bool Version::OnOrAfter(const Version& other) const {
  return encoded_value >= other.encoded_value;
}

std::string Version::ToString() const {
  return std::to_string(major) + "." +
         std::to_string(minor) + "." +
         std::to_string(bugfix) +
         (prerelease == 0 ? "" : "." + std::to_string(prerelease));
}

bool Version::operator==(const Version& other) const {
  return encoded_value == other.encoded_value;
}

bool Version::operator==(Version&& other) const {
  return encoded_value == other.encoded_value;
}

Version& Version::operator=(const Version& other) {
  major = other.major;
  minor = other.minor;
  bugfix = other.bugfix;
  prerelease = other.prerelease;
  encoded_value = other.encoded_value;
}

Version& Version::operator=(Version&& other) {
  major = other.major;
  minor = other.minor;
  bugfix = other.bugfix;
  prerelease = other.prerelease;
  encoded_value = other.encoded_value;
}

Version Version::FromBits(const uint8_t major,
                          const uint8_t minor,
                          const uint8_t bugfix) {
  return Version(major, minor, bugfix);
}

Version Version::Parse(std::string&& version) {
  return Version::Parse(version);
}

Version Version::Parse(const std::string& version) {
  const std::string expr_str =
  R"(^(\d{1,2}|1\d{2}|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d{2}|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d{2}|2[0-4]\d|25[0-5])(\.[0-2])?$)";
  std::regex e(expr_str);

  if (!std::regex_match(version, e)) {
    throw std::runtime_error(
      "Illegal version format. Format is "
      "`Major.Minor.Bugfix(.PreRelease)?.` Major, "
      "Minor and Bugfix's range should belong to [0, 255]"
      ", PreRelease's range should belong [0, 2]");
  }

  std::smatch m;
  std::regex_search(version, m, e);
  std::vector<std::string> parts;
  parts.reserve(4);

  auto it = m.begin();
  ++it;  // Skip a entire string.

  for ( ; it != m.end() ; ++it) {
    const std::string& token = *it;
    if (!token.empty() && token[0] == '.') {
      parts.push_back(std::string(token, 1));
    } else {
      parts.push_back(token);
    }
  }

  const uint8_t major = std::atoi(parts[0].c_str());
  const uint8_t minor = std::atoi(parts[1].c_str());
  const uint8_t bugfix = std::atoi(parts[2].c_str());

  if (parts.size() == 4) {
    const uint8_t prerelease = std::atoi(parts[3].c_str());
    return Version(major, minor, bugfix, prerelease);
  } else {
    return Version(major, minor, bugfix);
  }
}

Version Version::ParseLeniently(std::string&& version) {
  return Version::ParseLeniently(version);
}

Version Version::ParseLeniently(const std::string& version) {
  const std::string expr_str1 = R"(^LUCENE_(\d+)_(\d+)_(\d+)$)";
  const std::string expr_str2 = R"(^LUCENE_(\d+)_(\d+)$)";
  const std::string expr_str3 = R"(^LUCENE_(\d)(\d)$)";

  std::string version_cpy(version);
  for (char& ch : version_cpy) ch = std::toupper(ch);

  if (version_cpy == "LATEST" || version_cpy == "LUCENE_CURRENT") {
    return Version::LATEST;
  } else {
    std::string replaced;
    if (std::regex_match(version_cpy, std::regex(expr_str1))) {
      replaced = std::regex_replace(version_cpy,
                                    std::regex(expr_str1),
                                    "$1.$2.$3");
    } else if (std::regex_match(version_cpy, std::regex(expr_str2))) {
      replaced = std::regex_replace(version_cpy,
                                    std::regex(expr_str2),
                                    "$1.$2.0");
    } else if (std::regex_match(version_cpy, std::regex(expr_str3))) {
      replaced = std::regex_replace(version_cpy,
                                    std::regex(expr_str3),
                                    "$1.$2.0");
    }

    try {
      return Version::Parse(replaced);
    } catch(std::runtime_error& e) {
      throw std::runtime_error("Failed to parse lenient version string "
                               + version + ": "
                               + e.what());
    } catch(std::invalid_argument& e) {
      throw std::runtime_error("Failed to parse lenient version string "
                               + version + ": "
                               + e.what());
    }
  }
}
