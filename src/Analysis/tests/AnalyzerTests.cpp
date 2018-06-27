#include <memory>
#include <string>
#include <Analysis/TokenStream.h>
#include <Analysis/Attribute.h>
#include <Analysis/Analyzer.h>
#include <gtest/gtest.h>

using namespace lucene::core::analysis;
using namespace lucene::core::analysis::tokenattributes;

TEST(ANALYZER__TESTS, StringTokenStream) {
  std::string value("StringTokenStream is a cool stream");
  StringTokenStream sts(*TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY, value, value.size());
  std::shared_ptr<CharTermAttribute> char_term_attr = sts.AddAttribute<CharTermAttribute>();
  std::shared_ptr<OffsetAttribute> offset_attr = sts.AddAttribute<OffsetAttribute>();

  sts.Reset();
  sts.IncrementToken();

  EXPECT_EQ(value, std::string(char_term_attr->Buffer(), char_term_attr->Length()));
  EXPECT_EQ(0, offset_attr->StartOffset());
  EXPECT_EQ(value.size(), offset_attr->EndOffset());

  sts.End();
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
