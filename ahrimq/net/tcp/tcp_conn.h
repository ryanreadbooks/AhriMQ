#ifndef _TCPCONN_H_
#define _TCPCONN_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <sys/epoll.h>
#include <unistd.h>

#include "base/nocopyable.h"
#include "buffer/buffer.h"
#include "net/addr.h"
#include "net/epoller.h"
#include "net/eventloop.h"
#include "net/reactor_conn.h"
#include "net/utils.h"

namespace ahrimq {

class Epoller;
struct EventLoop;
class TCPConn;
class Reactor;
class TCPServer;

typedef std::function<void(TCPConn*, Buffer&)> TCPMessageCallback;
typedef std::function<void(TCPConn*)> TCPGenericCallback;

/// @brief TCPConn represents a tcp connection instance
class TCPConn : public NoCopyable {
  friend class Epoller;
  friend struct EventLoop;
  friend class Reactor;
  friend class TCPServer;

 public:
  /// @brief Status represents TCPConn status (open/closed)
  enum class Status { Open, Closed };

  explicit TCPConn(ReactorConn* conn);

  ~TCPConn();

  /// @brief retrieve all bytes from read buffer, this operation will consume all
  /// bytes in buffer
  /// @return
  std::vector<char> ReadAll();

  /// @brief return a reference to read buffer of TCPConn instance
  /// @return
  Buffer& GetReadBuffer() {
    return read_buf_;
  }

  /// @brief return a reference to write buffer of TCPConn instance
  /// @return
  Buffer& GetWriteBuffer() {
    return write_buf_;
  }

  /// @brief append a string to write buffer
  /// @param s
  void AppendWriteBuffer(const std::string& s);

  /// @brief append a char array with len to write buffer
  /// @param buf
  /// @param len
  void AppendWriteBuffer(const char* buf, size_t len);

  /// @brief append a std::vector<char> to write buffer
  /// @param buf
  void AppendWriteBuffer(const std::vector<char>& buf);

  /// @brief reset read buffer of TCPConn instance
  void ResetReadBuffer();

  /// @brief reset write buffer of TCPConn instance
  void ResetWriteBuffer();

  /// @brief send all bytes in write buffer
  void Send();

  /// @brief enable of disable tcp keepalive
  /// @param keepalive
  void SetTCPKeepAlive(bool keepalive);

  /// @brief check TCPConn is keepalive or not
  /// @return
  bool TCPKeepAlive() const {
    return tcp_keepalive_;
  }

  /// @brief set the interval between each tcp keepalive detection
  /// @param seconds
  void SetTCPKeepAlivePeriod(uint64_t seconds);

  /// @brief get the tcp keepalive interval
  /// @return
  int TCPKeepAlivePeriod() const {
    return tcp_keepalive_period_;
  }

  /// @brief set the maximum count of keepalive detection, if no response within cnt
  /// keepalive detection is received, tcp connection will be closed.
  /// @param cnt
  void SetTCPKeepAliveCount(int cnt);

  /// @brief get the maximum count of keepalive detection
  /// @return
  int TCPKeepAliveCnt() const {
    return tcp_keepalive_cnt_;
  }

  /// @brief enable/disable nagle algorithm
  /// @param nodelay
  void SetTCPNoDelay(bool nodelay);

  /// @brief check if nagle algorithm is enabled
  /// @return
  bool TCPNoDelay() const {
    return tcp_nodelay_;
  }

  /// @brief return local address
  /// @return
  IPAddr4Ptr LocalAddr() const;

  /// @brief return remote peer address
  /// @return
  IPAddr4Ptr PeerAddr() const;

  TCPConn::Status ConnStatus() const {
    return status_;
  }

  /// @brief check whether tcp connection is closed
  /// @return
  bool ConnClosed() const {
    return ConnStatus() == Status::Closed;
  }

  /// @brief check whether tcp connection is open
  /// @return
  bool ConnOpen() const {
    return ConnStatus() == Status::Open;
  }

  /// @brief get the file descriptor of tcp connection underneath
  /// @return
  int GetFd() const {
    return conn_->GetFd();
  }

  /// @brief get the name of tcp connection
  /// @return
  std::string GetName() const {
    return conn_->GetName();
  }

 protected:
  // read buffer
  Buffer read_buf_;
  // write buffer
  Buffer write_buf_;

  // corresponding conn, not owned
  ReactorConn* conn_;

  // connection status
  Status status_;

  // basic tcp connection configs
  bool tcp_nodelay_ = true;
  bool tcp_keepalive_ = true;
  int tcp_keepalive_period_ = 100;
  int tcp_keepalive_cnt_ = 2;
};

typedef std::shared_ptr<TCPConn> TCPConnPtr;

}  // namespace ahrimq

#endif  // _TCPCONN_H_