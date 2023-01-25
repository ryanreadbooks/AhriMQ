#include "net/http/http_response.h"

namespace ahrimq {
namespace http {

HTTPResponse::HTTPResponse() : header_(std::make_shared<HTTPHeader>()) {
  // add some default header fields into response header
  header_->Add("Server", "AhriMQ/1.0");

}

void HTTPResponse::SetStatus(int status) {
  if (StatusCodeStringMapping.count(status) != 0) {
    // status supported
    status_ = status;
  }
}

void HTTPResponse::Reset() {
  header_->Clear();
  status_ = StatusBadRequest;
}

void HTTPResponse::Organize(Buffer& wbuf) const {
  const static char* colon_seperator = ": ";
  // response line
  char buf[64] = {0};
  std::sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status_,
               StatusCodeStringMapping[status_].c_str());
  wbuf.Append(buf, std::strlen(buf));
  // response header
  for (const auto& item : header_->Members()) {
    const std::string& key = item.first;
    const std::vector<std::string>& values = item.second;
    if (values.empty()) {
      continue;
    }
    wbuf.Append(key);
    wbuf.Append(colon_seperator, 2);
    for (auto& value : values) {
      wbuf.Append(value);
      wbuf.Append("\r\n");
    }
  }
  wbuf.Append("\r\n");
  // request body
  // TODO, may be we should send request body by another way, because request body
  // may contain file data
  
}

}  //  namespace http
}  // namespace ahrimq