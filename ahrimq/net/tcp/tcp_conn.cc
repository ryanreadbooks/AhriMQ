#include "net/tcp/tcp_conn.h"

namespace ahrimq {

TCPConn::TCPConn(ReactorConn* conn) : read_buf_(32768), write_buf_(4096), conn_(conn) {
  if (conn != nullptr) {
    status_ = Status::Open;
    return;
  }
  status_ = Status::Closed;
}

TCPConn::~TCPConn() {
  read_buf_.Reset();
  write_buf_.Reset();
  conn_ = nullptr;
  status_ = Status::Closed;
}

/* copy all data from read_buf_ and consume all data in it */
std::vector<char> TCPConn::ReadAll() {
  size_t n = read_buf_.Size();
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
  if (write_buf_.Size() >= 0) {
    conn_->SetMaskWrite();
  }
}

void TCPConn::SetTCPKeepAlive(bool keepalive) {
  tcp_keepalive_ = keepalive;
  if (tcp_keepalive_) {
    EnableTCPKeepAlive(conn_->fd_);
  } else {
    DisableTCPKeepAlive(conn_->fd_);
  }
}

void TCPConn::SetTCPKeepAlivePeriod(uint64_t seconds) {
  if (tcp_keepalive_) {
    tcp_keepalive_period_ = seconds;
    SetKeepAliveIdle(conn_->fd_, seconds);
    SetKeepAliveInterval(conn_->fd_, seconds);
  }
}
void TCPConn::SetTCPKeepAliveCount(int cnt) {
  if (tcp_keepalive_) {
    tcp_keepalive_cnt_ = cnt;
    SetKeepAliveCnt(conn_->fd_, cnt);
  }
}
void TCPConn::SetTCPNoDelay(bool nodelay) {
  tcp_nodelay_ = nodelay;
  if (nodelay) {
    EnableTCPNoDelay(conn_->fd_);
  } else {
    DisableTCPNoDelay(conn_->fd_);
  }
}

IPAddr4Ptr TCPConn::LocalAddr() const {
  IPAddr4Ptr local = std::make_shared<IPAddr4>();
  socklen_t socklen = local->GetSockAddrLen();
  if (getsockname(conn_->fd_, local->GetAddr(), &socklen) == -1) {
    return nullptr;
  }
  local->SyncPort();
  return local;
}

IPAddr4Ptr TCPConn::PeerAddr() const {
  IPAddr4Ptr peer = std::make_shared<IPAddr4>();
  socklen_t socklen = peer->GetSockAddrLen();
  if (getpeername(conn_->fd_, peer->GetAddr(), &socklen) == -1) {
    return nullptr;
  }
  peer->SyncPort();
  return peer;
}

}  // namespace ahrimq