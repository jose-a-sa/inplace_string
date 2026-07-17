#include <benchmark/benchmark.h>

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

constexpr std::string_view kRepeatedPayload = "qx::inplace_string";

template <typename StringT>
void BM_InplaceStr_AppendSSO(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s;
        s.append(kSSOPayload.data(), kSSOPayload.size());
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_AppendSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_AppendSSO, qx::inplace_string<kEquivStackN>);

template <typename StringT>
void BM_InplaceStr_AppendRepeated(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s;
        s.reserve(254);
        for (int i = 0; i < 24; ++i)
        {
            s.append(kRepeatedPayload.data(), kRepeatedPayload.size());
        }
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_AppendRepeated, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_AppendRepeated, qx::inplace_string<509>);

} // namespace

BENCHMARK_MAIN();
