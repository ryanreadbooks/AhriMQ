#ifndef _HTTP_URL_H_
#define _HTTP_URL_H_

#include <string>
#include <ostream>
#include <memory>

namespace ahrimq {
namespace http {

// TODO
/// @brief URL is a wrapper for http reqeust-url
class URL {
 public:
  URL(std::string url = "/");

  void Set(const std::string& s) {
    url_ = s;
  }

  void Set(std::string&& s) {
    url_ = std::move(s);
  }

  std::string String() const {
    return url_;
  }

  void Reset() {
    url_ = "";
  }

  friend std::ostream& operator<<(std::ostream& os, const URL& url);

 private:
  std::string url_;
};

typedef std::shared_ptr<URL> URLPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_URL_H_