#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include "net/tcp/tcp_server.h"

namespace ahrimq {

namespace http {

class HTTPServer : public ahrimq::TCPServer {

};

} // namespace http

} // namespace ahrimq

#endif // _HTTP_SERVER_H_