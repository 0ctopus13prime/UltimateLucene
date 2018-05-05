#ifndef LUCENE_CORE_UTIL_ARRAY_UTIL_H_
#define LUCENE_CORE_UTIL_ARRAY_UTIL_H_

#include <utility>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <algorithm>

namespace lucene { namespace core { namespace util { namespace arrayutil {

template <typename T>
T* CopyOf(const T* original, const unsigned int original_length, const unsigned int new_length) {
  T* copy = new T[new_length];
  std::memcpy(copy, original, std::min(original_length, new_length));
  return copy;
}

template <typename T>
T* CopyOf(const T* original, const unsigned int original_length) {
  return CopyOf(original, original_length, original_length);
}

// TODO. I'm not sure this is a valid implementation. How can I limit maximum array size?
unsigned int Oversize(const unsigned int min_target_size, const unsigned int bytes_per_element);

template <typename T>
std::pair<T*, unsigned int> Grow(const T* array, const unsigned int length, const unsigned int min_size) {
  if(length < min_size) {
    unsigned int new_length = Oversize(min_size, sizeof(T));
    T* new_array = CopyOf(array, new_length);
    return std::pair<T*, unsigned int>(new_array, new_length);
  } 

  return std::pair<T*, unsigned int>(nullptr, 0);
}

template <typename T>
T* CopyOfRange(const T* original, const unsigned int from, const unsigned int to) {
  const unsigned int new_length = (to - from); 
  if(new_length < 0) throw std::invalid_argument("Range from > to, " + std::to_string(from) + " > " + std::to_string(to));
  if(new_length > 0) {
    T* copy = new T[new_length];
    std::memcpy(copy, original + from, new_length);
    return copy;
  } else {
    return nullptr;
  }
}

template <typename INTEGER1, typename INTEGER2, typename INTEGER3>
INTEGER1 CheckFromToIndex(INTEGER1 from_index, INTEGER2 to_index, INTEGER3 length) {
  if(from_index < 0 || from_index > to_index || to_index > length) {
    throw std::invalid_argument("Range [" + std::to_string(from_index) + ", " + std::to_string(to_index) + ") out of bounds for length " + std::to_string(length));
  }

  return from_index;
}

template <typename INTEGER1, typename INTEGER2, typename INTEGER3>
INTEGER1 CheckFromIndexSize(INTEGER1 from_index, INTEGER2 size, INTEGER3 length) {
  long end = from_index + size;
  if(from_index < 0 || from_index > end || end > length) {
    throw std::invalid_argument("Range [" + std::to_string(from_index) + ", " + std::to_string(from_index) + " + " + std::to_string(size) + ") out of bounds for length " + std::to_string(length));
  }

  return from_index;
}

template <typename INTEGER1, typename INTEGER2>
INTEGER1 CheckIndex(INTEGER1 index, INTEGER2 length) {
  if(index < 0 || index >= length) {
    throw std::invalid_argument("Index " + std::to_string(index) + " out-of-bounds for length " + std::to_string(length));
  }

  return index;
}


}}}} // End of namespace


#endif
