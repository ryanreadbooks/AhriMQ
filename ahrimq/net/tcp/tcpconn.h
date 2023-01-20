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

class TCPConn;
class Epoller;
struct EventLoop;
class TCPConn;
class Reactor;

typedef std::function<void(TCPConn*, bool&)> ConnHandler;

typedef std::function<void(TCPConn*)> TCPCallback;

/**
 * @brief represent a connection instance
 *
 */
class TCPConn : public NoCopyable {
  friend class Epoller;
  friend struct EventLoop;
  friend class Reactor;

 public:
  TCPConn(int fd, uint32_t mask, ConnHandler rhandler, ConnHandler whandler,
          EventLoop* loop, std::string name);

  ~TCPConn();

  std::vector<char> ReadAll();

  const Buffer& ReadBuffer() {
    return read_buf_;
  }

  Buffer& WriteBuffer() {
    return write_buf_;
  }

  void AppendWriteBuffer(const std::string& s);

  void AppendWriteBuffer(const char* buf, size_t len);

  void AppendWriteBuffer(const std::vector<char>& buf);

  void ResetReadBuffer();

  void ResetWriteBuffer();

  void Send();

  void SetKeepAlive(bool keepalive);

  bool KeepAlive() const {
    return keepalive_;
  }

  void SetKeepAlivePeriod(uint64_t seconds);

  int KeepAlivePeriod() const {
    return keepalive_period_;
  }

  void SetKeepAliveCount(int cnt);

  int KeepAliveCnt() const {
    return keepalive_cnt_;
  }

  void SetNoDelay(bool nodelay);

  bool NoDelay() const {
    return nodelay_;
  }

  IPAddr4Ptr LocalAddr();

  IPAddr4Ptr PeerAddr();

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
  /* corresponding fd */
  int fd_;
  /* our interested events */
  uint32_t mask_;
  /* fired events */
  uint32_t events_;
  /* read handler: EPOLLIN */
  ConnHandler read_proc_;
  /* write handler: EPOLLOUT */
  ConnHandler write_proc_;
  /* which eventloop it belongs */
  EventLoop* loop_;
  /* read buffer */
  Buffer read_buf_;
  /* write buffer */
  Buffer write_buf_;
  /* the name of this TCPConn */
  std::string name_;
  /* indicate TCPConn is being watched or not */
  bool watched_ = false;

  /* basic tcp connection configs */
  bool nodelay_ = true;
  bool keepalive_ = true;
  int keepalive_period_ = 100;
  int keepalive_cnt_ = 2;
};

typedef std::shared_ptr<TCPConn> TCPConnPtr;

}  // namespace ahrimq

#endif  // _TCPCONN_H_