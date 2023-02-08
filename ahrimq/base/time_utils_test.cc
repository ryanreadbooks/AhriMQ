#include "time_utils.h"

#include <gtest/gtest.h>

#include <iostream>
#include <vector>

using namespace ahrimq;

TEST(TimeUtilsTest, DurationTest) {
  struct testcase {
    int type;  // 0-second; 1-minute; 2-hour
    int64_t m;
    int64_t expect;
  };

  std::vector<testcase> cases{
      {0, 1, 1},          {0, 60, 60},      {1, 1, 1 * 60},   {1, 6, 6 * 60},
      {1, 162, 162 * 60}, {1, 0, 0 * 60},   {1, 15, 15 * 60}, {2, 61, 61 * 3600},
      {2, 1, 1 * 3600},   {2, 2, 2 * 3600}, {2, 7, 7 * 3600}, {2, 0, 0 * 3600},
  };

  for (auto& c : cases) {
    if (c.type == 0) {
      time::Second s = time::Second(c.m);
      time::Duration& d = s;
      EXPECT_EQ(d.Count(), c.expect);
    } else if (c.type == 1) {
      time::Minute m = time::Minute(c.m);
      time::Duration& d = m;
      EXPECT_EQ(d.Count(), c.expect);
    } else if (c.type == 2) {
      time::Hour h = time::Hour(c.m);
      time::Duration& d = h;
      EXPECT_EQ(d.Count(), c.expect);
    }
  }
}

TEST(TimeUtilsTest, BasicTest) {
  time::UTCTimePoint now = time::UTCTimePoint::Now();
  std::cout << now.ToFormatString() << '\n';
  std::cout << time::GMTTimeNowString() << '\n';
  now.Add(time::Minute(3));
  std::cout << now.ToFormatString() << '\n';
  sleep(1);
  now = time::UTCTimePoint::Now();
  std::cout << now.ToFormatString() << '\n';
  now.Sub(time::Hour(24));
  std::cout << now.ToFormatString() << '\n';
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}