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
#include <Util/ArrayUtil.h>
#include <Util/Bytes.h>
#include <Util/Numeric.h>
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
               lucene::core::util::numeric::Number> fields_data;
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
          std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int16_t. "
                             "It is not a Number");
  }

  virtual void SetIntValue(const int32_t value) {
    if (auto org_value =
          std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int32_t. "
                             "It is not a Number");
  }

  virtual void SetLongValue(const int64_t value) {
    if (auto org_value =
        std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from int64_t. "
                             "It is not a Number");
  }

  virtual void SetFloatValue(const float value) {
    if (auto org_value =
          std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from value. "
                             "It is not a Number");
  }

  virtual void SetDoubleValue(const double value) {
    if (auto org_value =
          std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
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

  std::optional<lucene::core::util::numeric::Number> NumericValue() noexcept {
    if (auto number =
          std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
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
      fields_data = lucene::core::util::numeric::Number(value);
  }

  StoredField(const std::string& name, int64_t value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }

  StoredField(const std::string& name, float value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }

  StoredField(const std::string& name, double value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }
};

class NumericDocValuesField : public Field {
 public:
  static FieldType TYPE; 

  NumericDocValuesField(const std::string& name, const int64_t value)
    : Field(name, TYPE) {
    fields_data = lucene::core::util::numeric::Number(value);
  }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class FloatDocValuesField : public NumericDocValuesField {
 public:
  FloatDocValuesField(const std::string& name, const float value)
    : NumericDocValuesField(
                name,
                lucene::core::util::numeric::Float::FloatToRawIntBits(value)) {
  }

  void SetFloatValue(const float value) {
    NumericDocValuesField::SetLongValue(
                  lucene::core::util::numeric::Float::FloatToRawIntBits(value));
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
              lucene::core::util::numeric::Double::DoubleToRawLongBits(value)) {
  }

  void SetDoubleValue(const double value) {
    NumericDocValuesField::SetLongValue(
      lucene::core::util::numeric::Double::DoubleToRawLongBits(value));
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
    fields_data = lucene::core::util::numeric::Number(value);
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
 private:
  static FieldType GetType(const int32_t num_dims) {

  }

  static lucene::core::util::BytesRef
  Pack(const std::initializer_list<double>& point) {
    uint32_t size = point.size();
    uint32_t packed_size = size * sizeof(double);
    char packed[packed_size];

    uint32_t offset = 0;
    for(const double p : point) {
      DoublePoint::EncodeDimension(p, packed, offset);
      offset += sizeof(double);
    }

    return lucene::core::util::BytesRef(packed, 0, packed_size);
  }

  static lucene::core::util::BytesRef
  Pack(const double* point,
       const uint32_t point_size) {
    uint32_t packed_size = point_size * sizeof(double);
    char packed[packed_size];

    for(uint32_t i = 0 ; i < point_size ; ++i) {
      DoublePoint::EncodeDimension(point[i], packed, i * sizeof(double));
    }

    return lucene::core::util::BytesRef(packed, 0, packed_size);
  }

  static void EncodeDimension(const double value,
                              char* dest,
                              const int32_t offset) {
     
  }

  static double DecodeDimension(char* value,
                                int32_t value_size,
                                const int32_t offset) {

  }


 public:
  static double NextUp(const double d) {

  }

  static double NextDown(const double d) {

  }

  void SetDoubleValue(const double value) {

  }

  void SetDoubleValues(const std::initializer_list<double>& point) {

  }

  void SetBytesValue(const lucene::core::util::BytesRef bytes) {

  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() noexcept {
    // TODO(0ctopus13prime): Implement it.
    return lucene::core::util::numeric::Number(1);
  }

  DoublePoint(const std::string& name,
              const double* point,
              int32_t point_size)
    : Field(name,
            DoublePoint::Pack(point, point_size),
            DoublePoint::GetType(point_size)) {
  }

  // TODO(0ctopus13prime): Implement Query stuffs
};

class DoubleRange : public Field {
 public:
  static constexpr int32_t BYTES = sizeof(double);

 private:
  static FieldType GetType(const uint32_t dimensions) {
    if (dimensions > 4) {
      throw std::invalid_argument("DoubleRange does not support greater"
                                  "than 4 dimensions");
    }
    
    return FieldTypeBuilder()
           .SetDimensions(dimensions * 2, DoubleRange::BYTES) 
           .Build();
  }

  static void CheckArgs(const double* min,
                        const double* max,
                        const uint32_t length) {
    if (min == nullptr || max == nullptr || length == 0) {
      throw std::invalid_argument("min/max range values cannot be "
                                  "null or empty");
    }

    if (length > 4) {
      throw std::invalid_argument("DoubleRange does not support "
                                  "greater than 4 dimensions");
    }
  }

  static void Encode(const double val, char* bytes, const int32_t offset) {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(
      lucene::core::util::numeric::NumericUtils::DoubleToSortableLong(val),
      bytes, offset);
  }

  static void VerifyAndEncode(const double* min,
                              const double* max,
                              const uint32_t length,
                              char* bytes) {
    for (int32_t d = 0, i = 0, j = length * DoubleRange::BYTES
         ; d < length
         ; ++d, i += DoubleRange::BYTES, j += DoubleRange::BYTES) {
      if (lucene::core::util::numeric::Double::IsNaN(min[d])) {
        throw std::invalid_argument(std::string("Invalid min value(")
                + std::to_string(lucene::core::util::numeric::DoubleConsts::NaN)
                + ") in DoubleRange");
      }
      if (lucene::core::util::numeric::Double::IsNaN(max[d])) {
        throw std::invalid_argument(std::string("Invalid max value(")
                + std::to_string(lucene::core::util::numeric::DoubleConsts::NaN)
                + ") in DoubleRange");
      }
      if (min[d] > max[d]) {
        throw std::invalid_argument(std::string("Min value (")
                                    + std::to_string(min[d])
                                    + ") is greater than nax value ("
                                    + std::to_string(max[d]) + ")");
      }

      DoubleRange::Encode(min[d], bytes, i);
      DoubleRange::Encode(max[d], bytes, j);
    }
  }

 public:
  static double DecodeMin(char* bytes, const uint32_t dimension) {
    const uint32_t offset = dimension * DoubleRange::BYTES; 
    return lucene::core::util::numeric::NumericUtils::SortableLongToDouble(
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(bytes,
                                                                   offset));
  }

  static double DecodeMax(char* bytes, const uint32_t dimension) {
    const uint32_t offset = (dimension >> 1) + DoubleRange::BYTES; 
    return lucene::core::util::numeric::NumericUtils::SortableLongToDouble(
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(bytes,
                                                                   offset));
  }

  // TODO(0ctopus13prime): Implement Query stuffs

 public:
  DoubleRange(const std::string& name,
              const double* min,
              const double* max,
              const uint32_t length)
    : Field(name, GetType(length)) {
    SetRangeValues(min, max, length);
  }

  void SetRangeValues(const double* min,
                      const double* max,
                      const uint32_t length) {
    CheckArgs(min, max, length);
    if (length * 2 != type.PointDimensionCount()) {
      throw
      std::invalid_argument(std::string("Field (name=")
                            + name + ") uses "
                            + std::to_string(type.PointDimensionCount() / 2)
                            + " dimensions; cannot change to (incoming) "
                            + std::to_string(length) + " dimensions");
    }

    if (!std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      fields_data =
      lucene::core::util::BytesRef(DoubleRange::BYTES * 2 * length);
    }

    lucene::core::util::BytesRef& bytes_ref = 
      std::get<lucene::core::util::BytesRef>(fields_data);

    DoubleRange::VerifyAndEncode(min,
                                 max,
                                 length,
                                 bytes_ref.bytes.get());

  }

  double GetMin(const int32_t dimension) {
    lucene::core::util::arrayutil::CheckIndex(dimension,
                                              type.PointDimensionCount() / 2);

    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);
    return DoubleRange::DecodeMin(bytes_ref.bytes.get(), dimension);
  }

  double GetMax(const int32_t dimension) {
    lucene::core::util::arrayutil::CheckIndex(dimension,
                                              type.PointDimensionCount() / 2);
    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);
    return DoubleRange::DecodeMax(bytes_ref.bytes.get(), dimension);
  }
};

class FloatPoint : public Field {
 public:
  static float NextUp(const float f) {
    if (lucene::core::util::numeric::Float::FloatToIntBits(f)
        == 0x80000000) {
      return +0.0F;
    }
    return lucene::core::util::numeric::NumericUtils::NextUp(f);
  }

  static float NextDown(const float f) {
    if (lucene::core::util::numeric::Float::FloatToIntBits(f)
        == 0) {
      return -0.0F;
    }
    return lucene::core::util::numeric::NumericUtils::NextDown(f);
  }

  static void EncodeDimension(const float value,
                              char* dest,
                              const uint32_t offset) {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(
      lucene::core::util::numeric::NumericUtils::FloatToSortableInt(value),
      dest, offset);
  }

  static float DecodeDimension(const char* value,
                               const uint32_t offset) {
    return lucene::core::util::numeric::NumericUtils::SortableIntToFloat(
      lucene::core::util::numeric::NumericUtils::SortableBytesToInt(value,
                                                                    offset));
  }

  // TODO(0ctopus13prime): Implement other Query stuffs

 private:
  static FieldType GetType(const uint32_t num_dims) {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, sizeof(float))
           .Build();
  }

  lucene::core::util::BytesRef
  Pack(const float* points, const uint32_t length) {
    if (points == nullptr) {
      throw std::invalid_argument("Points must not be null");
    }

    if (length == 0) {
      throw std::invalid_argument("Points must not be 0 dimensions");
    }

    uint32_t packed_size = length * sizeof(float);
    char packed[packed_size];

    for (uint32_t dim = 0 ; dim < length ; ++dim) {
      FloatPoint::EncodeDimension(points[dim], packed, dim * sizeof(float));
    }

    return lucene::core::util::BytesRef(packed, packed_size);
  }

 public:
  FloatPoint(const std::string& name,
             const float* points,
             const uint32_t length)
    : Field(name,
            FloatPoint::Pack(points, length),
            FloatPoint::GetType(length)) {
  }

  void SetFloatValue(const float value) {
    SetFloatValues(&value, 1);    
  }

  void SetFloatValues(const float* points,
                      const uint32_t length) {
    if (type.PointDimensionCount() != length) {
      throw std::domain_error(std::string("This field (name=")
                              + name + ") uses "
                              + std::to_string(type.PointDimensionCount())
                              + " dimensions; cannot change to (incoming) "
                              + std::to_string(length)
                              + " dimensions");
    }

    fields_data = FloatPoint::Pack(points, length);
  }

  void SetBytesValue(const lucene::core::util::BytesRef& value) {
    throw std::invalid_argument("Cannot change value type "
                                "from float to BytesRef");
  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() noexcept {
    lucene::core::util::BytesRef& bytes =
    std::get<lucene::core::util::BytesRef>(fields_data);
    lucene::core::util::numeric::Number number(FloatPoint::DecodeDimension(
                                              bytes.bytes.get(), bytes.offset));
    return number;
  }
};

class FloatRange : public Field {
 private:
  static FieldType GetType(const uint32_t dimensions) {
    return FieldTypeBuilder()
           .SetDimensions(dimensions * 2, sizeof(float))
           .Build();
  }

  static void CheckArgs(const float* min,
                        const float* max,
                        const uint32_t length) {
    if (min == nullptr
        || max == nullptr
        || length == 0) {
      throw std::invalid_argument("Min/Max range values cannot "
                                  "be null or empty");
    }

    if (length > 4) {
      throw std::invalid_argument("FloatRange does not support greater "
                                  "than 4 dimensions");
    }
  }

  static void Encode(const float val, char* bytes, const uint32_t offset) {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(
      lucene::core::util::numeric::NumericUtils::FloatToSortableInt(val),
      bytes,
      offset);
  }

  static void VerifyAndEncode(const float* min,
                              const float* max,
                              const uint32_t length,
                              char* bytes) {
    for (uint32_t d = 0, i = 0, j = length * sizeof(float)
         ; d < length
         ; ++d, i += sizeof(float), j += sizeof(float)) {
      if (lucene::core::util::numeric::Double::IsNaN(min[d])) {
        throw std::invalid_argument(std::string("Invalid min value (")
                + std::to_string(lucene::core::util::numeric::FloatConsts::NaN)
                + ") in FloatRange");
      }
      if (lucene::core::util::numeric::Double::IsNaN(max[d])) {
        throw std::invalid_argument(std::string("Invalid max value (")
                + std::to_string(lucene::core::util::numeric::FloatConsts::NaN)
                + ") in FloatRange");
      }
      if (min[d] > max[d]) {
        throw std::invalid_argument(std::string("Min value (")
                                    + std::to_string(min[d])
                                    + ") is greater than max value ("
                                    + std::to_string(max[d])
                                    + ")");
      }
      FloatRange::Encode(min[d], bytes, i);
      FloatRange::Encode(max[d], bytes, j);
    }
  }
  
 public:
  FloatRange(const std::string& name,
             const float* min,
             const float* max,
             const uint32_t length)
    : Field(name, FloatRange::GetType(length)) {
    SetRangeValues(min, max, length);
  }

  void SetRangeValues(const float* min,
                      const float* max,
                      const uint32_t length) {
    FloatRange::CheckArgs(min, max, length);
    if (length * 2 != type.PointDimensionCount()) {
      throw std::invalid_argument(std::string("Field (name=")
                                + name + ") uses"
                                + std::to_string(type.PointDimensionCount() / 2)
                                + " dimensions; cannot change to (incoming) "
                                + std::to_string(length)
                                + " dimensions");
    }

    if (!std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      fields_data = lucene::core::util::BytesRef(sizeof(float) * 2 * length); 
    }

    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    FloatRange::VerifyAndEncode(min, max, length, bytesref.bytes.get());
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

class IntPoint : public Field {
 private:
  static FieldType GetType(const uint32_t num_dims) {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, sizeof(int32_t))
           .Build();
  }

  static lucene::core::util::BytesRef Pack(const int32_t* points,
                                           const uint32_t length) {
    if (points == nullptr) {
      throw std::invalid_argument("Point must not be null");
    }
    if (length == 0) {
      throw std::invalid_argument("Point must not be 0 dimensions");
    }

    const uint32_t packed_size = length * sizeof(int32_t);
    char packed[packed_size];

    for (int32_t dim = 0 ; dim < length ; dim++) {
      IntPoint::EncodeDimension(points[dim], packed, dim * sizeof(int32_t));
    }

    return lucene::core::util::BytesRef(packed, packed_size);
  }

 public:
  static void EncodeDimension(const int32_t value,
                              char* dest,
                              const int32_t offset) {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(value,
                                                                  dest,
                                                                  offset);
  }

  static int32_t DecodeDimension(const char* value, const uint32_t offset) {
    return
    lucene::core::util::numeric::NumericUtils::SortableBytesToInt(value,
                                                                  offset);
  }

 public:
  IntPoint(const std::string& name, const int* points, const uint32_t length)
    : Field(name, IntPoint::Pack(points, length), IntPoint::GetType(length)) {
  }

  void SetIntValue(const int32_t value) {
    SetIntValues(&value, 1);
  }

  void SetIntValues(const int32_t* points, const uint32_t length) {
    if (type.PointDimensionCount() != length) {
      throw std::invalid_argument(std::string("This field (name=") + name
                                  + ") uses "
                                  + std::to_string(type.PointDimensionCount())
                                  + " dimensions; cannot change to (incoming) "
                                  + std::to_string(length) + " dimensions");
    }

    fields_data = IntPoint::Pack(points, length);
  }

  void SetBytesValue(const lucene::core::util::BytesRef&) {
    throw
    std::invalid_argument("Cannot change value type from int to BtyesRef");
  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() noexcept {
    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);
    
    return lucene::core::util::numeric::Number(
    IntPoint::DecodeDimension(bytesref.bytes.get(), bytesref.offset));
  }
};

class IntRange : public Field {
 private:
  static FieldType GetType(const int32_t dimensions) {
    if (dimensions > 4) {
      throw std::invalid_argument("IntRange does not support greater "
                                  "than 4 dimensions");
    }

    return FieldTypeBuilder()
           .SetDimensions(dimensions * 2, sizeof(int32_t))
           .Build();
  }

  static void CheckArgs(const int32_t* min,
                        const int32_t* max,
                        const uint32_t length) {
    if (min == nullptr
        || max == nullptr
        || length == 0) {
      throw std::invalid_argument("Min/Max range values cannot "
                                  "be null or empty");
    }

    if (length > 4) {
      throw std::invalid_argument("IntRange does not support "
                                  "greater than 4 dimensions");
    }
  }
  
  static void VerifyAndEncode(const int32_t* min,
                              const int32_t* max,
                              const uint32_t length,
                              char* bytes) {
    for (int32_t d = 0, i = 0, j = length * sizeof(int32_t)
         ; d < length
         ; ++d, i += sizeof(int32_t), j += sizeof(int32_t)) {
      if (lucene::core::util::numeric::Double::IsNaN(min[d])) {
        throw std::invalid_argument(std::string("Invalid min value (")
                + std::to_string(lucene::core::util::numeric::DoubleConsts::NaN)
                + ") in IntRange");
      }
      if (lucene::core::util::numeric::Double::IsNaN(max[d])) {
        throw std::invalid_argument(std::string("Invalid max value (")
                + std::to_string(lucene::core::util::numeric::DoubleConsts::NaN)
                + ") in IntRange");
      }
      if (min[d] > max[d]) {
        throw std::invalid_argument(std::string("Min value (")
                                    + std::to_string(min[d])
                                    + ") is greater than max value ("
                                    + std::to_string(max[d])
                                    + ")");
      }

      IntRange::Encode(min[d], bytes, i);
      IntRange::Encode(max[d], bytes, j);
    }
  }

  static void Encode(const int32_t val, char* bytes, const int32_t offset) {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(val,
                                                                  bytes,
                                                                  offset);
  }

 public:
  IntRange(const std::string& name,
           const int32_t* min,
           const int32_t* max,
           const uint32_t length)
    : Field(name, IntRange::GetType(length)) {
    SetRangeValues(min, max, length);
  }

  void SetRangeValues(const int32_t* min,
                      const int32_t* max,
                      const uint32_t length) {
    IntRange::CheckArgs(min, max, length);

    if (length * 2 != type.PointDimensionCount()) {
      throw std::domain_error(std::string("Field (name=")
                              + name + ") uses "
                              + std::to_string(type.PointDimensionCount() / 2)
                              + " dimension; cannot change to (incoming) "
                              + std::to_string(length)
                              + " dimensions");
    }

    if (!std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      fields_data = lucene::core::util::BytesRef(sizeof(int32_t) * length);
    }

    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    IntRange::VerifyAndEncode(min, max, length, bytesref.bytes.get());
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

class LongPoint : public Field {
 private: 
  static FieldType GetType(const int32_t num_dims) {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, sizeof(int64_t))
           .Build();
  }

  static lucene::core::util::BytesRef
  Pack(const int64_t* points, const uint32_t length) {
    if (points == nullptr) {
      throw std::invalid_argument("Point must not be null");
    }

    if (length == 0) {
      throw std::invalid_argument("Point must not be 0 dimensions");
    }

    const uint32_t packed_size = length * sizeof(int64_t);
    char packed[packed_size];

    for (int32_t dim = 0 ; dim < length ; ++dim) {
      LongPoint::EncodeDimension(points[dim], packed, dim * sizeof(int64_t));
    }

    return lucene::core::util::BytesRef(packed, packed_size);
  }

 public:
  static void EncodeDimension(const int64_t value,
                              char* dest,
                              const int32_t offset) {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(value,
                                                                   dest,
                                                                   offset);
  }

  static int64_t DecodeDimension(char* value, const int32_t offset) {
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(value,
                                                                   offset);
  }

 public:
  LongPoint(const std::string& name,
            const int64_t* points,
            const uint32_t length)
    : Field(name, LongPoint::Pack(points, length), LongPoint::GetType(length)) {
  }

  void SetLongValue(const int64_t value) {
    SetLongValues(&value, 1);
  }

  void SetLongValues(const int64_t* points, const int32_t length) {
    if (type.PointDimensionCount() != length) {
      throw std::invalid_argument(std::string("This field (name=")
                                  + name + ") uses "
                                  + std::to_string(type.PointDimensionCount())
                                  + " dimensions; cannot change to (incoming) "
                                  + std::to_string(length)
                                  + " dimensions");
    }

    fields_data = LongPoint::Pack(points, length);
  }

  void SetBytesValue(const lucene::core::util::BytesRef&) {
    throw std::invalid_argument("Cannot change value type from "
                                "long to BytesRef");
  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() noexcept {
    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return lucene::core::util::numeric::Number(
    LongPoint::DecodeDimension(bytesref.bytes.get(), bytesref.offset));
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

class LongRange : public Field {
 private:
  static FieldType GetType(const int32_t dimensions) {
    return FieldTypeBuilder()
           .SetDimensions(dimensions * 2, sizeof(int64_t))
           .Build();
  }

  static void CheckArgs(const int64_t* min,
                        const int64_t* max,
                        const int32_t length) {
    
    if (min == nullptr
        || max == nullptr
        || length == 0) {
      throw std::invalid_argument("Min/Max range values cannot "
                                  "be null or empty");
    }

    if (length > 4) {
      throw std::invalid_argument("LongRange does not support greater than "
                                  " 4 dimensions");
    }
  }

  static void Encode(const int64_t val,
                     char* bytes,
                     const int32_t offset) {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(val,
                                                                   bytes,
                                                                   offset);
  }

  static void VerifyAndEncode(const int64_t* min,
                              const int64_t* max,
                              const int32_t length,
                              char* bytes) {
    for (int32_t d = 0, i = 0, j = length * sizeof(int64_t)
         ; d < length
         ; ++d, i += sizeof(int64_t), j += sizeof(int64_t)) {
      if (lucene::core::util::numeric::Double::IsNaN(min[d])) {
        throw std::invalid_argument(std::string("Invalid min value (")
                + std::to_string(lucene::core::util::numeric::DoubleConsts::NaN)
                + ") in LongRange");
      }
      if (lucene::core::util::numeric::Double::IsNaN(max[d])) {
        throw std::invalid_argument(std::string("Invalid max value (")
                + std::to_string(lucene::core::util::numeric::DoubleConsts::NaN)
                + ") in LongRange");
      }
      if (min[d] > max[d]) {
        throw std::invalid_argument(std::string("Min value (")
                                    + std::to_string(min[d])
                                    + ") is greater than max value ("
                                    + std::to_string(max[d])
                                    + ")");
      }

      LongRange::Encode(min[d], bytes, i);
      LongRange::Encode(max[d], bytes, j);
    }
  }

 public:
  LongRange(const std::string name,
            const int64_t* min,
            const int64_t* max,
            const int32_t length)
    : Field(name, LongRange::GetType(length)) {
    SetRangeValues(min, max, length);
  }

  void SetRangeValues(const int64_t* min,
                      const int64_t* max,
                      const int32_t length) {
    LongRange::CheckArgs(min, max, length); 

    if (length * 2 != type.PointDimensionCount()) {
      throw std::invalid_argument(std::string("Field (name=")
                                + name + ") uses "
                                + std::to_string(type.PointDimensionCount() / 2)
                                + " dimensions; cannot change to (incoming) "
                                + std::to_string(length)
                                + " dimensions");

    }

    if (!std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      fields_data = lucene::core::util::BytesRef(sizeof(int64_t) * length);
    }

    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);
    LongRange::VerifyAndEncode(min, max, length, bytesref.bytes.get());
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

}  // namespace document
}  // namespace core
}  // namespace lucene

#endif  // SRC_DOCUMENT_FIELD_H_
