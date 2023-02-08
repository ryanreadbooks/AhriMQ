#include "net/http/cookie.h"

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

size_t ParseCookieString(const std::string& cookie, std::vector<Cookie>& out) {
  if (cookie.empty()) {
    return 0;
  }
  size_t sep_idx;
  size_t start = 0;
  size_t eq_idx = std::string::npos;
  size_t nvalid = 0;
  while ((eq_idx = cookie.find('=', start)) != std::string::npos) {
    // [start, eq_idx - 1] -> name
    std::string name = cookie.substr(start, eq_idx - start);
    if (name.empty()) {
      return nvalid;
    }
    // find kCookieSeperator
    start = eq_idx + 1;
    if ((sep_idx = cookie.find(kCookieSeperator, start)) == std::string::npos) {
      // the last cookie pair does not have kCookieSeperator at the end
      sep_idx = cookie.size();
    }
    // [start, sep_idx - 1] -> value
    std::string value = cookie.substr(start, sep_idx - start);
    out.emplace_back(std::move(name), std::move(value));
    nvalid++;
    start = sep_idx + 2;
  }
  return nvalid;
}

size_t ParseCookieString(const std::string& cookie, std::vector<Cookie>& out,
                         size_t estimate) {
  out.reserve(estimate);
  return ParseCookieString(cookie, out);
}

void Cookie::Serialize(std::string& out) const {
  if (name_.empty()) {
    return;
  }
  out.append(name_);
  out.push_back('=');
  out.append(value_);
  out.append(kCookieSeperator);

  if (!path_.empty()) {
    out.append("Path");
    out.push_back('=');
    out.append(path_);
    out.append(kCookieSeperator);
  }
  if (!domain_.empty()) {
    out.append("Domain");
    out.push_back('=');
    out.append(domain_);
    out.append(kCookieSeperator);
  }
  if (expireat_set_) {
    out.append("Expires");
    out.push_back('=');
    out.append(expireat_.ToFormatString());
    out.append(kCookieSeperator);
  }
  if (maxage_set_) {
    out.append("Max-Age");
    out.push_back('=');
    out.append(std::to_string(maxage_));
    out.append(kCookieSeperator);
  }
  if (httponly_) {
    out.append("HttpOnly");
    out.append(kCookieSeperator);
  }
  if (secure_) {
    out.append("Secure");
    out.append(kCookieSeperator);
  }
  if (samesite_set_) {
    out.append("SameSite");
    out.push_back('=');
    if (samesite_ == SameSite::Lax) {
      out.append("Lax");
    } else if (samesite_ == SameSite::Strict) {
      out.append("Strict");
    } else {
      out.append("None");
    }
    out.append(kCookieSeperator);
  }
  // remove trailing kCookieSeperator
  if (out.back() == ' ' && out[out.size() - 2] == ';') {
    out.erase(out.size() - 2, 2);
  }
}

void Cookie::Serialize(Buffer& out) const {
  if (name_.empty()) {
    return;
  }
  out.Append(name_);
  out.Append("=");
  out.Append(value_);
  out.Append(kCookieSeperator);

  if (!path_.empty()) {
    out.Append("Path");
    out.Append("=");
    out.Append(path_);
    out.Append(kCookieSeperator);
  }
  if (!domain_.empty()) {
    out.Append("Domain");
    out.Append("=");
    out.Append(domain_);
    out.Append(kCookieSeperator);
  }
  if (expireat_set_) {
    out.Append("Expires");
    out.Append("=");
    out.Append(expireat_.ToFormatString());
    out.Append(kCookieSeperator);
  }
  if (maxage_set_) {
    out.Append("Max-Age");
    out.Append("=");
    out.Append(std::to_string(maxage_));
    out.Append(kCookieSeperator);
  }
  if (httponly_) {
    out.Append("HttpOnly");
    out.Append(kCookieSeperator);
  }
  if (secure_) {
    out.Append("Secure");
    out.Append(kCookieSeperator);
  }
  if (samesite_set_) {
    out.Append("SameSite");
    out.Append("=");
    if (samesite_ == SameSite::Lax) {
      out.Append("Lax");
    } else if (samesite_ == SameSite::Strict) {
      out.Append("Strict");
    } else {
      out.Append("None");
    }
    out.Append(kCookieSeperator);
  }
  // remove trailing kCookieSeperator
  if (out.ReadableCharacterAt(out.Size() - 1) == ' ' && out.ReadableCharacterAt(out.Size() - 2) == ';') {
    out.WriterIdxBackward(2);
  }
}

std::ostream& operator<<(std::ostream& os, const Cookie& cookie) {
  os << cookie.name_ << " = " << cookie.value_;
  return os;
}

}  // namespace http
}  // namespace ahrimq