#ifndef _BASE_NOCOPYABLE_H_
#define _BASE_NOCOPYABLE_H_

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

#endif  // _BASE_NOCOPYABLE_H_