#ifndef LUCENE_CORE_UTIL_BYTES_H_
#define LUCENE_CORE_UTIL_BYTES_H_

#include <string>

namespace lucene { namespace core { namespace util {

class BytesRef {
  private:
    int CompareTo(const BytesRef& other) const;

  public:
    static char BYTES_REF_EMPTY_BYTES[1];
    char* bytes;
    unsigned int offset;
    unsigned int length;

  public:
    BytesRef();
    BytesRef(char* bytes, unsigned int offset, unsigned int length);
    BytesRef(const BytesRef& other);
    BytesRef(unsigned int capacity);
    BytesRef(std::string& text);
    ~BytesRef();
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
//char BytesRef::BYTES_REF_EMPTY_BYTES[1] = {'\0'};

class SharedBytesRef: public BytesRef {
  public:
    SharedBytesRef(char* bytes, unsigned int offset, unsigned int length);
    SharedBytesRef(const BytesRef& other);
    ~SharedBytesRef();
    SharedBytesRef& operator=(const BytesRef& other);
};

}}} // End of namespace

#endif
