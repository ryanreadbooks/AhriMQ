#ifndef _REACTOR_CONN_H_
#define _REACTOR_CONN_H_

#include <memory>

#include "base/nocopyable.h"
#include "buffer/buffer.h"
#include "net/addr.h"
#include "net/epoller.h"
#include "net/eventloop.h"

namespace ahrimq {

class Epoller;
class Reactor;
class EventLoop;
class ReactorConn;

typedef std::function<void(ReactorConn*, bool&)> EpollEventHandler;

class ReactorConn : public NoCopyable {
  friend class Epoller;
  friend class Reactor;
  friend class EventLoop;
  friend class TCPConn;

 public:
  ReactorConn(int fd, uint32_t mask, EpollEventHandler rhandler,
              EpollEventHandler whandler, EventLoop* loop, std::string name);

  ~ReactorConn();

  int GetFd() const {
    return fd_;
  }

  std::string GetName() const {
    return name_;
  }

  Buffer* GetReadBuffer() const {
    return read_buf_;
  }

  Buffer* GetWriteBuffer() const {
    return write_buf_;
  }

  void SetReadBuffer(Buffer* rbuf) {
    read_buf_ = rbuf;
  }

  void SetWriteBuffer(Buffer* wbuf) {
    write_buf_ = wbuf;
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
  // corresponding read buffer, not owned by ReactorConn
  Buffer* read_buf_;
  // corresponding write buffer, not owned by ReactorConn
  Buffer* write_buf_;
  // the name of this connection
  std::string name_;
  // indicate connection is being watched or not
  bool watched_ = false;
};

typedef std::shared_ptr<ReactorConn> ReactorConnPtr;

}  // namespace ahrimq

#endif  // _REACTOR_CONN_H_