#pragma once

#include <gmock/gmock.h>

MATCHER_P(DoubleEqNanInf, expected, "NaN/Inf-safe double comparison")
{
    double const actual = arg;

    // NaN handling
    if (std::isnan(expected))
        return std::isnan(actual);

    // Inf handling (preserve sign)
    if (std::isinf(expected))
        return std::isinf(actual) && (std::signbit(actual) == std::signbit(expected));

    // Signed zero handling
    if (expected == 0.0 && actual == 0.0)
        return std::signbit(actual) == std::signbit(expected);

    // Default: delegate to GoogleTest's ULP-based DoubleEq
    return ::testing::Matcher<double>(::testing::DoubleEq(expected)).MatchAndExplain(actual, result_listener);
}

MATCHER_P(FloatEqNanInf, expected, "NaN/Inf-safe float comparison")
{
    float const actual = arg;

    if (std::isnan(expected))
        return std::isnan(actual);

    if (std::isinf(expected))
        return std::isinf(actual) && (std::signbit(actual) == std::signbit(expected));

    if (expected == 0.0f && actual == 0.0f)
        return std::signbit(actual) == std::signbit(expected);

    return ::testing::Matcher<float>(::testing::FloatEq(expected)).MatchAndExplain(actual, result_listener);
}