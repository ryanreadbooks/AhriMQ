#include "eventloop.h"

namespace ahrimq {

EventLoop::EventLoop() : epoller(new (std::nothrow) Epoller(1024)), stopped(false) {
  if (epoller == nullptr) {
    std::cerr << "can not initialize epoller in event loop, program abort.\n";
    exit(EXIT_FAILURE);
  }
}

EventLoop::~EventLoop() {
  if (epoller != nullptr) {
    delete epoller;
    epoller = nullptr;
  }
  stopped.store(true);
}

void EventLoop::Loop() {
  if (epoller == nullptr) {
    return;
  }
  static int debug_n = 1;
  while (!stopped) {
    int timeout_ms = -1;
    int ready = epoller->Wait(timeout_ms);
    /* process events one by one */
    // std::cout << "ready = " << ready << std::endl;
    debug_n += ready;
    for (int i = 0; ready != -1 && i < ready; ++i) {
      uint32_t fired_events = epoller->GetEpollEvents()[i].events;
      TCPConn *conn = static_cast<TCPConn *>(epoller->GetEpollEvents()[i].data.ptr);
      if (!conn) {
        continue;
      }
      /*　flag to indicate whether conn is freed to avoid memory issue */
      bool closed = false;
      if (conn->mask_ & fired_events & EPOLLIN) {
        conn->read_proc_(conn, closed);
      }
      if (closed) {
        continue;
      }
      if (conn->mask_ & fired_events & EPOLLOUT) {
        if (conn->write_proc_) {
          conn->write_proc_(conn, closed);
        }
      }
    }
  }
}

void EventLoop::Stop() {
  if (epoller == nullptr) {
    return;
  }
  epoller->Stop();
  stopped.store(true, std::memory_order_relaxed);
}

}  // namespace ahrimq