#include "base/str_utils.h"

#include <cstring>
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

void StrInplaceToLowerCapitalize(std::string& str) {
  StrInplaceToLower(str);
  if (!str.empty()) {
    str[0] = std::toupper(str[0]);
  }
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

bool StrEqual(const std::string& s1, const std::string& s2, size_t end) {
  size_t l1 = s1.size();
  size_t l2 = s2.size();
  if (end > l1 || end > l2 || end == 0) {
    return false;
  }
  size_t idx = 0;
  while (idx < end) {
    if (s1[idx] != s2[idx]) {
      return false;
    }
    idx++;
  }
  return true;
}

bool StrCaseEqual(const std::string& s1, const std::string& s2) {
  return strcasecmp(s1.c_str(), s2.c_str()) == 0;
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

bool OnlyDigit(const unsigned char& ch) {
  return ch >= '0' && ch <= '9';
}

bool OnlyHexLowercase(const unsigned char& ch) {
  return ch >= 'a' && ch <= 'f';
}

bool OnlyHexUppercase(const unsigned char& ch) {
  return ch >= 'A' && ch <= 'F';
}

void ToHex(char* buf, unsigned char c) {
  static char hextable[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  buf[0] = hextable[c >> 4];
  buf[1] = hextable[c & 0x0f];
}

}  // namespace ahrimq