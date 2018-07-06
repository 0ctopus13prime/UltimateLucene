#ifndef LUCENE_CORE_UTIL_ARRAY_ETC_H_
#define LUCENE_CORE_UTIL_ARRAY_ETC_H_

#include <string>

namespace lucene { namespace core { namespace util { namespace etc {

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
    Version(const uint8_t major, const uint8_t minor, const uint8_t bugfix, const uint8_t prerelease);
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
    static Version FromBits(const uint8_t major, const uint8_t minor, const uint8_t bugfix);
};

}}}}

#endif
