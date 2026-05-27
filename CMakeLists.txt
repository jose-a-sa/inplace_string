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

# ... (FetchContent and library definition remain the same) ...

# 3. Only keep the individual test executables
file(GLOB_RECURSE _qx_tests "tests/*.cpp")

# Exclude any dummy files if they exist
list(REMOVE_ITEM _qx_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/coverage_dummy.cpp")
set(_qx_testnames)

foreach(_test_file IN LISTS _qx_tests)
  get_filename_component(_test_name "${_test_file}" NAME_WE)

  add_executable("${_test_name}" "${_test_file}")
  target_link_libraries("${_test_name}"
    PRIVATE
    gmock_main
    fmt::fmt
    qx::inplace_string
  )
  gtest_discover_tests("${_test_name}")
  list(APPEND _qx_testnames "${_test_name}")
endforeach()

option(ENABLE_COVERAGE "" ON)

if(ENABLE_COVERAGE)
  add_compile_options(--coverage -O0 -fno-inline)
  add_link_options(--coverage)
  find_program(LCOV_EXECUTABLE NAMES lcov)
  find_program(GENHTML_EXECUTABLE NAMES genhtml)

  # 2. Add a custom target that uses these found paths
  if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
    add_custom_target(coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

      # Note the use of nested quotes to protect the wildcards from shell expansion
      COMMAND ${LCOV_EXECUTABLE} --capture --directory .
              --output-file lcov.info
              --include "'*/include/qx/*'"
              --exclude "'*/_deps/*'"
              --exclude "'/opt/homebrew/*'"
              --ignore-errors inconsistent,unsupported
      COMMAND ${GENHTML_EXECUTABLE} lcov.info
              --output-directory coverage_report
              --ignore-errors inconsistent,unsupported,category
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating coverage using tools found in Kit environment..."
    )
    add_dependencies(coverage ${_qx_testnames})
  endif()
endif()