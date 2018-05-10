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
    unsigned int capacity;

  public:
    BytesRef();
    BytesRef(char* bytes, unsigned int offset, unsigned int length, unsigned int capacity);
    BytesRef(char* bytes, unsigned int capacity);
    BytesRef(const BytesRef& other);
    BytesRef(unsigned int capacity);
    BytesRef(std::string& text);
    ~BytesRef();
    void ShallowCopyTo(BytesRef& target);
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
  private:
    BytesRef ref;

  public:
    BytesRefBuilder();
    const char* Bytes() const;
    const unsigned int Length() const;
    void SetLength(const unsigned int length);
    char& operator[](const unsigned int idx);
    void Grow(unsigned int capacity);
    void Append(const char c);
    void Append(const char* c, const unsigned int off, const unsigned int len);
    void Append(BytesRef& ref);
    void Append(BytesRefBuilder& builder);
    void Clear();
    void CopyBytes(const char* c, const unsigned int off, unsigned int len);
    void CopyBytes(BytesRef& ref);
    void CopyBytes(BytesRefBuilder& builder);
    void CopyChars(std::string& text);
    void CopyChars(std::string& text, const unsigned int off, const unsigned int len);
    BytesRef& Get();
    BytesRef ToBytesRef() const;
};

}}} // End of namespace

#endif
