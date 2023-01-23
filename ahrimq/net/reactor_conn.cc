#include "net/reactor_conn.h"

namespace ahrimq {
ReactorConn::ReactorConn(int fd, uint32_t mask, EpollEventHandler rhandler,
                         EpollEventHandler whandler, EventLoop* loop,
                         std::string name)
    : fd_(fd),
      mask_(mask),
      read_proc_(std::move(rhandler)),
      write_proc_(std::move(whandler)),
      loop_(loop),
      name_(std::move(name)) {}

ReactorConn::~ReactorConn() {
  read_buf_ = nullptr;
  write_buf_ = nullptr;
  watched_ = false;
  close(fd_);
}
}  // namespace ahrimq