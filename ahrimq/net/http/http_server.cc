#include "net/http/http_server.h"

#include <atomic>
#include <chrono>

#include "mime/mime.h"

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

HTTPServer::~HTTPServer() {
  stopped_.store(true, std::memory_order_relaxed);
  reactor_.reset();
}

// TODO improve Run and Stop protocol
void HTTPServer::Run() {
  assert(reactor_ != nullptr);
  reactor_->React();
  reactor_->Wait();
}

void HTTPServer::Stop() {
  stopped_.store(true, std::memory_order_relaxed);
  reactor_->Stop();
}

void HTTPServer::InitReactorHandlers() {
  reactor_->SetEventAcceptHandler([this](ReactorConn* conn, bool allread) {
    this->OnStreamOpen(conn, allread);
  });

  reactor_->SetEventReadHandler(
      [this](ReactorConn* conn, bool allread, bool& close_after) {
        this->OnStreamReached(conn, allread, close_after);
      });

  reactor_->SetEventCloseHandler([this](ReactorConn* conn, bool& close_after) {
    this->OnStreamClosed(conn, close_after);
  });

  reactor_->SetEventWriteHandler([this](ReactorConn* conn, bool& close_after) {
    this->OnStreamWritten(conn, close_after);
  });
}

bool HTTPServer::Get(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterGet(pattern, callback);
}

bool HTTPServer::Head(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterHead(pattern, callback);
}

bool HTTPServer::Post(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterPost(pattern, callback);
}

bool HTTPServer::Put(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterPut(pattern, callback);
}

bool HTTPServer::Patch(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterPatch(pattern, callback);
}

bool HTTPServer::Delete(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterDelete(pattern, callback);
}

bool HTTPServer::Connect(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterConnect(pattern, callback);
}

bool HTTPServer::Options(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterOptions(pattern, callback);
}

bool HTTPServer::Trace(const std::string& pattern, const HTTPCallback& callback) {
  return router_.RegisterTrace(pattern, callback);
}

void HTTPServer::InitHTTPServer() {
  assert(reactor_ != nullptr);
  InitReactorHandlers();
  InitErrHandler();
  InitCleanup();
  stopped_.store(false);
}

void HTTPServer::InitErrHandler() {
  err_handlers_[StatusBadRequest] = Default400Handler;
  err_handlers_[StatusNotFound] = Default404Handler;
  err_handlers_[StatusMethodNotAllowed] = Default405Handler;
  err_handlers_[StatusInternalServerError] = Default500Handler;
}

void HTTPServer::InitCleanup() {
  std::thread th(std::bind(&HTTPServer::Cleanup, this));
  cleanup_wrk_.swap(th);
  cleanup_wrk_.detach();
}

void HTTPServer::OnStreamOpen(ReactorConn* conn, bool& close_after) {
  std::string conn_name = conn->GetName();
  // create a new http connection instance
  HTTPConnPtr httpconn = std::make_shared<HTTPConn>(conn);
  if (httpconn == nullptr) {
    printf("can not create http conn instance for tcp conn %s\n", conn_name.c_str());
    // can not open http connection
    close_after = true;
    return;
  }
  httpconn->SetTCPKeepAlive(config_.tcp_keepalive);
  httpconn->SetTCPKeepAlivePeriod(config_.tcp_keepalive_period);
  httpconn->SetTCPKeepAliveCount(config_.tcp_keepalive_count);
  httpconn->SetTCPNoDelay(config_.tcp_nodelay);
  mtx_.lock();
  httpconns_.insert({conn_name, httpconn});
  mtx_.unlock();
  conn->SetReadBuffer(&httpconn->read_buf_);
  conn->SetWriteBuffer(&httpconn->write_buf_);
#ifdef AHRIMQ_DEBUG
  // printf("HTTP connection %s opened\n", conn_name.c_str());
#endif
}

