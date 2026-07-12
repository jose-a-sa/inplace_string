#include <benchmark/benchmark.h>

#include <string>
#include <string_view>

#include <qx/inplace_string.h>

namespace
{
constexpr std::string_view kMediumPayload = "the quick brown fox jumps over the lazy dog";

template <typename StringT>
static void BM_Replace(benchmark::State& state)
{
    StringT value(kMediumPayload.data(), kMediumPayload.size());
    for (auto _ : state)
    {
        value.replace(4, 5, "rapid");
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK_TEMPLATE(BM_Replace, std::string);
BENCHMARK_TEMPLATE(BM_Replace, qx::inplace_string<126>);

} // namespace

BENCHMARK_MAIN();
