#include "buffer.h"

#include <iostream>

#include <gtest/gtest.h>

TEST(BufferTest, CopyTest) {
  ahrimq::Buffer buf;
  buf.Append("helloworld!!");
  std::vector<char> copy;
  int n = buf.ReadableBytes();
  copy.reserve(n);
  copy.insert(copy.begin(), buf.BeginRead(), buf.BeginRead() + n);
  std::string s(copy.begin(), copy.end());
  ASSERT_STREQ(s.c_str(), "helloworld!!");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}