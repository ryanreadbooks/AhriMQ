#include "ahrimq/net/tcp/tcp_server.h"

int main(int argc, char** argv) {
  ahrimq::TCPServerConfig config;
  config.ip = "127.0.0.1";
  config.port = 9527;
  config.tcp_keepalive = false;
  // echo server
  ahrimq::TCPServer server(config);

  server.SetOnMessageCallback([&](ahrimq::TCPConn* conn, ahrimq::Buffer& message) {
    std::vector<char> data(conn->ReadAll());
    std::cout << "Message received from " << conn->PeerAddr()->ToString() << ": "
              << std::string(data.begin(), data.end()) << std::endl;
    conn->AppendWriteBuffer(data);
    conn->Send();
  });

  server.SetOnClosedCallback([&](ahrimq::TCPConn* conn) {
    if (conn->ConnClosed()) {
      std::cout << "Connection " << conn->GetName() << " closed\n";
    }
  });

  server.Run();

  return 0;
}