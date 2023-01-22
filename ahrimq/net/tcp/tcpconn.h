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
#include "net/utils.h"

namespace ahrimq {

class Epoller;
struct EventLoop;
class TCPConn;
class Reactor;
class TCPServer;

typedef std::function<void(TCPConn*, bool&)> EpollEventHandler;

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
  enum class Status { OPEN, CLOSED };

  TCPConn(int fd, uint32_t mask, EpollEventHandler rhandler,
          EpollEventHandler whandler, EventLoop* loop, std::string name);

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
  void SetKeepAlive(bool keepalive);

  /// @brief check TCPConn is keepalive or not
  /// @return
  bool KeepAlive() const {
    return keepalive_;
  }

  /// @brief set the interval between each tcp keepalive detection
  /// @param seconds
  void SetKeepAlivePeriod(uint64_t seconds);

  /// @brief get the tcp keepalive interval
  /// @return
  int KeepAlivePeriod() const {
    return keepalive_period_;
  }

  /// @brief set the maximum count of keepalive detection, if no response within cnt
  /// keepalive detection is received, tcp connection will be closed.
  /// @param cnt
  void SetKeepAliveCount(int cnt);

  /// @brief get the maximum count of keepalive detection
  /// @return
  int KeepAliveCnt() const {
    return keepalive_cnt_;
  }

  /// @brief enable/disable nagle algorithm
  /// @param nodelay
  void SetNoDelay(bool nodelay);

  /// @brief check if nagle algorithm is enabled
  /// @return
  bool NoDelay() const {
    return nodelay_;
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
    return ConnStatus() == Status::CLOSED;
  }

  /// @brief check whether tcp connection is open
  /// @return
  bool ConnOpen() const {
    return ConnStatus() == Status::OPEN;
  }

  /// @brief get the file descriptor of tcp connection underneath
  /// @return
  int GetFd() const {
    return fd_;
  }

  /// @brief get the name of tcp connection
  /// @return
  std::string GetName() const {
    return name_;
  }

 private:
  void SetMaskRead() {
    mask_ = EPOLLIN;
  }

  void SetMaskWrite() {
    mask_ |= EPOLLOUT;
  }

  void DisableMaskWrite() {
    mask_ = EPOLLIN;
  }

  void SetMaskReadWrite() {
    mask_ = EPOLLIN | EPOLLOUT;
  }

 private:
  // corresponding fd
  int fd_;
  // our interested events
  uint32_t mask_;
  // fired events
  uint32_t events_;
  // read handler: EPOLLIN
  EpollEventHandler read_proc_;
  // write handler: EPOLLOUT
  EpollEventHandler write_proc_;
  // which eventloop it belongs
  EventLoop* loop_;
  // read buffer
  Buffer read_buf_;
  // write buffer
  Buffer write_buf_;
  // the name of this TCPConn
  std::string name_;
  // indicate TCPConn is being watched or not
  bool watched_ = false;
  // connection status
  Status status_;

  // basic tcp connection configs
  bool nodelay_ = true;
  bool keepalive_ = true;
  int keepalive_period_ = 100;
  int keepalive_cnt_ = 2;
};

typedef std::shared_ptr<TCPConn> TCPConnPtr;

}  // namespace ahrimq

#endif  // _TCPCONN_H_