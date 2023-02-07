#ifndef _AHRIMQ_INSTANCE_POOL_HPP_
#define _AHRIMQ_INSTANCE_POOL_HPP_

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "base/nocopyable.h"
#include "circular_queue.hpp"

const size_t kBlockSize = 64;

namespace ahrimq {

/// @brief InstancePool
/// @tparam T the object type
template <typename T>
class InstancePool : public NoCopyable {
 public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using const_reference = const T&;
  using deleter = std::function<void(void*)>;
  using unique_pointer = std::unique_ptr<value_type, deleter&>;
  using PoolPtr = unique_pointer;

  InstancePool(size_t n = kBlockSize);

  ~InstancePool();

  template <typename... Args>
  unique_pointer Construct(Args&&... args) {
    std::unique_lock<std::mutex> lck(mtx_);
    if (que_->Empty()) {
      Grow();
    }
    pointer allocated = que_->Front();  // size = sizeof(value_type)
    que_->Pop();
    // construct using placement new
    new (allocated) value_type(std::forward<Args>(args)...);
    return unique_pointer(allocated, deleter_);
  }

 private:
  // non-thread safe
  void Grow() {
    // allocate new space for more data
    std::unique_ptr<detail::CircularQueue<pointer>> newque =
        std::make_unique<detail::CircularQueue<pointer>>(que_->Capacity() * 2);
    // more space
    size_t s = sizeof(value_type) * que_->Capacity();
    pointer start = (pointer)::operator new[](s);
    start_.push_back(start);
    for (size_t i = 0; i < que_->Capacity(); i++) {
      newque->Push(start + i);
    }
    // move all existing data in que_ into newque
    while (!que_->Empty()) {
      newque->Push(que_->Front());
      que_->Pop();
    }
    // replace the old queue
    que_.reset();
    que_.swap(newque);
  }

 private:
  std::vector<pointer> start_;
  std::unique_ptr<detail::CircularQueue<pointer>> que_;
  mutable std::mutex mtx_;
  deleter deleter_;
};

template <typename T>
InstancePool<T>::InstancePool(size_t n)
    : que_(new detail::CircularQueue<pointer>(n)) {
  size_t s = sizeof(value_type) * n;
  pointer start = (pointer)::operator new[](s);
  start_.push_back(start);
  for (size_t i = 0; i < n; i++) {
    que_->Push(start + i);
  }

  deleter_ = [this](void* ptr) -> void {
    if (ptr != nullptr) {
      // ptr will be the pointer which is allocated
      // call value_type's destructor
      pointer p = static_cast<pointer>(ptr);
      p->~T();
      // put the pointer back
      std::unique_lock<std::mutex> lck(mtx_);
      que_->Push(p);
      memset(p, 0, sizeof(value_type));  // empty the allocated space
    }
  };
}

template <typename T>
InstancePool<T>::~InstancePool() {
  for (auto& start : start_) {
    ::operator delete[](start);
  }
}

}  // namespace ahrimq

#endif  //_AHRIMQ_INSTANCE_POOL_HPP_