#include <benchmark/benchmark.h>

#include <string>
#include <string_view>

#include <qx/inplace_string.h>

namespace
{

constexpr std::string_view kMediumPayload = "the quick brown fox jumps over the lazy dog";

template <typename StringT>
void BM_InplaceStr_FindInMedium(benchmark::State& state)
{
    StringT s(kMediumPayload.data(), kMediumPayload.size());
    for (auto _ : state)
    {
        auto pos = s.find("jumps");
        benchmark::DoNotOptimize(pos);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_FindInMedium, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_FindInMedium, qx::inplace_string<126>);

} // namespace

BENCHMARK_MAIN();
