#include "net/tcp/tcp_server.h"

using std::placeholders::_1;  // _1, _2, ...
using std::placeholders::_2;

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

TCPServer::TCPServer()
    : IServer(std::make_shared<Reactor>(defaultTCPConfig.ip, defaultTCPConfig.port,
                                        defaultTCPConfig.n_threads)),
      config_(defaultTCPConfig) {
  InitTCPServer();
}

TCPServer::TCPServer(const TCPServer::Config& config)
    : IServer(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config) {
  InitTCPServer();
}

TCPServer::TCPServer(const TCPServer::Config& config,
                     TCPMessageCallback on_message_cb)
    : IServer(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config),
      on_message_cb_(std::move(on_message_cb)) {
  InitTCPServer();
}

TCPServer::~TCPServer() {
  reactor_.reset();
}

void TCPServer::Run() {
  assert(reactor_ != nullptr);
  reactor_->React();
  reactor_->Wait();  // will block here
}

// TODO: implement it
void TCPServer::Stop() {

}

void TCPServer::InitReactorConfigs() {
  reactor_->SetConnNoDelay(config_.tcp_nodelay);
  reactor_->SetConnKeepAlive(config_.tcp_keepalive);
  reactor_->SetConnKeepAliveInterval(config_.tcp_keepalive_period);
  reactor_->SetConnKeepAliveCnt(config_.tcp_keepalive_count);
}

void TCPServer::InitReactorHandlers() {
  reactor_->SetEventReadHandler(
      std::bind(&TCPServer::OnStreamReached, this, _1, _2));
  reactor_->SetEventCloseHandler(std::bind(&TCPServer::OnStreamClosed, this, _1));
  reactor_->SetEventWriteHandler(std::bind(&TCPServer::OnStreamWritten, this, _1));
}

void TCPServer::InitTCPServer() {
  // FIXME optimize error handling
  assert(reactor_ != nullptr);
  // init reactor configs
  InitReactorConfigs();
  // set handlers
  InitReactorHandlers();
}

void TCPServer::OnStreamClosed(TCPConn* conn) {
  if (on_closed_cb_ != nullptr) {
    on_closed_cb_(conn);
  }
}

// we collect all bytes from fd buffer and invoke on_message_callback_
void TCPServer::OnStreamReached(TCPConn* conn, bool all_been_read) {
  Buffer& rbuf = conn->GetReadBuffer();
  if (on_message_cb_ != nullptr) {
    if (all_been_read) {
      on_message_cb_(conn, conn->read_buf_);
    }
  }
}

void TCPServer::OnStreamWritten(TCPConn* conn) {}

}  // namespace ahrimq