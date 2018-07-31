#ifndef SRC_DOCUMENT_DOCUMENT_H_
#define SRC_DOCUMENT_DOCUMENT_H_

// TEST
#include <iostream>

#include <Document/Field.h>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace lucene {
namespace core {
namespace document {

// TODO(0ctopus13prime): Custom allocator?
class Document {
 private:
  std::vector<lucene::core::document::Field> fields;

 public:
  Document(uint32_t capacity = 5)
    : fields() {
    fields.reserve(capacity);
  }

  std::vector<lucene::core::document::Field>::iterator
  begin() noexcept {
    return fields.begin();
  }

  std::vector<lucene::core::document::Field>::iterator
  end() noexcept {
    return fields.end();
  }

  void Add(lucene::core::document::Field&) = delete;

  void Add(lucene::core::document::Field&& field) {
    fields.push_back(std::forward<lucene::core::document::Field>(field));
  }

  void RemoveField(const std::string& name) {
    auto it = std::remove_if(fields.begin(),
                             fields.end(),
                             [&name](lucene::core::document::Field& field){
                               return field.Name() == name; 
                             });

    if (it != fields.end()) {
      fields.erase(it);
    }
  }

  void RemoveFields(const std::string& name) {
    fields.erase(
      std::remove_if(fields.begin(),
                     fields.end(),
                     [&name](lucene::core::document::Field& field){
                       return field.Name() == name; 
                     }),
      fields.end());
  }

  std::optional<std::reference_wrapper<lucene::core::util::BytesRef>>
  GetBinaryValue(const std::string& name) noexcept {
    for (lucene::core::document::Field& field : fields) {
      if (field.Name() == name && field.BinaryValue()) {
        lucene::core::util::BytesRef& bytesref = *(field.BinaryValue());
        return bytesref;
      }
    }

    return {};
  }

  uint32_t
  GetBinaryValues(const std::string& name,
                  lucene::core::util::BytesRef* dest[]) noexcept {
    uint32_t idx = 0;
    for (lucene::core::document::Field& field : fields) {
      if (field.Name() == name && field.BinaryValue()) {
        lucene::core::util::BytesRef& bytesref = *(field.BinaryValue());
        dest[idx++] = &bytesref;
      }
    }

    return idx;
  }

  std::optional<std::reference_wrapper<lucene::core::document::Field>>
  GetField(const std::string& name) {
    for (lucene::core::document::Field& field : fields) {
      if (field.Name() == name) {
        return field;
      }
    }

    return {};
  }

  uint32_t GetFields(const std::string& name,
                     lucene::core::document::Field* dest[]) noexcept {
    uint32_t idx = 0;
    for (lucene::core::document::Field& f : fields) {
      if (f.Name() == name) {
        dest[idx++] = &f;
      }
    }

    return idx;
  }

  uint32_t GetValues(const std::string& name,
                     std::string* dest[]) noexcept {
    uint32_t idx = 0;
    for (lucene::core::document::Field& field : fields) {
      if (field.Name() == name && field.StringValue()) {
        std::string& value = *(field.StringValue());
        dest[idx++] = &value;
      }
    }

    return idx;
  }

  std::optional<std::reference_wrapper<std::string>>
  Get(const std::string& name) noexcept {
    for (lucene::core::document::Field& field : fields) {
      if (field.Name() == name && field.StringValue()) {
        std::string& value = *(field.StringValue());
        return value;
      }
    }

    return {};
  }

  std::vector<lucene::core::document::Field>&
  GetFields() noexcept {
    return fields;
  }

  uint32_t Size() const noexcept {
    return fields.size();
  }

  void Clear() noexcept {
    fields.clear();
  }
};

}  // namespace document
}  // namespace core
}  // namespace lucene

#endif // SRC_DOCUMENT_DOCUMENT_H_
