/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SRC_UTIL_BYTES_H_
#define SRC_UTIL_BYTES_H_

#include <string>
#include <memory>

namespace lucene {
namespace core {
namespace util {

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
  BytesRef(const char* bytes,
           const uint32_t offset,
           const uint32_t length);
  BytesRef(const char* bytes,
           const uint32_t offset,
           const uint32_t length,
           const uint32_t capacity);
  BytesRef(const char* bytes, const uint32_t capacity);
  BytesRef(const BytesRef& other);
  explicit BytesRef(const uint32_t capacity);
  explicit BytesRef(const std::string& text);
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

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_BYTES_H_
