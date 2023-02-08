#include "ahrimq/net/http/http_header.h"

#include <gtest/gtest.h>

TEST(HTTPHeaderTest, CaseInsensitiveTest) {
  ahrimq::http::HTTPHeader header;
  header.Add("Content-Type", "text/html");
  header.Add("cookie", "name=ryan; age=12");
  header.Add("content-leNGth", "100");
  EXPECT_TRUE(header.Has("content-type"));
  EXPECT_TRUE(header.Has("content-length"));
  EXPECT_TRUE(header.Has("cOokIe"));
  header.Add("content-TYPE", "application/json");
  EXPECT_STREQ(header.Values("conteNT-TYpe")[0].c_str(), "text/html");
  EXPECT_STREQ(header.Values("CONTENT-type")[1].c_str(), "application/json");
  EXPECT_EQ(header.Size(), 3);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}