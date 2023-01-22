#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "net/reactor.h"
#include "net/tcp/tcpconn.h"

namespace ahrimq {

typedef short ReadWriteStatusCode;

// original status
constexpr static ReadWriteStatusCode kStatusNormal = 0;
// peer closed status
constexpr static ReadWriteStatusCode kStatusClosed = 1;
// all data in socket has been read
constexpr static ReadWriteStatusCode kStatusExhausted = 2;
// error status
constexpr static ReadWriteStatusCode kStatusError = 3;

struct IOReadWriteStatus {
  size_t n_bytes;
  ReadWriteStatusCode statuscode;
};

/// @brief Read all bytes from non-blocked fd to buf till fd's read buffer is
/// exhausted(EAGAIN | EWOULDBLOCK)
/// @param fd file descriptor to read from
/// @param buf ahrimq::Buffer to read to
/// @return status indicating result
IOReadWriteStatus ReadFromFdToBuf(int fd, Buffer& buf);

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 9527
#define DEFAULT_SERVER_NODELAY true
#define DEFAULT_SERVER_KEEPALIVE true
#define DEFALUT_SERVER_KEEPALIVE_PERIOD 100  // unit second
#define DEFALUT_SERVER_KEEPALIVE_COUNT 1
#define DEFAULT_SERVER_N_THREADS std::thread::hardware_concurrency()

// TCP basic configs
class TCPServerConfig {
 public:
  std::string ip = DEFAULT_SERVER_IP;
  uint16_t port = DEFAULT_SERVER_PORT;
  bool nodelay = DEFAULT_SERVER_NODELAY;
  bool keepalive = DEFAULT_SERVER_KEEPALIVE;
  int keepalive_period = DEFALUT_SERVER_KEEPALIVE_PERIOD;
  int keepalive_count = DEFALUT_SERVER_KEEPALIVE_COUNT;
  uint32_t n_threads = DEFAULT_SERVER_N_THREADS;
};

typedef std::shared_ptr<TCPServerConfig> TCPServerConfigPtr;

class TCPServer : public NoCopyable {
 public:
  TCPServer(const TCPServerConfig& config);

  TCPServer(const TCPServerConfig& config, TCPMessageCallback on_read_cb);

  TCPServer(const TCPServer&) = delete;

  ~TCPServer();

  void SetOnMessageCallback(TCPMessageCallback cb) {
    on_message_cb_ = std::move(cb);
  }

  void SetOnClosedCallback(TCPGenericCallback cb) {
    on_closed_cb_ = std::move(cb);
  }

  void Run();

 protected:
  void InitReactorConfig();

  void InitReactorHandlers();

  void OnStreamClosed(TCPConn* conn);

  void OnStreamReached(TCPConn* conn, bool eof_reached);

  void OnStreamWritten(TCPConn* conn);  // FIXME maybe this is not needed

 protected:
  ReactorPtr reactor_;
  TCPServerConfig config_;

  // user-specified callbacks
  TCPMessageCallback on_message_cb_;
  TCPGenericCallback on_closed_cb_;
};

}  // namespace ahrimq

#endif  // _TCPSERVER_H_