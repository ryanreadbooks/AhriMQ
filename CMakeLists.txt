cmake_minimum_required(VERSION 3.5)

project(AhriMQ)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -ggdb -Wall -Wno-unused")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall -Wno-unused")
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

option(BUILD_TESTING "Build all testings" ON)
message(STATUS "BUILD_TESTING = ${BUILD_TESTING}")
if (BUILD_TESTING)
  find_package(GTest REQUIRED)
  if (NOT ${GTEST_FOUND})
    message(FATAL_ERROR "Can not find GTest, abort generating - aborted")
  else()
    message(STATUS "GTest found in ${GTEST_MAIN_LIBRARIES} - done")
  endif()
  enable_testing()
endif()

list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_LIST_DIR}/CMake
)
include(AhriMQHelpers)

include_directories(ahrimq)
add_subdirectory(ahrimq)