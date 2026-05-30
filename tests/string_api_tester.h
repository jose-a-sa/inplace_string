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
    explicit constexpr string_api_tester(TestingStr& test_src) noexcept : test_src_{test_src}, expected_str_(std::string_view(test_src)) {}

    string_api_tester(string_api_tester const&) = delete;
    string_api_tester& operator=(string_api_tester const&) = delete;
    string_api_tester(string_api_tester&&) = delete;
    string_api_tester& operator=(string_api_tester&&) = delete;
    ~string_api_tester() = default;

    template <class Func>
    void result_eq(Func func)
    {
        auto const expected = func(expected_str_);
        auto const actual = func(test_src_);
        EXPECT_THAT(actual, TesterResultMatcher(expected));
    }

    template <class Func>
    void result_eq(Func func) const
    {
        auto const expected = func(expected_str_);
        auto const actual = func(test_src_);
        EXPECT_THAT(actual, TesterResultMatcher(expected));
    }

    template <class Func, class Result>
    void result_eq(Func func, Result res)
    {
        auto const expected = func(expected_str_);
        auto const actual = func(test_src_);
        EXPECT_THAT(actual, TesterResultMatcher(expected));
        EXPECT_EQ(expected, res);
    }

    template <class Func, class Result>
    void result_eq(Func func, Result res) const
    {
        auto const expected = func(expected_str_);
        auto const actual = func(test_src_);
        EXPECT_THAT(actual, TesterResultMatcher(expected));
        EXPECT_EQ(expected, res);
    }

    template <class Func>
    void self_eq(Func func)
    {
        func(expected_str_);
        func(test_src_);
        EXPECT_EQ(test_src_.size(), expected_str_.size());
        EXPECT_STREQ(test_src_.c_str(), expected_str_.c_str());
    }

private:
    TestingStr& test_src_;
    std::string expected_str_;
};

} // namespace qx
