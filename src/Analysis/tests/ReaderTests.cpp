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

#include <gtest/gtest.h>
#include <Analysis/Reader.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

using lucene::core::analysis::StringReader;

TEST(READER__TESTS, CONSTRUCTOR__TESTS) {
  const char* cstr = "Doochi core is so fast!";
  std::string str(cstr);

  {
    StringReader reader(cstr, 0, std::strlen(cstr));
    std::string line;
    reader.ReadLine(line);
    EXPECT_EQ(line, std::string(cstr));
  }

  {
    std::string str(cstr);
    StringReader reader(str);
    std::string line;
    reader.ReadLine(line);
    EXPECT_EQ(line, str);
  }

  {
    std::string str(cstr);
    StringReader reader1(str);
    StringReader reader2(reader1);

    std::string line1;
    reader1.ReadLine(line1);
    std::string line2;
    reader2.ReadLine(line2);
    EXPECT_EQ(line1, line2);
  }

  {
    std::string str(cstr);
    StringReader reader1(str);
    StringReader reader2(std::move(reader1));

    std::string line2;
    reader2.ReadLine(line2);
    EXPECT_EQ(str, line2);
  }
}

TEST(READER__TESTS, ASSGIN__TESTS) {
  const char* cstr = "Doochi core is so fast!";
  std::string str(cstr);
  StringReader reader(str);

  {
    StringReader reader1 = reader;
    std::string line;
    reader1.ReadLine(line);
    EXPECT_EQ(str, line);
  }

  {
    StringReader reader1 = std::move(reader);
    std::string line;
    reader1.ReadLine(line);
    EXPECT_EQ(str, line);
  }

  {
    StringReader reader1;
    std::istringstream iss(cstr);
    reader1 = iss;
    std::string line;
    reader1.ReadLine(line);
    EXPECT_EQ(str, line);
  }

  {
    StringReader reader1;
    std::istringstream iss(cstr);
    reader1 = std::move(iss);
    std::string line;
    reader1.ReadLine(line);
    EXPECT_EQ(str, line);
  }
}

TEST(READER_TESTS, SET__TESTS) {
  StringReader reader;
  const char* cstr = "Doochi core is so fast!";
  reader.SetValue(cstr, std::strlen(cstr));
  std::string line;
  reader.ReadLine(line);
  EXPECT_EQ(std::string(cstr), line);

  std::string str("Doochi Doochi Doochi!!");
  reader.SetValue(str);
  reader.ReadLine(line);
  EXPECT_EQ(str, line);
}

TEST(READER_TESTS, READ__TESTS) {
  const char* cstr = "Doochi! What should I do?";

  {
    StringReader reader;
    std::string str(cstr);
    reader.SetValue(str);
    for (uint32_t i = 0 ; i < std::strlen(cstr) ; ++i) {
      char expected = cstr[i];
      int actual = reader.Read();
      EXPECT_EQ(expected, actual);
      EXPECT_FALSE(reader.Eof());
    }

    EXPECT_EQ(-1, reader.Read());
    EXPECT_TRUE(reader.Eof());
  }

  {
    StringReader reader;
    reader.SetValue(cstr, std::strlen(cstr));
    char buf[512];
    int32_t read = reader.Read(buf, 0, sizeof(buf));
    EXPECT_EQ(std::strlen(cstr), read);
    buf[read] = '\0';
    EXPECT_STREQ(cstr, buf);
  }

  {
    StringReader reader(cstr, std::strlen(cstr));

    const char* skip_cstr = "Doochi! ";
    const char* part_cstr = "What should I do?";
    int skip_len = std::strlen(skip_cstr);
    reader.Skip(skip_len);

    std::string line;
    reader.ReadLine(line);
    EXPECT_EQ(std::string(part_cstr), line);
  }
}

TEST(READER_TESTS, MARK__TESTS) {
  StringReader reader;
  const char* cstr = "Bam! Bam!";
  reader.SetValue(cstr, std::strlen(cstr));
  EXPECT_TRUE(reader.MarkSupported());

  reader.Mark(1);
  reader.Read();  // B
  reader.Read();  // a
  reader.Read();  // m

  reader.Reset();
  EXPECT_EQ('a', reader.Read());

  std::string line;
  reader.ReadLine(line);
  reader.Reset();
  reader.ReadLine(line);
  EXPECT_EQ(std::string("am! Bam!"), line);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
