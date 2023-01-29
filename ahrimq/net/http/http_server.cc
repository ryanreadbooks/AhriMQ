#include "net/http/http_server.h"

using std::placeholders::_1;  // _1, _2, ...
using std::placeholders::_2;
using std::placeholders::_3;

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
  reactor_->SetEventAcceptHandler(
      std::bind(&HTTPServer::OnStreamOpen, this, _1, _2));
  reactor_->SetEventReadHandler(
      std::bind(&HTTPServer::OnStreamReached, this, _1, _2, _3));
  reactor_->SetEventCloseHandler(
      std::bind(&HTTPServer::OnStreamClosed, this, _1, _2));
  reactor_->SetEventWriteHandler(
      std::bind(&HTTPServer::OnStreamWritten, this, _1, _2));
}

void HTTPServer::InitHTTPServer() {
  assert(reactor_ != nullptr);  // FIXME: optimize error handling
  InitReactorHandlers();
}

void HTTPServer::OnStreamOpen(ReactorConn* conn, bool& close_after) {
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

void HTTPServer::OnStreamReached(ReactorConn* conn, bool allread,
                                 bool& close_after) {
  std::string conn_name = conn->GetName();
  HTTPConnPtr httpconn = httpconns_[conn_name];
StartParsingRequestDatagramTag:
  int retcode = ParseRequestDatagram(httpconn.get());
  if (retcode == StatusPrivatePending) {
    // In pending state, we do not need to send response
    return;
  } else if (retcode == StatusPrivateDone) {
    // do request
    DoRequest(httpconn.get());
  } else {
    // request datagram is abnormal, we need to do error handling
    if (retcode == StatusPrivateInvalid) {
      retcode = StatusBadRequest;
    }
    DoRequestError(httpconn.get(), retcode);
  }
  // send all response data out to client
  // TODO consider the situation where http request pipelining is needed
  // support http request pipelining
  httpconn->Send();
}

void HTTPServer::OnStreamClosed(ReactorConn* conn, bool& close_after) {
  // TODO handle connection close by reusing connections
  std::string conn_name = conn->GetName();
  httpconns_.erase(conn_name);
  std::cout << "HTTP connection " << conn_name << " closed!\n";
}

void HTTPServer::OnStreamWritten(ReactorConn* conn, bool& close_after) {
  std::string conn_name = conn->GetName();
  HTTPConnPtr httpconn = httpconns_[conn_name];
  // keepalive handling
  HTTPHeaderPtr& res_header = httpconn->CurrentResponseRef()->HeaderRef();
  if (!res_header->Has("Connection") ||
      res_header->Get("Connection") != "keep-alive") {
    // no keep-alive option used, we need to close the http connection
    close_after = true;  // let reactor help us close the underneath tcp connection
    httpconns_.erase(conn_name);
  } else {
    // the http connection is kept
    httpconn->CurrentRequestRef()->Reset();
    httpconn->CurrentResponseRef()->Reset();
  }
  std::cout << "HTTPServer::OnStreamWritten, Request and Response reset\n";
}

void HTTPServer::DoRequest(HTTPConn* conn) {
  HTTPRequestPtr& req = conn->CurrentRequestRef();
  HTTPHeaderPtr& req_header = req->HeaderRef();
  const URL& url = req->URLRef();
  std::cout << "Requested url: " << url << std::endl;
  // TODO debug the query parameters;
  std::cout << "url query is below: \n";
  std::cout << req->Query() << std::endl;
  // set http response data
  HTTPResponsePtr& res = conn->CurrentResponseRef();
  HTTPHeaderPtr& res_header = res->HeaderRef();
  if (!req_header->Equals("Connection", "keep-alive")) {
    res_header->Add("Connection", "close");
  } else {
    res_header->Add("Connection", "keep-alive");
  }
  // routing
  DoRouting(conn);

  // add response data
  // TODO: below is for debug purposes
  res->SetStatus(StatusOK);
  std::string body = "{\"name\" : \"ryanreadbooks\"}";
  size_t len = body.size();
  res_header->Add("Content-Length", std::to_string(len));
  res_header->Add("Content-Type", "application/json");
  res->Organize(conn->GetWriteBuffer());
  conn->GetWriteBuffer().Append(body);

  req->Reset();
}

// handle http request error state and create response
void HTTPServer::DoRequestError(HTTPConn* conn, int errcode) {
  // identify the kind of errcode
  int r = IdentifyStatusCode(errcode);
  int final_status_code;
  if (r == STATUS_CODE_INTERNAL_USAGE || r == STATUS_CODE_INVALID) {
    // we treat this kind of err as server internal error(500)
    final_status_code = StatusInternalServerError;
  } else {
    // STATUS_CODE_HTTP_STANDARD
    final_status_code = errcode;
  }
  // set http response data
  HTTPResponsePtr& res = conn->CurrentResponseRef();
  HTTPHeaderPtr& res_header = res->HeaderRef();
  res->SetStatus(final_status_code);
  if (IdentifyStatusCodeNeedCloseConnection(final_status_code)) {
    res_header->Add("Connection", "close");
  }
}

// TODO route tracing and call corresponding user-specific methods
void HTTPServer::DoRouting(HTTPConn* conn) {}

}  // namespace http
}  // namespace ahrimq