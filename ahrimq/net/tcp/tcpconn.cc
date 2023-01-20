#include "tcpconn.h"

namespace ahrimq {

TCPConn::TCPConn(int fd, uint32_t mask, ConnHandler rhandler, ConnHandler whandler,
                 EventLoop* loop, std::string name)
    : fd_(fd),
      mask_(mask),
      read_proc_(std::move(rhandler)),
      write_proc_(std::move(whandler)),
      loop_(loop),
      name_(std::move(name)) {}

TCPConn::~TCPConn() {
  read_buf_.Reset();
  write_buf_.Reset();
  watched_ = false;
  close(fd_);
}

/* copy all data from read_buf_ and consume all data in it */
std::vector<char> TCPConn::ReadAll() {
  size_t n = read_buf_.ReadableBytes();
  if (n == 0) {
    return {};
  }
  return std::move(read_buf_.ReadAll());
}

void TCPConn::AppendWriteBuffer(const std::string& s) {
  write_buf_.Append(s);
}

void TCPConn::AppendWriteBuffer(const char* buf, size_t len) {
  write_buf_.Append(buf, len);
}

void TCPConn::AppendWriteBuffer(const std::vector<char>& buf) {
  write_buf_.Append(buf.data(), buf.size());
}

void TCPConn::ResetReadBuffer() {
  read_buf_.Reset();
}

void TCPConn::ResetWriteBuffer() {
  write_buf_.Reset();
}

void TCPConn::Send() {
  if (write_buf_.ReadableBytes() >= 0) {
    SetMaskWrite();
  }
}

void TCPConn::SetKeepAlive(bool keepalive) {
  keepalive_ = keepalive;
  if (keepalive_) {
    EnableTCPKeepAlive(fd_);
  } else {
    DisableTCPKeepAlive(fd_);
  }
}

void TCPConn::SetKeepAlivePeriod(uint64_t seconds) {
  if (keepalive_) {
    keepalive_period_ = seconds;
    SetKeepAliveIdle(fd_, seconds);
    SetKeepAliveInterval(fd_, seconds);
  }
}
void TCPConn::SetKeepAliveCount(int cnt) {
  if (keepalive_) {
    keepalive_cnt_ = cnt;
    SetKeepAliveCnt(fd_, cnt);
  }
}
void TCPConn::SetNoDelay(bool nodelay) {
  nodelay_ = nodelay;
  if (nodelay) {
    EnableTCPNoDelay(fd_);
  } else {
    DisableTCPNoDelay(fd_);
  }
}

IPAddr4Ptr TCPConn::LocalAddr() {
  IPAddr4Ptr local = std::make_shared<IPAddr4>();
  socklen_t socklen = local->GetSockAddrLen();
  if (getsockname(fd_, local->GetAddr(), &socklen) == -1) {
    return nullptr;
  }
  local->SyncPort();
  return local;
}

IPAddr4Ptr TCPConn::PeerAddr() {
  IPAddr4Ptr peer = std::make_shared<IPAddr4>();
  socklen_t socklen = peer->GetSockAddrLen();
  if (getpeername(fd_, peer->GetAddr(), &socklen) == -1) {
    return nullptr;
  }
  peer->SyncPort();
  return peer;
}

}  // namespace ahrimq