// ATTENTION!! this method may be invoked in multiple threads
void HTTPServer::OnStreamReached(ReactorConn* conn, bool allread,
                                 bool& close_after) {
  std::string conn_name = conn->GetName();
  // get http connection
  mtx_.lock();
  HTTPConnPtr httpconn = httpconns_[conn_name];
  if (httpconn == nullptr) {
    // can not find http connection instance in existing http connections
    // simply close the underlying tcp connection
    close_after = true;
    mtx_.unlock();
    return;
  }
  mtx_.unlock();

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
  // centralized error handler processing
  CentrailzedStatusCodeHandling(httpconn.get());

  // send all response data out to client
  // TODO consider the situation where http request pipelining is needed
  httpconn->CurrentResponseRef()->Organize(httpconn->GetWriteBuffer());
  httpconn->CurrentRequestRef()->Reset();
  httpconn->Send();
}

// ATTENTION!! this method may be invoked in multiple threads
void HTTPServer::OnStreamClosed(ReactorConn* conn, bool& close_after) {
  // TODO handle connection close by reusing connections
  std::string conn_name = conn->GetName();
  mtx_.lock();
  httpconns_.erase(conn_name);
  mtx_.unlock();
#ifdef AHRIMQ_DEBUG
  // printf("HTTP connection %s closed!\n", conn_name.c_str());
#endif
}

// ATTENTION!! this method may be invoked in multiple threads
void HTTPServer::OnStreamWritten(ReactorConn* conn, bool& close_after) {
  std::string conn_name = conn->GetName();
  mtx_.lock();
  HTTPConnPtr httpconn = httpconns_[conn_name];
  mtx_.unlock();
  // keepalive handling
  HTTPHeaderPtr& res_header = httpconn->CurrentResponseRef()->HeaderRef();
  if (!res_header->Has("Connection") ||
      res_header->Get("Connection") != "keep-alive") {
    // no keep-alive option used, we need to close the http connection
    close_after = true;  // let reactor help us close the underneath tcp connection
    mtx_.lock();
    httpconns_.erase(conn_name);
    mtx_.unlock();
  } else {
    // the http connection is kept
    httpconn->CurrentRequestRef()->Reset();
    httpconn->CurrentResponseRef()->Reset();
  }
#ifdef AHRIMQ_DEBUG
  // printf("HTTPServer::OnStreamWritten, Request and Response reset\n");
#endif
}

void HTTPServer::DoRequest(HTTPConn* conn) {
  HTTPRequestPtr& req = conn->CurrentRequestRef();
  HTTPHeaderPtr& req_header = req->HeaderRef();

  // handle some special request data
  HTTPResponsePtr& res = conn->CurrentResponseRef();
  HTTPHeaderPtr& res_header = res->HeaderRef();
  // Connection behaviour
  if (!req_header->Equals("Connection", "keep-alive")) {
    res_header->Add("Connection", "close");
  } else {
    res_header->Add("Connection", "keep-alive");
  }
  auto m = req->Method();
  int status_code = StatusPrivateDone;
  if (m == HTTPMethod::Post || m == HTTPMethod::Put || m == HTTPMethod::Patch) {
    // we should parse request body here
    status_code = req->ParseForm();
  }
  if (status_code == StatusPrivateDone) {
    // ROUTING !!!!
    std::string response_page = DoRouting(conn);
    if (!response_page.empty()) {
      // if response_page is not empty, we do file sending operation
      // and any existing write buffer content should be clear to avoid confilce
      res->UserBuffer().Reset();
      std::string response_page_fullpath;
      PathJoin(config_.root, response_page, response_page_fullpath);
      // then open response page file
      int ofd = OpenFile(response_page_fullpath);
      if (ofd == -1) {
        std::cerr << "can not open file " << response_page_fullpath << ". ["
                  << std::strerror(errno) << "]\n";
        if (errno == ENOENT) {
          status_code = StatusNotFound;
        } else {
          status_code = StatusInternalServerError;
        }
        goto do_request_error;
      } else {
        // found
        ReactorConn* rc = conn->conn_;
        if (rc->PutFile(ofd, false) == false) {
          // treat it as 404
          status_code = StatusNotFound;
          goto do_request_error;
        }
        // put file ok
        // set some corresponding response header
        res_header->Set("Content-Length", std::to_string(rc->FileSize()));
        // content-type
        res->SetContentType(
            mime::DecideMimeTypeFromExtension(response_page_fullpath));
        // TODO: content-encoding
        res->SetStatus(StatusOK);
      }
    }
    return;
  }
do_request_error:
  // still has error
  DoRequestError(conn, status_code);
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
    // final errcode could be 400, 413, 500, 501, 505 here
    final_status_code = errcode;
  }
  // set http response data header, response body is handled in function
  // CentrailzedStatusCodeHandling
  HTTPResponsePtr& res = conn->CurrentResponseRef();
  HTTPHeaderPtr& res_header = res->HeaderRef();
  res->SetStatus(final_status_code);
  if (IdentifyStatusCodeNeedCloseConnection(final_status_code)) {
    res_header->Add("Connection", "close");
  }
}

