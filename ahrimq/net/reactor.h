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
#include "net/tcp/tcpconn.h"
#include "net/utils.h"

namespace ahrimq {

typedef std::function<void(TCPConn* conn, bool)> ReactorReadEventHandler;
typedef std::function<void(TCPConn* conn)> ReactorEventHandler;

constexpr static int kNetReadBufSize = 65536;

/// @brief Reactor model implementation
class Reactor : public NoCopyable {
 public:
  Reactor(const std::string& ip, uint16_t port, uint32_t num);

  ~Reactor();

  void React();

  void Wait();

  void SetConnNoDelay(bool no_delay) {
    conn_nodelay_ = no_delay;
  }

  bool ConnNoDelay() const {
    return conn_nodelay_;
  }

  void SetConnKeepAlive(bool keepalive) {
    conn_keepalive_ = keepalive;
  }

  bool ConnKeepAlive() const {
    return conn_keepalive_;
  }

  void SetConnKeepAliveInterval(int interval) {
    conn_keepalive_interval_ = interval;
  }

  int ConnKeepAliveInterval() const {
    return conn_keepalive_interval_;
  }

  void SetConnKeepAliveCnt(int cnt) {
    conn_keepalive_cnt_ = cnt;
  }

  int ConnKeepAliveCnt() const {
    return conn_keepalive_cnt_;
  }

  uint32_t NumLoops() const {
    return num_loop_;
  }

  void SetEventReadHandler(ReactorReadEventHandler hdr) {
    ev_read_handler_ = std::move(hdr);
  }

  void SetEventCloseHandler(ReactorEventHandler hdr) {
    ev_close_handler_ = std::move(hdr);
  }

  void SetEventWriteHandler(ReactorEventHandler hdr) {
    ev_write_handler_ = std::move(hdr);
  }

 private:
  bool InitEventLoops();

  bool InitAddr(const std::string& ip, uint16_t port);

  bool InitAcceptor();

  void Acceptor(TCPConn* conn, bool& closed);

  // void Reader(TCPConn* conn, bool& closed);

  void FixedSizeReader(TCPConn* conn, bool& closed);

  void Writer(TCPConn* conn, bool& closed);

  EventLoop* EventLoopSelector();

 private:
  // the number of eventloop in one reactor
  uint32_t num_loop_;
  // the eventloops in reactor
  std::vector<EventLoopPtr> eventloops_;
  // acceptor
  TCPConnPtr acceptor_;
  // ipv4 address
  IPAddr4Ptr addr_;
  // all connections
  std::unordered_map<std::string, TCPConnPtr> conns_;

  // TODO: all threads that run eventloops
  std::vector<std::thread> worker_threads_;
  std::mutex mtx_;
  std::condition_variable cond_;

  // connection id
  static uint64_t next_conn_id_;

  // tcp connection configs for new connections, these may be changed by TCPConn API
  bool conn_nodelay_;
  bool conn_keepalive_;
  int conn_keepalive_interval_;
  int conn_keepalive_cnt_;

  // random number generator
  std::mt19937_64 rand_engine_;

  // handlers
  // ev_read_handler_ is called every time EPOLLIN is reached
  ReactorReadEventHandler ev_read_handler_;
  // ev_close_handler_ is called every time a connection is closed
  ReactorEventHandler ev_close_handler_;
  // ev_write_handler_ is called every time EPOLLOUT is reached
  ReactorEventHandler ev_write_handler_;  // FIXME: may obselete
};

typedef std::shared_ptr<Reactor> ReactorPtr;

}  // namespace ahrimq

#endif  // _REACTOR_H_