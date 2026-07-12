#include <benchmark/benchmark.h>

#include <sstream>
#include <string>
#include <string_view>

#include <qx/inplace_string.h>

namespace
{
    
#ifdef QX_STL_LIBCPP
constexpr std::string_view kSSOPayload = "123456789012345678901";
constexpr std::size_t kEquivStackN = 22;
#else
constexpr std::string_view kSSOPayload = "12345678901234";
constexpr std::size_t kEquivStackN = 30;
#endif

template <class StringT>
static void BM_StreamInsertSSO(benchmark::State& state)
{
    for (auto _ : state)
    {
        std::ostringstream oss;
        oss << StringT(kSSOPayload.data(), kSSOPayload.size());
        benchmark::DoNotOptimize(oss.str());
    }
}

BENCHMARK_TEMPLATE(BM_StreamInsertSSO, std::string);
BENCHMARK_TEMPLATE(BM_StreamInsertSSO, qx::inplace_string<30>);

template <class StringT>
static void BM_StreamExtractSSO(benchmark::State& state)
{
    std::istringstream iss(std::string{kSSOPayload.data(), kSSOPayload.size()});
    for (auto _ : state)
    {
        StringT value;
        iss >> value;
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK_TEMPLATE(BM_StreamExtractSSO, std::string);
BENCHMARK_TEMPLATE(BM_StreamExtractSSO, qx::inplace_string<kEquivStackN>);

} // namespace

BENCHMARK_MAIN();
