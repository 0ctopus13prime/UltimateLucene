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
#include <cstring>
#include <initializer_list>
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
 protected:
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

  virtual void SetShortValue(const int16_t value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int16_t. "
                             "It is not a Number");
  }

  virtual void SetIntValue(const int32_t value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int32_t. "
                             "It is not a Number");
  }

  virtual void SetLongValue(const int64_t value) {
    if (auto org_value =
        std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int64_t. "
                             "It is not a Number");
  }

  virtual void SetFloatValue(const float value) {
    if (auto org_value =
          std::get_if<lucene::core::util::etc::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from value. "
                             "It is not a Number");
  }

  virtual void SetDoubleValue(const double value) {
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

  const lucene::core::index::IndexableFieldType& GetFieldType() noexcept {
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

class FieldTypeBuilder;

class FieldType: public lucene::core::index::IndexableFieldType {
 private:
  friend class FieldTypeBuilder;

  const bool stored;
  const bool tokenized;
  const bool store_term_vectors;
  const bool store_term_vector_offsets;
  const bool store_term_vector_positions;
  const bool store_term_vector_payloads;
  const bool omit_norms;
  const lucene::core::index::IndexOptions index_options;
  const lucene::core::index::DocValuesType doc_values_type;
  const uint32_t dimension_count;
  const uint32_t dimension_num_bytes;

 private:
  FieldType(const bool stored,
            const bool tokenized,
            const bool store_term_vectors,
            const bool store_term_vector_offsets,
            const bool store_term_vector_positions,
            const bool store_term_vector_payloads,
            const bool omit_norms,
            const lucene::core::index::IndexOptions index_options,
            const lucene::core::index::DocValuesType doc_values_type,
            const uint32_t dimension_count,
            const uint32_t dimension_num_bytes)
    : stored(stored),
      tokenized(tokenized),
      store_term_vectors(store_term_vectors),
      store_term_vector_offsets(store_term_vector_offsets),
      store_term_vector_positions(store_term_vector_positions),
      store_term_vector_payloads(store_term_vector_payloads),
      omit_norms(omit_norms),
      index_options(index_options),
      doc_values_type(doc_values_type),
      dimension_count(dimension_count),
      dimension_num_bytes(dimension_num_bytes) {
  }

 public:
  explicit FieldType(const lucene::core::index::IndexableFieldType& ref)
    : FieldType(ref.Stored(),
                ref.Tokenized(),
                ref.StoreTermVectors(),
                ref.StoreTermVectorOffsets(),
                ref.StoreTermVectorPositions(),
                ref.StoreTermVectorPayloads(),
                ref.OmitNorms(),
                ref.GetIndexOptions(),
                ref.GetDocValuesType(),
                ref.PointDimensionCount(),
                ref.PointNumBytes()) {
  }

  virtual ~FieldType() { }

  bool Stored() const noexcept {
    return stored;
  }

  bool Tokenized() const noexcept {
    return tokenized;
  }

  bool StoreTermVectors() const {
    return store_term_vectors;
  }

  bool StoreTermVectorOffsets() const noexcept {
    return store_term_vector_offsets;
  }

  bool StoreTermVectorPositions() const noexcept {
    return store_term_vector_positions;
  }

  bool StoreTermVectorPayloads() const noexcept {
    return store_term_vector_payloads;
  }

  bool OmitNorms() const noexcept {
    return omit_norms;
  }

  lucene::core::index::IndexOptions GetIndexOptions() const noexcept {
    return index_options;
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
};

class FieldTypeBuilder {
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
  FieldTypeBuilder()
    : stored(false),
      tokenized(true),
      store_term_vectors(false),
      store_term_vector_offsets(false),
      store_term_vector_positions(false),
      store_term_vector_payloads(false),
      omit_norms(false),
      index_options(lucene::core::index::IndexOptions::NONE),
      doc_values_type(lucene::core::index::DocValuesType::NONE),
      dimension_count(0),
      dimension_num_bytes(0) {
  }

  FieldTypeBuilder& SetStored(const bool value) noexcept {
    stored = value;
    return *this;
  }

  FieldTypeBuilder& SetTokenized(const bool value) noexcept {
    tokenized = value;
    return *this;
  }

  FieldTypeBuilder& SetStoreTermVectors(const bool value) noexcept {
    store_term_vectors = value;
    return *this;
  }

  FieldTypeBuilder& SetStoreTermVectorOffsets(const bool value) noexcept {
    store_term_vector_offsets = value;
    return *this;
  }

  FieldTypeBuilder& SetStoreTermVectorPositions(const bool value) noexcept {
    store_term_vector_positions = value;
    return *this;
  }

  FieldTypeBuilder& SetStoreTermVectorPayloads(const bool value) noexcept {
    store_term_vector_payloads = value;
    return *this;
  }

  FieldTypeBuilder& SetOmitNorms(const bool value) noexcept {
    omit_norms = value;
    return *this;
  }

  FieldTypeBuilder&
  SetIndexOptions(const lucene::core::index::IndexOptions value) noexcept {
    index_options = value;
    return *this;
  }

  FieldTypeBuilder&
  SetDocValuesType(const lucene::core::index::DocValuesType type) noexcept {
    doc_values_type = type;
    return *this;
  }

  FieldTypeBuilder&
  SetDimensions(const uint32_t new_dimension_count,
                const uint32_t new_dimension_num_bytes) noexcept {
    dimension_count = new_dimension_count; 
    dimension_num_bytes = new_dimension_num_bytes;
    return *this;
  }

  FieldType Build() {
    return FieldType(stored,
                     tokenized,
                     store_term_vectors,
                     store_term_vector_offsets,
                     store_term_vector_positions,
                     store_term_vector_payloads,
                     omit_norms,
                     index_options,
                     doc_values_type,
                     dimension_count,
                     dimension_num_bytes);
  }
};

class TextField : public Field {
 public:
  static FieldType TYPE_NOT_STORED;
  static FieldType TYPE_STORED;

 public:
  TextField(const std::string& name, lucene::core::analysis::Reader* reader)
    : Field(name, reader, TYPE_NOT_STORED) {
  }

  TextField(const std::string& name,
            const std::string& value,
            const Field::Store store)
    : Field(name,
            value,
            (store == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  TextField(const std::string& name,
            std::string&& value,
            const Field::Store store)
    : Field(name,
            std::forward<std::string>(value),
            (store == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  TextField(const std::string& name,
            lucene::core::analysis::TokenStream* tokenstream)
    : Field(name, tokenstream, TYPE_NOT_STORED) {
  }
};

class StringField : public Field {
 public:
  static FieldType TYPE_NOT_STORED;
  static FieldType TYPE_STORED;

 public:
  StringField(const std::string& name,
              const std::string& value,
              Field::Store stored)
    : Field(name, value, (stored == Field::Store::YES ?
                          TYPE_STORED : TYPE_NOT_STORED)) {
  }

  StringField(const std::string& name,
              std::string&& value,
              Field::Store stored)
    : Field(name,
            std::forward<std::string>(value),
            (stored == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  StringField(const std::string& name,
              const lucene::core::util::BytesRef& value,
              Field::Store stored)
    : Field(name,
            value,
            (stored == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  StringField(const std::string& name,
              lucene::core::util::BytesRef&& value,
              Field::Store stored)
    : Field(name,
            std::forward<lucene::core::util::BytesRef>(value),
            (stored == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }
};

class StoredField : public Field {
 public:
  static FieldType TYPE;

 protected:
  StoredField(const std::string& name, const FieldType& type)
    : Field(name, type) {
  }

 public:
  StoredField(const std::string& name,
              const lucene::core::util::BytesRef& bytes,
              const FieldType& type)
    : Field(name, bytes, type) {
  }

  StoredField(const std::string& name,
              lucene::core::util::BytesRef&& bytes,
              const FieldType& type)
    : Field(name, bytes, type) {
  }

  StoredField(const std::string& name,
              char* value,
              uint32_t length)
    : Field(name, value, length, TYPE) {
  }

  StoredField(const std::string& name,
              char* value,
              uint32_t offset,
              uint32_t length)
    : Field(name, value, offset, length, TYPE) {
  }

  StoredField(const std::string& name,
              const lucene::core::util::BytesRef& bytes)
    : Field(name, bytes, TYPE) {
  }
 
  StoredField(const std::string& name,
              lucene::core::util::BytesRef&& bytes)
    : Field(name, bytes, TYPE) {
  }

  StoredField(const std::string& name,
              const std::string& value)
    : Field(name, value, TYPE) {
  }

  StoredField(const std::string& name,
              std::string&& value)
    : Field(name, std::forward<std::string>(value), TYPE) {
  }

  StoredField(const std::string& name,
              const std::string& value,
              const FieldType& type)
    : Field(name, value, type) {
  }

  StoredField(const std::string& name,
              std::string&& value,
              const FieldType& type)
    : Field(name, std::forward<std::string>(value), type) {
  }

  StoredField(const std::string& name, int32_t value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::etc::Number(value);
  }

  StoredField(const std::string& name, int64_t value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::etc::Number(value);
  }

  StoredField(const std::string& name, float value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::etc::Number(value);
  }

  StoredField(const std::string& name, double value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::etc::Number(value);
  }
};

class NumericDocValuesField : public Field {
 public:
  static FieldType TYPE; 

  NumericDocValuesField(const std::string& name, const int64_t value)
    : Field(name, TYPE) {
    fields_data = lucene::core::util::etc::Number(value);
  }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class FloatDocValuesField : public NumericDocValuesField {
 public:
  FloatDocValuesField(const std::string& name, const float value)
    : NumericDocValuesField(
                    name,
                    lucene::core::util::etc::Float::FloatToRawIntBits(value)) {
  }

  void SetFloatValue(const float value) {
    NumericDocValuesField::SetLongValue(
                      lucene::core::util::etc::Float::FloatToRawIntBits(value));
  }

  void SetLongValue(const int64_t) {
    throw
    std::invalid_argument("Could not change value type from Float to Long");
  }
};

class DoubleDocValuesField : public NumericDocValuesField {
 public:
  DoubleDocValuesField(const std::string& name, const double value)
    : NumericDocValuesField(
                  name,
                  lucene::core::util::etc::Double::DoubleToRawLongBits(value)) {
  }

  void SetDoubleValue(const double value) {
    NumericDocValuesField::SetLongValue(
      lucene::core::util::etc::Double::DoubleToRawLongBits(value));
  }

  void SetLongValue(const int64_t) {
    throw
    std::invalid_argument("Could not change value type from Float to Long");
  }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class BinaryDocValuesField : public Field {
 public:
  static FieldType TYPE;

 public:
  BinaryDocValuesField(const std::string& name,
                       const lucene::core::util::BytesRef& value)
    : Field(name, TYPE) {
    fields_data = value;
  }

  BinaryDocValuesField(const std::string& name,
                       lucene::core::util::BytesRef&& value)
    : Field(name, TYPE) {
    fields_data = std::forward<lucene::core::util::BytesRef>(value);
  }
};

class SortedDocValuesField : public Field {
 public:
  static FieldType TYPE;

 public:
  SortedDocValuesField(const std::string& name,
                       const lucene::core::util::BytesRef& bytes)
    : Field(name, TYPE) {
    fields_data = bytes;
  }

  SortedDocValuesField(const std::string& name,
                       lucene::core::util::BytesRef&& bytes)
    : Field(name, TYPE) {
    fields_data = std::forward<lucene::core::util::BytesRef>(bytes);
  }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class SortedNumericDocValuesField : public Field {
 public:
  static FieldType TYPE;

 public:
  SortedNumericDocValuesField(const std::string& name, const uint64_t value)
    : Field(name, TYPE) {
    fields_data = lucene::core::util::etc::Number(value);
  }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class SortedSetDocValuesField : public Field {
 public:
  static FieldType TYPE;

 public:
  SortedSetDocValuesField(const std::string& name,
                          const lucene::core::util::BytesRef& bytes)
    : Field(name, TYPE) {
    fields_data = bytes;
  }

  SortedSetDocValuesField(const std::string& name,
                          lucene::core::util::BytesRef&& bytes)
    : Field(name, TYPE) {
    fields_data = std::forward<lucene::core::util::BytesRef>(bytes);
  }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class BinaryPoint : public Field {
 private:
  static FieldType GetType(const uint32_t num_dims,
                           const uint32_t bytes_per_dim) {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, bytes_per_dim)
           .Build();
  }

  /**
   *  Pack point values into a single byte array.
   *  For example, input [ [0,1], [1,2], [2,3] ] should be packed as
   *  [0, 1, 1, 2, 2, 3]
   *  But not like Java implementation we do not check if they have same
   *  number of bytes.
   *  Rather we expect given point values to have all same bytes_per_dim.
   *  So, in Java [ [0,1], [0, 1, 2] ] this values are not acceptable, 
   *  but in C++ implementation we just treat it as [ [0, 1], [0, 1] ]
   */
  static lucene::core::util::BytesRef
  Pack(const uint32_t bytes_per_dim,
       const std::initializer_list<const char*>& point) {
    uint32_t size = bytes_per_dim * point.size();  
    char packed[size];
    uint32_t idx = 0;
    for (const char* p : point) {
      std::memcpy(packed + idx, p, bytes_per_dim); 
      idx += bytes_per_dim;
    }

    return lucene::core::util::BytesRef(packed, 0, size);
  }

 public:
  BinaryPoint(const std::string& name,
              const std::initializer_list<const char*>& point,
              const uint32_t bytes_per_dim)
    : Field(name,
            Pack(bytes_per_dim, point),
            GetType(point.size(), bytes_per_dim)) {
  }

  BinaryPoint(const std::string& name,
              const char* point,
              const uint32_t bytes_per_dim,
              const lucene::core::index::IndexableFieldType& type)
    : Field(name, point, bytes_per_dim, type) {
    if (bytes_per_dim != type.PointDimensionCount() * type.PointNumBytes()) {
      throw std::runtime_error("Packed point is length=" +
                               std::to_string(bytes_per_dim) +
                               " but type.PointDimensionCount()=" + 
                               std::to_string(type.PointDimensionCount()) + 
                               " and type.PointNumBytes()=" +
                               std::to_string(type.PointNumBytes()));
    }
  }

  // static Query newExactQuery TODO(0ctopus13prime): Implement it.
  // static Query newRangeQuery TODO(0ctopus13prime): Implement it.
  // static Query newRangeQuery TODO(0ctopus13prime): Implement it.
  // static Query newSetQuery TODO(0ctopus13prime): Implement it.
};

class DoublePoint : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class DoubleRange : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class FloatPoint : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class FloatRange : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class IntPoint : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class IntRange : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class LongPoint : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class LongRange : public Field {
  // TODO(0ctopus13prime): Implement it.
};

class LongRange : public Field {
  // TODO(0ctopus13prime): Implement it.
};



}  // namespace document
}  // namespace core
}  // namespace lucene

#endif  // SRC_DOCUMENT_FIELD_H_
