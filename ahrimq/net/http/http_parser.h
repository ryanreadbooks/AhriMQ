#ifndef _AHRIMQ_NET_HTTP_HTTP_PARSER_H_
#define _AHRIMQ_NET_HTTP_HTTP_PARSER_H_

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

/// @brief This enum class represents the operation result after parsing.
enum class ParsingResCode { Complete, Pending, Invalid };

LineParsingState ParseSingleLine(HTTPConn* conn, std::string& line);

int ParseRequestLine(HTTPConn* conn);

int ParseRequestHeader(HTTPConn* conn);

int ParseRequestEmptyLine(HTTPConn* conn);

int ParseRequestBody(HTTPConn* conn);

/// @brief Parse a http request stream.
/// @param conn the http connection response
/// @return the parsing result code: possible result codes are : 6xx(for private
/// internal usage), 4xx(standard http response code), 5xx(standard http response
/// code)
int ParseRequestDatagram(HTTPConn* conn);

}  // namespace http

}  // namespace ahrimq

#endif  // _AHRIMQ_NET_HTTP_HTTP_PARSER_H_