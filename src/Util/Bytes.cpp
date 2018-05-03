#include <stdexcept>
#include <cstring>
#include <string>
#include <assert.h>
#include <Util/Bytes.h>

using namespace lucene::core::util;

BytesRef::BytesRef()
  : BytesRef(BytesRef::BYTES_REF_EMPTY_BYTES, 0, 1) {
}

// It's a shallow copy
BytesRef::BytesRef(const BytesRef& other)
  : BytesRef(other.bytes, other.offset, other.length) {
}

BytesRef::BytesRef(char* bytes, unsigned int offset, unsigned int length)
  : bytes(bytes),
    offset(offset),
    length(length) {
  assert(IsValid());
}

void BytesRef::DeepCopyOf(BytesRef& source, BytesRef& target) {
  target.length = source.length;
  target.offset = source.offset;
  int effective_length = source.length - source.offset;
  if(target.bytes != BytesRef::BYTES_REF_EMPTY_BYTES) delete[] target.bytes;
  target.bytes = new char[effective_length];
  std::memcpy(target.bytes, source.bytes + source.offset, effective_length);
}

BytesRef::BytesRef(unsigned int capacity)
  : BytesRef(new char[capacity], 0, 0) {
}

BytesRef::BytesRef(std::string& text) {
  if(text.empty()) {
    bytes = BytesRef::BYTES_REF_EMPTY_BYTES;
    offset = 0;
    length = 1;
  } else {
    const char* cstr = text.c_str();
    offset = 0;
    length = text.size();
    bytes = new char[length];
    std::memcpy(bytes, cstr, length);
  }
}

BytesRef::~BytesRef() {
  if(bytes != BytesRef::BYTES_REF_EMPTY_BYTES) {
    delete[] bytes;
  }
}

int BytesRef::CompareTo(BytesRef& other) {
  if(IsValid() && other.IsValid()) {
    unsigned int my_len = length - offset;
    unsigned int his_len = other.length - other.offset;
    unsigned int len = (my_len < his_len ? my_len : his_len);

    for(int i = 0 ; i < len ; ++i) {
      char mine = bytes[i];
      char his = other.bytes[i];
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

bool BytesRef::operator==(BytesRef& other) {
  return CompareTo(other) == 0;
}

bool BytesRef::operator!=(BytesRef& other) {
  return !operator==(other);
}

bool BytesRef::operator<(BytesRef& other) {
  return CompareTo(other) < 0;
}

bool BytesRef::operator<=(BytesRef& other) {
  return CompareTo(other) <= 0;
}

bool BytesRef::operator>(BytesRef& other) {
  return CompareTo(other) > 0;
}

bool BytesRef::operator>=(BytesRef& other) {
  return CompareTo(other) >= 0;
}

std::string BytesRef::UTF8ToString() {
  return std::string(bytes, offset, length);
}

bool BytesRef::IsValid() {
  if(bytes == BytesRef::BYTES_REF_EMPTY_BYTES && (offset != 0 || length != 1)) {
    throw std::runtime_error("bytes is BytesRef::BYTES_REF_EMPTY_BYTES, offset=" + std::to_string(offset) + ", length=" + std::to_string(length));
  }
  if(bytes == nullptr) {
    throw std::runtime_error("bytes is null");
  }
  if(offset > length) {
    throw std::overflow_error("Offset out of bounds: " + std::to_string(offset) + ", length=" + std::to_string(length));
  }

  return true;
}
