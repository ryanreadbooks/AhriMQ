#ifndef _AHRIMQ_NET_REACTOR_CONN_H_
#define _AHRIMQ_NET_REACTOR_CONN_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <memory>
#include <queue>

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
  ReactorConn(int fd, uint32_t mask, const EpollEventHandler& rhandler,
              const EpollEventHandler& whandler, EventLoop* loop, std::string name);

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

  /// @brief Attach a file to send every writing time.
  /// @param fd
  /// @param closeafter
  /// @return
  bool PutFile(int fd, bool closeafter = false);

  void ResetFileState();

  bool FileNeedSending() const {
    return file_state_.fd_ready_ != -1 && file_state_.target_size_ > 0;
  }

  size_t FileSize() const {
    return file_state_.filesize_;
  }

 private:
  void SetMaskRead() {
    mask_ = EPOLLIN | EPOLLONESHOT;
  }

  void SetMaskWrite() {
    mask_ |= EPOLLOUT;
  }

  void DisableMaskWrite() {
    mask_ = EPOLLIN | EPOLLONESHOT;
  }

  void SetMaskReadWrite() {
    mask_ = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
  }

 private:
  int fd_ = -1;
  // our interested events
  uint32_t mask_ = EPOLLIN;
  // fired events
  uint32_t events_ = 0;
  // read handler: EPOLLIN
  EpollEventHandler read_proc_;
  // write handler: EPOLLOUT
  EpollEventHandler write_proc_;
  // which eventloop it belongs
  EventLoop* loop_ = nullptr;
  // corresponding read buffer, not owned by ReactorConn
  Buffer* read_buf_ = nullptr;
  // corresponding write buffer, not owned by ReactorConn
  Buffer* write_buf_ = nullptr;
  // the name of this connection
  std::string name_;
  // indicate connection is being watched or not
  bool watched_ = false;
  // support sending file when write data out
  struct {
    int fd_ready_ = -1;
    size_t offset_ = 0;
    size_t target_size_ = 0;
    bool close_after_ = false;  // close file descriptor after sending it
    size_t filesize_ = 0;
  } file_state_;
};

typedef std::shared_ptr<ReactorConn> ReactorConnPtr;

}  // namespace ahrimq

#endif  // _AHRIMQ_NET_REACTOR_CONN_H_