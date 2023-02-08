#include "net/http/http_header.h"

#include <cstring>

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

HTTPHeader::HTTPHeader(HTTPHeader&& other) {
  other.members_.swap(members_);
}

void HTTPHeader::Add(const std::string& key, const std::string& value) {
  members_[key].emplace_back(value);
}

void HTTPHeader::Del(const std::string& key) {
  members_.erase(key);
}

std::string HTTPHeader::Get(const std::string& key) const {
  if (!Has(key)) {
    return "";
  }
  try {
    return members_.at(key)[0];
  } catch (std::exception& ex) {
    return "";
  }
}

std::vector<std::string> HTTPHeader::Values(const std::string& key) const {
  try {
    return members_.at(key);
  } catch (std::exception& ex) {
    return {};
  }
}

void HTTPHeader::Set(const std::string& key, const std::string& value) {
  if (members_.count(key) == 0) {
    members_.emplace(key, std::vector<std::string>{value});
  } else {
    members_[key].clear();
    members_[key].emplace_back(value);
  }
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

bool HTTPHeader::Equals(const std::string& key, const std::string& target) const {
  try {
    return members_.at(key)[0] == target;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::Equals(const std::string& key, const char* target) const {
  try {
    return std::strcmp(members_.at(key)[0].c_str(), target) == 0;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::CaseEquals(const std::string& key,
                            const std::string& target) const {
  try {
    return strcasecmp(members_.at(key)[0].c_str(), target.c_str()) == 0;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::CaseEquals(const std::string& key, const char* target) const {
  try {
    return strcasecmp(members_.at(key)[0].c_str(), target) == 0;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::Contains(const std::string& key, const std::string& target) const {
  try {
    return members_.at(key)[0].find(target) != std::string::npos;
  } catch (std::exception& ex) {
    return false;
  }
}

}  // namespace http
}  // namespace ahrimq