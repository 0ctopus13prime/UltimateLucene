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

#include <Util/Etc.h>
#include <gtest/gtest.h>
#include <string>

using lucene::core::util::Version;
using lucene::core::util::Crc32;

TEST(ETC__TESTS, VERSION__TESTS) {
  {
    // Latest test
    std::string version = Version::LATEST.ToString();
    EXPECT_EQ(std::string("7.3.0"), Version::LATEST.ToString());

    EXPECT_EQ(7, Version::LATEST.GetMajor());
    EXPECT_EQ(3, Version::LATEST.GetMinor());
    EXPECT_EQ(0, Version::LATEST.GetBugfix());
    EXPECT_EQ(0, Version::LATEST.GetPreRelease());
  }

  {
    // Version::Parse test

    Version v730(Version::Parse("7.3.0"));
    EXPECT_EQ(Version::LATEST, v730);

    Version v800_pre(Version::Parse("8.0.0.1"));
    EXPECT_TRUE(v800_pre.OnOrAfter(v730));
    EXPECT_FALSE(v730.OnOrAfter(v800_pre));

    Version v720(Version::Parse("7.2.0"));
    EXPECT_TRUE(Version::LATEST.OnOrAfter(Version::LATEST));
    EXPECT_TRUE(Version::LATEST.OnOrAfter(v720));
    EXPECT_TRUE(v730.OnOrAfter(v720));
    EXPECT_FALSE(v720.OnOrAfter(v730));

    Version v_last_lucene(Version::Parse("255.255.255"));
    EXPECT_TRUE(v_last_lucene.OnOrAfter(v730));
    EXPECT_FALSE(v730.OnOrAfter(v_last_lucene));
  }

  {
    // Version::FromBits test

    Version v730(Version::FromBits(7, 3, 0));
    EXPECT_EQ(Version::LATEST, v730);

    Version v720(Version::FromBits(7, 2, 0));
    EXPECT_TRUE(Version::LATEST.OnOrAfter(Version::LATEST));
    EXPECT_TRUE(Version::LATEST.OnOrAfter(v720));
    EXPECT_TRUE(v730.OnOrAfter(v720));
    EXPECT_FALSE(v720.OnOrAfter(v730));

    Version v_last_lucene(Version::FromBits(255, 255, 255));
    EXPECT_TRUE(v_last_lucene.OnOrAfter(v730));
    EXPECT_FALSE(v730.OnOrAfter(v_last_lucene));
  }

  {
    // Version::ParseLeniently test

    EXPECT_EQ(Version::LATEST, Version::ParseLeniently("LATEST"));
    EXPECT_EQ(Version::LATEST, Version::ParseLeniently("LUCENE_CURRENT"));
    EXPECT_EQ(Version::LATEST, Version::ParseLeniently("LUCENE_7_3_0"));
    EXPECT_EQ(Version::LATEST, Version::ParseLeniently("LUCENE_7_3"));
    EXPECT_EQ(Version::LATEST, Version::ParseLeniently("LUCENE_73"));

    try {
      Version::ParseLeniently("aaa.bbb.ccc.ddd");
      FAIL();
    } catch(...) {
    }
  }
}

TEST(ETC__TESTS, CRC32__TESTS) {
  Crc32 crc32;
  std::string str1("CRC32 is from zlib which is an amazing library!!");
  crc32.Update(str1.c_str(), 0, str1.size());
  EXPECT_EQ(4079133648L/*From Java*/, crc32.GetValue());
  std::string str2("I'm very tired today. After this test "
                   "I'm going to bath to relax myself");
  crc32.Update(str2.c_str(), 0, str2.size());
  EXPECT_EQ(785957158L/*From Java*/, crc32.GetValue());
  crc32.Reset();
  EXPECT_EQ(0, crc32.GetValue());
}

TEST(ETC__TESTS, CRC32__TESTS1) {
  Crc32 crc32;

  char b = 0;
  for (int i = 0 ; i < 100000 ; ++i) {
    crc32.Update(b++);
  }

  // Crc32 value got from Jdk8
  EXPECT_EQ(2865713097, crc32.GetValue());

  crc32.Reset();
  char buf[100000];
  b = 0;
  for (int i = 0 ; i < 100000 ; ++i) {
    buf[i] = b++;
  }

  crc32.Update(buf, 0, 100000);
  EXPECT_EQ(2865713097, crc32.GetValue());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
