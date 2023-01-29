#ifndef _HTTP_VERSION_H_
#define _HTTP_VERSION_H_

#include <algorithm>
#include <string>

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

constexpr static int VersionNotSupported = 0x00;
constexpr static int Version1_0 = 0x10;
constexpr static int Version1_1 = 0x11;

const static char* Version1_0_Str = "HTTP/1.0";
const static char* Version1_1_Str = "HTTP/1.1";

/// @brief Parse string to http version hex integer .
/// @param version 
/// @return 
int ParseHTTPVersion(const std::string& version);

/// @brief Check if given http version is supported.
/// @param version 
/// @return 
bool HTTPVersionSupported(const std::string& version);

bool HTTPVersionSupported(int v);

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_VERSION_H_