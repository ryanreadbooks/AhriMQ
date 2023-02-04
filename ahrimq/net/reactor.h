#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <cassert>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <sys/socket.h>

#include "buffer/buffer.h"
#include "net/addr.h"
#include "net/epoller.h"
#include "net/eventloop.h"
#include "net/reactor_conn.h"
#include "net/utils.h"

namespace ahrimq {

typedef std::function<void(ReactorConn* conn, bool, bool&)> ReactorReadEventHandler;
typedef std::function<void(ReactorConn* conn, bool&)> ReactorGenericEventHandler;

constexpr static int kNetReadBufSize = 65536;

/// @brief Reactor model implementation
class Reactor : public NoCopyable {
 public:
  Reactor(const std::string& ip, uint16_t port, uint32_t num);

  ~Reactor();

  void React();

  void Wait();

  void Stop();

  /// @brief Close given connection.
  /// @param conn
  void CloseConn(ReactorConn* conn);

  uint32_t NumLoops() const {
    return num_loop_;
  }

  void SetEventReadHandler(const ReactorReadEventHandler& hdr) {
    ev_read_handler_ = hdr;
  }

  void SetEventCloseHandler(const ReactorGenericEventHandler& hdr) {
    ev_close_handler_ = hdr;
  }

  void SetEventWriteHandler(const ReactorGenericEventHandler& hdr) {
    ev_write_handler_ = hdr;
  }

  void SetEventAcceptHandler(const ReactorGenericEventHandler& hdr) {
    ev_accept_handler_ = hdr;
  }

 private:
  bool InitEventLoops();

  bool InitAddr(const std::string& ip, uint16_t port);

  bool InitAcceptor();

  void Acceptor(ReactorConn* conn, bool& closed);

  void Reader(ReactorConn* conn, bool& closed);

  void SendFileAndUpdate(ReactorConn* conn, int outfd);

  void InvokeWriteDoneHandler(ReactorConn* conn, Buffer* wbuf);

  void Writer(ReactorConn* conn, bool& closed);

  EventLoop* EventLoopSelector();

 private:
  // the number of eventloop in one reactor
  uint32_t num_loop_;
  // the eventloops in reactor
  std::vector<EventLoopPtr> eventloops_;
  // acceptor
  ReactorConnPtr acceptor_;
  // ipv4 address
  IPAddr4Ptr addr_;
  // all connections
  std::unordered_map<std::string, ReactorConnPtr> conns_;

  // all threads that run eventloops
  std::vector<std::thread> worker_threads_;
  mutable std::mutex mtx_;
  std::condition_variable cond_;

  // connection id
  static uint64_t next_conn_id_;

  // random number generator
  std::mt19937_64 rand_engine_;

  // handlers
  // ev_read_handler_ is called every time EPOLLIN is reached
  ReactorReadEventHandler ev_read_handler_;
  // ev_close_handler_ is called every time a connection is closed
  ReactorGenericEventHandler ev_close_handler_;
  // ev_write_handler_ is called every time EPOLLOUT is reached
  ReactorGenericEventHandler ev_write_handler_;
  // ev_accept_handler_ is called every time a new connection is open
  ReactorGenericEventHandler ev_accept_handler_;
};

typedef std::shared_ptr<Reactor> ReactorPtr;

}  // namespace ahrimq

#endif  // _REACTOR_H_