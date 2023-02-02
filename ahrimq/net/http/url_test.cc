#include "net/http/url.h"

#include <gtest/gtest.h>

using namespace ahrimq::http;
using namespace ahrimq;

TEST(URLTest, URLEscapeUnEscapeTest) {
  std::string out;
  std::vector<std::string> cases = {
      "96你好世界%7@ADSew+PñÑ!q3zd-_.!~*\\():@&=+$,;[]",
      "αβγ吃了吗?？！+-/*adf0.+asdfwae49WEFDF8909sd98'\"]////.,   we!@#$%^&*()_+`~",
      "0.123456789abcdefghijkl“”。，、‘；【】、|"
      "mnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ|}{:\\<>,.?/';* ?WE",
      "مرحباً مع سّلامةإسمحْ لي أن أقدم بنفسيsdweأين تَسكن؟ vscode是一个很好用的IDE!!",
      "существительное, прилагательное  ][\\\"\"предлог",
      "✔😁阿😎🤣😆🧐👮‍♂️🧵🍉🛁"};

  for (const auto& c : cases) {
    URL::Escape(c, out);
    std::string dec;
    URL::UnEscape(out, dec);
    ASSERT_STREQ(c.c_str(), dec.c_str());
  }
}

TEST(URLTest, URLUnEscapeTest) {
  std::string out;

  struct pack {
    std::string raw;
    std::string expect;
  };
  std::vector<pack> cases = {
      {"name=paper%26clip&age=100+%26%2Bwe", "name=paper&clip&age=100 &+we"},
      {"http://www.xad.com/get?name=helloworld&age=100&are=are%20you%20ok?", "http://www.xad.com/get?name=helloworld&age=100&are=are you ok?"}
  };
  for (const auto& c : cases) {
    URL::UnEscape(c.raw, out);
    ASSERT_STREQ(c.expect.c_str(), out.c_str());
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}