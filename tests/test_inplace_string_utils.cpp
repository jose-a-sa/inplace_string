#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// ===========================================================================
// Utilities, Capacity & Iterators
// ===========================================================================

TEST(InplaceStringUtils, Resize)
{
    qx::inplace_string<15> s("abc");
    s.resize(6, 'x');
    EXPECT_EQ(s.size(), 6);
    EXPECT_STREQ(s.c_str(), "abcxxx");

    s.resize(3);
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "abc");

    qx::inplace_string<10> s2("abc");
    s2.resize(6); // Default zero-fill
    EXPECT_EQ(s2[3], '\0');

    qx::inplace_string<5> s_small("abc");
    EXPECT_THROW(s_small.resize(10), std::length_error);
}

TEST(InplaceStringUtils, ResizeAndOverwrite)
{
    qx::inplace_string<15> s("hello world");
    s.resize_and_overwrite(5, [](char* buf, std::size_t n) { return std::size_t{3}; });
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "hel");

    qx::inplace_string<20> s2("hi");
    s2.resize_and_overwrite(
        5,
        [](char* buf, std::size_t n)
        {
            for (std::size_t i = 2; i < n; ++i)
                buf[i] = 'x';
            return n;
        }
    );
    EXPECT_EQ(s2.size(), 5);
    EXPECT_STREQ(s2.c_str(), "hixxx");
}

TEST(InplaceStringUtils, ReserveAndCapacity)
{
    qx::inplace_string<10> s("abc");
    s.reserve(5);
    EXPECT_THROW(s.reserve(15), std::length_error);

    s.shrink_to_fit();
    EXPECT_EQ(s.capacity(), 10);
    EXPECT_EQ(s.length(), 3); // Alias test
}

TEST(InplaceStringUtils, Swap)
{
    qx::inplace_string<10> s1("foo");
    qx::inplace_string<10> s2("barbaz");

    s1.swap(s2);
    EXPECT_STREQ(s1.c_str(), "barbaz");
    EXPECT_STREQ(s2.c_str(), "foo");

    qx::swap(s1, s2);
    EXPECT_STREQ(s1.c_str(), "foo");
}

TEST(InplaceStringUtils, Iterators)
{
    qx::inplace_string<5> s("abc");
    EXPECT_EQ(*s.cbegin(), 'a');
    EXPECT_EQ(*s.crbegin(), 'c');

    auto it = s.begin();
    *it = 'x';
    EXPECT_STREQ(s.c_str(), "xbc");

    auto rit = s.rbegin();
    *rit = 'z';
    EXPECT_STREQ(s.c_str(), "xbz");
}

TEST(InplaceStringUtils, ToStringView)
{
    qx::inplace_string<15> const s("abc");
    auto const sv = std::string_view{s};
    EXPECT_EQ(sv, std::string_view("abc"));
}