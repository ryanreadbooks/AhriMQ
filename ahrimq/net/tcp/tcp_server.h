#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "net/reactor.h"
#include "tcpconn.h"

namespace ahrimq {

class TCPServerConfig {
 public:
  std::string ip = "127.0.0.1";
  uint16_t port = 9527;
  bool nodelay = true;
  bool keepalive = true;
  int keepalive_period = 100;  // unit second
  int keepalive_count = 1;
  uint32_t n_threads = std::thread::hardware_concurrency();
};

typedef std::shared_ptr<TCPServerConfig> TCPServerConfigPtr;

class TCPServer : public NoCopyable {
 public:
  TCPServer(const TCPServerConfig& config);

  TCPServer(const TCPServerConfig& config, TCPCallback on_read_cb);

  TCPServer(const TCPServer&) = delete;

  ~TCPServer();

  void OnRead(TCPCallback cb);

  void Run();

 private:
  void SetReactorConfig();

 private:
  ReactorPtr reactor_;
  TCPServerConfig config_;
};

}  // namespace ahrimq

#endif  // _TCPSERVER_H_