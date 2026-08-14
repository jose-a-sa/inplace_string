#include <gmock/gmock.h>

#include <cstdint>
#include <limits>
#include <stdexcept>

#include <qx/inplace_string.h>

TEST(InplaceString, ToInplaceStringSizedThrowsForInsufficientCapacity)
{
    EXPECT_THROW(((void)qx::to_inplace_string<2, int>(255)), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<2, std::uint8_t>(255)), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<2>(255)), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<3, int>(1000)), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<4, int>(-1000)), std::length_error);
}

TEST(InplaceString, TryToInplaceStringReportsCapacityFit)
{
    EXPECT_FALSE((qx::try_to_inplace_string<2, int>(255)));
    EXPECT_FALSE((qx::try_to_inplace_string<2, std::uint8_t>(255)));
    EXPECT_FALSE((qx::try_to_inplace_string<2>(255)));

    EXPECT_TRUE((qx::try_to_inplace_string<3, int>(255)));
    EXPECT_TRUE((qx::try_to_inplace_string<3, std::uint8_t>(255)));
    EXPECT_TRUE((qx::try_to_inplace_string<3>(255)));

    EXPECT_TRUE((qx::try_to_inplace_string<5, int>(-42)));
    EXPECT_FALSE((qx::try_to_inplace_string<4, int>(10000)));
    EXPECT_TRUE((qx::try_to_inplace_string<20, std::int64_t>(std::numeric_limits<std::int64_t>::min())));
}

TEST(InplaceString, UncheckedToInplaceStringUsesExplicitCapacity)
{
    auto const positive = qx::unchecked_to_inplace_string<5>(12345);
    EXPECT_EQ(positive, "12345");
    EXPECT_EQ(positive.size(), 5U);

    auto const negative = qx::unchecked_to_inplace_string<4>(-42);
    EXPECT_EQ(negative, "-42");
    EXPECT_EQ(negative.size(), 3U);

    auto const minimum = qx::unchecked_to_inplace_string<20>(std::numeric_limits<std::int64_t>::min());
    EXPECT_EQ(minimum, "-9223372036854775808");
    EXPECT_EQ(minimum.size(), 20U);
}

TEST(InplaceString, UncheckedToInplaceStringContractViolation)
{
    EXPECT_DEATH(((void)qx::unchecked_to_inplace_string<2>(255)), "contract violation");
}

