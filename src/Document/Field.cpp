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
using lucene::core::document::FieldType;
using lucene::core::document::FieldTypeBuilder;
using lucene::core::document::TextField;
using lucene::core::document::StringField;
using lucene::core::document::StoredField;
using lucene::core::document::NumericDocValuesField;
using lucene::core::document::BinaryDocValuesField;
using lucene::core::document::SortedDocValuesField;
using lucene::core::document::SortedNumericDocValuesField;
using lucene::core::document::SortedSetDocValuesField;
using lucene::core::analysis::TokenStream;
using lucene::core::index::IndexOptions;
using lucene::core::index::DocValuesType;
using lucene::core::util::BytesRef;
using lucene::core::util::numeric::Number;

/**
 *  Field
 */
TokenStream* Field::GetTokenStream(Analyzer& analyzer, TokenStream& reuse) {
  // TODO(0ctopus13prime): Implement this
  return nullptr;
}

/**
 *  TextField
 */
FieldType TextField::TYPE_NOT_STORED = []() {
  return FieldTypeBuilder()
          .SetIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS)
          .SetTokenized(true)
          .Build();
}();

FieldType TextField::TYPE_STORED = []() {
  return FieldTypeBuilder()
          .SetIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS)
          .SetTokenized(true)
          .SetStored(true)
          .Build();
}();

/**
 *  StringField
 */
FieldType StringField::TYPE_NOT_STORED = []() {
  return FieldTypeBuilder()
         .SetOmitNorms(true)
         .SetIndexOptions(IndexOptions::DOCS)
         .SetTokenized(false)
         .Build();
}();

FieldType StringField::TYPE_STORED = []() {
  return FieldTypeBuilder()
         .SetOmitNorms(true)
         .SetIndexOptions(IndexOptions::DOCS)
         .SetStored(true)
         .SetTokenized(false)
         .Build();
}();


/**
 *  StoredField 
 */
FieldType StoredField::TYPE = []() {
  return FieldTypeBuilder()
         .SetStored(true)
         .Build();
}();

/**
 *  NumericDocValuesField
 */
FieldType NumericDocValuesField::TYPE = []() {
  return FieldTypeBuilder()
         .SetDocValuesType(DocValuesType::NUMERIC)
         .Build();
}();

/**
 *  BinaryDocValuesField
 */
FieldType BinaryDocValuesField::TYPE = []() {
  return FieldTypeBuilder()
         .SetDocValuesType(DocValuesType::BINARY)
         .Build();
}();

/**
 *  SortedDocValuesField
 */
FieldType SortedDocValuesField::TYPE = []() {
  return FieldTypeBuilder()
         .SetDocValuesType(DocValuesType::SORTED)
         .Build();
}();

/**
 *  SortedNumericDocValuesField 
 */
FieldType SortedNumericDocValuesField::TYPE = []() {
  return FieldTypeBuilder()
         .SetDocValuesType(DocValuesType::SORTED_NUMERIC)
         .Build();
}();


/**
 *  SortedSetDocValuesField
 */
FieldType SortedSetDocValuesField::TYPE = []() {
  return FieldTypeBuilder()
         .SetDocValuesType(DocValuesType::SORTED_SET)
         .Build();
}();
