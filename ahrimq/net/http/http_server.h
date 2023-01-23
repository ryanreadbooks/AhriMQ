#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <memory>

#include "base/nocopyable.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "net/iserver.h"
#include "net/reactor_conn.h"
#include "net/tcp/tcp_server.h"
#include "net/http/http_conn.h"

namespace ahrimq {
namespace http {

#define DEFAULT_HTTP_SERVER_IP "127.0.0.1"
#define DEFAULT_HTTP_PORT 80

typedef std::function<void(const HTTPRequest&, HTTPResponsePtr)> HTTPCallback;

/// @brief HTTPServer implements a minimum HTTP/1.1 server
class HTTPServer : public NoCopyable, public IServer {
 public:
  /// @brief HTTP Server configuration
  class Config : public ahrimq::TCPServer::Config {
   public:
    // HTTP keepalive option
    bool http_keepalive;
    // indicate HTTPS
    bool http_secure;  // (reserved)
  };

 public:
  /// @brief construct a HTTP server
  HTTPServer();

  /// @brief construct a HTTP server with given configuration
  /// @param config
  HTTPServer(const HTTPServer::Config& config);

  /// @brief construct a HTTP server with given configuration and HTTP callback
  /// function
  /// @param config
  /// @param cb
  HTTPServer(const HTTPServer::Config& config, HTTPCallback cb);

  ~HTTPServer();

  void Run() override;

  void Stop() override;

 protected:
  void InitReactorHandlers() override;

  void InitHTTPServer();

  void OnStreamOpen(ReactorConn* conn) override;

  void OnStreamReached(ReactorConn* conn, bool allread) override;

  void OnStreamClosed(ReactorConn* conn) override;

  void OnStreamWritten(ReactorConn* conn) override;

 private:
  // HTTP config
  HTTPServer::Config config_;
  // HTTP Callback
  HTTPCallback on_request_cb_;
  // all HTTP connections
  std::unordered_map<std::string, HTTPConnPtr> httpconns_;
};

typedef std::shared_ptr<HTTPServer> HTTPServerPtr;
typedef HTTPServer::Config HTTPServerConfig;
typedef std::shared_ptr<HTTPServer::Config> HTTPServerConfigPtr;

static HTTPServer::Config defaultHTTPConfig;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_SERVER_H_