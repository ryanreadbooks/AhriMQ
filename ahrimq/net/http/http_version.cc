#include "net/http/http_version.h"

namespace ahrimq {
namespace http {

int ParseHTTPVersion(const std::string& version) {
  if (version == Version1_0_Str) {
    return Version1_0;
  } else if (version == Version1_1_Str) {
    return Version1_1;
  } else {
    return VersionNotSupported;
  }
}

bool HTTPVersionSupported(const std::string& version) {
  int v = ParseHTTPVersion(version);
  return v == Version1_0 || v == Version1_1;
}

bool HTTPVersionSupported(int v) {
  return v == Version1_0 || v == Version1_1;
}

}  // namespace http
}  // namespace ahrimq