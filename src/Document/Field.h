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

#ifndef SRC_DOCUMENT_FIELD_H_
#define SRC_DOCUMENT_FIELD_H_

#include <Analysis/Attribute.h>
#include <Analysis/TokenStream.h>
#include <Index/DocValue.h>
#include <Index/Field.h>
#include <Util/Bytes.h>
#include <Util/Etc.h>
#include <variant>
#include <optional>
#include <stdexcept>
#include <memory>
#include <string>
#include <utility>

namespace lucene {
namespace core {
namespace document {

class Field : public lucene::core::index::IndexableField {
 private:
  const lucene::core::index::IndexableFieldType& type;
  const std::string& name;
  std::variant<lucene::core::util::BytesRef,
               std::string,
               std::unique_ptr<lucene::core::analysis::Reader>,
               lucene::core::util::etc::Number> fields_data;
  std::unique_ptr<lucene::core::analysis::TokenStream> tokenstream;

 protected:
  Field(const std::string& name,
        const lucene::core::index::IndexableFieldType& type);

 public:
  /**
   *  @param name Field name
   *  @param reader Reader value. Field instance will manage the
   *                given Reader's life cycle
   *  @param type Field type
   */
  Field(const std::string& name,
        lucene::core::analysis::Reader* reader,
        const lucene::core::index::IndexableFieldType& type);

  /**
   *  @param name Field name
   *  @param tokenstream TokenStream value. Field instance will manage the 
   *                     given TokenStream's life cycle
   *  @param type Field type
   */
  Field(const std::string& name,
        lucene::core::analysis::TokenStream* tokenstream,
        const lucene::core::index::IndexableFieldType& type);

  Field(const std::string& name,
        const char* value,
        uint32_t length,
        const lucene::core::index::IndexableFieldType& type);

  Field(const std::string& name,
        const char* value,
        const uint32_t offset,
        const uint32_t length,
        const lucene::core::index::IndexableFieldType& type);

  Field(const std::string& name,
        const lucene::core::util::BytesRef& bytes,
        const lucene::core::index::IndexableFieldType& type);

  Field(const std::string& name,
        lucene::core::util::BytesRef&& bytes,
        const lucene::core::index::IndexableFieldType& type);

  Field(const std::string& name,
        const std::string& value,
        const lucene::core::index::IndexableFieldType& type);

  Field(const std::string& name,
        std::string&& value,
        const lucene::core::index::IndexableFieldType& type);

  virtual ~Field();

