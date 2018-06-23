#include <cstring>
#include <vector>
#include <Analysis/CharacterUtil.h>
#include <gtest/gtest.h>

using namespace lucene::core::analysis::characterutil;

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
    const char* cstr1 = "Doochi! Nice meet you!";
    const char* cstr2 = "Doochi! Nice meet u!";
    const char* cstr3 = "doochi! nice meet you!";
    CharPtrRangeInfo info1(cstr1, 0, std::strlen(cstr1));
    CharPtrRangeInfo info2(cstr1, 0, std::strlen(cstr1));
    CharPtrRangeInfoEqual equal1(false);

    // Same str, range
    EXPECT_TRUE(equal1(info1, info2));

    CharPtrRangeInfo info3(cstr2, 0, std::strlen(cstr2));
    // Lower case
    EXPECT_FALSE(equal1(info1, info3));

    // Same str, range
    CharPtrRangeInfo info4(cstr1, 0, 7);
    CharPtrRangeInfo info5(cstr1, 0, 7);
    EXPECT_TRUE(equal1(info4, info5));

    // Ignore lower case. Same
    CharPtrRangeInfo info6(cstr3, 0, std::strlen(cstr3));
    CharPtrRangeInfoEqual equal2(true);
    EXPECT_TRUE(equal2(info1, info6));

    // Hashing test
    CharPtrRangeInfoHasher hasher1(false);
    EXPECT_EQ(hasher1(info1), hasher1(info2));
    EXPECT_NE(hasher1(info1), hasher1(info3));

    CharPtrRangeInfoHasher hasher2(true);
    EXPECT_EQ(hasher2(info1), hasher2(info6));
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
