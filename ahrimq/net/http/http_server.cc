#include "net/http/http_server.h"

using std::placeholders::_1;  // _1, _2, ...
using std::placeholders::_2;

namespace ahrimq {
namespace http {

HTTPServer::HTTPServer()
    : IServer(std::make_shared<Reactor>(defaultTCPConfig.ip, DEFAULT_HTTP_PORT,
                                        defaultTCPConfig.n_threads)),
      config_(defaultHTTPConfig) {
  InitHTTPServer();
}

HTTPServer::HTTPServer(const HTTPServer::Config& config)
    : IServer(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config) {
  InitHTTPServer();
}

HTTPServer::HTTPServer(const HTTPServer::Config& config, HTTPCallback cb)
    : IServer(std::make_shared<Reactor>(config.ip, config.port, config.n_threads)),
      config_(config),
      on_request_cb_(std::move(cb)) {
  InitHTTPServer();
}

HTTPServer::~HTTPServer() {}

// TODO
void HTTPServer::Run() {}

// TODO
void HTTPServer::Stop() {}

void HTTPServer::InitReactorConfigs() {
  reactor_->SetConnNoDelay(config_.tcp_nodelay);
  reactor_->SetConnKeepAlive(config_.tcp_keepalive);
  reactor_->SetConnKeepAliveInterval(config_.tcp_keepalive_period);
  reactor_->SetConnKeepAliveCnt(config_.tcp_keepalive_count);
}

void HTTPServer::InitReactorHandlers() {
  reactor_->SetEventAcceptHandler(std::bind(&HTTPServer::OnStreamOpen, this, _1));
  reactor_->SetEventReadHandler(
      std::bind(&HTTPServer::OnStreamReached, this, _1, _2));
  reactor_->SetEventCloseHandler(std::bind(&HTTPServer::OnStreamClosed, this, _1));
  reactor_->SetEventWriteHandler(std::bind(&HTTPServer::OnStreamWritten, this, _1));
}

void HTTPServer::InitHTTPServer() {
  assert(reactor_ != nullptr);  // FIXME: optimize error handling
  InitReactorConfigs();
  InitReactorHandlers();
}

void HTTPServer::OnStreamOpen(ReactorConn* conn) {}

void HTTPServer::OnStreamReached(ReactorConn* conn, bool all_been_read) {}

void HTTPServer::OnStreamClosed(ReactorConn* conn) {}

void HTTPServer::OnStreamWritten(ReactorConn* conn) {}

}  // namespace http
}  // namespace ahrimq