#ifndef _AHRIMQ_BASE_NOCOPYABLE_H_
#define _AHRIMQ_BASE_NOCOPYABLE_H_

namespace ahrimq {
  
/// @brief This class is not copyable
class NoCopyable {
 public:
  NoCopyable(const NoCopyable&) = delete;
  NoCopyable& operator=(const NoCopyable&) = delete;

 protected:
  NoCopyable() = default;
  ~NoCopyable() = default;
};

}  // namespace ahrimq

#endif  // _AHRIMQ_BASE_NOCOPYABLE_H_