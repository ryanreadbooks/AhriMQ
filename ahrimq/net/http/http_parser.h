#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include "buffer/buffer.h"
#include "net/http/http_conn.h"

namespace ahrimq {
namespace http {

class HTTPConn;

const static char* kCRLF = "\r\n";

/// @brief indicates the current state when parsing http request datagram
enum class RequestParsingState { Invalid, Line, Header, Empty, Body };

void ParseRequestLine(HTTPConn* conn);

void ParseRequestHeader(HTTPConn* conn);

void ParseRequestEmpty(HTTPConn* conn);

void ParseRequestBody(HTTPConn* conn);

void ParseRequestDatagram(HTTPConn* conn);

}  // namespace http

}  // namespace ahrimq

#endif  // _HTTP_PARSER_H_