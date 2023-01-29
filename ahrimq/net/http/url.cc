#include "net/http/url.h"

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

void URL::Query::Add(const std::string& key, std::string value) {
  if (!Has(key)) {
    params_[key] = {std::move(value)};
    return;
  }
  params_[key].emplace_back(std::move(value));
}

void URL::Query::Set(const std::string& key, std::string value) {
  params_[key] = {std::move(value)};
}

std::string URL::Query::Get(const std::string& key) const {
  if (Has(key)) {
    return params_.at(key)[0];
  }
  return "";
}

std::vector<std::string> URL::Query::Values(const std::string& key) const {
  if (Has(key)) {
    return params_.at(key);
  }
  return {};
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

URL::URL(std::string url) : url_(std::move(url)) {
  ParseQuery();
}

std::ostream& operator<<(std::ostream& os, const URL& url) {
  os << url.url_;
  return os;
}

void URL::ParseQuery() {
  // we need to parse url_ member here
  if (url_.empty()) {
    return;
  }
  size_t pos = url_.find('?');
  if (pos == std::string::npos) {
    // no '?' found in url_, then we do not need to set query_
    return;
  }
  // '?' found
  // seperated by '&'
  std::string raw_query = url_.substr(pos + 1);
  if (raw_query.empty()) {
    return;
  }
  std::vector<std::string> q;
  // key1=value1&key2=value2&...&keyN=valueN
  StrSplit(raw_query, '&', q);
  // key=value
  for (const auto& argp : q) {
    pos = argp.find('=');
    if (pos != std::string::npos) {
      // set query items
      query_.Add(argp.substr(0, pos), argp.substr(pos + 1));
    }
  }
}

}  // namespace http
}  // namespace ahrimq