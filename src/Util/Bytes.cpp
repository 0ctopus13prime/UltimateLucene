#include <stdexcept>
#include <cstring>
#include <string>
#include <assert.h>
#include <Util/Bytes.h>

using namespace lucene::core::util;

/*
 * BytesRef
 */

BytesRef::BytesRef()
  : BytesRef(nullptr, 0, 0) {
}

BytesRef::BytesRef(const BytesRef& source)
  : bytes(source.bytes),
    offset(source.offset),
    length(source.length) {
}

BytesRef::BytesRef(char* new_bytes_ptr, unsigned int new_offset, unsigned int new_length) {
  bytes = std::shared_ptr<char>(nullptr, std::default_delete<char[]>());
  offset = new_offset;
  length = new_length;

  if(new_bytes_ptr && offset < length) {
    int effective_length = length - offset;
    char* bytes_ptr = new char[effective_length];
    std::memcpy(bytes_ptr, new_bytes_ptr + new_offset, effective_length);
    bytes.reset(bytes_ptr);
  } 

  assert(IsValid());
}

BytesRef::BytesRef(unsigned int capacity) {
  bytes = std::shared_ptr<char>(nullptr, std::default_delete<char[]>());
  offset = length = 0;

  if(capacity > 0) {
    char* bytes_ptr = new char[capacity];
    bytes.reset(bytes_ptr);
  }
}

BytesRef::BytesRef(std::string& text) {
  bytes = std::shared_ptr<char>(nullptr, std::default_delete<char[]>());

  if(text.empty()) {
    length = offset = 0;
  } else {
    const char* cstr = text.c_str();
    offset = 0;
    length = text.size();
    char* bytes_ptr = new char[length];
    std::memcpy(bytes_ptr, cstr, length);
    bytes.reset(bytes_ptr);
  }
}

BytesRef::~BytesRef() {
}

void BytesRef::DeepCopyOf(BytesRef& other) {
  offset = other.offset;
  length = other.length;
  unsigned int effective_length = (length - offset);
  char* his_bytes_ptr = other.bytes.get();
  
  if(his_bytes_ptr && offset < length) {
    char* my_bytes_ptr = new char[effective_length];
    std::memcpy(my_bytes_ptr, his_bytes_ptr + other.offset, other.length);
    bytes.reset(my_bytes_ptr);
  }
}

int BytesRef::CompareTo(const BytesRef& other) const {
  if(IsValid() && other.IsValid()) {
    if(bytes == other.bytes && offset == other.offset && length == other.length) {
      return 0;
    }

    unsigned int my_len = length - offset;
    unsigned int his_len = other.length - other.offset;
    unsigned int len = (my_len < his_len ? my_len : his_len);
    char* my_bytes_ptr = bytes.get();
    char* his_bytes_ptr = other.bytes.get();

    for(int i = 0 ; i < len ; ++i) {
      char mine = my_bytes_ptr[i];
      char his = his_bytes_ptr[i];
      char diff = (mine - his);
      if(diff != 0) {
        return diff;
      }
    }

    // One is a prefix of another, or, they are equal.
    return (my_len - his_len);
  }

  return false;
}

BytesRef& BytesRef::operator=(const BytesRef& source) {
  if(this != &source) {
    length = source.length;
    offset = source.offset;
    bytes = source.bytes;
  }

  return *this;
}

bool BytesRef::operator==(const BytesRef& other) const {
  return CompareTo(other) == 0;
}

bool BytesRef::operator!=(const BytesRef& other) const {
  return !operator==(other);
}

bool BytesRef::operator<(const BytesRef& other) const {
  return CompareTo(other) < 0;
}

bool BytesRef::operator<=(const BytesRef& other) const {
  return CompareTo(other) <= 0;
}

bool BytesRef::operator>(const BytesRef& other) const {
  return CompareTo(other) > 0;
}

bool BytesRef::operator>=(const BytesRef& other) const {
  return CompareTo(other) >= 0;
}

std::string BytesRef::UTF8ToString() {
  return std::string(bytes.get(), offset, length);
}

bool BytesRef::IsValid() const {
  // In C++ BytesRef allows null bytes
  if(bytes.use_count() == 0 && (offset != 0 || length != 0)) {
    throw std::runtime_error("bytes is empty, offset=" + std::to_string(offset) + ", length=" + std::to_string(length));
  }

  if(offset > length) {
    throw std::overflow_error("Offset out of bounds: " + std::to_string(offset) + ", length=" + std::to_string(length));
  }

  return true;
}
