#ifndef _AHRIMQ_NET_ISERVER_H_
#define _AHRIMQ_NET_ISERVER_H_

#include <signal.h>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "net/reactor.h"
#include "net/reactor_conn.h"

namespace ahrimq {

/// @brief IServer is an interface which defines a reactor-based server
class IServer {
 public:
  class Ignorer {
   public:
    explicit Ignorer(int signo) {
      ::signal(signo, SIG_IGN);
    }
  };

 public:
  IServer(ReactorPtr reactor)
      : reactor_(reactor), sigpipe_ignorer_(Ignorer(SIGPIPE)) {}

  virtual ~IServer() {
    reactor_.reset();
  }

  virtual void Run() = 0;

  virtual void Stop() = 0;

 protected:
  virtual void OnStreamOpen(ReactorConn* conn, bool& close_after) = 0;

  virtual void OnStreamClosed(ReactorConn* conn, bool& close_after) = 0;

  virtual void OnStreamReached(ReactorConn* conn, bool allread,
                               bool& close_after) = 0;

  virtual void OnStreamWritten(ReactorConn* conn, bool& close_after) = 0;

 protected:
  virtual void InitReactorHandlers() = 0;

 protected:
  ReactorPtr reactor_;
  mutable std::mutex mtx_;
  mutable std::condition_variable cond_;
  std::atomic<bool> stopped_{true};
  Ignorer sigpipe_ignorer_;
};
}  // namespace ahrimq

#endif  // _AHRIMQ_NET_ISERVER_H_