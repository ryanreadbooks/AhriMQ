#include "net/http/http_header.h"

#include <cstring>

namespace ahrimq {
namespace http {

HTTPHeader::HTTPHeader(HTTPHeader&& other) {
  other.members_.swap(members_);
}

void HTTPHeader::Add(std::string key, std::string value) {
  if (!Has(key)) {
    members_[std::move(key)].emplace_back(std::move(value));
  } else {
  // exists
    members_[key].emplace_back(std::move(value));
  }
}

void HTTPHeader::Del(const std::string& key) {
  members_.erase(key);
}

std::string HTTPHeader::Get(const std::string& key) {
  if (!Has(key)) {
    return "";
  }
  return members_[key][0];
}

std::vector<std::string> HTTPHeader::Values(const std::string& key) {
  if (!Has(key)) {
    return {};
  }
  return members_[key];
}

void HTTPHeader::Set(const std::string& key, std::string value) {
  members_[key] = {std::move(value)};
}

std::vector<std::string> HTTPHeader::AllFieldKeys() const {
  std::vector<std::string> keys;
  keys.reserve(members_.size());
  for (auto&& item : members_) {
    keys.emplace_back(item.first);
  }
  return keys;
}

std::vector<std::vector<std::string>> HTTPHeader::AllFieldValues() const {
  std::vector<std::vector<std::string>> values;
  values.reserve(members_.size());
  for (auto&& item : members_) {
    values.emplace_back(item.second);
  }
  return values;
}

void HTTPHeader::Clear() {
  members_.clear();
}

bool HTTPHeader::Has(const std::string& key) const {
  return members_.count(key) != 0;
}

bool HTTPHeader::Equals(const std::string& key, const std::string& target) {
  if (!Has(key)) {
    return false;
  }
  return members_[key][0] == target;
}

bool HTTPHeader::Equals(const std::string& key, const char* target) {
  if (!Has(key)) {
    return false;
  }
  return members_[key][0] == target;
}

}  // namespace http
}  // namespace ahrimq