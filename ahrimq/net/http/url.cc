#include "net/http/url.h"

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

void URL::Query::Add(const std::string& key, const std::string& value) {
  params_[key].emplace_back(value);
}

void URL::Query::Set(const std::string& key, const std::string& value) {
  params_[key] = {value};
}

std::string URL::Query::Get(const std::string& key) const {
  try {
    return params_.at(key)[0];
  } catch (std::exception& ex) {
    return "";
  }
}

std::vector<std::string> URL::Query::Values(const std::string& key) const {
  try {
    return params_.at(key);
  } catch (std::exception& ex) {
    return {};
  }
}

bool URL::Query::Has(const std::string& key) const {
  return params_.count(key) != 0;
}

void URL::Query::Del(const std::string& key) {
  params_.erase(key);
}

void URL::Query::Clear() {
  params_.clear();
}

std::vector<std::string> URL::Query::Keys() const {
  std::vector<std::string> r;
  r.reserve(params_.size());
  for (auto&& k : params_) {
    r.emplace_back(k.first);
  }
  return r;
}

void URL::Query::ParseString(const std::string& str, bool body) {
  if (str.empty()) {
    return;
  }
  size_t pos;
  std::vector<std::string> q;
  if (!body) {
    pos = str.find('?');
    if (pos == std::string::npos) {
      // no '?' found in url_, then we do not need to set query_
      return;
    }
    // '?' found
    // seperated by '&'
    std::string raw_query = str.substr(pos + 1);
    if (raw_query.empty()) {
      return;
    }
    // key1=value1&key2=value2&...&keyN=valueN
    StrSplit(raw_query, '&', q);
  } else {
    StrSplit(str, '&', q);
  }

  // key=value
  for (const auto& argp : q) {
    pos = argp.find('=');
    if (pos != std::string::npos) {
      // set query items
      // unescape the urlencoded content
      std::string unescaped_key;
      std::string unescaped_value;
      URL::UnEscape(argp.substr(0, pos), unescaped_key);
      URL::UnEscape(argp.substr(pos + 1), unescaped_value);
      Add(unescaped_key, unescaped_value);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const URL::Query& query) {
  size_t k = 0;
  for (const auto& item : query.params_) {
    os << item.first << "=[";
    for (size_t i = 0; i < item.second.size(); ++i) {
      os << item.second[i];
      if (i != item.second.size() - 1) {
        os << ", ";
      }
    }
    os << ']';
    if (k != query.Size() - 1) {
      os << ", ";
    }
    k++;
  }
  return os;
}

URL::URL(const std::string& url) : url_(url) {
  ParseQuery();
}

std::string URL::StringWithQuery() const {
  if (query_.Empty()) {
    return url_;
  }
  size_t pos = url_.find('?');
  if (pos == std::string::npos) {
    return url_;
  }
  return url_.substr(0, pos);
}

void URL::ParseQuery() {
  // we need to parse url_ member here
  query_.ParseString(url_);
}

std::ostream& operator<<(std::ostream& os, const URL& url) {
  os << url.url_;
  return os;
}

static bool RFC3986UnreservedChar(const unsigned char& ch) {
  return OnlyDigit(ch) || OnlyHexLowercase(ch) || OnlyHexUppercase(ch) ||
         ch == '-' || ch == '.' || ch == '_' || ch == '~';
}

void URL::Escape(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());
  char buf[3] = {'%'};
  for (size_t i = 0; i < in.size(); i++) {
    if (RFC3986UnreservedChar((unsigned char)in[i])) {
      out.push_back(in[i]);
    } else {
      // convert to percent-encoding
      // char to hex string
      ToHex(buf + 1, in[i]);
      out.append(buf, 3);
    }
  }
}

bool URL::UnEscape(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    switch (in[i]) {
      // %xx: next two characters should be transformed
      case '%':
        if (i + 3 <= in.size()) { // out of range
          unsigned int value = 0;
          // convert the next 2 hexadecimal digits to deicmal
          for (std::size_t j = i + 1; j <= i + 2; ++j) {
            if (OnlyDigit(in[j])) {
              value += in[j] - '0';
            } else if (OnlyHexLowercase(in[j])) {
              value += in[j] - 'a' + 10;
            } else if (OnlyHexUppercase(in[j])) {
              value += in[j] - 'A' + 10;
            } else {
              return false;
            }
            if (j == i + 1) {
              value <<= 4;  // make it high 4-bit
            }
          }
          // done convert %xx
          out.push_back(static_cast<char>(value));
          i += 2;
        } else {
          return false;
        }
        break;
      // remain unchanged
      case '-':
      case '_':
      case '.':
      case '~':
      case ':':
      case '/':
      case '?':
      case '#':
      case '[':
      case ']':
      case '@':
      case '!':
      case '$':
      case '&':
      case '\'':
      case '(':
      case ')':
      case '*':
      case ',':
      case ';':
      case '=':
        out.push_back(in[i]);
        break;
      case '+':
        out.push_back(' ');
        break;

      default:
        // not alphabet and not number
        if (!std::isalnum(in[i])) {
          return false;
        }
        // remain unchanged
        out.push_back(in[i]);
        break;
    }
  }
  return true;
}

}  // namespace http
}  // namespace ahrimq