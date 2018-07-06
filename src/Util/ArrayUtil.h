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
T* CopyOf(const T* original, const uint32_t original_length, const uint32_t new_length) {
  T* copy = new T[new_length];
  std::memcpy(copy, original, sizeof(T) * new_length);
  return copy;
}

template <typename T>
T* CopyOf(const T* original, const uint32_t original_length) {
  return CopyOf(original, original_length, original_length);
}

// TODO. I'm not sure this is a valid implementation. How can I limit maximum array size like Java does?
template <typename T>
uint32_t Oversize(const uint32_t min_target_size) {
  uint32_t extra = (min_target_size >> 3); // extra <- min_target_size / 8
  if(extra < 3) {
    extra = 3;
  }

  uint32_t new_size = (min_target_size + extra);
  return new_size;
}

template <typename T>
std::pair<T*, uint32_t> Grow(const T* array, const uint32_t length, const uint32_t min_size) {
  if(length < min_size) {
    uint32_t new_length = Oversize<T>(min_size);
    T* new_array = CopyOf(array, length, new_length);
    return std::pair<T*, uint32_t>(new_array, new_length);
  }

  return std::pair<T*, uint32_t>(nullptr, 0);
}

/**
 * @param original Source array
 * @param from Start index in original. Inclusive.
 * @param end end index in original. Exclusive.
 **/
template <typename T>
T* CopyOfRange(const T* original, const uint32_t from, const uint32_t to) {
  if(from >= to) throw std::invalid_argument("Range must guarantee from < to, from -> " + std::to_string(from) + ", to -> " + std::to_string(to));

  const uint32_t new_length = (to - from);
  T* copy = new T[new_length];
  std::memcpy(copy, original + from, sizeof(T) * new_length);
  return copy;
}

template <typename INTEGER1, typename INTEGER2, typename INTEGER3>
INTEGER1 CheckFromToIndex(INTEGER1 from_index, INTEGER2 to_index, INTEGER3 length) {
  // Checks if `from_index` belongs [0, to_indx] and `to_indx` <= length
  if(from_index < 0 || from_index > to_index || to_index > length) {
    throw std::invalid_argument("Range [" + std::to_string(from_index) + ", " + std::to_string(to_index) + ") out of bounds for length " + std::to_string(length));
  }

  return from_index;
}

template <typename INTEGER1, typename INTEGER2, typename INTEGER3>
INTEGER1 CheckFromIndexSize(INTEGER1 from_index, INTEGER2 size, INTEGER3 length) {
  // Checks if `from_index` in [0, from_index + size] and `from_index` + `size` < = `length`
  int64_t end = from_index + size;
  if(from_index < 0 || from_index > end || end > length) {
    throw std::invalid_argument("Range [" + std::to_string(from_index) + ", " + std::to_string(from_index) + " + " + std::to_string(size) + ") out of bounds for length " + std::to_string(length));
  }

  return from_index;
}

template <typename INTEGER1, typename INTEGER2>
INTEGER1 CheckIndex(INTEGER1 index, INTEGER2 length) {
  // Checks if `index` belongs [0, length) range
  if(index < 0 || index >= length) {
    throw std::invalid_argument("Index " + std::to_string(index) + " out-of-bounds for length " + std::to_string(length));
  }

  return index;
}


}}}} // End of namespace


#endif
