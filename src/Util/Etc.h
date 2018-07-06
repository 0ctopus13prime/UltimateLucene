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
    bool OnOrAfter(const Version& other) const;
    std::string ToString() const;
    bool operator==(const Version& other) const;

  public:
    static Version Parse(const std::string& version);
    static Version ParseLeniently(const std::string& version);
    static Version FromBits(const uint8_t major, const uint8_t minor, const uint8_t bugfix);
};

class StrictStringTokenizer {
  private:
    const std::string& s;
    const char delimiter;
    int32_t pos;

  public:
    StrictStringTokenizer(const std::string& s, char delimiter);
    const std::string next_token();
    const bool has_more_tokens() const {
      return pos >= 0;
    }
};

}}}}

#endif
