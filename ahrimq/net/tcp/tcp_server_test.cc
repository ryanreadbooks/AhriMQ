#include "net/tcp/tcp_server.h"

int main(int argc, char** argv) {
  ahrimq::TCPServerConfig config;
  config.ip = "127.0.0.1";
  config.port = 9527;
  // echo server
  ahrimq::TCPServer server(config);

  server.OnRead([&](ahrimq::TCPConn* conn) {
    std::vector<char> data(conn->ReadAll());
    std::cout << "Received from " << conn->PeerAddr()->ToString() << ": "
              << std::string(data.begin(), data.end()) << std::endl;
    conn->AppendWriteBuffer(data);
    conn->Send();
  });

  server.Run();

  return 0;
}