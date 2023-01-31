cmake_minimum_required(VERSION 3.5)

ahrimq_create_dependency(
  NAME
    base
  SRCS
    "str_utils.cc"
    "time_utils.cc"
  INCS
    "str_utils.h"
    "time_utils.h"
    "nocopyable.h"
)

ahrimq_add_cc_test(
  NAME
    str_utils_test
  SRCS
    "str_utils_test.cc"
  LINKS
    ahrimq::base
)