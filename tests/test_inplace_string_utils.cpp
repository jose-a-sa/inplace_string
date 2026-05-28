#include <gtest/gtest.h>

#include <qx/inplace_string.h>

TEST(InplaceStringUtilsTest, ToStringView)
{
    qx::inplace_string<15> const s("abc");
    auto const sv = std::string_view{s};
    EXPECT_EQ(sv, std::string_view("abc"));
}

TEST(InplaceStringUtilsTest, ResizeLarger)
{
    qx::inplace_string<15> s("abc");
    s.resize(6, 'x');
    EXPECT_EQ(s.size(), 6);
    EXPECT_STREQ(s.c_str(), "abcxxx");
}

TEST(InplaceStringUtilsTest, ResizeSmaller)
{
    qx::inplace_string<10> s("abcdef");
    s.resize(3);
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InplaceStringUtilsTest, ResizeExceedsCapacity)
{
    qx::inplace_string<5> s("abc");
    EXPECT_THROW(s.resize(10), std::length_error);
}

TEST(InplaceStringUtilsTest, ReserveAndShrink)
{
    qx::inplace_string<10> s("abc");
    // These are largely no-ops or basic bounds checks in qx::inplace_string,
    // but they must be called for line coverage.
    s.reserve(5);
    EXPECT_THROW(s.reserve(15), std::length_error);

    s.shrink_to_fit();
    EXPECT_EQ(s.capacity(), 10); // Capacity should remain N
}

TEST(InplaceStringUtilsTest, SwapSameCapacity)
{
    qx::inplace_string<10> s1("foo");
    qx::inplace_string<10> s2("barbaz");

    s1.swap(s2);
    EXPECT_STREQ(s1.c_str(), "barbaz");
    EXPECT_STREQ(s2.c_str(), "foo");
}

// TEST(InplaceStringUtilsTest, SwapDifferentCapacity)
// {
//     qx::inplace_string<10> s1("foo");
//     qx::inplace_string<20> s2("barbaz");
//
//     qx::swap(s1, s2); // Tests the global/friend swap
//     EXPECT_STREQ(s1.c_str(), "barbaz");
//     EXPECT_STREQ(s2.c_str(), "foo");
// }

TEST(InplaceStringUtilsTest, Iterators)
{
    qx::inplace_string<5> s("abc");

    // Const Iterators
    EXPECT_EQ(*s.cbegin(), 'a');
    EXPECT_EQ(*(s.cend() - 1), 'c');

    // Reverse Iterators
    EXPECT_EQ(*s.rbegin(), 'c');
    EXPECT_EQ(*(s.rend() - 1), 'a');

    // Const Reverse Iterators
    EXPECT_EQ(*s.crbegin(), 'c');
    EXPECT_EQ(*(s.crend() - 1), 'a');
}