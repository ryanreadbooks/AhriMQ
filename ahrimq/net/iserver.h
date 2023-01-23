#ifndef _ISERVER_H_
#define _ISERVER_H_

#include <mutex>
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
  virtual void OnStreamOpen(ReactorConn* conn) = 0;

  virtual void OnStreamClosed(ReactorConn* conn) = 0;

  virtual void OnStreamReached(ReactorConn* conn, bool allread) = 0;

  // FIXME maybe this is not needed
  virtual void OnStreamWritten(ReactorConn* conn) = 0;

 protected:
  virtual void InitReactorHandlers() = 0;

 protected:
  ReactorPtr reactor_;
  mutable std::mutex mtx_;
  std::condition_variable cond_;
};
}  // namespace ahrimq

#endif  // _ISERVER_H_