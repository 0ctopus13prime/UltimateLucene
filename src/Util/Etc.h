#ifndef LUCENE_CORE_UTIL_ARRAY_ETC_H_
#define LUCENE_CORE_UTIL_ARRAY_ETC_H_

#include <string>

namespace lucene { namespace core { namespace util { namespace etc {

class Version {
  public:
    static Version LATEST;

  private:
    Version(const int32_t major, const int32_t minor, const int32_t bugfix);
    Version(const int32_t major, const int32_t minor, const int32_t bugfix, const int32_t prerelease);
    bool EncodedIsValid() const;

  public:
    bool OnOrAfter(const Version& other) const;
    std::string ToString() const;
    bool operator=(const Version& other) const;

  public:
    static Version Parse(const std::string& version);
    static Version ParseLeniently(const std::string& version);
    static Version FromBits(const int32_t major, const int32_t minor, const int32_t bugfix);
};

}}}}

#endif