TEST(InplaceString, ToInplaceStringUsesExplicitCapacity)
{
    auto const explicit_three = qx::to_inplace_string<3>(255);
    EXPECT_EQ(explicit_three, "255");
    EXPECT_EQ(explicit_three.size(), 3U);

    auto const explicit_signed = qx::to_inplace_string<4>(-42);
    EXPECT_EQ(explicit_signed, "-42");
    EXPECT_EQ(explicit_signed.size(), 3U);

    auto const explicit_u64 = qx::to_inplace_string<20>(std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(explicit_u64, "18446744073709551615");
    EXPECT_EQ(explicit_u64.size(), 20U);
}

TEST(InplaceString, ToInplaceStringUsesDefaultCapacityForIntegerLiterals)
{
    auto const zero = qx::to_inplace_string(std::uint8_t{0});
    EXPECT_EQ(zero, "0");
    EXPECT_EQ(zero.size(), 1U);

    auto const forty_two = qx::to_inplace_string(std::int8_t{42});
    EXPECT_EQ(forty_two, "42");
    EXPECT_EQ(forty_two.size(), 2U);

    auto const negative = qx::to_inplace_string(std::int16_t{-12345});
    EXPECT_EQ(negative, "-12345");
    EXPECT_EQ(negative.size(), 6U);

    auto const large_unsigned = qx::to_inplace_string(std::uint32_t{1234567890});
    EXPECT_EQ(large_unsigned, "1234567890");
    EXPECT_EQ(large_unsigned.size(), 10U);

    auto const large_signed = qx::to_inplace_string(std::int32_t{-1234567890});
    EXPECT_EQ(large_signed, "-1234567890");
    EXPECT_EQ(large_signed.size(), 11U);
}

TEST(InplaceString, ToInplaceStringUsesDefaultCapacityForIntegerLimits)
{
    auto const max_u32 = qx::to_inplace_string(std::numeric_limits<std::uint32_t>::max());
    EXPECT_EQ(max_u32, "4294967295");
    EXPECT_EQ(max_u32.size(), 10U);

    auto const max_i32 = qx::to_inplace_string(std::numeric_limits<std::int32_t>::max());
    EXPECT_EQ(max_i32, "2147483647");
    EXPECT_EQ(max_i32.size(), 10U);

    auto const min_i32 = qx::to_inplace_string(std::numeric_limits<std::int32_t>::min());
    EXPECT_EQ(min_i32, "-2147483648");
    EXPECT_EQ(min_i32.size(), 11U);

    auto const max_u64 = qx::to_inplace_string(std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(max_u64, "18446744073709551615");
    EXPECT_EQ(max_u64.size(), 20U);

    auto const min_i64 = qx::to_inplace_string(std::numeric_limits<std::int64_t>::min());
    EXPECT_EQ(min_i64, "-9223372036854775808");
    EXPECT_EQ(min_i64.size(), 20U);
}

TEST(InplaceString, ToInplaceStringUsesDefaultCapacityForFloatingPointValues)
{
    auto const decimal_float = qx::to_inplace_string(3.5f);
    EXPECT_EQ(decimal_float, "3.5");
    EXPECT_EQ(decimal_float.size(), 3U);

    auto const decimal_double = qx::to_inplace_string(1.25);
    EXPECT_EQ(decimal_double, "1.25");
    EXPECT_EQ(decimal_double.size(), 4U);
}

TEST(InplaceString, ToInplaceStringUsesDefaultCapacityForFloatingPointLimits)
{
    auto const max_float = qx::to_inplace_string(std::numeric_limits<float>::max());
    EXPECT_EQ(max_float, "3.4028235e+38");
    EXPECT_EQ(max_float.size(), 13U);

    auto const lowest_float = qx::to_inplace_string(std::numeric_limits<float>::lowest());
    EXPECT_EQ(lowest_float, "-3.4028235e+38");
    EXPECT_EQ(lowest_float.size(), 14U);

    auto const max_double = qx::to_inplace_string(std::numeric_limits<double>::max());
    EXPECT_EQ(max_double, "1.7976931348623157e+308");
    EXPECT_EQ(max_double.size(), 23U);

    auto const lowest_double = qx::to_inplace_string(std::numeric_limits<double>::lowest());
    EXPECT_EQ(lowest_double, "-1.7976931348623157e+308");
    EXPECT_EQ(lowest_double.size(), 24U);
}

TEST(InplaceString, ToInplaceStringSizedThrowsForInsufficientFloatingPointCapacity)
{
    EXPECT_NO_THROW(((void)qx::to_inplace_string<13>(std::numeric_limits<float>::max())));
    EXPECT_THROW(((void)qx::to_inplace_string<12>(std::numeric_limits<float>::max())), std::length_error);
    EXPECT_NO_THROW(((void)qx::to_inplace_string<23>(std::numeric_limits<double>::max())));
    EXPECT_THROW(((void)qx::to_inplace_string<22>(std::numeric_limits<double>::max())), std::length_error);
}

TEST(InplaceString, TryToInplaceStringFloatingPointCapacityBoundary)
{
    EXPECT_TRUE(qx::try_to_inplace_string<13>(std::numeric_limits<float>::max()));
    EXPECT_FALSE(qx::try_to_inplace_string<12>(std::numeric_limits<float>::max()));
    EXPECT_TRUE(qx::try_to_inplace_string<23>(std::numeric_limits<double>::max()));
    EXPECT_FALSE(qx::try_to_inplace_string<22>(std::numeric_limits<double>::max()));
}

TEST(InplaceString, ToInplaceStringHandlesFloatingPointExponentExtremes)
{
    auto const float_denorm = qx::to_inplace_string(std::numeric_limits<float>::denorm_min());
    auto const double_denorm = qx::to_inplace_string(std::numeric_limits<double>::denorm_min());
    auto const float_min = qx::to_inplace_string(std::numeric_limits<float>::min());
    auto const double_min = qx::to_inplace_string(std::numeric_limits<double>::min());

    EXPECT_FALSE(float_denorm.empty());
    EXPECT_FALSE(double_denorm.empty());
    EXPECT_FALSE(float_min.empty());
    EXPECT_FALSE(double_min.empty());
}

TEST(InplaceString, ToInplaceStringHandlesNegativeFloatingPointLimits)
{
    auto const float_value = qx::to_inplace_string(std::numeric_limits<float>::lowest());
    auto const double_value = qx::to_inplace_string(std::numeric_limits<double>::lowest());

    EXPECT_FALSE(float_value.empty());
    EXPECT_FALSE(double_value.empty());

    EXPECT_EQ(float_value.front(), '-');
    EXPECT_EQ(double_value.front(), '-');
}

TEST(InplaceString, ToInplaceStringHandlesFloatingPointSpecialValues)
{
    EXPECT_EQ(qx::to_inplace_string(std::numeric_limits<float>::infinity()), "inf");
    EXPECT_EQ(qx::to_inplace_string(-std::numeric_limits<float>::infinity()), "-inf");
    EXPECT_EQ(qx::to_inplace_string(std::numeric_limits<double>::infinity()), "inf");
    EXPECT_EQ(qx::to_inplace_string(-std::numeric_limits<double>::infinity()), "-inf");

    auto const nan = qx::to_inplace_string(std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(nan, "nan");
}

TEST(InplaceString, ToInplaceStringHandlesFloatingPointExplicitCapacity)
{
    EXPECT_NO_THROW({
        auto const max_float = qx::to_inplace_string<13>(std::numeric_limits<float>::max());
        EXPECT_EQ(max_float, "3.4028235e+38");
    });
    EXPECT_NO_THROW({
        auto const negative_float = qx::to_inplace_string<14>(std::numeric_limits<float>::lowest());
        EXPECT_EQ(negative_float, "-3.4028235e+38");
    });
    EXPECT_NO_THROW({ 
        auto const max_double = qx::to_inplace_string<23>(std::numeric_limits<double>::max());
        EXPECT_EQ(max_double, "1.7976931348623157e+308");
    });
    EXPECT_NO_THROW({ 
        auto const negative_double = qx::to_inplace_string<24>(std::numeric_limits<double>::lowest());
        EXPECT_EQ(negative_double, "-1.7976931348623157e+308");
    });
}

TEST(InplaceString, ToInplaceStringSizedThrowsForInsufficientFloatingPointCapacityAtExtremes)
{
    EXPECT_THROW(((void)qx::to_inplace_string<12>(std::numeric_limits<float>::max())), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<13>(std::numeric_limits<float>::lowest())), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<21>(std::numeric_limits<double>::max())), std::length_error);
    EXPECT_THROW(((void)qx::to_inplace_string<22>(std::numeric_limits<double>::lowest())), std::length_error);
}

TEST(InplaceString, ToInplaceStringHandlesSignedZero)
{
    auto const negative_float = qx::to_inplace_string(-0.0f);
    auto const negative_double = qx::to_inplace_string(-0.0);

    EXPECT_EQ(negative_float, "-0");
    EXPECT_EQ(negative_double, "-0");
}