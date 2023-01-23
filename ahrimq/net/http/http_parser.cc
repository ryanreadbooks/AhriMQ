#include "net/http/http_parser.h"

namespace ahrimq {
namespace http {
  
void ParseRequestLine(HTTPConn* conn) {

}

void ParseRequestHeader(HTTPConn* conn) {

}

void ParseRequestEmpty(HTTPConn* conn) {

}

void ParseRequestBody(HTTPConn* conn) {

}

void ParseRequestDatagram(HTTPConn* conn) {
  if (conn == nullptr) {
    return;
  }
  RequestParsingState state = conn->GetCurrentParsingState();
}

}// namespace ahrimq
} // namespace ahrimq