std::string HTTPServer::DoRouting(HTTPConn* conn) {
  HTTPRequestPtr& req_ref = conn->CurrentRequestRef();
  HTTPResponsePtr& res_ref = conn->CurrentResponseRef();
  std::string path = conn->CurrentRequestRef()->URLRef().StringWithQuery();
  // use http router to decide which handler callback function should be invoked.
  return router_.Route(req_ref->Method(), path, *req_ref, *res_ref);
}

void HTTPServer::CentrailzedStatusCodeHandling(HTTPConn* conn) {
  auto req = conn->CurrentRequestRef();
  auto res = conn->CurrentResponseRef();
  if (err_handlers_.count(res->Status()) != 0) {
    auto cb = err_handlers_[res->Status()];
    cb(*req, *res);
  }
}

void HTTPServer::Default400Handler(const HTTPRequest& req, HTTPResponse& res) {
  res.SetStatus(StatusBadRequest);
  res.MakeContentSimpleHTML(DEFAULT_400_PAGE);
}

void HTTPServer::Default404Handler(const HTTPRequest& req, HTTPResponse& res) {
  res.SetStatus(StatusNotFound);
  res.MakeContentSimpleHTML(DEFAULT_404_PAGE);
}

void HTTPServer::Default405Handler(const HTTPRequest& req, HTTPResponse& res) {
  res.SetStatus(StatusMethodNotAllowed);
  res.MakeContentSimpleHTML(DEFAULT_405_PAGE);
}

void HTTPServer::Default500Handler(const HTTPRequest& req, HTTPResponse& res) {
  res.SetStatus(StatusInternalServerError);
  res.MakeContentSimpleHTML(DEFAULT_500_PAGE);
}

// ATTENTION!! this method may be invoked in multiple threads
int HTTPServer::OpenFile(const std::string& filename) {
  std::lock_guard<std::mutex> lck(mtx_);
  int fd = -1;
  if (file_mappings_.count(filename) == 0) {
    // can not find in cache
    // we need to open file
    fd = open(filename.c_str(), O_RDONLY);
    if (fd != -1) {
      file_mappings_.insert({filename, std::make_shared<_OpenedFileStatus>(fd)});
    }
  } else {
    // in cache
    fd = file_mappings_[filename]->fd;
    file_mappings_[filename]->count++;
    file_mappings_[filename]->last = time::GetCurrentSec();
  }
  return fd;
}

void HTTPServer::Cleanup() {
  // this function works in background executing clean-up operation periodically
  while (!stopped_) {
    std::unique_lock<std::mutex> lck(mtx_);
    // wait up every 10 seconds
    cond_.wait_for(lck, std::chrono::seconds(10),
                   [this] { return stopped_.load(); });
    // we have lock here
    // 1. randomly close some opened files
    for (auto it = file_mappings_.begin(); it != file_mappings_.end();) {
      if (it->second->NeedClose()) {
        it = file_mappings_.erase(it);
      } else {
        it++;
      }
    }

    // TODO 2. close idle http connections
  }
}

}  // namespace http
}  // namespace ahrimq