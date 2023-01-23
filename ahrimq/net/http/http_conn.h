#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_

#include "net/http/http_parser.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "net/tcp/tcp_conn.h"

namespace ahrimq {
namespace http {

class HTTPServer;

/// @brief HTTPConn represents a http connection over tcp connection
class HTTPConn : public TCPConn {
  friend class HTTPServer;

 public:
  /// @brief construct a http connection instance
  /// @param conn
  explicit HTTPConn(ReactorConn* conn);

  ~HTTPConn();

  RequestParsingState GetCurrentParsingState() const {
    return current_parsing_state_;
  }

  HTTPRequestPtr GetCurrentRequest() const {
    return current_request_;
  }

  HTTPResponsePtr GetCurrentResponse() const {
    return current_response_;
  }

 private:
  // which state this HTTP connection is at when parsing request datagram
  RequestParsingState current_parsing_state_;
  // current HTTP request
  HTTPRequestPtr current_request_;
  // current HTTP response
  HTTPResponsePtr current_response_;
};

typedef std::shared_ptr<HTTPConn> HTTPConnPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTPCONN_H_