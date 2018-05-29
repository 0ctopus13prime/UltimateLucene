#ifndef LUCENE_CORE_UTIL_BYTES_H_
#define LUCENE_CORE_UTIL_BYTES_H_

#include <string>
#include <memory>

namespace lucene { namespace core { namespace util {

class BytesRef {
  private:
    static std::shared_ptr<char> DEFAULT_BYTES;

  private:
    int32_t CompareTo(const BytesRef& other) const;

  public:
    std::shared_ptr<char> bytes;
    uint32_t offset;
    uint32_t length;
    uint32_t capacity;

  public:
    BytesRef();
    BytesRef(char* bytes, uint32_t offset, uint32_t length, uint32_t capacity);
    BytesRef(char* bytes, uint32_t capacity);
    BytesRef(const BytesRef& other);
    BytesRef(uint32_t capacity);
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
    const uint32_t Length() const;
    void SetLength(const uint32_t length);
    char& operator[](const uint32_t idx);
    void Grow(uint32_t capacity);
    void Append(const char c);
    void Append(const char* c, const uint32_t off, const uint32_t len);
    void Append(BytesRef& ref);
    void Append(BytesRefBuilder& builder);
    void Clear();
    void CopyBytes(const char* c, const uint32_t off, uint32_t len);
    void CopyBytes(BytesRef& ref);
    void CopyBytes(BytesRefBuilder& builder);
    void CopyChars(std::string& text);
    void CopyChars(std::string& text, const uint32_t off, const uint32_t len);
    BytesRef& Get();
    BytesRef ToBytesRef() const;
};

}}} // End of namespace

#endif
