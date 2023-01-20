cmake_minimum_required(VERSION 3.5)

ahrimq_create_dependency(
  NAME
    net
    SRCS
    "addr.cc"
    "epoller.cc"
    "eventloop.cc"
    "reactor.cc"
    "utils.cc"
    "tcp/tcpconn.cc"
    "tcp/tcp_server.cc"
  INCS
    "addr.h"
    "epoller.h"
    "eventloop.h"
    "reactor.h"
    "utils.h"
    "tcp/tcpconn.h"
    "tcp/tcp_server.h"
  LINKS
    pthread
)

ahrimq_add_cc_test(
  NAME
    reactor_test
  SRCS
    "reactor_test.cc"
  LINKS
    ahrimq::net
    ahrimq::buffer
)

ahrimq_add_cc_test(
  NAME
    tcpserver_test
  SRCS
    "tcp/tcp_server_test.cc"
  LINKS
    ahrimq::net
    ahrimq::buffer
)
