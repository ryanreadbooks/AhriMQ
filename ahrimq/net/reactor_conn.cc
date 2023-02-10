#include "net/reactor_conn.h"

namespace ahrimq {
ReactorConn::ReactorConn(int fd, uint32_t mask, const EpollEventHandler& rhandler,
                         const EpollEventHandler& whandler, EventLoop* loop,
                         std::string name)
    : fd_(fd),
      mask_(mask),
      read_proc_(rhandler),
      write_proc_(whandler),
      loop_(loop),
      name_(std::move(name)) {}

ReactorConn::~ReactorConn() {
  read_buf_ = nullptr;
  write_buf_ = nullptr;
  watched_ = false;
  close(fd_);
  if (loop_ != nullptr && loop_->epoller != nullptr) {
    loop_->epoller->DetachConn(this);
  }
  fd_ = -1;
}

bool ReactorConn::PutFile(int fd, bool closeafter) {
  file_state_.fd_ready_ = fd;
  struct stat statbuf = {0};
  if (fstat(fd, &statbuf) == -1) {
    return false;
  }
  file_state_.target_size_ = statbuf.st_size;
  file_state_.filesize_ = statbuf.st_size;
  file_state_.offset_ = 0;
  file_state_.close_after_ = closeafter;
  return true;
}

void ReactorConn::ResetFileState() {
  file_state_.fd_ready_ = -1;
  file_state_.target_size_ = 0;
  file_state_.offset_ = 0;
  file_state_.close_after_ = false;
  file_state_.filesize_ = 0;
}

}  // namespace ahrimq