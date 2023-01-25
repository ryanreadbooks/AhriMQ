#include "net/http/url.h"

namespace ahrimq {
namespace http {

URL::URL(std::string url) : url_(std::move(url)) {}

std::ostream& operator<<(std::ostream& os, const URL& url) {
  os << url.url_;
  return os;
}

}  // namespace http
}  // namespace ahrimq