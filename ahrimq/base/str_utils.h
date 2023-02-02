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

/// @brief Convert all characters in string to lower case inplace, then capitalize it
/// @param str
void StrInplaceToLowerCapitalize(std::string &str);

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

bool StrCaseEqual(const std::string& s1, const char* s2);

bool StrCaseEqual(const std::string &s1, const std::string &s2);

bool CanConvertToInt64(const std::string &str, int64_t &val);

bool CanConvertToInt32(const std::string &str, int &val);

bool CanConvertToUInt64(const std::string &str, uint64_t &val);

bool CanConvertToDouble(const std::string &str, double &val);

bool OnlyDigit(const unsigned char &ch);

bool OnlyHexLowercase(const unsigned char &ch);

bool OnlyHexUppercase(const unsigned char &ch);

void ToHex(char *buf, unsigned char c);

// simple linux path joining operation
std::string PathJoin(const std::string &base, const std::string &location);

void PathJoin(const std::string &base, const std::string &location,
              std::string &out);

bool StartsWith(const std::string &str, const char *prefix);

bool EndsWith(const std::string &str, const char *suffix);

std::string FileExtension(const std::string &str);

}  // namespace ahrimq

#endif  // _BASE_STR_UTILS_H_