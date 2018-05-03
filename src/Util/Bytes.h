#ifndef LUCENE_CORE_UTIL_BYTES_H_
#define LUCENE_CORE_UTIL_BYTES_H_

#include <string>
#include <memory>

namespace lucene { namespace core { namespace util {

class BytesRef {
  private:
    int CompareTo(const BytesRef& other) const;

  public:
    std::shared_ptr<char> bytes;
    unsigned int offset;
    unsigned int length;

  public:
    BytesRef();
    BytesRef(char* bytes, unsigned int offset, unsigned int length);
    BytesRef(const BytesRef& other);
    BytesRef(unsigned int capacity);
    BytesRef(std::string& text);
    ~BytesRef();
    void DeepCopyOf(BytesRef& other);
    BytesRef& operator=(const BytesRef& other);
    bool operator==(const BytesRef& other) const;
    bool operator!=(const BytesRef& other) const;
    bool operator<(const BytesRef& other) const;
    bool operator<=(const BytesRef& other) const;
    bool operator>(const BytesRef& other) const;
    bool operator>=(const BytesRef& other) const;
    std::string UTF8ToString();
    bool IsValid() const;
};

class BytesRefBuilder {

};

}}} // End of namespace

#endif
