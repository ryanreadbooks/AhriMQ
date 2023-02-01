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
      {"ab", "abcde", 1}, {"", "hokwe", -1},       {"weo", "", -1},
  };

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

  auto cases = std::vector<testPBStruct>{
      {"/a/b/c", "", -1},
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
      {"/a/b/c/d/{}/e", "", -2},
  };

  doTest(cases);
}

TEST(HTTPRouterTest, CheckWildcardTest) {
  struct cwtStruct {
    std::string s;
    std::string outname;
    int expect;
  };
  auto cases = std::vector<cwtStruct>{
      {"{name}", "name", 0},
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
      {"", "", -1},
  };

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

  auto cases = std::vector<testRNIStruct>{
      {"/hello/api/v1/kind", true},
      {"/hello/v3/user", true},
      {"/person/age/name", true},
      {"/hello/api/v2/ok", true},
      {"/hello/api", true},
      {"/hello/v3/user", false},
      {"/", true},
      {"/", false},
  };

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

  auto cases = std::vector<testRNSStruct>{
      {"/hello/api/v2/ok", "ok", false},
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
      {"/home/page/codes", "代码", true},
  };

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
  auto inserts = std::vector<insert_pack>{
      {"/", gen_fn("index")},
      {"/home/{name}", gen_fn("/home/name")},
      {"/r/image/{id}", gen_fn("/r/image/id")},
      {"/r/music/user_{uid}", gen_fn("/r/music/user_uid")},
      {"/r/music", gen_fn("/r/music")},
      {"/user_{name}", gen_fn("/user_name")},
      {"/user_{name}/about", gen_fn("/user_name/about")},
  };

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
      {"/r/music/user_", false, "", "", ""},
      {"/user_ryan", true, "name", "ryan", "/user_name"},
      {"/user_ryan/about", true, "name", "ryan", "/user_name/about"},
  };

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

TEST(HTTPRouterTest, RouteNodeMultipleWildcardTest) {
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
  auto inserts = std::vector<insert_pack>{
      {"/home/{name}/{id}", gen_fn("/home/name/id")},
      {"/home/{name}/{id}/about", gen_fn("/home/name/id/about")},
      {"/search/{query}/project/{item}", gen_fn("/search/query/project/item")},
      {"/image/{kk}/type", gen_fn("/image/kk/type")},
      {"/music/user_{name}/page/{id}", gen_fn("/music/user_name/page/id")},
      {"/data/lib{id}", gen_fn("/data/libid")},
  };

  for (const auto& i : inserts) {
    bool have = root.InsertRoute(i.url, i.cb);
    if (!have) {
      FAIL() << "adding " << i.url << " failed\n";
    }
  }

  // search
  struct request_pack {
    std::string request_url;
    bool should_found;
    ahrimq::http::URLParams params;
    std::string cb_return;
  };
  using namespace ahrimq::http;
  URLParams empty;
  auto requests = std::vector<request_pack>{
      {"/page/name", false, empty, ""},
      {"/number/qer/ewpoja", false, empty, ""},
      {"/home/ryan/100", true, URLParams{{{"name", "ryan"}, {"id", "100"}}},
       "/home/name/id"},
      {"/home/valen/200", true, URLParams{{{"name", "valen"}, {"id", "200"}}},
       "/home/name/id"},
      {"/home/lily", false, empty, ""},
      {"/home/ryan/100/about", true, URLParams{{{"name", "ryan"}, {"id", "100"}}},
       "/home/name/id/about"},
      {"/search/city/project", false, empty, ""},
      {"/search/city/project/building", true,
       URLParams{{{"query", "city"}, {"item", "building"}}},
       "/search/query/project/item"},
      {"/image/qwe/wer/gsd", false, empty, ""},
      {"/image/abc/type", true, URLParams{{{"kk", "abc"}}}, "/image/kk/type"},
      {"/music/username/page/id", false, empty, ""},
      {"/music/user_ryan/page/100", true,
       URLParams{{{"name", "ryan"}, {"id", "100"}}}, "/music/user_name/page/id"},
      {"/music/user_li/page", false, empty, ""},
      {"/music/user_/page/23o", false, empty, ""},
      {"/data/liba.so", true, URLParams{{{"id", "a.so"}}}, "/data/libid"},
      {"/data/liba.so/name", false, empty, ""},
      {"/data/lic.so", false, empty, ""},
      {"/data/libimage", true, URLParams{{{"id", "image"}}}, "/data/libid"}
  };

  ahrimq::http::HTTPRequest req(nullptr);
  ahrimq::http::HTTPResponse res(nullptr);

  for (const auto& s : requests) {
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
      continue;
    }
    if (params != s.params) {
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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}