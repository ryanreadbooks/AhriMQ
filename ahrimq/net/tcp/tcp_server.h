#ifndef __AHRIMQ_NET_TCP_TCP_SERVER_H_
#define __AHRIMQ_NET_TCP_TCP_SERVER_H_

#include "net/iserver.h"
#include "net/reactor.h"
#include "net/tcp/tcp_conn.h"

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
/// exhausted(EAGAIN | EWOULDBLOCK).
/// @param fd file descriptor to read from
/// @param buf ahrimq::Buffer to read to
/// @return status indicating result
IOReadWriteStatus ReadFromFdToBuf(int fd, Buffer& buf);

#define DEFAULT_TCP_SERVER_IP "127.0.0.1"
#define DEFAULT_TCP_SERVER_PORT 9527
#define DEFAULT_TCP_SERVER_NODELAY true
#define DEFAULT_TCP_SERVER_KEEPALIVE true
#define DEFALUT_TCP_SERVER_KEEPALIVE_PERIOD 100  // unit second
#define DEFALUT_TCP_SERVER_KEEPALIVE_COUNT 2
#define DEFAULT_TCP_SERVER_N_THREADS std::thread::hardware_concurrency()

/// @brief TCPServer implementation
class TCPServer : public NoCopyable, public IServer {
 public:
  /// @brief TCP basic configs
  class Config {
   public:
    std::string ip = DEFAULT_TCP_SERVER_IP;
    uint16_t port = DEFAULT_TCP_SERVER_PORT;
    bool tcp_nodelay = DEFAULT_TCP_SERVER_NODELAY;
    bool tcp_keepalive = DEFAULT_TCP_SERVER_KEEPALIVE;
    int tcp_keepalive_period = DEFALUT_TCP_SERVER_KEEPALIVE_PERIOD;
    int tcp_keepalive_count = DEFALUT_TCP_SERVER_KEEPALIVE_COUNT;
    uint32_t n_threads = DEFAULT_TCP_SERVER_N_THREADS;
  };

 public:
  /// @brief Construct a TCPServer with default configs.
  TCPServer();

  /// @brief Construct a TCPServer.
  /// @param config TCPServerConfig instance
  explicit TCPServer(const TCPServer::Config& config);

  /// @brief Construct a TCPServer with message callback.
  /// @param config TCPServerConfig instance
  /// @param on_message_cb on message coming in callback function
  TCPServer(const TCPServer::Config& config, TCPMessageCallback on_message_cb);

  ~TCPServer();

  /// @brief Set on message callback.
  /// @param cb callback function
  void SetOnMessageCallback(TCPMessageCallback cb) {
    on_message_cb_ = std::move(cb);
  }

  /// @brief Set connection closed callback function.
  /// @param cb
  void SetOnClosedCallback(TCPGenericCallback cb) {
    on_closed_cb_ = std::move(cb);
  }

  /// @brief Start the server.
  void Run() override;

  /// @brief Stop the server.
  void Stop() override;

 protected:
  void InitReactorHandlers() override;

  void InitTCPServer();

  void OnStreamOpen(ReactorConn* conn, bool& close_after) override;

  void OnStreamClosed(ReactorConn* conn, bool& close_after) override;

  void OnStreamReached(ReactorConn* conn, bool allread, bool& close_after) override;

  void OnStreamWritten(ReactorConn* conn, bool& close_after) override;

 private:
  // ReactorPtr reactor_;
  TCPServer::Config config_;

  // all tcp connections
  std::unordered_map<std::string, TCPConnPtr> tcpconns_;

  // user-specified callbacks
  TCPMessageCallback on_message_cb_;
  TCPGenericCallback on_closed_cb_;
};

typedef std::shared_ptr<TCPServer> TCPServerPtr;
typedef TCPServer::Config TCPServerConfig;
typedef std::shared_ptr<TCPServer::Config> TCPServerConfigPtr;

// must be static
static TCPServer::Config defaultTCPConfig;

}  // namespace ahrimq

#endif  // __AHRIMQ_NET_TCP_TCP_SERVER_H_