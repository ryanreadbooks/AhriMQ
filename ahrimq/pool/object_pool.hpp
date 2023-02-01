#ifndef _POOL_OBJECT_POOL_H_
#define _POOL_OBJECT_POOL_H_

#include <cstddef>
#include <list>

namespace ahrimq {

/// @brief Object pool defines a straightforward object pool.
/// @tparam T the object type
template<typename T>
class ObjectPool {
public:
  ObjectPool(size_t n);

  ~ObjectPool()
};

// implementation
template<typename T>
ObjectPool<T>::ObjectPool(size_t n) {

}

template<typename T>
ObjectPool<T>::~ObjectPool() {
  
}

} // namespace ahrimq

#endif //_POOL_OBJECT_POOL_H_