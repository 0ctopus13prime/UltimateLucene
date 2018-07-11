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

#include <Analysis/CharacterUtil.h>
#include <gtest/gtest.h>
#include <Util/ArrayUtil.h>
#include <cstring>
#include <iostream>
#include <set>
#include <vector>

using lucene::core::analysis::characterutil::CharMap;
using lucene::core::analysis::characterutil::CharPtrRangeInfo;
using lucene::core::analysis::characterutil::CharPtrRangeInfoEqual;
using lucene::core::analysis::characterutil::CharPtrRangeInfoHasher;
using lucene::core::analysis::characterutil::CharSet;
using lucene::core::analysis::characterutil::IsPrefix;
using lucene::core::analysis::characterutil::Split;
using lucene::core::analysis::characterutil::SplitRegex;
using lucene::core::analysis::characterutil::ToLowerCase;
using lucene::core::analysis::characterutil::Trim;

TEST(CHARACTER__UTILS, FUNCTIONS) {
  {
    char buf[] = "AbCdEf";
    ToLowerCase(buf, 0, sizeof(buf));
    EXPECT_STREQ("abcdef", buf);
  }

  {
    // <space>abcdefg<space><space><tab><newline>
    std::string str = " abcdefg   \
";
    Trim(str);
    EXPECT_EQ("abcdefg", str);
  }

  {
    std::string prefix = "I got a, got a";
    std::string str = prefix + " pocketful of sunshine";
    EXPECT_TRUE(IsPrefix(str, prefix));

    prefix += " Doochi";
    EXPECT_FALSE(IsPrefix(str, prefix));
  }

  {
    //                   0     1    2   3   4   5      6     7   8   9    10   11
    std::string str = "Doochi, how and why did this project even no body pays you?";
    std::vector<std::string> tokens = Split(str, " ");
    EXPECT_EQ(12, tokens.size());

    tokens = Split(str, " ", 5);
    EXPECT_EQ(5, tokens.size());

    tokens = Split(str, " ", 10000);
    EXPECT_EQ(12, tokens.size());

    str = "|a|b|c|";
    tokens = Split(str, "|");
    EXPECT_EQ(3, tokens.size());

    str = "aaa";
    tokens = Split(str, "a");
    EXPECT_EQ(0, tokens.size());
  }

  {
    //                 vvv vv   vvvvvvv
    std::string str = "abcaabbacabababcba";
    std::vector<std::string> tokens = SplitRegex(str, "abc?");
    EXPECT_EQ(3, tokens.size());
    EXPECT_EQ("a", tokens[0]);
    EXPECT_EQ("bac", tokens[1]);
    EXPECT_EQ("ba", tokens[2]);

    tokens = SplitRegex(str, "abc?", 2);
    EXPECT_EQ(2, tokens.size());
  }

  {
    const char* upper_case_cstr = "Doochi! Nice meet you!";
    const char* lower_case_cstr = "doochi! nice meet you!";
    const char* different_cstr = "Doochi! Nice meet u!";
    CharPtrRangeInfo upper_case_info(upper_case_cstr,
                                     0,
                                     std::strlen(upper_case_cstr));
    CharPtrRangeInfo upper_case_info1(upper_case_cstr,
                                      0,
                                      std::strlen(upper_case_cstr));
    CharPtrRangeInfoEqual care_care_equal(false);

    // Same str, range
    EXPECT_TRUE(care_care_equal(upper_case_info, upper_case_info1));

    CharPtrRangeInfo lower_case_info(lower_case_cstr,
                                     0,
                                     std::strlen(lower_case_cstr));
    // Lower case
    EXPECT_FALSE(care_care_equal(upper_case_info, lower_case_info));

    // Same str, range
    CharPtrRangeInfo upper_case_info2(upper_case_cstr, 0, 7);
    CharPtrRangeInfo upper_case_info3(upper_case_cstr, 0, 7);
    EXPECT_TRUE(care_care_equal(upper_case_info2, upper_case_info3));

    // Ignore lower case. Same
    CharPtrRangeInfoEqual dont_care_equal(true);
    EXPECT_TRUE(dont_care_equal(upper_case_info, lower_case_info));

    // Hashing test
    CharPtrRangeInfoHasher care_case_hasher(false);
    CharPtrRangeInfo different_info(different_cstr,
                                    0,
                                    std::strlen(different_cstr));
    EXPECT_EQ(care_case_hasher(upper_case_info),
              care_case_hasher(upper_case_info1));
    EXPECT_NE(care_case_hasher(upper_case_info),
              care_case_hasher(different_info));

    std::string str1("Doochi! Nice meet you!");
    CharPtrRangeInfo
    tmp_upper_case_info(lucene::core::util::arrayutil::CopyOfRange(str1.c_str(),
                                                                   0,
                                                                   str1.size()),
                        0,
                        str1.size());
    CharPtrRangeInfo tmp_upper_case_info1(str1.c_str(), 0, str1.size());
    EXPECT_EQ(care_case_hasher(tmp_upper_case_info),
              care_case_hasher(tmp_upper_case_info1));
    EXPECT_TRUE(care_care_equal(tmp_upper_case_info, tmp_upper_case_info1));

    CharPtrRangeInfoHasher dont_care_case_hasher(true);
    EXPECT_EQ(dont_care_case_hasher(upper_case_info),
              dont_care_case_hasher(lower_case_info));
  }
}

