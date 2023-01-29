#include "net/http/http_request.h"

namespace ahrimq {
namespace http {

// TODO do not use "/" as default url
HTTPRequest::HTTPRequest(Buffer* rbuf)
    : header_(std::make_shared<HTTPHeader>()), url_("/"), body_(rbuf) {}

void HTTPRequest::Reset() {
  header_->Clear();
  method_ = HTTPMethod::Get;
  version_ = VersionNotSupported;
  url_.Reset();
  body_ = nullptr;
}

}  // namespace http
}  // namespace ahrimq