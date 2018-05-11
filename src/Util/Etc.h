#ifndef LUCENE_CORE_UTIL_ARRAY_ETC_H_
#define LUCENE_CORE_UTIL_ARRAY_ETC_H_

namespace lucene { namespace core { namespace util { namespace etc {

class Version {
  public:
    static Version LATEST;

  public:
    Version();
    Version(Version& other);
};

}}}}

#endif
