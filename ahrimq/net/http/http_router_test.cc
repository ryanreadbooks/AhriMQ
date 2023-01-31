#include "ahrimq/net/http/http_router.h"

#include <gtest/gtest.h>

ahrimq::http::detail::RouteNode::Params fake_params;

template <typename T>
void display(T have, T expect) {
  std::cout << "have: " << have << ", expect: " << expect << std::endl;
}

TEST(HTTPRouterTest, LongestCommonPrefixTest) {
  struct testLCPStruct {
    std::string s1;
    std::string s2;
    int expect;
  };

  auto doTest = [=](const std::vector<testLCPStruct>& cases) {
    for (auto& example : cases) {
      EXPECT_EQ(ahrimq::http::detail::LongestCommonPrefix(example.s1, example.s2),
                example.expect);
    }
  };

  auto cases = std::vector<testLCPStruct>{
      {"", "", -1},       {"hello", "world", -1},  {"abc", "a", 0},
      {"abc", "ab", 1},   {"abcdef", "abcdef", 5}, {"abcd", "abc", 2},
      {"ab", "abcde", 1}, {"", "hokwe", -1},       {"weo", "", -1}};

  doTest(cases);
}

TEST(HTTPRouterTest, FindParamBracketTest) {
  struct testPBStruct {
    std::string path;
    std::string outname;
    int expect;
  };

  auto doTest = [=](const std::vector<testPBStruct>& cases) {
    for (const auto& example : cases) {
      std::string tmp;
      int real = ahrimq::http::detail::FindParamBracket(example.path, tmp);
      EXPECT_EQ(real, example.expect);
      EXPECT_STREQ(example.outname.c_str(), tmp.c_str());
    }
  };

  auto cases = std::vector<testPBStruct>{{"/a/b/c", "", -1},
                                         {"/a/{name}/cs", "name", 3},
                                         {"/test/home_{address}/xxx", "address", 11},
                                         {"/a/b{{sd}", "", -2},
                                         {"/a/b/{{ae", "", -2},
                                         {"/a/b/c{name}}", "", -2},
                                         {"/a/b/c/d/{name}xx/", "", -2},
                                         {"/a/b/c/d/name}}", "", -1},
                                         {"/a/{b}/{c}", "b", 3},
                                         {"/{name}/b/c", "name", 1},
                                         {"/{a}/{b}/{c}/d", "a", 1},
                                         {"/a/b/c/d/{}/e", "", -2}};

  doTest(cases);
}

TEST(HTTPRouterTest, CheckWildcardTest) {
  struct cwtStruct {
    std::string s;
    std::string outname;
    int expect;
  };
  auto cases = std::vector<cwtStruct>{{"{name}", "name", 0},
                                      {"{abi}", "abi", 0},
                                      {"{}", "", -2},
                                      {"adwerad{adc}", "adc", 7},
                                      {"abi{info}", "info", 3},
                                      {"abi{info}xxx", "", -2},
                                      {"abc{info", "", -2},
                                      {"abc{info}}", "", -2},
                                      {"{}}", "", -2},
                                      {"{adc}xdwe", "", -2},
                                      {"hello", "", -1},
                                      {"swd", "", -1},
                                      {"", "", -1}};

  for (const auto& c : cases) {
    std::string tmp;
    int have = ahrimq::http::detail::CheckWildcard(c.s, tmp);
    if (tmp != c.outname || have != c.expect) {
      ADD_FAILURE() << "have: " << tmp << ", expect: " << c.outname
                    << ". real = " << have << ", expect = " << c.expect << std::endl;
    }
  }
}

TEST(HTTPRouterTest, RouteNodeInsertTest) {
  ahrimq::http::detail::RouteNode root;

  struct testRNIStruct {
    std::string url;
    bool expect;
  };

  auto cases = std::vector<testRNIStruct>{{"/hello/api/v1/kind", true},
                                          {"/hello/v3/user", true},
                                          {"/person/age/name", true},
                                          {"/hello/api/v2/ok", true},
                                          {"/hello/api", true},
                                          {"/hello/v3/user", false},
                                          {"/", true},
                                          {"/", false}};

  auto doTest = [&](const std::vector<testRNIStruct>& cases) {
    for (const auto& example : cases) {
      EXPECT_EQ(example.expect, root.InsertRoute(example.url, nullptr));
    }
  };

  doTest(cases);
}

TEST(HTTPRouterTest, RouteNodeSearchRawTest) {
  ahrimq::http::detail::RouteNode root;

  std::vector<std::string> insert_cases = {"/hello/api/v1/kind", "/hello/v3/user",
                                           "/person/age/name",   "/hello/api/v2/ok",
                                           "/hello/api",         "/",
                                           "/well/known/info",   "/你好/世界"};

  for (auto& route : insert_cases) {
    root.InsertRoute(route, nullptr);
  }
  auto node = root.SearchRoute("/hello/api/v2/ok", fake_params);

  struct testRNSStruct {
    std::string route;
    std::string expect_segment;
    bool expect_null;
  };

  auto cases = std::vector<testRNSStruct>{{"/hello/api/v2/ok", "ok", false},
                                          {"/hello/v3/user", "user", false},
                                          {"/hello/api/v1/kind", "kind", false},
                                          {"/person/age/name", "name", false},
                                          {"/hello/api", "api", false},
                                          {"/", "/", false},
                                          {"/well/known/info", "info", false},
                                          {"/a/b/c", "c", true},
                                          {"/", "/", false},
                                          {"/abc/edf/we", "we", true},
                                          {"/abc/{hello/name", "name", true},
                                          {"/你好/世界", "世界", false},
                                          {"/home/page/codes", "代码", true}};

  auto doTest = [&](const std::vector<testRNSStruct>& cases) {
    for (const auto& c : cases) {
      auto ans = root.SearchRoute(c.route, fake_params);
      EXPECT_EQ(c.expect_null, ans == nullptr);
      if (ans) {
        EXPECT_STREQ(c.expect_segment.c_str(), ans->Segment().c_str());
      }
    }
  };

  doTest(cases);
}

