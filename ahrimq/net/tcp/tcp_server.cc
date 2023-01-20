#include "tcp_server.h"

namespace ahrimq {

TCPServer::TCPServer(const TCPServerConfig& config)
    : reactor_(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config) {
  assert(reactor_ != nullptr);
  SetReactorConfig();
}

TCPServer::TCPServer(const TCPServerConfig& config, TCPCallback on_read_cb)
    : reactor_(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config) {
  assert(reactor_ != nullptr);
  reactor_->SetTCPCallback(std::move(on_read_cb));
  SetReactorConfig();
}

TCPServer::~TCPServer() {
  reactor_.reset();
}

void TCPServer::OnRead(TCPCallback cb) {
  reactor_->SetTCPCallback(std::move(cb));
}

void TCPServer::Run() {
  assert(reactor_ != nullptr);
  reactor_->React();
  reactor_->Wait();  // will block here
}

void TCPServer::SetReactorConfig() {
  reactor_->SetConnNoDelay(config_.nodelay);
  reactor_->SetConnKeepAlive(config_.keepalive);
  reactor_->SetConnKeepAliveInterval(config_.keepalive_period);
  reactor_->SetConnKeepAliveCnt(config_.keepalive_count);
}

}  // namespace ahrimq