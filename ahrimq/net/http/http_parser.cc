#include "net/http/http_parser.h"

#include <algorithm>

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

// line store the parsed single line as result
// CRLF is line seperator
LineParsingState ParseSingleLine(HTTPConn* conn, std::string& line) {
  Buffer& rbuf = conn->GetReadBuffer();
  if (rbuf.Size() == 0) {
    return LineParsingState::LinePending;
  }
  // line.replace(line.begin(), line.end(), rbuf.ReadStringAndForwardTill(kCRLF));
  bool crlf_found;
  std::string tmp = rbuf.ReadStringAndForwardTill(kCRLF, crlf_found);
  if (!crlf_found) {
    return LineParsingState::LinePending;
  }
  line.replace(line.begin(), line.end(), std::move(tmp));
  // found
  if (line.empty()) {
    // CRLF is at the begining of the buffer
    // so the parsed line is an empty line
    return LineParsingState::LineCompleteEmptyLine;
  }

  return LineParsingState::LineComplete;
}

int ParseRequestLine(HTTPConn* conn) {
  Buffer& rbuf = conn->GetReadBuffer();
  // we should always ignore any leading CRLF when parsing request line
  rbuf.TrimLeft();
  std::string request_line;
  LineParsingState line_state = ParseSingleLine(conn, request_line);
  if (line_state == LineParsingState::LinePending ||
      line_state == LineParsingState::LineCompleteEmptyLine) {
    return StatusPrivatePending;
  }
  // check request line
  std::vector<std::string> v = StrSplit(request_line, ' ');
  // HTTPMethod Request-URL HTTPVersion
  if (v.size() != 3) {  // syntax error
    conn->SetCurrentParsingStateInvalid();
    std::cerr << "ParseRequestLine " << StatusBadRequest << '\n';
    return StatusBadRequest;  // 400
  }

  HTTPRequestPtr& req_ref = conn->CurrentRequestRef();
  // method
  const std::string& request_method = v[0];
  if (!HTTPMethodSupported(request_method)) {
    conn->SetCurrentParsingStateInvalid();  // invalidate
    std::cerr << "ParseRequestLine " << StatusNotImplemented << '\n';
    return StatusNotImplemented;  // 501
  }
  req_ref->SetMethod(request_method);

  // url
  const std::string& request_url = v[1];
  req_ref->SetURL(request_url);

  // version
  const std::string& request_http_version = v[2];
  int httpver = ParseHTTPVersion(request_http_version);
  if (!HTTPVersionSupported(httpver)) {
    conn->SetCurrentParsingStateInvalid();  // invalidate
    std::cerr << "ParseRequestLine " << StatusHTTPVersionNotSupported << '\n';
    return StatusHTTPVersionNotSupported;  // 505
  }
  req_ref->SetHTTPVersion(httpver);

  conn->SetCurrentParsingStateHeader();  // we expect to parse request headers next
  return StatusPrivateComplete;
}

int ParseRequestHeader(HTTPConn* conn) {
  Buffer& rbuf = conn->GetReadBuffer();
  HTTPRequestPtr& req_ref = conn->CurrentRequestRef();
  HTTPHeaderPtr& header_ref = req_ref->HeaderRef();
  LineParsingState line_state = LineParsingState::LineComplete;
  std::string field;

  while ((line_state = ParseSingleLine(conn, field)) ==
         LineParsingState::LineComplete) {
    // parse field-key and field-value
    auto where_is_colon = std::find(field.begin(), field.end(), ':');
    if (where_is_colon != field.end()) {
      // found colon-seperator(':'), we assume this field is value;
      std::string field_key = std::string(field.begin(), where_is_colon);
      std::string field_value = std::string(where_is_colon + 1, field.end());
      field_value = StrTrimLeft(field_value);
      // FIXME field-value may be a list whose elements are seperate by comma(,)
      if (strcasecmp(field_key.c_str(), "cookie") == 0) {
        // special handling for cookie header
        ParseCookieString(field_value, req_ref->Cookies(), 16);
      } else {
        header_ref->Add(field_key, field_value);
      }
      // continue to parse next field
      continue;
    } else {
      // we could not found colon-seperator, assume this is a bad request
      line_state == LineParsingState::LineInvalid;
      break;
    }
  }

  if (line_state == LineParsingState::LinePending) {
    return StatusPrivatePending;
  } else if (line_state == LineParsingState::LineCompleteEmptyLine) {
    conn->SetCurrentParsingStateBody();
    return StatusPrivateComplete;
  } else if (line_state == LineParsingState::LineInvalid) {
    // LineInvalid
    conn->SetCurrentParsingStateInvalid();
    std::cerr << "ParseRequestHeader " << StatusBadRequest << '\n';
    return StatusBadRequest;  // 400
  }
  // below is not reachable
  conn->SetCurrentParsingStateEmptyLine();
  return StatusPrivateComplete;
}

