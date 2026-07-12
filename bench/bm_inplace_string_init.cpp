#include <benchmark/benchmark.h>

#include <array>
#include <string>
#include <string_view>
#include <vector>

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

constexpr std::string_view kMediumPayload = "hello darkness my old friend, I have come to talk with you again";

template <class StringT>
static void BM_InplaceStr_InitEmpty(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s;
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitEmpty, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitEmpty, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_InplaceStr_InitFromCStrSSO(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s(kSSOPayload.data());
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_InplaceStr_InitFromCStrSizedSSO(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s(kSSOPayload.data(), kSSOPayload.size());
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrSizedSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrSizedSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_CopyFromStringTSSO(benchmark::State& state)
{
    StringT source(kSSOPayload);
    for (auto _ : state)
    {
        StringT copy(source);
        benchmark::DoNotOptimize(copy);
    }
}

BENCHMARK_TEMPLATE(BM_CopyFromStringTSSO, std::string);
BENCHMARK_TEMPLATE(BM_CopyFromStringTSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_InplaceStr_InitFromCStrMediumPayload(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s(kMediumPayload.data());
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrMediumPayload, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrMediumPayload, qx::inplace_string<126>);

template <class StringT>
static void BM_InplaceStr_InitFromCStrSizedMediumPayload(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s(kMediumPayload.data(), kMediumPayload.size());
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrSizedMediumPayload, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromCStrSizedMediumPayload, qx::inplace_string<126>);

template <class StringT>
static void BM_InplaceStr_InitFromOtherStringSSO(benchmark::State& state)
{
    std::string base(kSSOPayload.data(), kSSOPayload.size());
    for (auto _ : state)
    {
        StringT s(base);
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromOtherStringSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromOtherStringSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_InplaceStr_InitFillWithCharSSO(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT s(15, 'x');
        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFillWithCharSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFillWithCharSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_InplaceStr_InitFromArrayIteratorsSSO(benchmark::State& state)
{
    std::array<char, 15> values{};
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        values[i] = static_cast<char>('a' + (i % 26));
    }

    for (auto _ : state)
    {
        StringT s(values.begin(), values.end());
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromArrayIteratorsSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromArrayIteratorsSSO, qx::inplace_string<kEquivStackN>);

template <class StringT>
static void BM_InplaceStr_InitFromVectorIteratorsSSO(benchmark::State& state)
{
    std::vector<char> values(15, '0');
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        values[i] = static_cast<char>('a' + (i % 26));
    }
    for (auto _ : state)
    {
        StringT s(values.begin(), values.end());
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromVectorIteratorsSSO, std::string);
BENCHMARK_TEMPLATE(BM_InplaceStr_InitFromVectorIteratorsSSO, qx::inplace_string<kEquivStackN>);

} // namespace

BENCHMARK_MAIN();
