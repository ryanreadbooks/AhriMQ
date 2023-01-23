#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <memory>

namespace ahrimq {
namespace http {

class HTTPRequest {};

typedef std::shared_ptr<HTTPRequest> HTTPRequestPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_REQUEST_H_