#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <memory>

#include "base/nocopyable.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "net/iserver.h"
#include "net/reactor_conn.h"
#include "net/tcp/tcp_server.h"

namespace ahrimq {
namespace http {

#define DEFAULT_HTTP_SERVER_IP "127.0.0.1"
#define DEFAULT_HTTP_PORT 80

typedef std::function<void(const HTTPRequest&, HTTPResponsePtr)> HTTPCallback;

/// @brief HTTPServer implements a minimum HTTP/1.1 server
class HTTPServer : public NoCopyable, public IServer {
 public:
  class Config : public ahrimq::TCPServer::Config {
   public:
    // HTTP keepalive option
    bool http_keepalive;
    // indicate HTTPS
    bool http_secure;  // (reserved)
  };

 public:
  HTTPServer();

  HTTPServer(const HTTPServer::Config& config);

  HTTPServer(const HTTPServer::Config& config, HTTPCallback cb);

  ~HTTPServer();

  void Run() override;

  void Stop() override;

 protected:
  void InitReactorConfigs() override;

  void InitReactorHandlers() override;

  void InitHTTPServer();

  void OnStreamOpen(ReactorConn* conn) override;

  void OnStreamReached(ReactorConn* conn, bool all_been_read) override;

  void OnStreamClosed(ReactorConn* conn) override;

  void OnStreamWritten(ReactorConn* conn) override;

 private:
  // ReactorPtr reactor_;
  // http config
  HTTPServer::Config config_;
  // HTTP Callback
  HTTPCallback on_request_cb_;
};

typedef std::shared_ptr<HTTPServer> HTTPServerPtr;
typedef HTTPServer::Config HTTPServerConfig;
typedef std::shared_ptr<HTTPServer::Config> HTTPServerPtr;

HTTPServerConfig defaultHTTPConfig;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_SERVER_H_