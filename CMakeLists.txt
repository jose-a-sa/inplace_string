cmake_minimum_required(VERSION 3.23)
project(qxlib VERSION 1.0 LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ==============================================================================
# Component Registration
# ==============================================================================

set(PROJECT_NAMESPACE qx)
set(PROJECT_COMPONENTS)

function(qx_register_component target name)
  set_target_properties(${target} PROPERTIES EXPORT_NAME ${name})
  add_library(${PROJECT_NAMESPACE}::${name} ALIAS ${target})
  install(TARGETS ${target}
    EXPORT ${PROJECT_NAME}-${name}-targets
    FILE_SET HEADERS
    COMPONENT ${name}
  )
  install(EXPORT ${PROJECT_NAME}-${name}-targets
    FILE "${PROJECT_NAME}-${name}-targets.cmake"
    NAMESPACE ${PROJECT_NAMESPACE}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    COMPONENT ${name}
  )
  set(PROJECT_COMPONENTS ${PROJECT_COMPONENTS} ${name} PARENT_SCOPE)
endfunction()

# ==============================================================================
# inplace_string
# ==============================================================================

add_library(qx_inplace_string INTERFACE)
target_compile_features(qx_inplace_string INTERFACE cxx_std_17)
target_sources(qx_inplace_string 
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS "include"
    FILES "include/qx/inplace_string.h"
)
qx_register_component(qx_inplace_string inplace_string)

# ==============================================================================
# Testing
# ==============================================================================
option(QX_BUILD_TESTS "Build tests for qxlib" ${PROJECT_IS_TOP_LEVEL})

if(QX_BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  include(FetchContent)

  FetchContent_Declare(fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
  )
  set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
  # FetchContent_MakeAvailable(fmtlib)

  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
  )

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  file(GLOB_RECURSE _tests CONFIGURE_DEPENDS "tests/*.cpp")

  foreach(_test_file IN LISTS _tests)
    get_filename_component(_test_name "${_test_file}" NAME_WE)
    add_executable(${_test_name} "${_test_file}")
    target_include_directories(${_test_name} PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    target_link_libraries(${_test_name} PRIVATE
      GTest::gmock_main
      # fmt::fmt
      ${PROJECT_NAMESPACE}::inplace_string
    )
    target_compile_definitions(${_test_name} PRIVATE
      QX_HARDENING_MODE=QX_HARDENING_MODE_ALL
      QX_ASSERT_MODE=QX_ASSERT_MODE_LOG_TRAP
    )
    gtest_discover_tests(${_test_name})
  endforeach()

  unset(_tests)
endif()

# ==============================================================================
# Package Config
# ==============================================================================
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in" [=[
@PACKAGE_INIT@

set(_supported_components @PROJECT_COMPONENTS@)

if(${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
  set(_components ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
else()
  set(_components ${_supported_components})
endif()

set(_found_components)
foreach(_comp IN LISTS _components)
  if(NOT _comp IN_LIST _supported_components)
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE "Unsupported component: ${_comp}")
    return()
  endif()
  include("${CMAKE_CURRENT_LIST_DIR}/${CMAKE_FIND_PACKAGE_NAME}-${_comp}-targets.cmake")
  list(APPEND _found_components "${_comp}")
  set(${CMAKE_FIND_PACKAGE_NAME}_${_comp}_FOUND TRUE)
endforeach()

unset(_supported_components)
unset(_components)

check_required_components(${CMAKE_FIND_PACKAGE_NAME})

string(REPLACE ";" ", " _found_components "${_found_components}")
message(STATUS "Found ${CMAKE_FIND_PACKAGE_NAME} ${${CMAKE_FIND_PACKAGE_NAME}_VERSION} (${CMAKE_CURRENT_LIST_FILE}) [components: ${_found_components}]")
unset(_found_components)
]=])

configure_package_config_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
