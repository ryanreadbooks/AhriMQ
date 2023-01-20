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

#include "addr.h"
#include "buffer/buffer.h"
#include "epoller.h"
#include "eventloop.h"
#include "tcp/tcpconn.h"
#include "utils.h"

namespace ahrimq {

typedef short ReadWriteStatusCode;

constexpr static int kNetReadBufSize = 65536;

/* original status */
const static ReadWriteStatusCode kStatusNormal = 0;
/* peer closed status */
const static ReadWriteStatusCode kStatusClosed = 1;
/* all data in socket has been read */
const static ReadWriteStatusCode kStatusExhausted = 2;
/* error status */
const static ReadWriteStatusCode kStatusError = 3;

struct IOReadWriteStatus {
  size_t n_bytes;
  ReadWriteStatusCode statuscode;
};

/**
 * @brief Read all bytes from non-blocked fd to buf till fd's read buffer is
 * exhausted(EAGAIN | EWOULDBLOCK)
 *
 * @param fd file descriptor to read from
 * @param buf dynamic buffer to read to
 * @return IOReadWriteStatus status indicating result
 */
IOReadWriteStatus ReadFromFdToBuf(int fd, Buffer& buf);

/**
 * @brief reactor model implementation
 *
 */
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

  void SetTCPCallback(TCPCallback cb) {
    on_read_done_cb_ = std::move(cb);
  }

  uint32_t NumLoops() const {
    return num_loop_;
  }

 private:
  bool InitEventLoops();

  bool InitAddr(const std::string& ip, uint16_t port);

  bool InitAcceptor();

  void Acceptor(TCPConn* conn, bool& closed);

  void Reader(TCPConn* conn, bool& closed);

  void Writer(TCPConn* conn, bool& closed);

  EventLoop* EventLoopSelector();

 private:
  /* the number of eventloop in one reactor */
  uint32_t num_loop_;
  /* the eventloops in reactor */
  std::vector<EventLoopPtr> eventloops_;
  /* acceptor */
  TCPConnPtr acceptor_;
  /* ipv4 address */
  IPAddr4Ptr addr_;
  /* all connections */
  std::unordered_map<std::string, TCPConnPtr> conns_;

  /* TODO: all threads that run eventloops */
  std::vector<std::thread> worker_threads_;
  std::mutex mtx_;
  std::condition_variable cond_;

  /* connection id */
  static uint64_t next_conn_id_;

  /* tcp connection configs for new connections, these may be changed by TCPConn API
   */
  bool conn_nodelay_;
  bool conn_keepalive_;
  int conn_keepalive_interval_;
  int conn_keepalive_cnt_;

  /* random number generator */
  std::mt19937_64 rand_engine_;

  /* callback */
  TCPCallback on_read_done_cb_;
};

typedef std::shared_ptr<Reactor> ReactorPtr;

}  // namespace ahrimq

#endif  // _REACTOR_H_