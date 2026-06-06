#include <gmock/gmock.h>

#include <cstdint>
#include <gtest/gtest.h>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "matchers.h"
#include <qx/inplace_string.h>
namespace
{

template <class T>
void test_to_inplace_str(std::initializer_list<std::pair<T, char const*>> pairs)
{
    for (auto [val, expected] : pairs)
    {
        auto const str = qx::to_inplace_string<62>(val);
        auto const exp_sv = std::string_view{expected};
        EXPECT_EQ(str, expected);
        // EXPECT_EQ(str, exp_sv);
        EXPECT_STREQ(str.c_str(), expected);
        EXPECT_EQ(str.size(), exp_sv.size());
    }
}

} // namespace

TEST(InplaceString, ToInplaceStringSized)
{
    EXPECT_THROW(((void)qx::to_inplace_string<2, int>(255)), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<2, uint8_t>(255)), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<2>(255)), std::length_error);
}

TEST(InplaceString, TryToInplaceString)
{
    EXPECT_FALSE((qx::try_to_inplace_string<2, int>(255).has_value()));
    EXPECT_FALSE((qx::try_to_inplace_string<2, uint8_t>(255).has_value()));
    EXPECT_FALSE((qx::try_to_inplace_string<2>(255).has_value()));

    EXPECT_TRUE((qx::try_to_inplace_string<3, int>(255).has_value()));
    EXPECT_TRUE((qx::try_to_inplace_string<3, uint8_t>(255).has_value()));
    EXPECT_TRUE((qx::try_to_inplace_string<3>(255).has_value()));
}

TEST(InplaceString, ToInplaceString)
{
    test_to_inplace_str<uint8_t>({{0U, "0"}, {42U, "42"}, {123U, "123"}, {255U, "255"}});

    // int8_t
    test_to_inplace_str<int8_t>({{0, "0"}, {42, "42"}, {-42, "-42"}, {127, "127"}, {-128, "-128"}});

    // uint16_t
    test_to_inplace_str<uint16_t>({{0U, "0"}, {42U, "42"}, {12345U, "12345"}, {65535U, "65535"}});

    // int16_t
    test_to_inplace_str<int16_t>(
        {{0, "0"}, {42, "42"}, {-42, "-42"}, {12345, "12345"}, {-12345, "-12345"}, {32767, "32767"}, {-32768, "-32768"}}
    );

    // uint32_t
    test_to_inplace_str<uint32_t>(
        {{0U, "0"}, {42U, "42"}, {1234567890U, "1234567890"}, {std::numeric_limits<uint32_t>::max(), "4294967295"}}
    );

    // int32_t
    test_to_inplace_str<int32_t>(
        {{0, "0"},
         {42, "42"},
         {-42, "-42"},
         {1234567890, "1234567890"},
         {-1234567890, "-1234567890"},
         {std::numeric_limits<int32_t>::max(), "2147483647"},
         {std::numeric_limits<int32_t>::min(), "-2147483648"}}
    );

    // uint64_t
    test_to_inplace_str<uint64_t>(
        {{0UL, "0"}, {42UL, "42"}, {1234567890UL, "1234567890"}, {std::numeric_limits<uint64_t>::max(), "18446744073709551615"}}
    );

    // int64_t
    test_to_inplace_str<int64_t>(
        {{0L, "0"},
         {42LL, "42"},
         {-42LL, "-42"},
         {1234567890LL, "1234567890"},
         {-1234567890LL, "-1234567890"},
         {std::numeric_limits<int64_t>::max(), "9223372036854775807"},
         {std::numeric_limits<int64_t>::min(), "-9223372036854775808"}}
    );

    // float
    test_to_inplace_str<float>(
        {{1.23456789555555F, "1.2345679"},
         {23.43F, "23.43"},
         {1e-9F, "1e-09"},
         {std::numeric_limits<float>::infinity(), "inf"},
         {1e-40F, "1e-40"},
         {123456789.0F, "123456792"}}
    );

    // double
    test_to_inplace_str<double>(
        {{1.23456789555555, "1.23456789555555"},
         {23.43, "23.43"},
         {1e-9, "1e-09"},
         {1e40, "1e+40"},
         {1e-40, "1e-40"},
         {123456789.0, "123456789"}}
    );
}

