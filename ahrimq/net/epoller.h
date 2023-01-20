#ifndef _EPOLLER_H_
#define _EPOLLER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include <sys/epoll.h>
#include <unistd.h>

#include "tcp/tcpconn.h"

typedef struct epoll_event epoll_event;

namespace ahrimq {

struct TCPConn;

class Epoller : public NoCopyable {
 public:
  explicit Epoller(size_t max_events);

  ~Epoller();

  bool AddFd(int fd, uint32_t events);

  bool DelFd(int fd);

  bool ModifyFd(int fd, uint32_t events);

  int Wait(int timeout_ms);

  std::vector<epoll_event> &GetEpollEvents() {
    return ep_events_;
  }

  bool AttachConn(TCPConn *conn);

  bool ModifyConn(TCPConn *conn);

  bool DetachConn(TCPConn *conn);

  int GetFd() const {
    return epfd_;
  }

  void Stop();

 private:
  /* epoll file descriptor */
  int epfd_ = -1;
  /* struct epoll_event */
  std::vector<epoll_event> ep_events_;
};

typedef std::shared_ptr<Epoller> EpollerPtr;

}  // namespace ahrimq

#endif  // _EPOLLER_H_