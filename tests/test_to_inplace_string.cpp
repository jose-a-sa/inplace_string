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

    auto const infinity = qx::to_inplace_string(std::numeric_limits<float>::infinity());
    EXPECT_EQ(infinity, "inf");
    EXPECT_EQ(infinity.size(), 3U);
}