  void SetStringValue(const std::string& value) {
    if (auto org_value = std::get_if<std::string>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  void SetStringValue(std::string&& value) {
    if (auto org_value = std::get_if<std::string>(&fields_data)) {
      *org_value = std::forward<std::string>(value);
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  void SetReaderValue(lucene::core::analysis::Reader* value) {
    if (auto uniq_reader =
          std::get_if<std::unique_ptr<lucene::core::analysis::Reader>>(
                                                                &fields_data)) {
      (*uniq_reader).reset(value);
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  void SetBytesValue(const char* value, uint32_t length) {
    SetBytesValue(lucene::core::util::BytesRef(value, 0, length));
  }

  void SetBytesValue(const lucene::core::util::BytesRef& value) {
    if (auto org_value =
          std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  void SetBytesValue(lucene::core::util::BytesRef&& value) {
    if (auto org_value =
          std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      *org_value = std::forward<lucene::core::util::BytesRef>(value);
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  void SetShortValue(const int16_t value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int16_t. "
                             "It is not a Number");
  }

  void SetIntValue(const int32_t value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int32_t. "
                             "It is not a Number");
  }

  void SetLongValue(const int64_t value) {
    if (auto org_value =
        std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int64_t. "
                             "It is not a Number");
  }

  void SetFloatValue(const float value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from value. "
                             "It is not a Number");
  }

  void SetDoubleValue(const double value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from double. "
                             "It is not a Number");
  }

  void
  SetTokenStream(lucene::core::analysis::TokenStream* new_tokenstream) {
    tokenstream.reset(new_tokenstream);
  }

  const std::string& Name() noexcept {
    return name;
  }

  std::optional<std::reference_wrapper<std::string>>
  StringValue() noexcept {
    if (auto str = std::get_if<std::string>(&fields_data)) {
      return *str;
    }

    return {};
  }

  std::optional<std::reference_wrapper<lucene::core::analysis::Reader>>
  ReaderValue() noexcept {
    if (auto uniq_reader =
          std::get_if<std::unique_ptr<lucene::core::analysis::Reader>>(
                                                                &fields_data)) {
      return *((*uniq_reader).get());
    }

    return {};
  }

  std::optional<lucene::core::util::etc::Number> NumericValue() noexcept {
    if (auto number =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      return *number;
    }

    return {};
  }

  std::optional<std::reference_wrapper<lucene::core::util::BytesRef>>
  BinaryValue() noexcept {
    if (auto bytes = std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      return *bytes;
    }

    return {};
  }

  const lucene::core::index::IndexableFieldType& FieldType() noexcept {
    return type;
  }

  lucene::core::analysis::TokenStream*
  GetTokenStream(lucene::core::analysis::Analyzer& analyzer,
                 lucene::core::analysis::TokenStream& reuse);

 public:
  enum class Store {
    YES,
    NO
  };

 private:
  class BinaryTokenStream;
  class StringTokenStream;
};

class Field::BinaryTokenStream: public lucene::core::analysis::TokenStream {
 private:
  std::shared_ptr<lucene::core::analysis::tokenattributes::BytesTermAttribute>
    bytes_att;
  bool used;
  lucene::core::util::BytesRef value;

 public:
  BinaryTokenStream() { }
  virtual ~BinaryTokenStream() { }

  void SetValue(const lucene::core::util::BytesRef& new_value) {
    value = new_value;
  }

  void SetValue(lucene::core::util::BytesRef&& value) {
    value = std::forward<lucene::core::util::BytesRef>(value);
  }

  bool IncrementToken() {
    if (used) {
      return false;
    }

    ClearAttributes();
    bytes_att->SetBytesRef(value);
    return (used = true);
  }

  void Reset() noexcept {
    used = false;
  }

  void Close() noexcept {
    // In Java, it assigns a null to value but we do nothing here.
  }
};

class Field::StringTokenStream: public lucene::core::analysis::TokenStream {
 private:
  std::shared_ptr<lucene::core::analysis::tokenattributes::CharTermAttribute>
    term_attr;
  std::shared_ptr<lucene::core::analysis::tokenattributes::OffsetAttribute>
    offset_attr;
  bool used;
  std::string value;

 public:
  StringTokenStream() { }
  virtual ~StringTokenStream() { }

  void SetValue(const std::string& new_value) {
    value = new_value;
  }

  void SetValue(std::string&& new_value) {
    value = std::forward<std::string>(new_value);
  }

  bool IncrementToken() {
    if (used) {
      return false;
    }

    ClearAttributes();
    term_attr->Append(value);
    offset_attr->SetOffset(0, value.size());
    return (used = true);
  }

  void End() {
    lucene::core::analysis::TokenStream::End();
    const uint32_t final_offset = value.size();
    offset_attr->SetOffset(final_offset, final_offset);
  }

  void Reset() noexcept {
    used = true;
  }

  void Close() noexcept {
    // In Java, it assigns a null to value but we do nothing here.
  }
};

class FieldType: public lucene::core::index::IndexableFieldType {
 private:
  bool stored;
  bool tokenized;
  bool store_term_vectors;
  bool store_term_vector_offsets;
  bool store_term_vector_positions;
  bool store_term_vector_payloads;
  bool omit_norms;
  lucene::core::index::IndexOptions index_options;
  lucene::core::index::DocValuesType doc_values_type;
  uint32_t dimension_count;
  uint32_t dimension_num_bytes;

 public:
  FieldType()
    : lucene::core::index::IndexableFieldType() {
    tokenized = true;
    index_options = lucene::core::index::IndexOptions::NONE;
    doc_values_type = lucene::core::index::DocValuesType::NONE;
  }

  explicit FieldType(lucene::core::index::IndexableFieldType& ref)
    : FieldType() {
    stored = ref.Stored();
    tokenized = ref.Tokenized();
    store_term_vectors = ref.StoreTermVectors();
    store_term_vector_offsets = ref.StoreTermVectorOffsets();
    store_term_vector_positions = ref.StoreTermVectorPositions();
    store_term_vector_payloads = ref.StoreTermVectorPayloads();
    omit_norms = ref.OmitNorms();
    index_options = ref.GetIndexOptions();
    doc_values_type = ref.GetDocValuesType();
    dimension_count = ref.PointDimensionCount();
    dimension_num_bytes = ref.PointNumBytes();
  }

  virtual ~FieldType() {
  }

  bool Stored() const noexcept {
    return stored;
  }

  void SetStored(const bool value) noexcept {
    stored = value;
  }

  bool Tokenized() const noexcept {
    return tokenized;
  }

  void SetTokenized(const bool value) noexcept {
    tokenized = value;
  }

  bool StoreTermVectors() const {
    return store_term_vectors;
  }

  void SetStoreTermVectors(const bool value) noexcept {
    store_term_vectors = value;
  }

  bool StoreTermVectorOffsets() const noexcept {
    return store_term_vector_offsets;
  }

  void SetStoreTermVectorOffsets(const bool value) noexcept {
    store_term_vector_offsets = value;
  }

  bool StoreTermVectorPositions() const noexcept {
    return store_term_vector_positions;
  }

  void SetStoreTermVectorPositions(const bool value) noexcept {
    store_term_vector_positions = value;
  }

  bool StoreTermVectorPayloads() const noexcept {
    return store_term_vector_payloads;
  }

  void SetStoreTermVectorPayloads(const bool value) noexcept {
    store_term_vector_payloads = value;
  }

  bool OmitNorms() const noexcept {
    return omit_norms;
  }

  void SetOmitNorms(const bool value) noexcept {
    omit_norms = value;
  }

  lucene::core::index::IndexOptions GetIndexOptions() const noexcept {
    return index_options;
  }

  void SetIndexOptions(const lucene::core::index::IndexOptions value) noexcept {
    index_options = value;
  }

  void SetDimensions(const uint32_t new_dimension_count,
                     const uint32_t new_dimension_num_bytes) noexcept {
    dimension_count = new_dimension_count;
    dimension_num_bytes = new_dimension_num_bytes;
  }

  uint32_t PointDimensionCount() const noexcept {
    return dimension_count;
  }

  uint32_t PointNumBytes() const noexcept {
    return dimension_num_bytes;
  }

  lucene::core::index::DocValuesType GetDocValuesType() const noexcept {
    return doc_values_type;
  }

  void
  SetDocValuesType(const lucene::core::index::DocValuesType type) noexcept {
    doc_values_type = type;
  }
};

}  // namespace document
}  // namespace core
}  // namespace lucene

#endif  // SRC_DOCUMENT_FIELD_H_
