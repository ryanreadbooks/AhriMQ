#ifndef _AHRIMQ_CIRCULAR_POOL_HPP_
#define _AHRIMQ_CIRCULAR_POOL_HPP_

#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace ahrimq {
namespace detail {

/// @brief CircularQueue implements a non-safe, fixed-sized circular queue which will
/// not grow automatically when needed.
/// @tparam T
template <typename T>
class CircularQueue {
  friend class InstancePool;

 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;

  /// @brief Construct a circular queue with fixed capacity.
  /// @param capacity
  explicit CircularQueue(size_t capacity);

  /// @brief Construct a circular queue from another circular queue
  /// @param
  CircularQueue(const CircularQueue&);

  /// @brief Copy assignment operator overload
  /// @param
  /// @return
  CircularQueue& operator=(const CircularQueue&);

  /// @brief Move constructor
  /// @param
  CircularQueue(CircularQueue&&) noexcept;

  /// @brief Destroy the circular queue. Note that the destructor only free the
  /// memory of its own. If items in queue has allocated memory in heap, they should
  /// free heap memory on their own.
  ~CircularQueue();

  /// @brief Check if queue is empty.
  /// @return
  bool Empty() const {
    return front_ == rear_;
  }

  /// @brief Check if queue is full.
  /// @return
  bool Full() const {
    return front_ == ((rear_ + 1) % capacity_);
  }

  /// @brief Get number of items in queue.
  /// @return
  size_t Size() const {
    return num_;
  }

  /// @brief Get capacity of pool.
  /// @return
  size_t Capacity() const {
    return capacity_ - 1;
  }

  /// @brief Pushes the given element value to the end of the queue. Throw exception
  /// if queue is full.
  /// @param value
  void Push(const value_type& value) {
    if (Full()) {
      throw std::out_of_range("circular pool is full");
    }
    pointer dest = data_ + rear_;
    *dest = value;
    rear_ = (rear_ + 1) % capacity_;
    num_++;
  }

  /// @brief Pushes the given element value to the end of the queue. Throw exception
  /// if queue is full.
  /// @param value
  void Push(value_type&& value) {
    if (Full()) {
      throw std::out_of_range("circular pool is full");
    }
    pointer dest = data_ + rear_;  // the space where the value should be inserted
    *dest = std::move(value);
    rear_ = (rear_ + 1) % capacity_;
    num_++;
  }

  template <typename... Args>
  void Emplace(Args&&... args) {
    if (Full()) {
      throw std::out_of_range("circular pool is full");
    }
    pointer dest = data_ + rear_;
    new (dest) value_type(std::forward<Args>(args)...);
    rear_ = (rear_ + 1) % capacity_;
    num_++;
  }

  /// @brief Removes an element from the front of the queue. Throw exception if queue
  /// is empty.
  void Pop() {
    if (Empty()) {
      throw std::out_of_range("circular pool is empty");
    }
    // we should pop the front element out of queue and return
    pointer p = data_ + front_;
    front_ = (front_ + 1) % capacity_;
    num_--;
    // invoke destructor
    p->~T();
    // reset space
    memset(p, 0, sizeof(value_type));
  }

  /// @brief Returns reference to the first element in the queue.
  /// @return
  reference Front() {
    if (Empty()) {
      throw std::out_of_range("circular pool is empty");
    }
    return data_[front_];
  }

  /// @brief Returns reference to the first element in the queue.
  /// @return
  const_reference Front() const {
    return data_[front_];
  }

 private:
  /// @brief Get front index.
  /// @return
  size_t FrontIdx() const {
    return front_;
  }

  /// @brief Get rear index.
  /// @return
  size_t RearIdx() const {
    return rear_;
  }

  void IterateAssignFrom(const CircularQueue& other) {
    size_t ofront = other.front_;
    size_t orear = other.rear_;
    // reorganize data placement
    front_ = 0;
    rear_ = 0;
    while (ofront != orear) {
      pointer dest = data_ + rear_;
      // this will invoke value_type's operator=(xxx)
      *dest = *(other.data_ + ofront);
      ofront = ((ofront + 1) % other.capacity_);
      rear_++;
    }
    num_ = other.num_;
  }

  void DeepCopy(const CircularQueue& other) {
    // deep copy
    capacity_ = other.capacity_;
    Allocate();
    // copy construct existing items
    IterateAssignFrom(other);
  }

  void Allocate() {
    size_t s = sizeof(value_type) * capacity_;
    data_ = (pointer)::operator new[](s);
    memset(data_, 0, s);  // !!WE MUST SET ALLOCATED SPACE TO 0!!
  }

  void Destroy() {
    if (data_ != nullptr) {
      // call existing items destructor
      while (!Empty()) {
        Pop();
      }
      ::operator delete[](data_);
      data_ = nullptr;
    }
  }

 private:
  size_t capacity_;
  size_t num_ = 0;
  // front_ indicates the index of next pop item
  size_t front_ = 0;
  // rear_ indicates the index of next push item
  size_t rear_ = 0;
  pointer data_ = nullptr;
};

template <typename T>
CircularQueue<T>::CircularQueue(size_t capacity) : capacity_(capacity + 1) {
  // allocate space but not construct
  Allocate();
}

template <typename T>
CircularQueue<T>::~CircularQueue() {
  Destroy();
}

template <typename T>
CircularQueue<T>::CircularQueue(const CircularQueue& other) {
  DeepCopy(other);
}

template <typename T>
CircularQueue<T>& CircularQueue<T>::operator=(const CircularQueue& other) {
  if (this == &other) {
    return *this;
  }

  if (capacity_ >= other.capacity_) {
    // reuse existing space and copy construct existing items
    IterateAssignFrom(other);
  } else {
    // existing space is not enough, we should use new space
    // but first we have to free the existing space and destruct existing items
    Destroy();
    DeepCopy(other);
  }

  return *this;
}

template <typename T>
CircularQueue<T>::CircularQueue(CircularQueue&& other) noexcept {
  // steal from other
  data_ = other.data_;
  capacity_ = other.capacity_;
  num_ = other.num_;
  front_ = other.front_;
  rear_ = other.rear_;

  other.data_ = nullptr;
  other.capacity_ = 1;
  other.num_ = 0;
  other.front_ = 0;
  other.rear_ = 0;
}

}  // namespace detail
}  // namespace ahrimq

#endif  // _AHRIMQ_CIRCULAR_POOL_HPP_