TEST(InplaceString, RoundTripConversionDouble)
{
    auto test_cases = {
        // Zeros
        +0.0,
        -0.0,
        // Infinities
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        // NaN
        std::numeric_limits<double>::quiet_NaN(),
        // Powers of 10
        1e-0,
        1e+1,
        1e+10,
        1e+100,
        1e+200,
        1e+308,
        1e-308,
        1e-309,
        1e-310,
        1e-320,
        1e-323,
        // Boundary values
        DBL_MAX,
        -DBL_MAX,
        std::nextafter(DBL_MAX, 0.0),
        std::numeric_limits<double>::min(),        // smallest normal
        std::numeric_limits<double>::denorm_min(), // smallest subnormal
        std::numeric_limits<double>::min() * std::numeric_limits<double>::epsilon(),
        // Subnormals
        1e-310,
        1e-320,
        // Rounding-sensitive values
        0.1 + 0.2,
        2.675,
        1.005,
        1.0000000000000002,
        1.9999999999999998,
        0.49999999999999994,
        0.50000000000000006,
        // Neighbours of 1.0
        std::nextafter(1.0, 2.0),
        std::nextafter(1.0, 0.0),
        // Large integers
        123456789.0,
        123456789012345.0,
        123456789012345678.0,
        // Negative variants of tricky values
        -1.0000000000000002,
        -0.49999999999999994,
        -std::numeric_limits<double>::min(),
        -std::numeric_limits<double>::denorm_min(),
        // Values around powers of 2 (ULP boundaries)
        std::nextafter(2.0, 3.0),
        std::nextafter(2.0, 1.0),
        std::nextafter(0.5, 1.0),
        std::nextafter(0.5, 0.0),
        // Long decimal expansions that stress buffer length
        1.0 / 3.0,
        2.0 / 3.0,
        1.0 / 7.0,
        1.0 / 9.0,
        1.0 / 6.0,
        // Pi and e
        M_PI,
        M_E,
        M_SQRT2,
        // Values that round-trip poorly with shorter formats
        0.10000000000000001,
        0.20000000000000001,
        0.30000000000000004,
        0.70000000000000007,
    };

    for (auto original : test_cases)
    {
        auto from_chars = qx::to_inplace_string<30>(original);

        double parsed{};
        std::from_chars(from_chars.begin(), from_chars.end(), parsed);

        EXPECT_THAT(parsed, DoubleEqNanInf(original));

        auto parsed_str = qx::to_inplace_string<30>(parsed);
        EXPECT_EQ(from_chars.size(), parsed_str.size());
        EXPECT_STREQ(from_chars.c_str(), parsed_str.c_str());
    }
}

TEST(InplaceString, RoundTripConversionFloat)
{
    auto test_cases = {
        // Zeros
        +0.0f,
        -0.0f,
        // Infinities
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        // NaN
        std::numeric_limits<float>::quiet_NaN(),
        // Powers of 10
        1e+0f,
        1e+1f,
        1e+10f,
        1e+20f,
        1e+38f,
        1e-38f,
        1e-40f,
        1e-44f, // near smallest subnormal
        // Boundary values
        FLT_MAX,
        -FLT_MAX,
        std::nextafter(FLT_MAX, 0.0f),
        std::numeric_limits<float>::min(),        // smallest normal
        std::numeric_limits<float>::denorm_min(), // smallest subnormal
        // Rounding-sensitive values
        0.1f + 0.2f,
        2.675f,
        1.005f,
        1.0000001f,
        0.49999997f,
        0.50000003f,
        // Neighbours of 1.0
        std::nextafter(1.0f, 2.0f),
        std::nextafter(1.0f, 0.0f),
        // Large integers
        123456.0f,
        12345678.0f,
        1234567890.0f,
        // Negative variants of tricky values
        -1.0000001f,
        -0.49999997f,
        -std::numeric_limits<float>::min(),
        -std::numeric_limits<float>::denorm_min(),
        // Values around powers of 2 (ULP boundaries)
        std::nextafter(2.0f, 3.0f),
        std::nextafter(2.0f, 1.0f),
        std::nextafter(0.5f, 1.0f),
        std::nextafter(0.5f, 0.0f),
        // Long decimal expansions
        1.0f / 3.0f,
        2.0f / 3.0f,
        1.0f / 7.0f,
        1.0f / 9.0f,
        // Pi and e (float precision)
        static_cast<float>(M_PI),
        static_cast<float>(M_E),
        static_cast<float>(M_SQRT2),
        // Values that round-trip poorly with shorter formats
        0.10000001f,
        0.20000000f,
        0.30000001f,
        0.69999999f,
    };

    for (auto original : test_cases)
    {
        auto from_chars = qx::to_inplace_string<22>(original);

        float parsed{};
        std::from_chars(from_chars.begin(), from_chars.end(), parsed);

        EXPECT_THAT(parsed, FloatEqNanInf(original));

        auto parsed_str = qx::to_inplace_string<22>(parsed);
        EXPECT_EQ(from_chars.size(), parsed_str.size());
        EXPECT_STREQ(from_chars.c_str(), parsed_str.c_str());
    }
}