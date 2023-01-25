#include "net/http/http_server.h"

#include "base/time_utils.h"

using std::placeholders::_1;  // _1, _2, ...
using std::placeholders::_2;

namespace ahrimq {
namespace http {

HTTPServer::HTTPServer()
    : IServer(std::make_shared<Reactor>(ahrimq::defaultTCPConfig.ip,
                                        DEFAULT_HTTP_PORT,
                                        ahrimq::defaultTCPConfig.n_threads)),
      config_(ahrimq::http::defaultHTTPConfig) {
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
void HTTPServer::Run() {
  assert(reactor_ != nullptr);
  reactor_->React();
  reactor_->Wait();
}

// TODO
void HTTPServer::Stop() {}

void HTTPServer::InitReactorHandlers() {
  reactor_->SetEventAcceptHandler(std::bind(&HTTPServer::OnStreamOpen, this, _1));
  reactor_->SetEventReadHandler(
      std::bind(&HTTPServer::OnStreamReached, this, _1, _2));
  reactor_->SetEventCloseHandler(std::bind(&HTTPServer::OnStreamClosed, this, _1));
  reactor_->SetEventWriteHandler(std::bind(&HTTPServer::OnStreamWritten, this, _1));
}

void HTTPServer::InitHTTPServer() {
  assert(reactor_ != nullptr);  // FIXME: optimize error handling
  InitReactorHandlers();
}

void HTTPServer::OnStreamOpen(ReactorConn* conn) {
  std::string conn_name = conn->GetName();
  // create a new http connection instance
  HTTPConnPtr httpconn = std::make_shared<HTTPConn>(conn);
  httpconn->SetTCPKeepAlive(config_.tcp_keepalive);
  httpconn->SetTCPKeepAlivePeriod(config_.tcp_keepalive_period);
  httpconn->SetTCPKeepAliveCount(config_.tcp_keepalive_count);
  httpconn->SetTCPNoDelay(config_.tcp_nodelay);
  mtx_.lock();
  httpconns_.insert({conn_name, httpconn});
  mtx_.unlock();
  conn->SetReadBuffer(&httpconn->read_buf_);
  conn->SetWriteBuffer(&httpconn->write_buf_);
  std::cout << "HTTP connection " << conn_name << " opened\n";
}

void HTTPServer::OnStreamReached(ReactorConn* conn, bool allread) {
  std::string conn_name = conn->GetName();
  HTTPConnPtr httpconn = httpconns_[conn_name];
StartParseRequestDatagramTag:
  int retcode = ParseRequestDatagram(httpconn.get());
  if (httpconn->CurrentResponseIsNull()) {
    httpconn->CurrentResponseRef() = std::make_shared<HTTPResponse>();
  }
  if (retcode == StatusPrivateDone) {
    // do request
    httpconn->SetCurrentParsingStateLine();
    DoRequest(httpconn.get());
  } else {
    // request datagram is abnormal, we need to do error handling
    if (retcode == StatusPrivateInvalid) {
      retcode = StatusBadRequest;
    }
    DoRequestError(httpconn.get(), retcode);
  }
  // TODO consider the situation where http request pipelining is needed
  // support http request pipelining
  
  // send all response data out to client
  httpconn->Send();
}

void HTTPServer::OnStreamClosed(ReactorConn* conn) {
  // TODO handle connection close
}

void HTTPServer::OnStreamWritten(ReactorConn* conn) {
  // FIXME may be this is not needed
  // std::string conn_name = conn->GetName();
  // HTTPConnPtr httpconn = httpconns_[conn_name];
  // httpconn->CurrentRequestRef()->Reset();
  // httpconn->CurrentResponseRef()->Reset();
  // std::cout << "HTTPServer::OnStreamWritten, Request and Response reset\n";
}

void HTTPServer::DoRequest(HTTPConn* conn) {
  HTTPRequestPtr& req = conn->CurrentRequestRef();
  HTTPHeaderPtr& req_header = req->HeaderRef();
  const URL& url = req->URLRef();
  std::cout << "Requested url: " << url << std::endl;
  // set http response data
  HTTPResponsePtr& res = conn->CurrentResponseRef();
  HTTPHeaderPtr& res_header = res->HeaderRef();
  // TODO route tracing and call corresponding user-specific methods

  // add response date
  res->SetStatus(StatusOK);
  res_header->Add("Date", GMTTimeNow());
  // TODO: for debug purpose
  std::string body = "{\"name\" : \"ryanreadbooks\"}";
  size_t len = body.size();
  res_header->Add("Content-Length", std::to_string(len));
  res_header->Add("Content-Type", "application/json");
  res->Organize(conn->GetWriteBuffer());
  // TODO: for debug purpose
  conn->GetWriteBuffer().Append(body);
  req->Reset();
}

void HTTPServer::DoRequestError(HTTPConn* conn, int errcode) {
  
}

}  // namespace http
}  // namespace ahrimq