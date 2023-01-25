#include "net/http/http_request.h"

namespace ahrimq {
namespace http {

HTTPRequest::HTTPRequest() : header_(std::make_shared<HTTPHeader>()) {}

void HTTPRequest::Reset() {
  header_->Clear();
  method_ = HTTPMethod::Get;
  version_ = VersionNotSupported;
  url_.Reset();
}

}  // namespace http
}  // namespace ahrimq