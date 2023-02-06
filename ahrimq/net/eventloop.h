#ifndef _AHRIMQ_NET_EVENTLOOP_H_
#define _AHRIMQ_NET_EVENTLOOP_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "net/epoller.h"
#include "net/reactor_conn.h"

namespace ahrimq {

class Epoller;
class ReactorConn;

struct EventLoop : public NoCopyable {
  Epoller *epoller = nullptr;
  std::atomic_bool stopped{false};

  EventLoop();

  ~EventLoop();

  void Loop();

  void Stop();
};

typedef std::shared_ptr<EventLoop> EventLoopPtr;

}  // namespace ahrimq

#endif  // _AHRIMQ_NET_EVENTLOOP_H_