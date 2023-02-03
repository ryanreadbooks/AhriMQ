#ifndef _ISERVER_H_
#define _ISERVER_H_

#include <mutex>
#include <atomic>
#include <condition_variable>

#include "net/reactor.h"
#include "net/reactor_conn.h"

namespace ahrimq {

/// @brief IServer is an interface which defines a reactor-based server
class IServer {
 public:
  IServer(ReactorPtr reactor) : reactor_(reactor) {}

  virtual ~IServer() {
    reactor_.reset();
  }

  virtual void Run() = 0;

  virtual void Stop() = 0;

 protected:
  virtual void OnStreamOpen(ReactorConn* conn, bool& close_after) = 0;

  virtual void OnStreamClosed(ReactorConn* conn, bool& close_after) = 0;

  virtual void OnStreamReached(ReactorConn* conn, bool allread, bool& close_after) = 0;

  virtual void OnStreamWritten(ReactorConn* conn, bool& close_after) = 0;

 protected:
  virtual void InitReactorHandlers() = 0;

 protected:
  ReactorPtr reactor_;
  mutable std::mutex mtx_;
  mutable std::condition_variable cond_;
  std::atomic<bool> stopped_;
};
}  // namespace ahrimq

#endif  // _ISERVER_H_