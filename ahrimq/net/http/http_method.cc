#include "net/http/http_method.h"

namespace ahrimq {
namespace http {

bool HTTPMethodSupported(const std::string& method) {
  // HTTP method is case sentative
  return httpStringMethodMapping.count(method) != 0;
}
}  // namespace http
}  // namespace ahrimq