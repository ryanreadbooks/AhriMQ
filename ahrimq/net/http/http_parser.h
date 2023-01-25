#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include "base/str_utils.h"
#include "buffer/buffer.h"
#include "net/http/http_conn.h"
#include "net/http/http_header.h"
#include "net/http/http_method.h"
#include "net/http/http_request.h"
#include "net/http/http_status.h"
#include "net/http/http_version.h"

namespace ahrimq {
namespace http {

class HTTPConn;
class HTTPRequest;
enum class RequestParsingState;
enum class LineParsingState;

const static char* kCRLF = "\r\n";

/// @brief this enum class represents the operation result after parsing
enum class ParsingResCode {
  Complete,
  Pending,
  Invalid
};

LineParsingState ParseSingleLine(HTTPConn* conn, std::string& line);

int ParseRequestLine(HTTPConn* conn);

int ParseRequestHeader(HTTPConn* conn);

int ParseRequestEmptyLine(HTTPConn* conn);

int ParseRequestBody(HTTPConn* conn);

int ParseRequestDatagram(HTTPConn* conn);

}  // namespace http

}  // namespace ahrimq

#endif  // _HTTP_PARSER_H_