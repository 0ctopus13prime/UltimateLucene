#include <string>
#include <Util/Etc.h>
#include <gtest/gtest.h>

using namespace lucene::core::util::etc;

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

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
