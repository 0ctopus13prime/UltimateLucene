#ifndef SRC_DOCUMENT_DOCUMENT_H_
#define SRC_DOCUMENT_DOCUMENT_H_

#include <Index/Field.h>
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
  std::vector<lucene::core::index::IndexableField*> fields;

 public:
  Document(uint32_t capacity = 5)
    : fields() {
    fields.reserve(capacity);
  }

  ~Document() {
    std::for_each(fields.begin(),
                  fields.end(),
                  std::default_delete<lucene::core::index::IndexableField>());
  }

  std::vector<lucene::core::index::IndexableField*>::iterator
  begin() noexcept {
    return fields.begin();
  }

  std::vector<lucene::core::index::IndexableField*>::iterator
  end() noexcept {
    return fields.end();
  }

  void Add(lucene::core::index::IndexableField* field) {
    fields.push_back(field);
  }

  void RemoveField(const std::string& name) {
    fields.erase(
      std::find_if(fields.begin(),
                   fields.end(),
                   [&name](lucene::core::index::IndexableField* field){
                     return field->Name() == name;
    }));
  }

  void RemoveFields(const std::string& name) {
    fields.erase(
      std::remove_if(fields.begin(),
                     fields.end(),
                     [&name](lucene::core::index::IndexableField* field){
                       return field->Name() == name; 
                     }),
      fields.end());
  }

  std::optional<std::reference_wrapper<lucene::core::util::BytesRef>>
  GetBinaryValue(const std::string& name) noexcept {
    for (lucene::core::index::IndexableField* field : fields) {
      if (field->Name() == name && field->BinaryValue()) {
        lucene::core::util::BytesRef& bytesref = *(field->BinaryValue());
        return bytesref;
      }
    }

    return {};
  }

  uint32_t
  GetBinaryValues(const std::string& name,
                  lucene::core::util::BytesRef* dest[]) noexcept {
    uint32_t idx = 0;
    for (lucene::core::index::IndexableField* field : fields) {
      if (field->Name() == name && field->BinaryValue()) {
        lucene::core::util::BytesRef& bytesref = *(field->BinaryValue());
        dest[idx++] = &bytesref;
      }
    }

    return idx;
  }

  std::optional<std::reference_wrapper<lucene::core::index::IndexableField>>
  GetField(const std::string& name) {
    for (lucene::core::index::IndexableField* field : fields) {
      if (field->Name() == name) {
        return *field;
      }
    }

    return {};
  }

  uint32_t GetFields(const std::string& name,
                     lucene::core::index::IndexableField* dest[]) noexcept {
    uint32_t idx = 0;
    for (lucene::core::index::IndexableField* field : fields) {
      if (field->Name() == name) {
        dest[idx++] = field;
      }
    }

    return idx;
  }

  uint32_t GetValues(const std::string& name,
                     std::string* dest[]) noexcept {
    uint32_t idx = 0;
    for (lucene::core::index::IndexableField* field : fields) {
      if (field->Name() == name && field->StringValue()) {
        std::string& value = *(field->StringValue());
        dest[idx++] = &value;
      }
    }

    return idx;
  }

  std::optional<std::reference_wrapper<std::string>>
  Get(const std::string& name) noexcept {
    for (lucene::core::index::IndexableField* field : fields) {
      if (field->Name() == name && field->StringValue()) {
        std::string& value = *(field->StringValue());
        return value;
      }
    }

    return {};
  }

  const
  std::vector<lucene::core::index::IndexableField*>&
  GetFields() const noexcept {
    return fields;
  }

  uint32_t Size() const noexcept {
    return fields.size();
  }

  void Clear() {
    fields.clear();
  }
};

}  // namespace document
}  // namespace core
}  // namespace lucene

#endif // SRC_DOCUMENT_DOCUMENT_H_
