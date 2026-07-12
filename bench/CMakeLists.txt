include(FetchContent)

FetchContent_Declare(benchmark
    GIT_REPOSITORY git@github.com:google/benchmark.git
    GIT_TAG v1.9.5
)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

file(GLOB_RECURSE bench_files CONFIGURE_DEPENDS "bm_*.cpp")

foreach(_bench_file IN LISTS bench_files)
    get_filename_component(_bench_name "${_bench_file}" NAME_WE)
    add_executable(${_bench_name} "${_bench_file}")
    target_link_libraries(${_bench_name} PRIVATE
        benchmark::benchmark
        ${PROJECT_NAMESPACE}::inplace_string
    )
endforeach()

unset(bench_files)