TEST(CHARACTER__UTILS, CHAR__MAP__TESTS) {
  std::string upper_case_str = "Doochi! I love you!";
  std::string lower_case_str = "doochi! i love you!";
  std::string different_str = "Doochi! Don't let me down";

  {
    // For ordinary string
    CharMap<int> char_map;
    char_map.Put(upper_case_str, 13);
    EXPECT_EQ(1, char_map.Size());
    EXPECT_TRUE(char_map.ContainsKey(upper_case_str));
    EXPECT_EQ(13, char_map.Get(upper_case_str)->second);
    EXPECT_FALSE(char_map.ContainsKey(lower_case_str));
    EXPECT_FALSE(char_map.ContainsKey(different_str));

    auto it = char_map.Begin();
    EXPECT_EQ(upper_case_str,
              std::string(it->first.str, it->first.offset, it->first.length));
    it++;
    EXPECT_EQ(char_map.End(), it);

    char_map.Put(different_str, 1313);
    EXPECT_EQ(2, char_map.Size());
    EXPECT_TRUE(char_map.ContainsKey(different_str));

    // Because CharMap has an unordinary_map inside,
    // insertion order is not guaranteed.
    std::set<std::string> check_set;
    for (auto it = char_map.Begin() ; it != char_map.End() ; ++it) {
      check_set.insert(std::string(it->first.str,
                                   it->first.offset,
                                   it->first.length));
    }

    EXPECT_EQ(2, check_set.size());
    EXPECT_EQ(char_map.End(), it);

    EXPECT_TRUE(char_map.Erase(upper_case_str));
    EXPECT_TRUE(char_map.Erase(different_str));
    EXPECT_FALSE(char_map.Erase(lower_case_str));
    EXPECT_EQ(0, char_map.Size());
    char_map.Erase(upper_case_str);
    char_map.Erase(different_str);

    char_map.Clear();
    EXPECT_EQ(0, char_map.Size());
  }

  {
    // For ordinary string but not case sensitive
    CharMap<int> char_map(true);
    char_map.Put(upper_case_str, 13);
    char_map.Put(lower_case_str, 1313);
    EXPECT_EQ(1, char_map.Size());
    EXPECT_TRUE(char_map.ContainsKey(upper_case_str));
    EXPECT_TRUE(char_map.ContainsKey(lower_case_str));
    EXPECT_FALSE(char_map.ContainsKey(different_str));
    EXPECT_EQ(13, char_map.Get(lower_case_str)->second);
    EXPECT_TRUE(char_map.Erase(upper_case_str));
    EXPECT_EQ(0, char_map.Size());
  }
}

TEST(CHARACTER__UTILS, CHAR__SET__TESTS) {
  std::string uppercase_str = "Doochi! Kantata!";
  std::string lowercase_str = "doochi! kantata!";
  std::string different_str = "dadatata da tadadada!!";

  {
    CharSet carecase_set(false);

    carecase_set.Add(uppercase_str);
    EXPECT_EQ(1, carecase_set.Size());
    EXPECT_TRUE(carecase_set.Contains(uppercase_str));
    EXPECT_FALSE(carecase_set.Contains(lowercase_str));
    EXPECT_TRUE(carecase_set.Contains(uppercase_str.c_str(),
                                      0,
                                      uppercase_str.size()));
    EXPECT_FALSE(carecase_set.Contains(lowercase_str.c_str(),
                                       0,
                                       lowercase_str.size()));
    EXPECT_FALSE(carecase_set.Contains(different_str));

    carecase_set.Clear();
    EXPECT_EQ(0, carecase_set.Size());

    CharSet dont_carecase_set(true);
    dont_carecase_set.Add(uppercase_str);
    EXPECT_EQ(1, dont_carecase_set.Size());
    EXPECT_TRUE(dont_carecase_set.Contains(uppercase_str));
    EXPECT_TRUE(dont_carecase_set.Contains(lowercase_str));
    EXPECT_TRUE(dont_carecase_set.Contains(uppercase_str.c_str(),
                                           0,
                                           uppercase_str.size()));
    EXPECT_TRUE(dont_carecase_set.Contains(lowercase_str.c_str(),
                                           0,
                                           lowercase_str.size()));
    EXPECT_FALSE(dont_carecase_set.Contains(different_str));

    dont_carecase_set.Add(lowercase_str);
    EXPECT_EQ(1, dont_carecase_set.Size());
    EXPECT_TRUE(dont_carecase_set.Contains(uppercase_str));
    EXPECT_TRUE(dont_carecase_set.Contains(lowercase_str));
    EXPECT_TRUE(dont_carecase_set.Contains(uppercase_str.c_str(),
                                           0,
                                           uppercase_str.size()));
    EXPECT_TRUE(dont_carecase_set.Contains(lowercase_str.c_str(),
                                           0,
                                           lowercase_str.size()));
  }

  // Initialization
  {
    std::vector<std::string> list {uppercase_str, lowercase_str, different_str};
    CharSet carecase_set(list, false);
    EXPECT_EQ(3, carecase_set.Size());
    EXPECT_TRUE(carecase_set.Contains(uppercase_str));
    EXPECT_TRUE(carecase_set.Contains(lowercase_str));
    EXPECT_TRUE(carecase_set.Contains(different_str));

    CharSet dont_carecase_set(list, true);
    EXPECT_EQ(2, dont_carecase_set.Size());
    EXPECT_TRUE(dont_carecase_set.Contains(uppercase_str));
    EXPECT_TRUE(dont_carecase_set.Contains(lowercase_str));
    EXPECT_TRUE(dont_carecase_set.Contains(different_str));
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