int ParseRequestEmptyLine(HTTPConn* conn) {
  Buffer& rbuf = conn->GetReadBuffer();
  HTTPRequestPtr& req_ref = conn->CurrentRequestRef();
  // we just need one CRLF
  if (rbuf.Size() < 2) {
    // buffer size is not long enough for reading a CRLF
    return StatusPrivatePending;
  }
  // size >= 2
  if (rbuf.ReadableCharacterAt(0) == '\r' && rbuf.ReadableCharacterAt(1) == '\n') {
    rbuf.ReaderIdxForward(2);
    // we need to decide if we collect request body according to
    // the existence of "Content-Length" field in request headers
    if (req_ref->HeaderRef()->Has("Content-Length")) {
      // we collect request body
      conn->SetCurrentParsingStateBody();
    } else {
      // we do not collect request body
      conn->SetCurrentParsingStateDone();
    }
    return StatusPrivateComplete;
  }
  // request empty line is invalid
  conn->SetCurrentParsingStateInvalid();
  std::cerr << "ParseRequestEmptyLine " << StatusBadRequest << '\n';

  return StatusBadRequest;  // 400
}

int ParseRequestBody(HTTPConn* conn) {
  HTTPHeaderPtr& header_ref = conn->CurrentRequestRef()->HeaderRef();
  // re-check the existence of field "Content-Length"
  if (!header_ref->Has("Content-Length")) {
    conn->SetCurrentParsingStateDone();
    return StatusPrivateComplete;
  }
  std::string content_length = header_ref->Get("Content-Length");
  // convert it to uint64_t type
  uint64_t clen;
  if (!CanConvertToUInt64(content_length, clen)) {
    // we can not content-length string to integer
    conn->SetCurrentParsingStateDone();
    std::cerr << "ParseRequestBody " << StatusBadRequest << '\n';
    return StatusBadRequest;  // 400
  }
  Buffer& rbuf = conn->GetReadBuffer();
  if (rbuf.Size() >= clen) {
    // already receive content whose length is clen
    conn->SetCurrentParsingStateDone();
    return StatusPrivateComplete;
  }
  return StatusPrivatePending;
}

// conclude the process to parse request data
int ParseRequestDatagram(HTTPConn* conn) {
  if (conn == nullptr) {
    std::cerr << "ParseRequestDatagram " << StatusInternalServerError << '\n';
    return StatusInternalServerError;
  }
  RequestParsingState state = RequestParsingState::RequestLine;
  int retcode = StatusPrivateComplete;
  while (state != RequestParsingState::Invalid &&
         state != RequestParsingState::Done && retcode != StatusPrivatePending &&
         retcode != StatusPrivateInvalid) {
    state = conn->GetCurrentParsingState();
    switch (state) {
      case RequestParsingState::RequestLine: {
        retcode = ParseRequestLine(conn);
        break;
      }
      case RequestParsingState::RequestHeader: {
        retcode = ParseRequestHeader(conn);
        break;
      }
      case RequestParsingState::RequestEmptyLine: {
        retcode = ParseRequestEmptyLine(conn);
        break;
      }
      case RequestParsingState::RequestBody: {
        retcode = ParseRequestBody(conn);
        break;
      }
      case RequestParsingState::Invalid: {
        // free resources:
        // 1. clear up invalid buffer
        conn->ResetReadBuffer();
        conn->ResetWriteBuffer();
        // TODO 2. close client connection if needed (408)
        printf("case RequestParsingState::Invalid\n");
        retcode = StatusPrivateInvalid;
        break;
      }
      case RequestParsingState::Done: {
        // reset the procedure state for the next request parsing
        conn->SetCurrentParsingStateLine();
        retcode = StatusPrivateDone;
        break;
      }
      default: {
        // error, internal error
        printf("default branch ParseRequestDatagram %d \n",
               StatusInternalServerError);
        return StatusInternalServerError;  // 500
      }
    }
  }
  return retcode;
}

}  // namespace http
}  // namespace ahrimq