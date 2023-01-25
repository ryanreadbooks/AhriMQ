#include "buffer/buffer.h"
#include "net/http/http_server.h"

int main(int argc, char** argv) {
  ahrimq::http::HTTPServerConfig config;
  config.port = 9527;

  ahrimq::http::HTTPServer server(config);

  server.Run();

  return 0;
}