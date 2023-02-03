#include "base/mutexes.h"

namespace ahrimq {

Semaphore::Semaphore(uint32_t count) {
  sem_init(&m_semaphore, 0, count);
}

Semaphore::~Semaphore() {
  sem_destroy(&m_semaphore);
}

void Semaphore::Wait() {
  sem_wait(&m_semaphore);
}

void Semaphore::Post() {
  sem_post(&m_semaphore);
}

}  // namespace ahrimq