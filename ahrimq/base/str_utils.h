#ifndef _BASE_STR_UTILS_H_
#define _BASE_STR_UTILS_H_

#include <algorithm>
#include <string>
#include <vector>

namespace ahrimq {

std::vector<std::string> StrSplit(const std::string &str, char delim);

void StrInplaceToUpper(std::string &str);

void StrInplaceToLower(std::string &str);

std::string StrTrimLeft(const std::string &str);

std::string StrTrimRight(const std::string &str);

bool CanConvertToInt64(const std::string &str, int64_t &val);

bool CanConvertToInt32(const std::string &str, int &val);

bool CanConvertToUInt64(const std::string &str, uint64_t &val);

bool CanConvertToDouble(const std::string &str, double &val);

}  // namespace ahrimq

#endif  // _BASE_STR_UTILS_H_