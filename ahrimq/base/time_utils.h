#ifndef _AHRIMQ_BASE_TIME_UTILS_H_
#define _AHRIMQ_BASE_TIME_UTILS_H_

#include <sys/time.h>

#include <cstring>
#include <ctime>
#include <string>

namespace ahrimq {

/// @brief Simple time and date utility based on time.h
namespace time {

class Duration;

/// @brief Month representation.
enum class Month { Jan = 0, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };

/// @brief UTCTimePoint represents an instant time point in UTC.
class UTCTimePoint {
 public:
  UTCTimePoint() = default;

  UTCTimePoint(const timespec& spec);

  UTCTimePoint(const struct tm& stm);

  UTCTimePoint(int year, Month month, int day = 0, int hour = 0, int min = 0,
               int sec = 0);

  std::string ToFormatString() const;

  static UTCTimePoint Now();

  static std::string NowFormatString();

  void Add(const Duration& duration);

  UTCTimePoint NewAdd(const Duration& duration);

  void Sub(const Duration& duration);

  UTCTimePoint NewSub(const Duration& duration);

  UTCTimePoint& operator+=(const Duration& duration);

  UTCTimePoint& operator-=(const Duration& duration);

 private:
  struct tm impl_ = {0};
};

/// @brief Duration represents a timespan
class Duration {
 public:
  Duration(int64_t d) : cnt_(d) {}

  virtual int64_t Count() const {
    return cnt_;
  }

 protected:
  int64_t cnt_ = 0;  // given in second
};

using Second = Duration;

/// @brief Duration represented in minute.
class Minute : public Duration {
 public:
  Minute(int64_t m) : Duration(m) {}

  int64_t Count() const override {
    return cnt_ * 60;
  }
};

/// @brief Duration represented in hour.
class Hour : public Duration {
 public:
  Hour(int64_t h) : Duration(h) {}

  int64_t Count() const override {
    return cnt_ * 3600;
  }
};

UTCTimePoint operator+(const UTCTimePoint& tp, const Duration& dur);

UTCTimePoint operator-(const UTCTimePoint& tp, const Duration& dur);

void SetUTCTimeZone();

std::string GMTTimeNowString();

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

static void GetCurrentTime(long* seconds, long* milliseconds) {
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

}  // namespace time
}  // namespace ahrimq

#endif  // _AHRIMQ_BASE_TIME_UTILS_H_