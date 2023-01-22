#include "net/tcp/tcp_server.h"

using namespace std::placeholders;  // _1, _2, ...

namespace ahrimq {

IOReadWriteStatus ReadFromFdToBuf(int fd, Buffer& buf) {
  IOReadWriteStatus status{.n_bytes = 0, .statuscode = kStatusNormal};
  if (fd == -1) {
    status.n_bytes = 0;
    status.statuscode = kStatusError;
    return status;
  }
  size_t total_bytes = 0;
  char tmpbuf[kNetReadBufSize] = {0};
  while (true) {
    ssize_t nbytes = read(fd, tmpbuf, sizeof(tmpbuf));
    if (nbytes == 0) {
      // remote closed
      status.statuscode = kStatusClosed;
      break;
    } else if (nbytes > 0) {
      total_bytes += nbytes;
      buf.Append(tmpbuf, nbytes);
    } else {  // nbytes == -1
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        status.statuscode = kStatusExhausted;
        break;
      } else if (errno == EINTR) {
        continue;
      } else {
        status.statuscode = kStatusError;
        break;
      }
    }
  }
  status.n_bytes = total_bytes;
  return status;
}

TCPServer::TCPServer(const TCPServerConfig& config)
    : reactor_(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config) {
  assert(reactor_ != nullptr);
  InitReactorConfig();
  // set handlers
  InitReactorHandlers();
}

TCPServer::TCPServer(const TCPServerConfig& config, TCPMessageCallback on_read_cb)
    : reactor_(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config) {
  assert(reactor_ != nullptr);
  InitReactorConfig();
  InitReactorHandlers();
}

TCPServer::~TCPServer() {
  reactor_.reset();
}

void TCPServer::Run() {
  assert(reactor_ != nullptr);
  reactor_->React();
  reactor_->Wait();  // will block here
}

void TCPServer::InitReactorConfig() {
  reactor_->SetConnNoDelay(config_.nodelay);
  reactor_->SetConnKeepAlive(config_.keepalive);
  reactor_->SetConnKeepAliveInterval(config_.keepalive_period);
  reactor_->SetConnKeepAliveCnt(config_.keepalive_count);
}

void TCPServer::InitReactorHandlers() {
  reactor_->SetEventReadHandler(
      std::bind(&TCPServer::OnStreamReached, this, _1, _2));
  reactor_->SetEventCloseHandler(std::bind(&TCPServer::OnStreamClosed, this, _1));
  reactor_->SetEventWriteHandler(std::bind(&TCPServer::OnStreamWritten, this, _1));
}

void TCPServer::OnStreamClosed(TCPConn* conn) {
  if (on_closed_cb_ != nullptr) {
    on_closed_cb_(conn);
  }
}

// we collect all bytes from fd buffer and invoke on_message_callback_
void TCPServer::OnStreamReached(TCPConn* conn, bool eof_reached) {
  Buffer& rbuf = conn->GetReadBuffer();
  if (on_message_cb_ != nullptr) {
    if (eof_reached) {
      on_message_cb_(conn, conn->read_buf_);
    }
  }
}

void TCPServer::OnStreamWritten(TCPConn* conn) {}

}  // namespace ahrimq