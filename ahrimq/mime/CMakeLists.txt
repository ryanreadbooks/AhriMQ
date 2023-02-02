cmake_minimum_required(VERSION 3.5)

ahrimq_create_dependency(
  NAME
    mime
  SRCS
    "mime.cc"
  INCS
    "mime.h"
  LINKS
    pthread
    ahrimq::base
)