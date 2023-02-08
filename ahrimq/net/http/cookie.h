#ifndef _AHRIMQ_NET_HTTP_COOKIE_H_
#define _AHRIMQ_NET_HTTP_COOKIE_H_

#include <unordered_map>
#include <vector>
#include <ostream>
#include <sstream>

#include "base/time_utils.h"
#include "buffer/buffer.h"

namespace ahrimq {
namespace http {

static const char* kCookieSeperator = "; ";

class Cookie;

/// @brief Try to parse from string cookie and put cookie instance into out vector.
/// @param cookie 
/// @param out 
/// @return 
size_t ParseCookieString(const std::string& cookie, std::vector<Cookie>& out);

/// @brief Try to parse from string cookie and put cookie instance into out vector.
/// @param cookie 
/// @param out 
/// @param estimate estimated number of cookies
/// @return 
size_t ParseCookieString(const std::string& cookie, std::vector<Cookie>& out, size_t estimate);

/// @brief HTTP cookie
class Cookie {
 public:
  /// @brief Cookie samesite policy
  enum class SameSite {
    Lax,
    Strict,
    None,
  };

 public:
  Cookie(const std::string& name, const std::string& value)
      : name_(name), value_(value) {}

  void SetName(const std::string& name) {
    name_ = name;
  }

  void SetValue(const std::string& value) {
    value_ = value;
  }

  void SetPath(const std::string& path) {
    path_ = path;
  }

  void SetDomain(const std::string& domain) {
    domain_ = domain;
  }

  void SetMaxAge(int64_t maxage) {
    maxage_ = maxage;
    maxage_set_ = true;
  }

  void SetSecure(bool secure) {
    secure_ = secure;
  }

  void SetHttpOnly(bool httponly) {
    httponly_ = httponly;
  }

  void SetSameSite(SameSite samesite) {
    samesite_ = samesite;
    samesite_set_ = true;
  }

  void SetExpireAt(const time::UTCTimePoint& at) {
    expireat_ = at;
    expireat_set_ = true;
  }

  const std::string& Name() const {
    return name_;
  }

  const std::string& Value() const {
    return value_;
  }

  const std::string& Path() const {
    return path_;
  }

  const std::string& Domain() const {
    return domain_;
  }

  int64_t MaxAge() const {
    return maxage_;
  }

  Cookie::SameSite SameSitePolicy() const {
    return samesite_;
  }

  bool Secure() const {
    return secure_;
  }

  bool HttpOnly() const {
    return httponly_;
  }

  const time::UTCTimePoint& ExpireAt() const {
    return expireat_;
  }

  /// @brief Serialize cookie instance into string representation.
  /// @param out output string
  void Serialize(std::string& out) const;

  /// @brief Serialize cookie instance into Buffer
  /// @param out 
  void Serialize(Buffer& out) const;

  friend std::ostream& operator<<(std::ostream& os, const Cookie& cookie);

 private:
  std::string name_;
  std::string value_;

  std::string path_;
  std::string domain_;
  int64_t maxage_;
  Cookie::SameSite samesite_;
  time::UTCTimePoint expireat_;

  bool secure_ = false;
  bool httponly_ = false;

  bool maxage_set_ = false;
  bool expireat_set_ = false;
  bool samesite_set_ = false;
};

}  // namespace http
}  // namespace ahrimq

#endif  // _AHRIMQ_NET_HTTP_COOKIE_H_