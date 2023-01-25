#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <memory>
#include <string>

#include "net/http/http_header.h"
#include "net/http/http_method.h"
#include "net/http/http_version.h"
#include "net/http/url.h"

namespace ahrimq {
namespace http {

class HTTPRequest {
 public:
  HTTPRequest();

  ~HTTPRequest() = default;

  std::string GetMethod() {
    return httpMethodStringMapping[method_];
  }

  HTTPMethod Method() const {
    return method_;
  }

  void SetMethod(const std::string& method) {
    if (HTTPMethodSupported(method)) {
      method_ = httpStringMethodMapping[method];
    }
  }

  void SetMethod(HTTPMethod method) {
    method_ = method;
  }

  HTTPHeaderPtr GetHeader() const {
    return header_;
  }

  HTTPHeaderPtr& HeaderRef() {
    return header_;
  }

  URL GetURL() const {
    return url_;
  }

  const URL& URLRef() const {
    return url_;
  }

  void SetURL(std::string url) {
    url_.Set(std::move(url));
  }

  void SetHTTPVersion(int v) {
    if (HTTPVersionSupported(v)) {
      version_ = v;
    }
  }

  int GetHTTPVersion() const {
    return version_;
  }

  void Reset();

 private:
  HTTPHeaderPtr header_;
  HTTPMethod method_;
  int version_;
  URL url_;
};

typedef std::shared_ptr<HTTPRequest> HTTPRequestPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_REQUEST_H_