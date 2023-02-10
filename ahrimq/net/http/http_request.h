#ifndef _AHRIMQ_NET_HTTP_HTTP_REQUEST_H_
#define _AHRIMQ_NET_HTTP_HTTP_REQUEST_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "buffer/buffer.h"
#include "net/http/http_header.h"
#include "net/http/http_method.h"
#include "net/http/http_status.h"
#include "net/http/http_version.h"
#include "net/http/url.h"
#include "net/http/cookie.h"

namespace ahrimq {
namespace http {

typedef URL::Query BodyForm;

/// @brief HTTPRequest represents a http request instance.
class HTTPRequest {
 public:
  explicit HTTPRequest(Buffer* rbuf);

  ~HTTPRequest();

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

  void SetURL(const std::string& url) {
    url_.Set(url);
  }

  void SetHTTPVersion(int v) {
    if (HTTPVersionSupported(v)) {
      version_ = v;
    }
  }

  int GetHTTPVersion() const {
    return version_;
  }

  /// @brief Reset the http request instance, including http header, method, body,
  /// etc.
  void Reset();

  /// @brief Get http request body.
  /// @return the pointer to the Buffer which contains request body content.
  Buffer* Body() const {
    return body_;
  }

  /// @brief Get url query.
  /// @return
  const URL::Query& Query() const {
    return url_.GetQuery();
  }

  /// @brief Check if url query is empty.
  /// @return
  bool QueryEmpty() const {
    return url_.GetQuery().Empty();
  }

  /// @brief Try to parse form from request body and fill form_ member.
  /// @return
  int ParseForm();

  const BodyForm& Form() const {
    return form_;
  }

  bool FormEmpty() const {
    return form_.Empty();
  }

  const std::vector<Cookie>& Cookies() const {
    return cookies_;
  }

  std::vector<Cookie>& Cookies() {
    return cookies_;
  }

  bool CookiesEmpty() const {
    return cookies_.empty();
  }

 private:
  HTTPHeaderPtr header_;
  HTTPMethod method_;
  int version_;
  URL url_;
  Buffer* body_;
  BodyForm form_;
  std::vector<Cookie> cookies_;
};

typedef std::shared_ptr<HTTPRequest> HTTPRequestPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _AHRIMQ_NET_HTTP_HTTP_REQUEST_H_