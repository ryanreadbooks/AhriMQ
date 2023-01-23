cmake_minimum_required(VERSION 3.5)

ahrimq_create_dependency(
  NAME
    net
    SRCS
    "addr.cc"
    "epoller.cc"
    "eventloop.cc"
    "reactor.cc"
    "reactor_conn.cc"
    "utils.cc"
    "tcp/tcpconn.cc"
    "tcp/tcp_server.cc"
  INCS
    "addr.h"
    "epoller.h"
    "eventloop.h"
    "reactor.h"
    "reactor_conn.h"
    "utils.h"
    "tcp/tcpconn.h"
    "tcp/tcp_server.h"
  LINKS
    pthread
)

ahrimq_add_cc_test(
  NAME
    tcp_server_test
  SRCS
    "tcp/tcp_server_test.cc"
  LINKS
    ahrimq::net
    ahrimq::buffer
)
