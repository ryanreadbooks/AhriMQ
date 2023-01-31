cmake_minimum_required(VERSION 3.5)

ahrimq_add_cc_executable(
  NAME
    echo_server
  SRCS
    "echo_server.cc"
  LINKS
    pthread
    ahrimq::base
    ahrimq::buffer
    ahrimq::net
)

ahrimq_add_cc_executable(
  NAME
    http_server
  SRCS
    "http_server.cc"
  LINKS
    pthread
    ahrimq::net
    ahrimq::buffer
    ahrimq::base
)