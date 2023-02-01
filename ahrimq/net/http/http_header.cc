#include "net/http/http_header.h"

#include <cstring>

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

HTTPHeader::HTTPHeader(HTTPHeader&& other) {
  other.members_.swap(members_);
}

void HTTPHeader::Add(const std::string& key, const std::string& value) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  members_[std::move(k)].emplace_back(value);
}

void HTTPHeader::Del(const std::string& key) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  members_.erase(k);
}

std::string HTTPHeader::Get(const std::string& key) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return "";
  }
  return members_[k][0];
}

std::vector<std::string> HTTPHeader::Values(const std::string& key) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return {};
  }
  return members_[k];
}

void HTTPHeader::Set(const std::string& key, const std::string& value) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  members_[std::move(k)] = {value};
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
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return false;
  }
  return members_[k][0] == target;
}

bool HTTPHeader::Equals(const std::string& key, const char* target) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return false;
  }
  return members_[k][0].c_str() == target;
}

bool HTTPHeader::CaseEquals(const std::string& key, const std::string& target) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return false;
  }
  return strcasecmp(members_[k][0].c_str(), target.c_str()) == 0;
}

bool HTTPHeader::CaseEquals(const std::string& key, const char* target) {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return false;
  }
  return strcasecmp(members_[k][0].c_str(), target) == 0;
}

}  // namespace http
}  // namespace ahrimq