TEST(HTTPRouterTest, RouteNodeSearchWithWildcardTest) {
  ahrimq::http::detail::RouteNode root;
  auto gen_fn = [](std::string arg) -> ahrimq::http::HTTPCallback {
    return [arg](const ahrimq::http::HTTPRequest&, ahrimq::http::HTTPResponse&,
                 const ahrimq::http::URLParams&) -> std::string { return arg; };
  };

  struct insert_pack {
    std::string url;
    ahrimq::http::HTTPCallback cb;
  };
  // insert some routes with wildcards
  auto inserts =
      std::vector<insert_pack>{{"/", gen_fn("index")},
                               {"/home/{name}", gen_fn("/home/name")},
                               {"/r/image/{id}", gen_fn("/r/image/id")},
                               {"/r/music/user_{uid}", gen_fn("/r/music/user_uid")},
                               {"/r/music", gen_fn("/r/music")}};

  for (const auto& i : inserts) {
    bool have = root.InsertRoute(i.url, i.cb);
    if (!have) {
      ADD_FAILURE() << "adding " << i.url << " failed\n";
    }
  }

  struct request_pack {
    std::string request_url;
    bool should_found;
    std::string param_key;
    std::string param_value;
    std::string cb_return;
  };

  auto search_cases = std::vector<request_pack>{
      {"/", true, "", "", "index"},
      {"/home/rita", true, "name", "rita", "/home/name"},
      {"/home/valen", true, "name", "valen", "/home/name"},
      {"/hello/world", false, "", "", ""},
      {"/home", false, "", "", ""},
      {"/home/", false, "", "", ""},
      {"/home/lika/page", false, "", "", ""},
      {"/a/c/d", false, "", "", ""},
      {"/r/image/123", true, "id", "123", "/r/image/id"},
      {"/r/image/1234", true, "id", "1234", "/r/image/id"},
      {"/r/image", false, "", "", ""},
      {"/r/image/100/eoo/ds_we", false, "", "", ""},
      {"/r/music", true, "", "", "/r/music"},
      {"/r/musicsd", false, "", "", ""},
      {"/r/music/user_100", true, "uid", "100", "/r/music/user_uid"},
      {"/r/music/user_aEWKdxc-+", true, "uid", "aEWKdxc-+", "/r/music/user_uid"},
      {"/r/music/mkie", false, "", "", ""},
      {"/r/music/user_", false, "", "", ""}};

  ahrimq::http::HTTPRequest req(nullptr);
  ahrimq::http::HTTPResponse res(nullptr);

  for (const auto& s : search_cases) {
    ahrimq::http::URLParams params;
    auto node = root.SearchRoute(s.request_url, params);
    if (s.should_found) {
      if (node == nullptr || node->Handler() == nullptr) {
        ADD_FAILURE() << "can not found for " << s.request_url << '\n';
        continue;
      }
    } else {
      if (node != nullptr && node->Handler() != nullptr) {
        ADD_FAILURE() << s.request_url << " should not be found\n";
        continue;
      }
    }

    if (node == nullptr || node->Handler() == nullptr) {
      std::cout << s.request_url << " node == nullptr, skip\n";
      continue;
    }
    if (params.Get(s.param_key) != s.param_value) {
      std::stringstream ss;
      ss << s.request_url << " params wrong: ";
      auto ks = params.Keys();
      auto vs = params.Values();
      for (size_t i = 0; i < ks.size(); i++) {
        ss << ks[i] << "=" << vs[i] << ",";
      }
      ADD_FAILURE() << ss.str() << '\n';
    }
    auto have_cb_return = node->Handler()(req, res, params);
    if (have_cb_return != s.cb_return) {
      ADD_FAILURE() << "cb return have: " << have_cb_return
                    << ", expect: " << s.cb_return << std::endl;
    }
  }
}

TEST(HTTPRouterTest, RouteNodeInsertWithWildcardTest) {
  ahrimq::http::detail::RouteNode root;

  struct testRNIStruct {
    std::string url;
    std::string wildcard_name;
    bool expect;
  };

  auto cases = std::vector<testRNIStruct>{{"/api/{version}/name", "version", true},
                                          {"/api/v1/age", "", false},
                                          {"/api/v2/people/name", "", false},
                                          {"/api/hello{acb}/item", "acb", false},

                                          {"/a/b/c", "", true},
                                          {"/a/{pa}/c", "pa", false},
                                          {"/a/b/c/d/e", "", true},
                                          {"/a/b/{we}", "we", false},

                                          {"/hello/world/lice_{age}", "age", true},
                                          {"/hello/world", "", true},
                                          {"/hello", "", true},
                                          {"/hello/world/li", "", true},
                                          {"/hello/world/lice", "", true},
                                          {"/hello/world/lice_", "", true},
                                          {"/hello/world/lice_a", "", false},
                                          {"/hello/tickle/lic_{info}", "info", true}

  };

  auto doTest = [&](std::vector<testRNIStruct>& cases) {
    for (const auto& example : cases) {
      bool have = root.InsertRoute(example.url, nullptr);
      if (have != example.expect) {
        ADD_FAILURE() << "have: " << have << ", expect: " << example.expect
                      << std::endl;
      }
    }
  };

  doTest(cases);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}