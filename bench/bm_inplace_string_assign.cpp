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

template <typename StringT>
static void BM_InplaceStr_AssignSSO(benchmark::State& state)
{
    StringT s;
    for (auto _ : state)
    {
        s.assign(kSSOPayload.data(), kSSOPayload.size());
    }
    benchmark::DoNotOptimize(s);
}

BENCHMARK_TEMPLATE(BM_InplaceStr_AssignSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_AssignSSO, qx::inplace_string<kEquivStackN>);

} // namespace

BENCHMARK_MAIN();
