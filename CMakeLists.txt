cmake_minimum_required(VERSION 3.23)
project(qxlib VERSION 0.1 LANGUAGES CXX)

set(PROJECT_NS qx)
option(QX_BUILD_TESTS "Build tests for qxlib" ${PROJECT_IS_TOP_LEVEL})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ==============================================================================
# Components
# ==============================================================================
set(${PROJECT_NAME}_COMPONENTS)

function(qx_add_component NAME)
  add_library(${NAME} INTERFACE)
  add_library(${PROJECT_NS}::${NAME} ALIAS ${NAME})
  target_compile_features(${NAME} INTERFACE cxx_std_17)

  # Don't install here — caller must add sources first
  list(APPEND ${PROJECT_NAME}_COMPONENTS ${NAME})
  set(${PROJECT_NAME}_COMPONENTS "${${PROJECT_NAME}_COMPONENTS}" PARENT_SCOPE)
endfunction()

function(qx_install_component NAME)
  install(TARGETS ${NAME}
    EXPORT ${PROJECT_NAME}-config
    FILE_SET HEADERS
    COMPONENT ${NAME}
  )
endfunction()

qx_add_component(inplace_string)
target_sources(inplace_string INTERFACE
  FILE_SET HEADERS 
  BASE_DIRS "include"
  FILES "include/qx/inplace_string.h"
)
qx_install_component(inplace_string)

# ==============================================================================
# Testing
# ==============================================================================

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

  file(GLOB_RECURSE _tests CONFIGURE_DEPENDS "tests/*.cpp")

  foreach(_test_file IN LISTS _tests)
    get_filename_component(_test_name "${_test_file}" NAME_WE)
    add_executable("${_test_name}" "${_test_file}")
    target_include_directories("${_test_name}" PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )
    target_link_libraries("${_test_name}" PRIVATE
      GTest::gmock_main
      ${PROJECT_NS}::inplace_string
    )
    gtest_discover_tests("${_test_name}")
  endforeach()

  unset(_tests)
endif()

# ==============================================================================
# Package Config
# ==============================================================================
install(
  EXPORT "${PROJECT_NAME}-config"
  FILE "${PROJECT_NAME}-targets.cmake"
  NAMESPACE ${PROJECT_NS}::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in" [=[
@PACKAGE_INIT@
include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")

set(_found_components)
foreach(_comp @qxlib_COMPONENTS@)
  if(NOT TARGET @PROJECT_NS@::${_comp})
    set(${CMAKE_FIND_PACKAGE_NAME}_${_comp}_FOUND FALSE)
  else()
    set(${CMAKE_FIND_PACKAGE_NAME}_${_comp}_FOUND TRUE)
    list(APPEND _found_components "${_comp}")
  endif()
endforeach()

check_required_components(@PROJECT_NAME@)

string(REPLACE ";" ", " _found_components "${_found_components}")
message(STATUS "Found qxlib ${qxlib_VERSION}: ${CMAKE_CURRENT_LIST_FILE} (found components: ${_found_components})")
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
