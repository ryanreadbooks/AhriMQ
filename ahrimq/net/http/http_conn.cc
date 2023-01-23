#include "net/http/http_conn.h"

namespace ahrimq {
namespace http {

HTTPConn::HTTPConn(ReactorConn* conn)
    : TCPConn(conn), current_parsing_state_(RequestParsingState::Line) {}

HTTPConn::~HTTPConn() {}

}  // namespace http
}  // namespace ahrimq