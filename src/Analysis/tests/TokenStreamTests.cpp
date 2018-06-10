#include <gtest/gtest.h>

TEST(StrCompare, CStrEqual) {
  const char* expectVal      = "hello gtest";
  const char* actualValTrue  = "hello gtest";

  EXPECT_STREQ(expectVal, actualValTrue);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
