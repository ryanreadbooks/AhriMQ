#ifndef _ISERVER_H_
#define _ISERVER_H_

#include "net/reactor.h"

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
  virtual void OnStreamClosed(TCPConn* conn) = 0;

  virtual void OnStreamReached(TCPConn* conn, bool all_been_read) = 0;

  virtual void OnStreamWritten(TCPConn* conn) = 0;  // FIXME maybe this is not needed

 protected:
  virtual void InitReactorConfigs() = 0;

  virtual void InitReactorHandlers() = 0;

 protected:
  ReactorPtr reactor_;
};
}  // namespace ahrimq

#endif  // _ISERVER_H_