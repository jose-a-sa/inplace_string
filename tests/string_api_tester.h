#pragma once

#include <gmock/gmock.h>

#include <string>
#include <string_view>

MATCHER_P(TesterResultMatcher, expected, (negation ? "isn't" : "is") + std::string(" equal to ") + ::testing::PrintToString(expected))
{
    if (arg != expected)
    {
        *result_listener << "whose value is " << ::testing::PrintToString(arg);
        return false;
    }
    return true;
}

namespace qx
{

template <class TestingStr>
class string_api_tester
{
public:
    explicit constexpr string_api_tester(TestingStr& test_src) noexcept
        : str_proxy_{test_src}
        , str_(std::string_view(test_src))
    {}

    string_api_tester(string_api_tester const&) = delete;
    string_api_tester& operator=(string_api_tester const&) = delete;
    string_api_tester(string_api_tester&&) = delete;
    string_api_tester& operator=(string_api_tester&&) = delete;
    ~string_api_tester() = default;

    template <class Func>
    string_api_tester& result_eq(Func func)
    {
        result_eq_cmp(func(str_), func(str_proxy_));
        return *this;
    }

    template <class Func>
    string_api_tester const& result_eq(Func func) const
    {
        result_eq_cmp(func(str_), func(str_proxy_));
        return *this;
    }

    template <class Func, class Result>
    string_api_tester& result_eq(Func func, Result res)
    {
        result_eq_cmp(func(str_), func(str_proxy_), std::move(res));
        return *this;
    }

    template <class Func, class Result>
    string_api_tester const& result_eq(Func func, Result res) const
    {
        result_eq_cmp(func(str_), func(str_proxy_), std::move(res));
        return *this;
    }

    template <class Func>
    string_api_tester& self_eq(Func func)
    {
        func(str_);
        func(str_proxy_);
        EXPECT_EQ(str_proxy_.size(), str_.size());
        EXPECT_STREQ(str_proxy_.c_str(), str_.c_str());
        return *this;
    }

private:
    TestingStr& str_proxy_;
    std::string str_;

    template <class ProxyRes, class StrRes>
    static void result_eq_cmp(ProxyRes proxy_res, StrRes str_res)
    {
        EXPECT_THAT(proxy_res, TesterResultMatcher(str_res));
    }

    template <class StrRes, class ProxyRes, class ExpectedRes>
    static void result_eq_cmp(StrRes str_res, ProxyRes proxy_res, ExpectedRes expected_res)
    {
        EXPECT_THAT(proxy_res, TesterResultMatcher(str_res));
        EXPECT_EQ(proxy_res, expected_res);
    }
};

} // namespace qx
