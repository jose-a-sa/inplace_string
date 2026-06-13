#include <gmock/gmock.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string_view>

#include <qx/inplace_string.h>

namespace
{

template <class T>
void test_to_inplace_str(std::initializer_list<std::pair<T, char const*>> pairs)
{
    for (auto [val, expected] : pairs)
    {
        auto const str = qx::to_inplace_string(val);
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
    EXPECT_FALSE((qx::try_to_inplace_string<2, int>(255)));
    EXPECT_FALSE((qx::try_to_inplace_string<2, uint8_t>(255)));
    EXPECT_FALSE((qx::try_to_inplace_string<2>(255)));

    EXPECT_TRUE((qx::try_to_inplace_string<3, int>(255)));
    EXPECT_TRUE((qx::try_to_inplace_string<3, uint8_t>(255)));
    EXPECT_TRUE((qx::try_to_inplace_string<3>(255)));
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
