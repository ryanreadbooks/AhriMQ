#include "ahrimq/base/str_utils.h"

#include <gtest/gtest.h>

TEST(StrUtilsTest, StrEqualTest) {
  struct suite {
    std::string s1;
    std::string s2;
    size_t end;
    bool expect;
  };

  auto cases = std::vector<suite>{
      {"hello", "hell", 1, true},
      {"hello", "hell", 2, true},
      {"hello", "hell", 3, true},
      {"hello", "hell", 4, true},
      {"abc", "we", 0, false},
      {"abc", "abc", 0, false},
      {"a", "ab", 1, true},
      {"a", "ab", 2, false},
      {"", "", 1, false},
      {"qwer", "asdv", 1, false},
      {"qwer", "asdv", 2, false},
      {"qwer", "asdv", 3, false},
      {"qwer", "asdv", 4, false},
  };

  for (const auto& c : cases) {
    EXPECT_EQ(ahrimq::StrEqual(c.s1, c.s2, c.end), c.expect);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}