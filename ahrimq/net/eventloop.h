#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

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

#include "epoller.h"
#include "tcp/tcpconn.h"

namespace ahrimq {

struct TCPConn;
class Epoller;

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

#endif  // _EVENTLOOP_H_