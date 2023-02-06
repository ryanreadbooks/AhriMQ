#ifndef _AHRIMQ_BASE_TIME_UTILS_H_
#define _AHRIMQ_BASE_TIME_UTILS_H_

#include <sys/time.h>
#include <cstring>
#include <string>

namespace ahrimq {

std::string GMTTimeNow();

static uint64_t GetCurrentSec() {
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}

static uint64_t GetCurrentMs() {
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

static uint64_t GetCurrentUs() {
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

static void GetCurrentTime(long *seconds, long *milliseconds) {
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  *seconds = tv.tv_sec;
  *milliseconds = tv.tv_usec / 1000;
}

static uint64_t AddSecondToNowSec(uint64_t seconds) {
  return GetCurrentSec() + seconds; /* seconds */
}

static uint64_t AddMillSecToNowMillSec(uint64_t milliseconds) {
  return GetCurrentMs() + milliseconds; /* milliseconds */
}

static uint64_t AddSecondToNowMillSec(uint64_t seconds) {
  return GetCurrentMs() + seconds * 1000ul; /* milliseconds */
}

}  // namespace ahrimq

#endif  // _AHRIMQ_BASE_TIME_UTILS_H_