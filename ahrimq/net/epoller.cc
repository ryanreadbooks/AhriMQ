#include "epoller.h"

namespace ahrimq {

Epoller::Epoller(size_t max_events)
    : epfd_(epoll_create(max_events)), ep_events_(max_events) {}

Epoller::~Epoller() {
  close(epfd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
  if (epfd_ == -1) {
    return false;
  }
  struct epoll_event ev {
    0
  };
  ev.data.fd = fd;
  ev.events = events;
  return epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::DelFd(int fd) {
  if (epfd_ == -1) {
    return false;
  }
  return epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

bool Epoller::ModifyFd(int fd, uint32_t events) {
  if (epfd_ == -1) {
    return false;
  }
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = fd;
  ev.events = events;
  return epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

int Epoller::Wait(int timeout_ms) {
  if (epfd_ == -1) {
    return -1;
  }
  return epoll_wait(epfd_, &ep_events_[0], static_cast<int>(ep_events_.size()),
                    timeout_ms);
}

bool Epoller::AttachConn(TCPConn *conn) {
  if (!conn) {
    return false;
  }
  epoll_event epev{0};
  epev.data.ptr = static_cast<void *>(conn);
  epev.events = conn->mask_;
  int ans = -1;
  if (!conn->watched_) {
    if ((ans = epoll_ctl(epfd_, EPOLL_CTL_ADD, conn->fd_, &epev)) == 0) {
      // std::cout << "fd = " << conn->fd << " attached\n";
      conn->watched_ = true;
    }
  } else {
    ans = epoll_ctl(epfd_, EPOLL_CTL_MOD, conn->fd_, &epev);
  }
  return ans == 0;
}

bool Epoller::ModifyConn(TCPConn *conn) {
  return AttachConn(conn);
}

bool Epoller::DetachConn(TCPConn *conn) {
  if (!conn) {
    return false;
  }
  int ans = -1;
  if ((ans = epoll_ctl(epfd_, EPOLL_CTL_DEL, conn->fd_, nullptr)) == 0) {
    conn->watched_ = false;
  }
  return ans == 0;
}

void Epoller::Stop() {
  close(epfd_);
}

}  // namespace ahrimq