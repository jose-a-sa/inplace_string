#include <benchmark/benchmark.h>

#include <sstream>
#include <string>
#include <string_view>

#include <qx/inplace_string.h>

namespace
{

#ifdef QX_STL_LIBCPP
constexpr std::string_view kSSOPayload = "123456789012345678901";
constexpr std::size_t kSSOSize = 22;
#else
constexpr std::string_view kSSOPayload = "12345678901234";
constexpr std::size_t kSSOSize = 15;
#endif

constexpr std::size_t kEquivStackN = (((sizeof(std::string) + sizeof(void*) - 1) / sizeof(void*)) * sizeof(void*)) - 2;
constexpr std::string_view kMediumPayload = "hello darkness my old friend, I have come to talk with you again";

template <class StringT>
void BM_InplaceStr_StreamInsertSSO(benchmark::State& state)
{
    StringT value(kSSOPayload.data(), kSSOPayload.size());
    for (auto _ : state)
    {
        std::ostringstream oss;
        oss << value;
        benchmark::DoNotOptimize(oss.str());
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_StreamInsertSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_StreamInsertSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
void BM_InplaceStr_StreamExtractSSO(benchmark::State& state)
{
    std::istringstream iss(std::string{kSSOPayload.data(), kSSOPayload.size()});
    for (auto _ : state)
    {
        StringT value;
        iss >> value;
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_StreamExtractSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_StreamExtractSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
void BM_InplaceStr_StreamInsertMedium(benchmark::State& state)
{
    StringT value(kMediumPayload.data(), kMediumPayload.size());
    for (auto _ : state)
    {
        std::ostringstream oss;
        oss << value;
        benchmark::DoNotOptimize(oss.str());
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_StreamInsertMedium, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_StreamInsertMedium, qx::inplace_string<kMediumPayload.size()>);

template <class StringT>
void BM_InplaceStr_StreamExtractMedium(benchmark::State& state)
{
    std::istringstream iss(std::string{kSSOPayload.data(), kSSOPayload.size()});
    StringT value;
    value.reserve(kSSOPayload.size());
    for (auto _ : state)
    {
        value.clear();
        iss >> value;
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_StreamExtractMedium, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_StreamExtractMedium, qx::inplace_string<kMediumPayload.size()>);

} // namespace

BENCHMARK_MAIN();
