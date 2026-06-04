cmake_minimum_required(VERSION 3.23)
project(qxlib VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ==============================================================================
# Components
# ==============================================================================
set(qxlib_COMPONENTS)

function(qx_add_component NAME)
  add_library(${NAME} INTERFACE)
  add_library(qx::${NAME} ALIAS ${NAME})

  target_compile_features(${NAME} INTERFACE cxx_std_17)

  install(TARGETS ${NAME}
    EXPORT qxlibTargets
    FILE_SET HEADERS
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  list(APPEND qxlib_COMPONENTS ${NAME})
  set(qxlib_COMPONENTS "${qxlib_COMPONENTS}" PARENT_SCOPE)
endfunction()

qx_add_component(inplace_string)

target_sources(inplace_string INTERFACE
  FILE_SET HEADERS
  BASE_DIRS "include"
  FILES "include/qx/inplace_string.h"
)

# ==============================================================================
# Testing
# ==============================================================================
option(QX_BUILD_TESTS "Build tests for qxlib" ${PROJECT_IS_TOP_LEVEL})

if(QX_BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  include(FetchContent)

  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
  )

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  file(GLOB_RECURSE _qx_tests "tests/*.cpp")

  foreach(_test_file IN LISTS _qx_tests)
    get_filename_component(_test_name "${_test_file}" NAME_WE)
    add_executable("${_test_name}" "${_test_file}")
    target_include_directories("${_test_name}" PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    target_link_libraries("${_test_name}" PRIVATE
      gmock_main
      qx::inplace_string
    )
    gtest_discover_tests("${_test_name}")
  endforeach()

  unset(_qx_tests)
endif()

# ==============================================================================
# Package Config
# ==============================================================================
install(
  EXPORT qxlibTargets
  FILE qxlibTargets.cmake
  NAMESPACE qx::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qxlib
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/qxlibConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qxlibConfig.cmake.in" [=[
@PACKAGE_INIT@
include(CMakeFindDependencyMacro)

include("${CMAKE_CURRENT_LIST_DIR}/qxlibTargets.cmake")
set(_qxlib_known_components "inplace_string")
set(_qxlib_found_components)

foreach(_component IN LISTS qxlib_FIND_COMPONENTS)
    if(_component IN_LIST _qxlib_known_components)
        set(qxlib_${_component}_FOUND TRUE)
        list(APPEND _qxlib_found_components ${_component})
    else()
        set(qxlib_${_component}_FOUND FALSE)
        set(qxlib_FOUND FALSE)
        set(qxlib_NOT_FOUND_MESSAGE "Unknown qxlib component: ${_component}")
    endif()
endforeach()

check_required_components(qxlib)

string(REPLACE ";" ", " _qxlib_found_components "${_qxlib_found_components}")
message(STATUS "Found qxlib ${qxlib_VERSION}: ${CMAKE_CURRENT_LIST_FILE} (found components: ${_qxlib_found_components})")

unset(_qxlib_known_components)
unset(_qxlib_found_components)
]=])

configure_package_config_file(
  "${CMAKE_CURRENT_BINARY_DIR}/qxlibConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/qxlibConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qxlib
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/qxlibConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/qxlibConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qxlib
)
