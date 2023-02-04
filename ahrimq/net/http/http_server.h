#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <atomic>
#include <memory>

#include "base/nocopyable.h"
#include "base/time_utils.h"
#include "net/http/http_conn.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "net/http/http_router.h"
#include "net/iserver.h"
#include "net/reactor_conn.h"
#include "net/tcp/tcp_server.h"

namespace ahrimq {
namespace http {

#define DEFAULT_HTTP_SERVER_IP "127.0.0.1"
#define DEFAULT_HTTP_PORT 80

/// @brief HTTPServer implements a minimum HTTP/1.1 server
class HTTPServer : public NoCopyable, public IServer {
 public:
  /// @brief HTTP Server configuration
  class Config : public ahrimq::TCPServer::Config {
   public:
    // http server root path
    std::string root;
    // indicate HTTPS
    bool _http_secure;  // (reserved)
  };

 public:
  /// @brief Construct a HTTP server.
  HTTPServer();

  /// @brief Construct a HTTP server with given configuration.
  /// @param config
  HTTPServer(const HTTPServer::Config& config);

  ~HTTPServer();

  /// @brief Start the server.
  void Run() override;

  /// @brief Stop the server.
  void Stop() override;

  /// @brief Add callback function for http GET method on given url pattern.
  /// @param pattern url pattern
  /// @param callback the callback function to handle url pattern
  /// @return true on success, false on failure(already exists callback fucntion on
  /// pattern or other error)
  bool Get(const std::string& pattern, const HTTPCallback& callback);

  /// @brief Add callback function for http HEAD method on given url pattern.
  /// @param pattern
  /// @param callback
  /// @return
  bool Head(const std::string& pattern, const HTTPCallback& callback);

  /// @brief Add callback function for http POST method on given url pattern.
  /// @param pattern url pattern
  /// @param callback the callback function to handle url pattern
  /// @return true on success, false on failure(already exists callback fucntion on
  /// pattern or other error)
  bool Post(const std::string& pattern, const HTTPCallback& callback);

  bool Put(const std::string& pattern, const HTTPCallback& callback);

  bool Patch(const std::string& pattern, const HTTPCallback& callback);

  bool Delete(const std::string& pattern, const HTTPCallback& callback);

  bool Connect(const std::string& pattern, const HTTPCallback& callback);

  bool Options(const std::string& pattern, const HTTPCallback& callback);

  bool Trace(const std::string& pattern, const HTTPCallback& callback);

 protected:
  void InitReactorHandlers() override;

  void InitHTTPServer();

  void InitErrHandler();

  void InitCleanup();

  void OnStreamOpen(ReactorConn* conn, bool& close_after) override;

  void OnStreamReached(ReactorConn* conn, bool allread, bool& close_after) override;

  void OnStreamClosed(ReactorConn* conn, bool& close_after) override;

  void OnStreamWritten(ReactorConn* conn, bool& close_after) override;

  /// @brief Handle one single http request, and organize http response.
  /// @param conn
  void DoRequest(HTTPConn* conn);

  /// @brief Handle one single http invalid request and organize http response.
  /// @param conn
  /// @param errcode
  void DoRequestError(HTTPConn* conn, int errcode);

  std::string DoRouting(HTTPConn* conn);

  void CentrailzedStatusCodeHandling(HTTPConn* conn);

  static void Default400Handler(const HTTPRequest& req, HTTPResponse& res);

  static void Default404Handler(const HTTPRequest& req, HTTPResponse& res);

  static void Default405Handler(const HTTPRequest& req, HTTPResponse& res);

  static void Default500Handler(const HTTPRequest& req, HTTPResponse& res);

  static void DefaultErrHandler(const HTTPRequest& req, HTTPResponse& res);

  int OpenFile(const std::string& filename);

  void Cleanup();

 private:
  typedef std::function<void(const HTTPRequest&, HTTPResponse&)>
      InternHTTPErrHandler;
  // HTTP config
  HTTPServer::Config config_;
  // all HTTP connections
  std::unordered_map<std::string, HTTPConnPtr> httpconns_;
  // http router
  HTTPRouter router_;
  // default error status code handlers
  std::unordered_map<int, InternHTTPErrHandler> err_handlers_;
  
  // file name and its status fd
  struct _OpenedFileStatus {
    size_t count;
    uint64_t last;
    int fd;
    _OpenedFileStatus(int f) : count(1), last(GetCurrentSec()), fd(f) {}

    ~_OpenedFileStatus() {
      if (close(fd) != -1) {
        printf("file-%d closed\n", fd);
      }
    }

    /// @brief Define a simple rule to close opened file. If opened file is not used
    /// within 60 seconds, then we close it.
    /// @return
    bool NeedClose() {
      return GetCurrentSec() - last >= 60;
      ;
    }
  };
  typedef std::shared_ptr<_OpenedFileStatus> _OpenedFileStatusPtr;

  std::unordered_map<std::string, _OpenedFileStatusPtr> file_mappings_;
  // cleaner worker
  std::thread cleanup_wrk_;
};

typedef std::shared_ptr<HTTPServer> HTTPServerPtr;
typedef HTTPServer::Config HTTPServerConfig;
typedef std::shared_ptr<HTTPServer::Config> HTTPServerConfigPtr;

// must be static
static HTTPServer::Config defaultHTTPConfig;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_SERVER_H_