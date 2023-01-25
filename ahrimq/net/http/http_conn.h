#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_

#include "net/http/http_parser.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "net/tcp/tcp_conn.h"

namespace ahrimq {
namespace http {

enum class RequestParsingState;
enum class LineParsingState;
class HTTPServer;
class HTTPConn;

/// @brief indicates the current state when parsing http request datagram, current
/// state means expected data to see next
///   Invalid: Request parsing enounters error
///
///   RequestLine: When parsing request line, this state is set
///
///   RequestHeader: When parsing request header, this state is set
///
///   RequestEmptyLine: When parsing request empty line, this state is set. This
///   state is quite rare because request empty line is often parsed in RequestHeader
///   state
///
///   RequestBody: When parsing request body, this state is set.
///
///   Done: When a request is full parsed, this state is set.
enum class RequestParsingState {
  Invalid,
  RequestLine,
  RequestHeader,
  RequestEmptyLine,
  RequestBody,
  Done
};

/// @brief this enum class indicates the line parsing status
enum class LineParsingState {
  LineComplete,
  LinePending,
  LineInvalid,
  LineCompleteEmptyLine
};

/// @brief HTTPConn represents a http connection over tcp connection
class HTTPConn : public TCPConn {
  friend class HTTPServer;

 public:
  /// @brief construct a http connection instance
  /// @param conn
  explicit HTTPConn(ReactorConn* conn);

  ~HTTPConn();

  void SetCurrentParsingState(RequestParsingState state) {
    current_parsing_state_ = state;
  }

  void SetCurrentParsingStateLine() {
    SetCurrentParsingState(RequestParsingState::RequestLine);
  }

  void SetCurrentParsingStateHeader() {
    SetCurrentParsingState(RequestParsingState::RequestHeader);
  }

  void SetCurrentParsingStateEmptyLine() {
    SetCurrentParsingState(RequestParsingState::RequestEmptyLine);
  }

  void SetCurrentParsingStateBody() {
    SetCurrentParsingState(RequestParsingState::RequestBody);
  }

  void SetCurrentParsingStateInvalid() {
    SetCurrentParsingState(RequestParsingState::Invalid);
  }

  void SetCurrentParsingStateDone() {
    SetCurrentParsingState(RequestParsingState::Done);
  }

  RequestParsingState GetCurrentParsingState() const {
    return current_parsing_state_;
  }

  /// @brief return a copy of current http request shared pointer
  /// @return
  HTTPRequestPtr GetCurrentRequest() const {
    return current_request_;
  }

  /// @brief return the reference of current http request
  /// @return
  HTTPRequestPtr& CurrentRequestRef() {
    return current_request_;
  }

  /// @brief return a copy of current http response shared pointer
  /// @return
  HTTPResponsePtr GetCurrentResponse() const {
    return current_response_;
  }

  /// @brief return the reference of current http request
  /// @return
  HTTPResponsePtr& CurrentResponseRef() {
    return current_response_;
  }

  bool CurrentRequestIsNull() const {
    return current_request_ == nullptr;
  }

  bool CurrentResponseIsNull() const {
    return current_response_ == nullptr;
  }

 private:
  // the state this HTTP connection is at when parsing request datagram
  RequestParsingState current_parsing_state_;
  // the state this HTTP connection is at when parsing line
  LineParsingState current_line_state_;
  // current HTTP request
  HTTPRequestPtr current_request_;
  // current HTTP response
  HTTPResponsePtr current_response_;
  // request instance inited
  bool request_inited_;
};

typedef std::shared_ptr<HTTPConn> HTTPConnPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTPCONN_H_