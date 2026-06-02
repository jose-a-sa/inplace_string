cmake_minimum_required(VERSION 3.23)
project(InplaceString VERSION 1.0 LANGUAGES CXX)

include(FetchContent)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(NOT CMAKE_CXX_STANDARD_REQUIRED)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(NOT CMAKE_CXX_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

# 2. Library definition and C++17 requirement
add_library(inplace_string INTERFACE)
add_library(qx::inplace_string ALIAS inplace_string)

# Enforce C++17 requirement transitively
target_compile_features(inplace_string INTERFACE cxx_std_17)

target_sources(inplace_string INTERFACE
  FILE_SET HEADERS
  BASE_DIRS "include"
  FILES "include/qx/inplace_string.h"
)

# 3. Tests
enable_testing()
include(GoogleTest)

file(GLOB_RECURSE _qx_tests "tests/*.cpp")

foreach(_test_file IN LISTS _qx_tests)
  get_filename_component(_test_name "${_test_file}" NAME_WE)

  add_executable("${_test_name}" "${_test_file}")
  target_link_libraries("${_test_name}"
    PRIVATE
    gmock_main
    qx::inplace_string
  )
  target_include_directories("${_test_name}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  gtest_discover_tests("${_test_name}")
endforeach()