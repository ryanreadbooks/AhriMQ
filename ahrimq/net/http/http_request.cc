#include "net/http/http_request.h"

namespace ahrimq {
namespace http {

size_t MAX_BODY_BYTES = (1ul << 31) - 1;

HTTPRequest::HTTPRequest(Buffer* rbuf)
    : header_(std::make_shared<HTTPHeader>()), url_("/"), body_(rbuf) {}

void HTTPRequest::Reset() {
  header_->Clear();
  method_ = HTTPMethod::Get;
  version_ = VersionNotSupported;
  url_.Reset();
  body_ = nullptr;
  form_.Clear();
}

int HTTPRequest::ParseForm() {
  std::string ct = header_->Get("Content-Type");
  if (ct.empty()) {
    // FIXME: we should decide which type from the content
  }
  if (ct == "application/x-www-form-urlencoded") {
    if (body_->Size() > MAX_BODY_BYTES) {
      // body too large
      return StatusContentTooLarge;  // 413
    }
    std::string content = body_->ReadAllAsString();
    form_.ParseString(content, true);
    return StatusPrivateDone;
  } else if (ct == "multipart/form-data") {
    // TODO this is a little bit complicated
    return StatusNotImplemented;  // 501
  }
  return StatusInternalServerError;  // 500
}

}  // namespace http
}  // namespace ahrimq