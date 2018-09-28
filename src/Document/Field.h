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
#include <Util/Ref.h>
#include <Util/Numeric.h>
#include <optional>
#include <variant>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <string>
#include <utility>

namespace lucene {
namespace core {
namespace document {

class FieldTypeBuilder;

class FieldType {
  friend class FieldTypeBuilder;

 public:
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

/**
 * Field class is only movable, not assignable.
 * Thus if you want to assign one field to another, use move assignment
 * 1. Field(std::move(original_field))
 * 2. field = std::move(original_field)
 */
class Field {
 protected:
  FieldType type;
  const std::string* name;
  // TODO(0ctopus13prime): Can we just have each item instead of using variant?
  std::variant<lucene::core::util::BytesRef,
               std::string,
               std::unique_ptr<lucene::core::analysis::Reader>,
               lucene::core::util::numeric::Number> fields_data;
  std::unique_ptr<lucene::core::analysis::TokenStream> tokenstream;

 protected:
  Field(const std::string& name,
        const FieldType& type)
    : type(type),
      name(&name),
      fields_data(),
      tokenstream() {
  }

 public:
  /**
   *  @param name Field name
   *  @param reader Reader value. Field instance will manage the
   *                given Reader's life cycle
   *  @param type Field type
   */
  Field(const std::string& name,
       lucene::core::analysis::Reader* reader,
       const FieldType& type)
    : type(type),
      name(&name),
      fields_data(std::unique_ptr<lucene::core::analysis::Reader>(reader)),
      tokenstream() {
  }

  /**
   *  @param name Field name
   *  @param tokenstream TokenStream value. Field instance will manage the 
   *                     given TokenStream's life cycle
   *  @param type Field type
   */
  Field(const std::string& name,
        lucene::core::analysis::TokenStream* tokenstream,
        const FieldType& type)
    : type(type),
      name(&name),
      fields_data(),
      tokenstream(tokenstream) {
  }

  Field(const std::string& name,
        const char* value,
        const uint32_t length,
        const FieldType& type)
    : Field(name, value, 0, length, type) {
  }


  Field(const std::string& name,
        const char value[],
        const uint32_t offset,
        const uint32_t length,
        const FieldType& type)
    : Field(name, lucene::core::util::BytesRef(value, offset, length), type) {
  }

  Field(const std::string& name,
        const lucene::core::util::BytesRef& bytes,
        const FieldType& type)
    : type(type),
      name(&name),
      fields_data(bytes),
      tokenstream() {
  }

  Field(const std::string& name,
        lucene::core::util::BytesRef&& bytes,
        const FieldType& type)
    : type(type),
      name(&name),
      fields_data(std::forward<lucene::core::util::BytesRef>(bytes)),
      tokenstream() {
  }

  Field(const std::string& name,
        const std::string& value,
        const FieldType& type)
    : type(type),
      name(&name),
      fields_data(value),
      tokenstream() {
  }

  Field(const std::string& name,
        std::string&& value,
        const FieldType& type)
    : type(type),
      name(&name),
      fields_data(std::forward<std::string>(value)),
      tokenstream() {
  }

  Field(const Field&) = delete;

  Field(Field&& other)
    : type(other.type),
      name(other.name),
      fields_data(std::move(other.fields_data)),
      tokenstream(std::move(other.tokenstream)) {
  }

  Field& operator=(const Field&) = delete;

  /**
   * Only for same field assignment is allowed.
   * Must have same name, same type.
   * This asignment does not check it's equivalence at the beginning.
   */
  Field& operator=(Field&& other) {
    type = other.type;
    name = other.name;
    tokenstream = std::move(other.tokenstream);
    fields_data = std::move(other.fields_data);
  }

  virtual ~Field() { }

