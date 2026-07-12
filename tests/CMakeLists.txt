include(GoogleTest)
include(FetchContent)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE test_files CONFIGURE_DEPENDS "test_*.cpp")

foreach(_test_file IN LISTS test_files)
    get_filename_component(_test_name "${_test_file}" NAME_WE)
    add_executable(${_test_name} "${_test_file}")
    target_link_libraries(${_test_name} PRIVATE
        GTest::gmock_main
        ${PROJECT_NAMESPACE}::inplace_string
    )
    target_compile_definitions(${_test_name} PRIVATE
        QX_HARDENING_MODE=QX_HARDENING_MODE_ALL
        QX_ASSERT_MODE=QX_ASSERT_MODE_LOG_TRAP
    )
    gtest_discover_tests(${_test_name})
endforeach()

unset(test_files)