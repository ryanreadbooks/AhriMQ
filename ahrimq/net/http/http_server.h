#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <memory>

#include "base/nocopyable.h"
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

/// @brief HTTPCallback is the callback function for handling http request.
/// HTTPCallback will take http request instance const reference and http response
/// instance as arguments and should return a page name as response page.
// typedef std::function<std::string(const HTTPRequest&, HTTPResponse&)> HTTPCallback;

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

 private:
  // HTTP config
  HTTPServer::Config config_;
  // all HTTP connections
  std::unordered_map<std::string, HTTPConnPtr> httpconns_;
  // http router
  HTTPRouter router_;
};

typedef std::shared_ptr<HTTPServer> HTTPServerPtr;
typedef HTTPServer::Config HTTPServerConfig;
typedef std::shared_ptr<HTTPServer::Config> HTTPServerConfigPtr;

// must be static
static HTTPServer::Config defaultHTTPConfig;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_SERVER_H_