  virtual void SetStringValue(const std::string& value) {
    if (auto org_value = std::get_if<std::string>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  virtual void SetStringValue(std::string&& value) {
    if (auto org_value = std::get_if<std::string>(&fields_data)) {
      *org_value = std::forward<std::string>(value);
      return;
    }

    throw std::runtime_error("Field data can not be assigned from std::string");
  }

  virtual void SetReaderValue(lucene::core::analysis::Reader* value) {
    if (auto uniq_reader =
          std::get_if<std::unique_ptr<lucene::core::analysis::Reader>>(
                                                                &fields_data)) {
      (*uniq_reader).reset(value);
      return;
    }

    throw std::runtime_error("Field data can not be assigned from Reader");
  }

  virtual void SetBytesValue(const char value[], const uint32_t length) {
    SetBytesValue(lucene::core::util::BytesRef(value, 0, length));
  }

  virtual void SetBytesValue(const lucene::core::util::BytesRef& value) {
    if (auto org_value =
          std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      *org_value = value;
      return;
    }

    throw std::runtime_error("Field data can not be assigned from BytesRef");
  }

  virtual void SetBytesValue(lucene::core::util::BytesRef&& value) {
    if (auto org_value =
          std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      *org_value = std::forward<lucene::core::util::BytesRef>(value);
      return;
    }

    throw std::runtime_error("Field data can not be assigned from BytesRef");
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

  virtual void
  SetTokenStream(lucene::core::analysis::TokenStream* new_tokenstream) {
    tokenstream.reset(new_tokenstream);
  }

  const std::string& Name() const noexcept {
    return *name;
  }

  virtual std::optional<std::reference_wrapper<std::string>>
  StringValue() {
    if (auto str = std::get_if<std::string>(&fields_data)) {
      return *str;
    }

    return {};
  }

  virtual std::optional<std::reference_wrapper<lucene::core::analysis::Reader>>
  ReaderValue() {
    if (auto uniq_reader =
          std::get_if<std::unique_ptr<lucene::core::analysis::Reader>>(
                                                                &fields_data)) {
      return *((*uniq_reader).get());
    }

    return {};
  }

  virtual std::optional<lucene::core::util::numeric::Number>
  NumericValue() {
    if (auto number =
          std::get_if<lucene::core::util::numeric::Number>(&fields_data)) {
      return *number;
    }

    return {};
  }

  virtual std::optional<std::reference_wrapper<lucene::core::util::BytesRef>>
  BinaryValue() {
    if (auto bytes = std::get_if<lucene::core::util::BytesRef>(&fields_data)) {
      return *bytes;
    }

    return {};
  }

  const FieldType& GetFieldType() const noexcept {
    return type;
  }

  virtual lucene::core::analysis::TokenStream*
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

  void SetValue(const lucene::core::util::BytesRef& new_value) noexcept {
    value = new_value;
  }

  void SetValue(lucene::core::util::BytesRef&& value) noexcept {
    value = std::forward<lucene::core::util::BytesRef>(value);
  }

  bool IncrementToken() {
    if (!used) {
      ClearAttributes();
      bytes_att->SetBytesRef(value);
      return (used = true);
    } else {
      return false;
    }
  }

  void Reset() {
    used = false;
  }

  void Close() {
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

  void SetValue(const std::string& new_value) noexcept {
    value = new_value;
  }

  void SetValue(std::string&& new_value) noexcept {
    value = std::forward<std::string>(new_value);
  }

  bool IncrementToken() {
    if (!used) {
      ClearAttributes();
      term_attr->Append(value);
      offset_attr->SetOffset(0, value.size());
      return (used = true);
    } else {
      return false;
    }
  }

  void End() {
    lucene::core::analysis::TokenStream::End();
    const uint32_t final_offset = value.size();
    offset_attr->SetOffset(final_offset, final_offset);
  }

  void Reset() {
    used = true;
  }

  void Close() {
    // In Java, it assigns a null to value but we do nothing here.
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

  virtual ~TextField() { }
};

class StringField : public Field {
 public:
  static FieldType TYPE_NOT_STORED;
  static FieldType TYPE_STORED;

 public:
  StringField(const std::string& name,
              const std::string& value,
              const Field::Store stored)
    : Field(name, value, (stored == Field::Store::YES ?
                          TYPE_STORED : TYPE_NOT_STORED)) {
  }

  StringField(const std::string& name,
              std::string&& value,
              const Field::Store stored)
    : Field(name,
            std::forward<std::string>(value),
            (stored == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  StringField(const std::string& name,
              const lucene::core::util::BytesRef& value,
              const Field::Store stored)
    : Field(name,
            value,
            (stored == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  StringField(const std::string& name,
              lucene::core::util::BytesRef&& value,
              const Field::Store stored)
    : Field(name,
            std::forward<lucene::core::util::BytesRef>(value),
            (stored == Field::Store::YES ? TYPE_STORED : TYPE_NOT_STORED)) {
  }

  virtual ~StringField() { }
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
    : Field(name, std::forward<lucene::core::util::BytesRef>(bytes), type) {
  }

  StoredField(const std::string& name,
              const char value[],
              const uint32_t length)
    : Field(name, value, length, TYPE) {
  }

  StoredField(const std::string& name,
              const char value[],
              const uint32_t offset,
              const uint32_t length)
    : Field(name, value, offset, length, TYPE) {
  }

  StoredField(const std::string& name,
              const lucene::core::util::BytesRef& bytes)
    : Field(name, bytes, TYPE) {
  }

  StoredField(const std::string& name,
              lucene::core::util::BytesRef&& bytes)
    : Field(name, std::forward<lucene::core::util::BytesRef>(bytes), TYPE) {
  }

  StoredField(const std::string& name,
              const std::string& value)
    : Field(name, value, TYPE) {
  }

  StoredField(const std::string& name,
              std::string&& value,
              const FieldType& type)
    : Field(name, std::forward<std::string>(value), type) {
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

  StoredField(const std::string& name, const int32_t value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }

  StoredField(const std::string& name, const int64_t value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }

  StoredField(const std::string& name, const float value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }

  StoredField(const std::string& name, const double value)
    : Field(name, TYPE) {
      fields_data = lucene::core::util::numeric::Number(value);
  }

  virtual ~StoredField() { }
};

class NumericDocValuesField : public Field {
 public:
  static FieldType TYPE;

  NumericDocValuesField(const std::string& name, const int64_t value)
    : Field(name, TYPE) {
    fields_data = lucene::core::util::numeric::Number(value);
  }

  virtual ~NumericDocValuesField() { }

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

  virtual ~FloatDocValuesField() { }

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

  virtual ~DoubleDocValuesField() { }

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

  virtual ~BinaryDocValuesField() { }
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

  virtual ~SortedDocValuesField() { }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class SortedNumericDocValuesField : public Field {
 public:
  static FieldType TYPE;

 public:
  SortedNumericDocValuesField(const std::string& name, const int64_t value)
    : Field(name, TYPE) {
    fields_data = lucene::core::util::numeric::Number(value);
  }

  virtual ~SortedNumericDocValuesField() { }

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

  virtual ~SortedSetDocValuesField() { }

  // static newSlowRangeQuery TODO(0ctopus13prime): Implement it.
  // static newSlowExactQuery TODO(0ctopus13prime): Implement it.
};

class BinaryPoint : public Field {
 private:
  static FieldType GetType(const uint32_t num_dims,
                           const uint32_t bytes_per_dim) noexcept {
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
  Pack(const char* points,
       const uint32_t points_length,
       const uint32_t bytes_per_dim) {
    const uint32_t size = bytes_per_dim * points_length;
    return lucene::core::util::BytesRef(points, 0, size);
  }

 public:
  BinaryPoint(const std::string& name,
              const char* points,
              const uint32_t points_length,
              const uint32_t bytes_per_dim)
    : Field(name,
            Pack(points, points_length, bytes_per_dim),
            GetType(points_length, bytes_per_dim)) {
  }

  BinaryPoint(const std::string& name,
              const char* packed_point,
              const uint32_t bytes_per_dim,
              const FieldType& type)
    : Field(name, packed_point, bytes_per_dim, type) {
    if (bytes_per_dim != type.dimension_count * type.dimension_num_bytes) {
      throw std::runtime_error("Packed point is length=" +
                               std::to_string(bytes_per_dim) +
                               " but type.dimension_count=" +
                               std::to_string(type.dimension_count) +
                               " and type.dimension_num_bytes=" +
                               std::to_string(type.dimension_num_bytes));
    }
  }

  virtual ~BinaryPoint() { }

  // static Query newExactQuery TODO(0ctopus13prime): Implement it.
  // static Query newRangeQuery TODO(0ctopus13prime): Implement it.
  // static Query newRangeQuery TODO(0ctopus13prime): Implement it.
  // static Query newSetQuery TODO(0ctopus13prime): Implement it.
};

class DoublePoint : public Field {
 private:
  static FieldType GetType(const uint32_t num_dims) {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, sizeof(double))
           .Build();
  }

  static lucene::core::util::BytesRef
  Pack(const std::initializer_list<const double>& point) {
    const uint32_t size = point.size();
    const uint32_t packed_size = size * sizeof(double);
    char packed[packed_size];

    uint32_t offset = 0;
    for (const double p : point) {
      DoublePoint::EncodeDimension(p, packed, offset);
      offset += sizeof(double);
    }

    return lucene::core::util::BytesRef(packed, 0, packed_size);
  }

 public:
  static void EncodeDimension(const double value,
                              char dest[],
                              const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(
      lucene::core::util::numeric::NumericUtils::DoubleToSortableLong(value),
      dest,
      offset);
  }

  static double DecodeDimension(const char value[],
                                const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::SortableLongToDouble(
      lucene::core::util::numeric::NumericUtils::SortableBytesToLong(value,
                                                                     offset));
  }

  static double NextUp(const double d) noexcept {
    if (lucene::core::util::numeric::Double::DoubleToRawLongBits(d)
        != 0x8000000000000000 /*-0.0D*/) {
      return lucene::core::util::numeric::NumericUtils::DoubleNextUp(d);
    } else {
      return +0.0D;
    }
  }

  static double NextDown(const double d) noexcept {
    if (lucene::core::util::numeric::Double::DoubleToRawLongBits(d)
        != 0.0D) {
      return lucene::core::util::numeric::NumericUtils::DoubleNextDown(d);
    } else {
      return -0.0D;
    }
  }

  void SetDoubleValue(const double value) {
    SetDoubleValues({value});
  }

  void SetDoubleValues(const std::initializer_list<const double>& point) {
    if (type.dimension_count == point.size()) {
      fields_data = DoublePoint::Pack(point);
    } else {
      throw std::invalid_argument(std::string("This field (name=") + *name
            + ") uses" + std::to_string(type.dimension_count)
            + " dimensions; cannot change to (incoming) "
            + std::to_string(point.size()) + " dimensions");
    }
  }

  void SetBytesValue(const lucene::core::util::BytesRef bytes) {
    throw
    std::invalid_argument("Cannot change value type from double to BytesRef");
  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() {
    lucene::core::util::BytesRef& bytes_ref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return lucene::core::util::numeric::Number(
      DoublePoint::DecodeDimension(bytes_ref.Bytes(), bytes_ref.Offset()));
  }

  DoublePoint(const std::string& name,
              const std::initializer_list<const double>& point)
    : Field(name,
            DoublePoint::Pack(point),
            DoublePoint::GetType(point.size())) {
  }

  virtual ~DoublePoint() { }

  // TODO(0ctopus13prime): Implement Query stuffs
};

class DoubleRange : public Field {
 private:
  static FieldType GetType(const uint32_t dimensions) {
    if (dimensions <= 4) {
      return FieldTypeBuilder()
             .SetDimensions(dimensions * 2, sizeof(double))
             .Build();
    } else {
      throw std::invalid_argument("DoubleRange does not support greater"
                                  "than 4 dimensions");
    }
  }

  static void CheckArgs(const double min[],
                        const double max[],
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

  static void
  Encode(const double val, char bytes[], const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(
      lucene::core::util::numeric::NumericUtils::DoubleToSortableLong(val),
      bytes,
      offset);
  }

  static void VerifyAndEncode(const double min[],
                              const double max[],
                              const uint32_t length,
                              char bytes[]) {
    for (uint32_t d = 0, i = 0, j = length * sizeof(double)
         ; d < length
         ; ++d, i += sizeof(double), j += sizeof(double)) {
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
    }

    for (uint32_t d = 0, i = 0, j = length * sizeof(double)
         ; d < length
         ; ++d, i += sizeof(double), j += sizeof(double)) {
      DoubleRange::Encode(min[d], bytes, i);
      DoubleRange::Encode(max[d], bytes, j);
    }
  }

 public:
  static double
  DecodeMin(const char bytes[], const uint32_t dimension) noexcept {
    const uint32_t offset = dimension * sizeof(double);

    return lucene::core::util::numeric::NumericUtils::SortableLongToDouble(
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(bytes,
                                                                   offset));
  }

  static double DecodeMax(const char bytes[],
                          const uint32_t bytes_length,
                          const uint32_t dimension) noexcept {
    const uint32_t offset =
    (bytes_length >> 1) + dimension * sizeof(double);

    return lucene::core::util::numeric::NumericUtils::SortableLongToDouble(
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(bytes,
                                                                   offset));
  }

  // TODO(0ctopus13prime): Implement Query stuffs

 public:
  DoubleRange(const std::string& name,
              const double min[],
              const double max[],
              const uint32_t length)
    : Field(name, GetType(length)) {
    SetRangeValues(min, max, length);
  }

  virtual ~DoubleRange() { }

  void SetRangeValues(const double min[],
                      const double max[],
                      const uint32_t length) {
    CheckArgs(min, max, length);

    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);

    const uint32_t required_min_capacity = sizeof(double) * 2 * length;

    if (bytes_ref.Capacity() >= required_min_capacity) {
      DoubleRange::VerifyAndEncode(min,
                                   max,
                                   length,
                                   bytes_ref.Bytes());
    } else {
      bytes_ref = lucene::core::util::BytesRef(required_min_capacity);
      DoubleRange::VerifyAndEncode(min,
                                   max,
                                   length,
                                   bytes_ref.Bytes());
    }
  }

  double GetMin(const uint32_t dimension) noexcept {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);

    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);

    return DoubleRange::DecodeMin(bytes_ref.Bytes(),
                                  dimension);
  }

  double GetMax(const uint32_t dimension) noexcept {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);
    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);

    return DoubleRange::DecodeMax(bytes_ref.Bytes(),
                                  bytes_ref.Length(),
                                  dimension);
  }
};

class FloatPoint : public Field {
 public:
  static float NextUp(const float f) noexcept {
    if (lucene::core::util::numeric::Float::FloatToIntBits(f)
        != 0x80000000) {
      return lucene::core::util::numeric::NumericUtils::FloatNextUp(f);
    } else {
      return +0.0F;
    }
  }

  static float NextDown(const float f) noexcept {
    if (lucene::core::util::numeric::Float::FloatToIntBits(f)
        != 0) {
      return lucene::core::util::numeric::NumericUtils::FloatNextDown(f);
    } else {
      return -0.0F;
    }
  }

  static void EncodeDimension(const float value,
                              char dest[],
                              const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(
      lucene::core::util::numeric::NumericUtils::FloatToSortableInt(value),
      dest, offset);
  }

  static float DecodeDimension(const char value[],
                               const uint32_t offset) noexcept {
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
    if (points == nullptr || length == 0) {
      throw std::invalid_argument("Points must not be null and "
                                  "points must not be 0 dimensions");
    }

    const uint32_t packed_size = length * sizeof(float);
    char packed[packed_size];

    for (uint32_t dim = 0 ; dim < length ; ++dim) {
      FloatPoint::EncodeDimension(points[dim], packed, dim * sizeof(float));
    }

    return lucene::core::util::BytesRef(packed, packed_size);
  }

 public:
  FloatPoint(const std::string& name,
             const float points[],
             const uint32_t length)
    : Field(name,
            FloatPoint::Pack(points, length),
            FloatPoint::GetType(length)) {
  }

  virtual ~FloatPoint() { }

  void SetFloatValue(const float value) {
    SetFloatValues(&value, 1);
  }

  void SetFloatValues(const float points[],
                      const uint32_t length) {
    if (type.dimension_count != length) {
      throw std::domain_error(std::string("This field (name=")
                              + *name + ") uses "
                              + std::to_string(type.dimension_count)
                              + " dimensions; cannot change to (incoming) "
                              + std::to_string(length)
                              + " dimensions");
    }

    fields_data = FloatPoint::Pack(points, length);
  }

  void SetBytesValue(const lucene::core::util::BytesRef&) {
    throw std::invalid_argument("Cannot change value type "
                                "from float to BytesRef");
  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() {
    lucene::core::util::BytesRef& bytes =
    std::get<lucene::core::util::BytesRef>(fields_data);

    lucene::core::util::numeric::Number number(FloatPoint::DecodeDimension(
                                              bytes.Bytes(),
                                              bytes.Offset()));
    return number;
  }
};

class FloatRange : public Field {
 private:
  static FieldType GetType(const uint32_t dimensions) noexcept {
    return FieldTypeBuilder()
           .SetDimensions(dimensions * 2, sizeof(float))
           .Build();
  }

  static void CheckArgs(const float min[],
                        const float max[],
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

  static float
  DecodeMin(const char bytes[], const uint32_t dimension) noexcept {
    const uint32_t offset = dimension * sizeof(float);
    return lucene::core::util::numeric::NumericUtils::SortableIntToFloat(
    lucene::core::util::numeric::NumericUtils::SortableBytesToInt(bytes,
                                                                  offset));
  }

  static float
  DecodeMax(const char bytes[],
            const uint32_t bytes_length,
            const uint32_t dimension) noexcept {
    const uint32_t offset = (bytes_length >> 1) + dimension * sizeof(float);
    return lucene::core::util::numeric::NumericUtils::SortableIntToFloat(
    lucene::core::util::numeric::NumericUtils::SortableBytesToInt(bytes,
                                                                  offset));
  }

  static void
  Encode(const float val, char bytes[], const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(
      lucene::core::util::numeric::NumericUtils::FloatToSortableInt(val),
      bytes,
      offset);
  }

  static void VerifyAndEncode(const float min[],
                              const float max[],
                              const uint32_t length,
                              char bytes[]) {
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
    }

    for (uint32_t d = 0, i = 0, j = length * sizeof(float)
         ; d < length
         ; ++d, i += sizeof(float), j += sizeof(float)) {
      FloatRange::Encode(min[d], bytes, i);
      FloatRange::Encode(max[d], bytes, j);
    }
  }

 public:
  FloatRange(const std::string& name,
             const float min[],
             const float max[],
             const uint32_t length)
    : Field(name, FloatRange::GetType(length)) {
    SetRangeValues(min, max, length);
  }

  virtual ~FloatRange() { }

  void SetRangeValues(const float min[],
                      const float max[],
                      const uint32_t length) {
    FloatRange::CheckArgs(min, max, length);
    if (length * 2 == type.dimension_count) {
      lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);

      const uint32_t required_min_capacity = sizeof(float) * 2 * length;

      if (bytes_ref.Capacity() >= required_min_capacity) {
        FloatRange::VerifyAndEncode(min, max, length, bytes_ref.Bytes());
      } else {
        bytes_ref = lucene::core::util::BytesRef(required_min_capacity);
        FloatRange::VerifyAndEncode(min, max, length, bytes_ref.Bytes());
      }
    } else {
      throw std::invalid_argument(std::string("Field (name=")
                                + *name + ") uses"
                                + std::to_string(type.dimension_count / 2)
                                + " dimensions; cannot change to (incoming) "
                                + std::to_string(length)
                                + " dimensions");
    }
  }

  float GetMin(const uint32_t dimension) {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);

    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);

    return FloatRange::DecodeMin(bytes_ref.Bytes(), dimension);
  }

  float GetMax(const uint32_t dimension) {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);

    lucene::core::util::BytesRef& bytes_ref =
      std::get<lucene::core::util::BytesRef>(fields_data);

    return FloatRange::DecodeMax(bytes_ref.Bytes(),
                                 bytes_ref.Length(),
                                 dimension);
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

class IntPoint : public Field {
 private:
  static FieldType GetType(const uint32_t num_dims) noexcept {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, sizeof(int32_t))
           .Build();
  }

  static lucene::core::util::BytesRef Pack(const int32_t points[],
                                           const uint32_t length) {
    if (points == nullptr || length == 0) {
      throw std::invalid_argument("Point must not be null "
                                  "and must not be 0 dimensions");
    }

    const uint32_t packed_size = length * sizeof(int32_t);
    char packed[packed_size];

    for (uint32_t dim = 0 ; dim < length ; ++dim) {
      IntPoint::EncodeDimension(points[dim], packed, dim * sizeof(int32_t));
    }

    return lucene::core::util::BytesRef(packed, packed_size);
  }

 public:
  static void EncodeDimension(const int32_t value,
                              char dest[],
                              const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(value,
                                                                  dest,
                                                                  offset);
  }

  static int32_t
  DecodeDimension(const char value[], const uint32_t offset) noexcept {
    return
    lucene::core::util::numeric::NumericUtils::SortableBytesToInt(value,
                                                                  offset);
  }

 public:
  IntPoint(const std::string& name,
           const int32_t points[],
           const uint32_t length)
    : Field(name, IntPoint::Pack(points, length), IntPoint::GetType(length)) {
  }

  virtual ~IntPoint() { }

  void SetIntValue(const int32_t value) {
    SetIntValues(&value, 1);
  }

  void SetIntValues(const int32_t points[], const uint32_t length) {
    if (type.dimension_count != length) {
      throw std::invalid_argument(std::string("This field (name=") + *name
                                  + ") uses "
                                  + std::to_string(type.dimension_count)
                                  + " dimensions; cannot change to (incoming) "
                                  + std::to_string(length) + " dimensions");
    }

    fields_data = IntPoint::Pack(points, length);
  }

  void SetBytesValue(const lucene::core::util::BytesRef&) {
    throw
    std::invalid_argument("Cannot change value type from int to BtyesRef");
  }

  std::optional<lucene::core::util::numeric::Number> NumericValue() {
    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return lucene::core::util::numeric::Number(
      IntPoint::DecodeDimension(bytesref.Bytes(), bytesref.Offset()));
  }
};

class IntRange : public Field {
 private:
  static FieldType GetType(const uint32_t dimensions) {
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

  static void VerifyAndEncode(const int32_t min[],
                              const int32_t max[],
                              const uint32_t length,
                              char bytes[]) {
    for (uint32_t d = 0, i = 0, j = length * sizeof(int32_t)
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
    }

    for (uint32_t d = 0, i = 0, j = length * sizeof(int32_t)
         ; d < length
         ; d++, i += sizeof(int32_t), j += sizeof(int32_t)) {
      IntRange::Encode(min[d], bytes, i);
      IntRange::Encode(max[d], bytes, j);
    }
  }

  static void
  Encode(const int32_t val, char bytes[], const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::IntToSortableBytes(val,
                                                                  bytes,
                                                                  offset);
  }

  static int32_t DecodeMin(const char bytes[],
                           const uint32_t dimension) noexcept {
    const uint32_t offset = dimension * sizeof(int32_t);
    return
    lucene::core::util::numeric::NumericUtils::SortableBytesToInt(bytes,
                                                                  offset);
  }

  static int32_t DecodeMax(const char bytes[],
                           const uint32_t bytes_length,
                           const uint32_t dimension) noexcept {
    const uint32_t offset = (bytes_length >> 1) + dimension * sizeof(int32_t);
    return
    lucene::core::util::numeric::NumericUtils::SortableBytesToInt(bytes,
                                                                  offset);
  }

 public:
  IntRange(const std::string& name,
           const int32_t min[],
           const int32_t max[],
           const uint32_t length)
    : Field(name, IntRange::GetType(length)) {
    SetRangeValues(min, max, length);
  }

  virtual ~IntRange() { }

  void SetRangeValues(const int32_t min[],
                      const int32_t max[],
                      const uint32_t length) {
    IntRange::CheckArgs(min, max, length);

    if (length * 2 != type.dimension_count) {
      throw std::domain_error(std::string("Field (name=")
                              + *name + ") uses "
                              + std::to_string(type.dimension_count / 2)
                              + " dimension; cannot change to (incoming) "
                              + std::to_string(length)
                              + " dimensions");
    }

    lucene::core::util::BytesRef& bytes_ref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    const uint32_t minimum_required = length * 2 * sizeof(int32_t);

    if (bytes_ref.Capacity() < minimum_required) {
      bytes_ref = lucene::core::util::BytesRef(minimum_required);
    }

    IntRange::VerifyAndEncode(min, max, length, bytes_ref.Bytes());
  }

  int32_t GetMin(const uint32_t dimension) noexcept {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);
    lucene::core::util::BytesRef& bytes_ref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return IntRange::DecodeMin(bytes_ref.Bytes(), dimension);
  }

  int32_t GetMax(const uint32_t dimension) noexcept {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);
    lucene::core::util::BytesRef& bytes_ref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return IntRange::DecodeMax(bytes_ref.Bytes(),
                               bytes_ref.Length(),
                               dimension);
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

class LongPoint : public Field {
 private:
  static FieldType GetType(const uint32_t num_dims) noexcept {
    return FieldTypeBuilder()
           .SetDimensions(num_dims, sizeof(int64_t))
           .Build();
  }

  static lucene::core::util::BytesRef
  Pack(const int64_t points[], const uint32_t length) {
    if (points == nullptr || length == 0) {
      throw std::invalid_argument("Point must not be null");
    }

    const uint32_t packed_size = length * sizeof(int64_t);
    char packed[packed_size];

    for (int32_t dim = 0 ; dim < length ; dim++) {
      LongPoint::EncodeDimension(points[dim], packed, dim * sizeof(int64_t));
    }

    return lucene::core::util::BytesRef(packed, packed_size);
  }

 public:
  static void EncodeDimension(const int64_t value,
                              char dest[],
                              const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(value,
                                                                   dest,
                                                                   offset);
  }

  static int64_t
  DecodeDimension(const char value[], const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(value,
                                                                   offset);
  }

 public:
  LongPoint(const std::string& name,
            const int64_t points[],
            const uint32_t length)
    : Field(name, LongPoint::Pack(points, length), LongPoint::GetType(length)) {
  }

  virtual ~LongPoint() { }

  void SetLongValue(const int64_t value) {
    SetLongValues(&value, 1);
  }

  void SetLongValues(const int64_t points[], const uint32_t length) {
    if (type.dimension_count != length) {
      throw std::invalid_argument(std::string("This field (name=")
                                  + *name + ") uses "
                                  + std::to_string(type.dimension_count)
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

  std::optional<lucene::core::util::numeric::Number> NumericValue() {
    lucene::core::util::BytesRef& bytesref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return lucene::core::util::numeric::Number(
    LongPoint::DecodeDimension(bytesref.Bytes(), bytesref.Offset()));
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

class LongRange : public Field {
 private:
  static FieldType GetType(const uint32_t dimensions) noexcept {
    return FieldTypeBuilder()
           .SetDimensions(dimensions * 2, sizeof(int64_t))
           .Build();
  }

  static void CheckArgs(const int64_t min[],
                        const int64_t max[],
                        const uint32_t length) {
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
                     char bytes[],
                     const uint32_t offset) noexcept {
    lucene::core::util::numeric::NumericUtils::LongToSortableBytes(val,
                                                                   bytes,
                                                                   offset);
  }

  static int64_t
  DecodeMin(const char bytes[], const uint32_t dimension) noexcept {
    const uint32_t offset = dimension * sizeof(int64_t);
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(bytes,
                                                                   offset);
  }

  static int64_t DecodeMax(const char bytes[],
                           const uint32_t bytes_length,
                           const uint32_t dimension) noexcept {
    const uint32_t offset = (bytes_length >> 1) + dimension * sizeof(int64_t);
    lucene::core::util::numeric::NumericUtils::SortableBytesToLong(bytes,
                                                                   offset);
  }

  static void VerifyAndEncode(const int64_t min[],
                              const int64_t max[],
                              const uint32_t length,
                              char bytes[]) {
    for (uint32_t d = 0, i = 0, j = length * sizeof(int64_t)
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
    }

    for (uint32_t d = 0, i = 0, j = length * sizeof(int64_t)
         ; d < length
         ; ++d, i += sizeof(int64_t), j += sizeof(int64_t)) {
      LongRange::Encode(min[d], bytes, i);
      LongRange::Encode(max[d], bytes, j);
    }
  }

 public:
  LongRange(const std::string& name,
            const int64_t min[],
            const int64_t max[],
            const uint32_t length)
    : Field(name, LongRange::GetType(length)) {
    SetRangeValues(min, max, length);
  }

  virtual ~LongRange() { }

  void SetRangeValues(const int64_t min[],
                      const int64_t max[],
                      const uint32_t length) {
    LongRange::CheckArgs(min, max, length);

    if (length * 2  == type.dimension_count) {
      lucene::core::util::BytesRef& bytes_ref =
        std::get<lucene::core::util::BytesRef>(fields_data);

      const uint32_t minimum_required = length * 2 * sizeof(int64_t);
      if (bytes_ref.Capacity() < minimum_required) {
        bytes_ref = lucene::core::util::BytesRef(minimum_required);
      }

      LongRange::VerifyAndEncode(min, max, length, bytes_ref.Bytes());
    } else {
      throw std::invalid_argument(std::string("Field (name=")
                                + *name + ") uses "
                                + std::to_string(type.dimension_count / 2)
                                + " dimensions; cannot change to (incoming) "
                                + std::to_string(length)
                                + " dimensions");
    }
  }

  int64_t GetMin(const uint32_t dimension) noexcept {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);
    lucene::core::util::BytesRef& bytes_ref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return LongRange::DecodeMin(bytes_ref.Bytes(), dimension);
  }

  int64_t GetMax(const uint32_t dimension) noexcept {
    lucene::core::util::ArrayUtil::CheckIndex(dimension,
                                              type.dimension_count / 2);
    lucene::core::util::BytesRef& bytes_ref =
    std::get<lucene::core::util::BytesRef>(fields_data);

    return LongRange::DecodeMax(bytes_ref.Bytes(),
                                bytes_ref.Length(),
                                dimension);
  }

  // TODO(0ctopus13prime): Other Query stuffs are need to be implemented.
};

}  // namespace document
}  // namespace core
}  // namespace lucene

#endif  // SRC_DOCUMENT_FIELD_H_
