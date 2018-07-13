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

#include <Analysis/Reader.h>
#include <Document/Field.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using lucene::core::analysis::StringReader;
using lucene::core::document::Field;
using lucene::core::document::FieldType;

TEST(FIELD__TESTS, BASIC__TEST) {
  StringReader* reader = new StringReader(); 
  std::string value("xxxxxxxx");
  reader->SetValue(value);
  std::string name("kkk");
  FieldType type;

  // Field field(name, reader, type);
  const char buf[] = "asdfasdf";
  Field field(name, buf, sizeof(buf), type);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
