#include "base/time_utils.h"

namespace ahrimq {

std::string GMTTimeNow() {
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = gmtime(&rawtime);  // get GMT time
  char buf[48];
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
  return std::string(buf, std::strlen(buf));
}

}  // namespace ahrimq