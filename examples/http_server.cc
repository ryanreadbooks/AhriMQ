#include "net/http/http_server.h"
#include "buffer/buffer.h"

using namespace ahrimq;

std::string handler1(const http::HTTPRequest& req, http::HTTPResponse& res,
                     const http::URLParams& params) {
  res.MakeContentPlainText("Greetings from http server!!");
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
    res.Redirect(loc, ahrimq::http::StatusTemporaryRedirect);
  } else {
    res.MakeContentPlainText("Welcome, no redirect!!");
    res.SetStatus(http::StatusOK);
  }

  return "";
}

int main(int argc, char** argv) {
  ahrimq::http::HTTPServerConfig config;
  config.port = 9527;
  config.root = "/";

  ahrimq::http::HTTPServer server(config);
  bool r = server.Get("/get", handler1);
  r = server.Get("/get/{username}", handler2);
  r = server.Get("/redirect", handler3);

  server.Run();

  return 0;
}