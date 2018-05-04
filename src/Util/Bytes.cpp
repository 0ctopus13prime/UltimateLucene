#include <memory>
#include <stdexcept>
#include <cstring>
#include <string>
#include <assert.h>
#include <Util/Bytes.h>
#include <Util/ArrayUtil.h>

using namespace lucene::core::util;

/*
 * BytesRef
 */

BytesRef::BytesRef()
  : BytesRef(std::shared_ptr<char>(nullptr, std::default_delete<char[]>()), 0, 0, 0) {
}

BytesRef::BytesRef(const BytesRef& source)
  : BytesRef(source.bytes, source.offset, source.length, source.capacity) {
}

BytesRef::BytesRef(std::shared_ptr<char> new_bytes, unsigned int new_offset, unsigned int new_length, unsigned int new_capacity)
  : bytes(new_bytes),
    offset(new_offset),
    length(new_length),
    capacity(new_capacity) {
  assert(IsValid());
}

BytesRef::BytesRef(std::shared_ptr<char> new_bytes, unsigned int new_capacity)
  : BytesRef(new_bytes, 0, new_capacity, new_capacity) {
}

BytesRef::BytesRef(unsigned int new_capacity)
  : BytesRef(std::shared_ptr<char>(new char[new_capacity], std::default_delete<char[]>()), new_capacity) {
}

BytesRef::BytesRef(std::string& text) {
  if(text.empty()) {
    bytes = std::shared_ptr<char>(nullptr, std::default_delete<char[]>());
    capacity = length = offset = 0;
  } else {
    const char* cstr = text.c_str();
    offset = 0;
    capacity = length = text.size();
    char* bytes_ptr = new char[capacity];
    std::memcpy(bytes_ptr, cstr, capacity);
    bytes.reset(bytes_ptr);
  }
}

BytesRef::~BytesRef() {
}

BytesRef BytesRef::DeepCopyOf() {
  BytesRef copy;
  
  copy.offset = 0;
  copy.length = length;
  copy.capacity = length;
  char* new_byte_arr = arrayutil::CopyOfRange(bytes.get(), offset, offset + length);
  copy.bytes.reset(new_byte_arr);

  return copy;
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
    bytes = source.bytes;
    offset = source.offset;
    length = source.length;
    capacity = source.capacity;
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
  if(!bytes.get() && (offset != 0 || length != 0)) {
    throw std::runtime_error("bytes is nullptr, offset=" + std::to_string(offset) + ", length=" + std::to_string(length));
  }

  if(offset > length) {
    throw std::overflow_error("Offset out of bounds: " + std::to_string(offset) + ", length=" + std::to_string(length));
  }

  return true;
}


/**
 * BytesRefBuilder
 */
BytesRefBuilder::BytesRefBuilder()
  : ref() {
}

const char* BytesRefBuilder::Bytes() const {
  return ref.bytes.get();
}

const unsigned int BytesRefBuilder::Length() const {
  return ref.length;
}

void BytesRefBuilder::SetLength(const unsigned int new_length) {
  ref.length = new_length;
}

char& BytesRefBuilder::operator[](const unsigned int idx) {
  return ref.bytes.get()[idx];
}

void BytesRefBuilder::Grow(unsigned int new_capacity) {
  std::pair<char*, unsigned int> new_bytes_pair = arrayutil::Grow(ref.bytes.get(), ref.capacity, new_capacity);
  if(new_bytes_pair.first) {
    ref.bytes.reset(new_bytes_pair.first);
    ref.capacity = new_bytes_pair.second;
  }
}

void BytesRefBuilder::Append(const char c) {
  Grow(ref.length + 1);
  char* byte_arr = ref.bytes.get();
  byte_arr[ref.length++] = c;
}

void BytesRefBuilder::Append(const char* new_bytes, const unsigned int off, const unsigned int len) {
  Grow(ref.length + len);
  std::memcpy(ref.bytes.get() + ref.length, new_bytes + off, len);
  ref.length += len;
}

void BytesRefBuilder::Append(BytesRef& ref) {
  Append(ref.bytes.get(), ref.offset, ref.length);
}

void BytesRefBuilder::Append(BytesRefBuilder& builder) {
  Append(builder.Get());
}

void BytesRefBuilder::Clear() {
  SetLength(0);
}

void BytesRefBuilder::CopyBytes(const char* new_bytes, const unsigned int off, const unsigned int len) {
  Clear();
  Append(new_bytes, off, len);
}

void BytesRefBuilder::CopyBytes(BytesRef& ref) {
  Clear();
  Append(ref);
}

void BytesRefBuilder::CopyBytes(BytesRefBuilder& builder) {
  Clear();
  Append(builder);
}

void BytesRefBuilder::CopyChars(std::string& text) {
  CopyChars(text, 0, text.size());
}

void BytesRefBuilder::CopyChars(std::string& text, const unsigned int off, const unsigned int len) {
  Grow(len);
  std::memcpy(ref.bytes.get(), text.c_str() + off, len);
  ref.length = len;
}

BytesRef& BytesRefBuilder::Get() {
  return ref;
}

BytesRef BytesRefBuilder::ToBytesRef() const {
  return BytesRef(ref.bytes, 0, ref.capacity, ref.capacity);
}
