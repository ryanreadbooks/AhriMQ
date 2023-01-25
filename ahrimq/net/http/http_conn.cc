#include "net/http/http_conn.h"

namespace ahrimq {
namespace http {

HTTPConn::HTTPConn(ReactorConn* conn)
    : TCPConn(conn),
      current_parsing_state_(RequestParsingState::RequestLine),
      current_line_state_(LineParsingState::LineComplete) {
  current_request_ = std::make_shared<HTTPRequest>();
  request_inited_ = true;
}

HTTPConn::~HTTPConn() {}

}  // namespace http
}  // namespace ahrimq