#ifndef _EPOLLER_H_
#define _EPOLLER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include <sys/epoll.h>
#include <unistd.h>

#include "net/reactor_conn.h"

typedef struct epoll_event linux_epoll_event_t;

namespace ahrimq {

class ReactorConn;

/// @brief Epoller represents an epoll instance in linux
class Epoller : public NoCopyable {
 public:
  explicit Epoller(size_t max_events);

  ~Epoller();

  bool AddFd(int fd, uint32_t events);

  bool DelFd(int fd);

  bool ModifyFd(int fd, uint32_t events);

  int Wait(int timeout_ms);

  std::vector<linux_epoll_event_t> &GetEpollEvents() {
    return ep_events_;
  }

  bool AttachConn(ReactorConn *conn);

  bool ModifyConn(ReactorConn *conn);

  bool DetachConn(ReactorConn *conn);

  int GetFd() const {
    return epfd_;
  }

  void Stop();

 private:
  // epoll file descriptor
  int epfd_ = -1;
  // struct epoll_event
  std::vector<linux_epoll_event_t> ep_events_;
};

typedef std::shared_ptr<Epoller> EpollerPtr;

}  // namespace ahrimq

#endif  // _EPOLLER_H_