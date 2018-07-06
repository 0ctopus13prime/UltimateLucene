#ifndef LUCENE_CORE_UTIL_ARRAY_ETC_H_
#define LUCENE_CORE_UTIL_ARRAY_ETC_H_

#include <string>

namespace lucene { namespace core { namespace util { namespace etc {

class Version {
  public:
    static Version LATEST;
    const uint8_t major;
    const uint8_t minor;
    const uint8_t bugfix;
    const uint8_t prerelease;

  private:
    const uint32_t encoded_value;

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
    bool operator==(const Version& other) const;
    bool operator==(Version&& other) const;

  public:
    static Version Parse(const std::string& version);
    static Version Parse(std::string&& version);
    static Version ParseLeniently(const std::string& version);
    static Version ParseLeniently(std::string&& version);
    static Version FromBits(const uint8_t major, const uint8_t minor, const uint8_t bugfix);
};

}}}}

#endif
