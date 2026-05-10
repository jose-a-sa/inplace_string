cmake_minimum_required(VERSION 3.16)
project(QxProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# fetchconcent fmt
include(FetchContent)

# 1. Set fmt-specific flags to disable extras
set(FMT_TEST OFF CACHE INTERNAL "")
set(FMT_DOC OFF CACHE INTERNAL "")
set(FMT_INSTALL OFF CACHE INTERNAL "") # Usually want this if you're installing your project

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

add_library(inplace_string INTERFACE)
add_library(qx::inplace_string ALIAS inplace_string)
target_sources(inplace_string INTERFACE
  FILE_SET HEADERS
  BASE_DIRS "include"
  FILES "include/qx/inplace_string.h"
)


enable_testing()
include(GoogleTest)

file(GLOB_RECURSE _qx_tests CONFIGURE_DEPENDS "tests/*.cpp")

add_executable(inplace_string_tests)

target_sources(inplace_string_tests
  PRIVATE "${_qx_tests}"
)

target_link_libraries(inplace_string_tests
  PRIVATE
  gmock_main
  fmt::fmt
  qx::inplace_string
)

gtest_discover_tests(inplace_string_tests)
