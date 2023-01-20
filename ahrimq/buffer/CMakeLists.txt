cmake_minimum_required(VERSION 3.5)

ahrimq_create_dependency(
  NAME
    buffer
  SRCS
    "buffer.cc"
  INCS
    "buffer.h"
)

ahrimq_add_cc_test(
  NAME
    buffer_test
  SRCS
    "buffer_test.cc"
    "buffer.cc"
)
