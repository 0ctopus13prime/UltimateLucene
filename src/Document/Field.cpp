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

#include <Document/Field.h>

using lucene::core::analysis::Reader;
using lucene::core::analysis::Analyzer;
using lucene::core::document::Field;
using lucene::core::analysis::TokenStream;
using lucene::core::index::IndexableField;
using lucene::core::index::IndexableFieldType;
using lucene::core::util::BytesRef;
using lucene::core::util::etc::Number;

/**
 *  Field
 */
Field::Field(const std::string& name,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(),
    tokenstream() {
}

Field::Field(const std::string& name,
             Reader* reader,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(std::unique_ptr<Reader>(reader)),
    tokenstream() {
}

Field::Field(const std::string& name,
             TokenStream* tokenstream,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(),
    tokenstream(tokenstream) {
}

Field::Field(const std::string& name,
             const char* value,
             const uint32_t length,
             const IndexableFieldType& type)
  : Field(name, value, 0, length, type) {
}

Field::Field(const std::string& name,
             const char* value,
             const uint32_t offset,
             const uint32_t length,
             const IndexableFieldType& type)
  : Field(name, BytesRef(value, offset, length), type) {
}

Field::Field(const std::string& name,
             const BytesRef& bytes,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(bytes),
    tokenstream() {
}

Field::Field(const std::string& name,
             BytesRef&& bytes,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(std::forward<BytesRef>(bytes)),
    tokenstream() {
}

Field::Field(const std::string& name,
             const std::string& value,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(value),
    tokenstream() {
}

Field::Field(const std::string& name,
             std::string&& value,
             const IndexableFieldType& type)
  : IndexableField(),
    type(type),
    name(name),
    fields_data(std::forward<std::string>(value)),
    tokenstream() {
}

Field::~Field() {
}

TokenStream* Field::GetTokenStream(Analyzer& analyzer, TokenStream& reuse) {
  // TODO(0ctopus13prime): Implement this
  return nullptr;
}
