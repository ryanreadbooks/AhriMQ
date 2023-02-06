#ifndef _AHRIMQ_BASE_MUTEXES_H_
#define _AHRIMQ_BASE_MUTEXES_H_
#include <pthread.h>
#include <semaphore.h>
#include <cstdint>

#include "nocopyable.h"

namespace ahrimq {

class Semaphore {
 public:
  explicit Semaphore(uint32_t count = 0);

  ~Semaphore();

  void Wait();

  void Post();

 private:
  sem_t m_semaphore;
};

class ReadWriteMutex : public NoCopyable {
 public:
  ReadWriteMutex() {
    m_mutex = PTHREAD_RWLOCK_INITIALIZER;
  }

  ~ReadWriteMutex() {
    pthread_rwlock_destroy(&m_mutex);
  }

  void LockForRead() {
    pthread_rwlock_rdlock(&m_mutex);
  }

  void LockForWrite() {
    pthread_rwlock_wrlock(&m_mutex);
  }

  void Unlock() {
    pthread_rwlock_unlock(&m_mutex);
  }

 private:
  pthread_rwlock_t m_mutex;
};

class ReadLockGuard : public NoCopyable {
 public:
  ReadLockGuard(ReadWriteMutex& mutex) : m_mutex(mutex) {
    m_mutex.LockForRead();
    m_locked = true;
  }

  ~ReadLockGuard() {
    if (m_locked) {
      m_mutex.Unlock();
      m_locked = false;
    }
  }

 private:
  ReadWriteMutex& m_mutex;
  bool m_locked = false;
};

class WriteLockGuard : public NoCopyable {
 public:
  WriteLockGuard(ReadWriteMutex& mutex) : m_mutex(mutex) {
    m_mutex.LockForWrite();
    m_locked = true;
  }

  ~WriteLockGuard() {
    if (m_locked) {
      m_mutex.Unlock();
      m_locked = false;
    }
  }

 private:
  ReadWriteMutex& m_mutex;
  bool m_locked = false;
};

}  // namespace ahrimq

#endif  //_AHRIMQ_BASE_MUTEXES_H_