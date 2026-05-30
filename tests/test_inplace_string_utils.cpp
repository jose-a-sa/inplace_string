#include <qx/inplace_string.h>

#include <gtest/gtest.h>

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

TEST(InplaceStringSubstrTest, SubstrExtraction)
{
    qx::inplace_string<12> const s("hello world");

    // Exact bounds slice
    auto sub1 = s.substr(0, 5);
    EXPECT_STREQ(sub1.c_str(), "hello");

    // Offset match to end using default count (npos)
    auto sub2 = s.substr(6);
    EXPECT_STREQ(sub2.c_str(), "world");

    // Count exceeding remaining size clamped correctly
    auto sub3 = s.substr(6, 50);
    EXPECT_STREQ(sub3.c_str(), "world");

    // Out of range offset validation
    EXPECT_THROW(s.substr(20, 1), std::out_of_range);
}

TEST(InplaceStringCompareTest, CompareMemberFunctions)
{
    qx::inplace_string<10> const s1("abc");
    qx::inplace_string<10> const s2("def");
    qx::inplace_string<10> const s3("abc");

    // Direct comparison states
    EXPECT_LT(s1.compare(s2), 0);
    EXPECT_GT(s2.compare(s1), 0);
    EXPECT_EQ(s1.compare(s3), 0);

    // Interaction with string_view/std::string overrides
    std::string_view sv("abc");
    EXPECT_EQ(s1.compare(sv), 0);

    // Substring variations (pos, count, target)
    qx::inplace_string<20> const long_str("prefix_abc_suffix");
    EXPECT_EQ(long_str.compare(7, 3, "abc"), 0);
    EXPECT_EQ(long_str.compare(7, 3, s1), 0);
}

TEST(InplaceStringCopyTest, CopyToRawBuffer)
{
    qx::inplace_string<10> const s("abcdef");
    char buffer[8] = {0};

    // Copy a subset segment cleanly
    size_t written = s.copy(buffer, 3, 1); // Extract "bcd"
    EXPECT_EQ(written, 3);
    EXPECT_EQ(buffer[0], 'b');
    EXPECT_EQ(buffer[1], 'c');
    EXPECT_EQ(buffer[2], 'd');
    EXPECT_EQ(buffer[3], '\0');

    // Clamp behavior when count exceeds length
    written = s.copy(buffer, 20, 4); // Extract "ef"
    EXPECT_EQ(written, 2);
    EXPECT_EQ(buffer[0], 'e');
    EXPECT_EQ(buffer[1], 'f');

    // Throw checks for out of range offset anchor point
    EXPECT_THROW(s.copy(buffer, 2, 15), std::out_of_range);
}

TEST(InplaceStringIteratorTest, MutableIteratorModification)
{
    qx::inplace_string<10> s("abc");

    // Modify head through mutable begin()
    auto it = s.begin();
    *it = 'x';
    EXPECT_STREQ(s.c_str(), "xbc");

    // Modify tail through mutable reverse iterator rbegin()
    auto rit = s.rbegin();
    *rit = 'z';
    EXPECT_STREQ(s.c_str(), "xbz");
}
