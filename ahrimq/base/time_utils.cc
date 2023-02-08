#include "base/time_utils.h"

namespace ahrimq {
namespace time {

UTCTimePoint::UTCTimePoint(const timespec& spec) {
  gmtime_r(&spec.tv_sec, &impl_);
}

UTCTimePoint::UTCTimePoint(const struct tm& stm) : impl_(stm) {}

UTCTimePoint::UTCTimePoint(int year, Month month, int day, int hour, int min,
                           int sec) {
  impl_.tm_sec = sec;
  impl_.tm_min = min;
  impl_.tm_hour = hour;
  impl_.tm_mday = day;
  impl_.tm_mon = int(month);
  impl_.tm_year = year - 1900;
}

std::string UTCTimePoint::ToFormatString() const {
  char buf[48];
  std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &impl_);
  return std::string(buf, std::strlen(buf));
}

UTCTimePoint UTCTimePoint::Now() {
  timespec tspec;
  timespec_get(&tspec, TIME_UTC);
  return UTCTimePoint(tspec);
}

std::string UTCTimePoint::NowFormatString() {
  return Now().ToFormatString();
}

void UTCTimePoint::Add(const Duration& duration) {
  impl_.tm_sec += duration.Count();
  time_t tt = mktime(&impl_);
  gmtime_r(&tt, &impl_);
}

UTCTimePoint UTCTimePoint::NewAdd(const Duration& duration) {
  UTCTimePoint ret = *this;
  ret.Add(duration);
  return ret;
}

void UTCTimePoint::Sub(const Duration& duration) {
  impl_.tm_sec -= duration.Count();
  time_t tt = mktime(&impl_);
  gmtime_r(&tt, &impl_);
}

UTCTimePoint UTCTimePoint::NewSub(const Duration& duration) {
  UTCTimePoint ret = *this;
  ret.Sub(duration);
  return ret;
}

UTCTimePoint& UTCTimePoint::operator+=(const Duration& duration) {
  Add(duration);
  return *this;
}

UTCTimePoint& UTCTimePoint::operator-=(const Duration& duration) {
  Sub(duration);
  return *this;
}

UTCTimePoint operator+(const UTCTimePoint& tp, const Duration& dur) {
  UTCTimePoint rtp(tp);
  rtp.Add(dur);
  return rtp;
}

UTCTimePoint operator-(const UTCTimePoint& tp, const Duration& dur) {
  UTCTimePoint rtp(tp);
  rtp.Sub(dur);
  return rtp;
}

void SetUTCTimeZone() {
  setenv("TZ", "UTC", 1);
  tzset();
}

std::string GMTTimeNowString() {
  time_t rawtime;
  struct tm* timeinfo;
  std::time(&rawtime);
  timeinfo = gmtime(&rawtime);  // get GMT time
  char buf[48];
  std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
  return std::string(buf, std::strlen(buf));
}

struct UTCTimeZoneIniter {
  UTCTimeZoneIniter() {
    SetUTCTimeZone();
  }
} _g_utc_tz_initer;

}  // namespace time
}  // namespace ahrimq