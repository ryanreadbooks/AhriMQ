#include "net/http/http_conn.h"

namespace ahrimq {
namespace http {

HTTPConn::HTTPConn(ReactorConn* conn)
    : TCPConn(conn),
      current_parsing_state_(RequestParsingState::RequestLine),
      current_line_state_(LineParsingState::LineComplete) {
  current_request_ = std::make_shared<HTTPRequest>(&read_buf_);
  current_response_ = std::make_shared<HTTPResponse>(&write_buf_);
}

HTTPConn::~HTTPConn() {
  current_request_.reset();
  current_response_.reset();
}

}  // namespace http
}  // namespace ahrimq