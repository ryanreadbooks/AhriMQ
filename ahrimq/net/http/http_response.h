#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include <memory>

#include "buffer/buffer.h"
#include "net/http/http_status.h"
#include "net/http/http_header.h"

namespace ahrimq {
namespace http {

class HTTPResponse {
 public:
  HTTPResponse();

  HTTPHeaderPtr GetHeader() const {
    return header_;
  }

  HTTPHeaderPtr& HeaderRef() {
    return header_;
  }

  int Status() const {
    return status_;
  }

  void SetStatus(int status);

  void Reset();

  /// @brief organize response content into write buffer
  /// @param wbuf 
  void Organize(Buffer& wbuf) const;

 private:
  int status_ = StatusBadRequest;
  HTTPHeaderPtr header_;
  // TODO we need body pointer to point to the response body
};

typedef std::shared_ptr<HTTPResponse> HTTPResponsePtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_RESPONSE_H_