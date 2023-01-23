#include "net/tcp/tcp_server.h"
#include "buffer/buffer.h"

int main(int argc, char** argv) {
  ahrimq::TCPServerConfig config;
  config.ip = "127.0.0.1";
  config.port = 9527;
  config.tcp_keepalive = false;
  // echo server
  ahrimq::TCPServer server(config);

  server.SetOnMessageCallback([&](ahrimq::TCPConn* conn, ahrimq::Buffer& message) {
    std::vector<char> data(conn->ReadAll());
    std::cout << "Message received from " << conn->PeerAddr()->ToString() << std::endl;
    std::string resp =
        "HTTP/1.1 200 OK\r\n"
        "Server: AhriMQ\r\n"
        "Content-Type: application/json\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: 17\r\n"
        "Content-Language: zh-CN\r\n"
        "\r\n"
        "{\"name\": \"hello\"}\n";
    conn->AppendWriteBuffer(resp);
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