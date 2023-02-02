#include "net/http/http_server.h"
#include "buffer/buffer.h"

using namespace ahrimq;

std::string handler1(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  res.MakeContentPlainText("Greetings from http server!!\n");
  res.SetStatus(http::StatusOK);
  return "";
}

std::string handler2(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  std::string name = params.Get("username");
  std::string greetings = "Welcome, " + name;
  res.MakeContentPlainText(greetings);
  res.SetStatus(http::StatusOK);
  return "";
}

std::string handler3(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  if (!req.QueryEmpty()) {
    std::string loc = req.Query().Get("location");
    if (loc == "httpbin") {
      loc = "http://httpbin.org/get";
    }
    res.RedirectTo(loc, ahrimq::http::StatusTemporaryRedirect);
  } else {
    res.MakeContentPlainText("Welcome, no redirect!!\n");
    res.SetStatus(http::StatusOK);
  }

  return "";
}

std::string handler4(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  // get post form
  if (!req.FormEmpty()) {
    auto form = req.Form();
    // get all form
    auto keys = form.Keys();
    std::stringstream ss;
    for (auto&& k : keys) {
      ss << k << ": ";
      auto vs = form.Values(k);
      if (vs.size() != 1) {
        ss << '[';
        for (size_t i = 0; i < vs.size(); i++) {
          ss << vs[i];
          if (i == vs.size() - 1) {
            ss << "]\n";
          } else {
            ss << ", ";
          }
        }
      } else {
        ss << vs[0] << '\n';
      }
    }
    res.MakeContentPlainText(ss.str());
  } else {
    res.MakeContentPlainText("Form is empty\n");
  }
  res.SetStatus(http::StatusOK);
  return "";
}

std::string handler5(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  // get post form
  if (!req.QueryEmpty()) {
    auto query = req.Query();
    std::string name = query.Get("name");
    std::string age = query.Get("age");
    std::string result = "name = " + name + ", age = " + age + "\n";
    std::cout << query << std::endl;
    res.MakeContentPlainText(result);
  } else {
    res.MakeContentPlainText("Query is empty\n");
  }
  res.SetStatus(http::StatusOK);
  return "";
}

std::string handler6(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  res.SetStatus(http::StatusOK);
  return "index.html";
}

int main(int argc, char** argv) {
  ahrimq::http::HTTPServerConfig config;
  config.port = 9527;
  config.root = "/home/ryan/codes/AhriMQ/examples/";

  ahrimq::http::HTTPServer server(config);
  bool r = server.Get("/get", handler1);
  r = server.Get("/get/{username}", handler2);
  r = server.Get("/redirect", handler3);
  r = server.Post("/post", handler4);
  r = server.Get("/query", handler5);
  r = server.Get("/index", handler6);

  server.Run();

  return 0;
}