#include "base/str_utils.h"

#include <iostream>
#include <sstream>

namespace ahrimq {

std::vector<std::string> StrSplit(const std::string& str, char delim) {
  if (str.empty()) {
    return {};
  }
  std::vector<std::string> splits;
  std::stringstream ss(str);
  std::string tmp;
  while (std::getline(ss, tmp, delim)) {
    if (!tmp.empty()) {
      splits.emplace_back(tmp);
    }
  }
  return splits;
}

void StrSplit(const std::string& str, char delim, std::vector<std::string>& out) {
  if (str.empty()) {
    return;
  }
  std::stringstream ss(str);
  std::string tmp;
  out.clear();
  while (std::getline(ss, tmp, delim)) {
    if (!tmp.empty()) {
      out.emplace_back(tmp);
    }
  }
}

void StrInplaceToUpper(std::string& str) {
  std::transform(str.cbegin(), str.cend(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });
}

void StrInplaceToLower(std::string& str) {
  std::transform(str.cbegin(), str.cend(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

std::string StrTrimLeft(const std::string& str) {
  if (str.empty()) {
    return "";
  }
  auto idx = str.find_first_not_of(" \t\r\n");
  return std::string(str.begin() + idx, str.end());
}

std::string StrTrimRight(const std::string& str) {
  if (str.empty()) {
    return "";
  }
  auto idx = str.find_last_not_of(" \t\r\n");
  return std::string(str.begin(), str.begin() + idx + 1);
}

bool CanConvertToInt64(const std::string& str, int64_t& ans) {
  if (str.empty() || str.size() > 20) {
    return false;
  }
  try {
    int64_t tmp = std::stoll(str);
    if (std::to_string(tmp) == str) {
      ans = tmp;
      return true;
    }
  } catch (const std::exception& ex) {
    return false;
  }
  return false;
}

bool CanConvertToInt32(const std::string& str, int& ans) {
  if (str.empty() || str.size() > 20) {
    return false;
  }
  try {
    int tmp = std::stoi(str);
    if (std::to_string(tmp) == str) {
      ans = tmp;
      return true;
    }
  } catch (const std::exception& ex) {
    return false;
  }
  return false;
}

bool CanConvertToUInt64(const std::string& str, uint64_t& val) {
  if (str.empty()) {
    return false;
  }
  try {
    val = std::stoull(str);
    return true;
  } catch (const std::exception& ec) {
    return false;
  }
}

bool CanConvertToDouble(const std::string& str, double& val) {
  if (str.empty()) {
    return false;
  }
  try {
    val = std::stod(str);
    return true;
  } catch (const std::exception& ex) {
    return false;
  }
}

}  // namespace ahrimq