#ifndef _BASE_STR_UTILS_H_
#define _BASE_STR_UTILS_H_

#include <algorithm>
#include <string>
#include <vector>

namespace ahrimq {

/// @brief Split string by delim.
/// @param str 
/// @param delim 
/// @return 
std::vector<std::string> StrSplit(const std::string &str, char delim);

void StrSplit(const std::string &str, char delim, std::vector<std::string> &out);

/// @brief Convert all characters in string to upper case inplace. 
/// @param str 
void StrInplaceToUpper(std::string &str);

/// @brief Convert all characters in string to lower case inplace. 
/// @param str 
void StrInplaceToLower(std::string &str);

/// @brief Trim all leading spaces in string.
/// @param str 
/// @return 
std::string StrTrimLeft(const std::string &str);

/// @brief Trim all trailing spaces in string.
/// @param str 
/// @return 
std::string StrTrimRight(const std::string &str);

/// @brief Compare s1[0:end) and s2[0:end)
/// @param s1 
/// @param s2 
/// @param end 
/// @return 
bool StrEqual(const std::string &s1, const std::string &s2, size_t end);

bool CanConvertToInt64(const std::string &str, int64_t &val);

bool CanConvertToInt32(const std::string &str, int &val);

bool CanConvertToUInt64(const std::string &str, uint64_t &val);

bool CanConvertToDouble(const std::string &str, double &val);

}  // namespace ahrimq

#endif  // _BASE_STR_UTILS_H_