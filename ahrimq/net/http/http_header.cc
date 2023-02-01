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

std::string HTTPHeader::Get(const std::string& key) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  if (!Has(k)) {
    return "";
  }
  try {
    return members_.at(k)[0];
  } catch (std::exception& ex) {
    return "";
  }
}

std::vector<std::string> HTTPHeader::Values(const std::string& key) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  try {
    return members_.at(k);
  } catch (std::exception& ex) {
    return {};
  }
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
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  return members_.count(k) != 0;
}

bool HTTPHeader::Equals(const std::string& key, const std::string& target) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  try {
    return members_.at(k)[0] == target;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::Equals(const std::string& key, const char* target) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  try {
    return members_.at(k)[0].c_str() == target;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::CaseEquals(const std::string& key,
                            const std::string& target) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  try {
    return strcasecmp(members_.at(k)[0].c_str(), target.c_str()) == 0;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::CaseEquals(const std::string& key, const char* target) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  try {
    return strcasecmp(members_.at(k)[0].c_str(), target) == 0;
  } catch (std::exception& ex) {
    return false;
  }
}

bool HTTPHeader::Contains(const std::string& key, const std::string& target) const {
  std::string k = key;
  StrInplaceToLowerCapitalize(k);
  try {
    return members_.at(k)[0].find(target) != std::string::npos;
  } catch (std::exception& ex) {
    return false;
  }
}

}  // namespace http
}  // namespace ahrimq