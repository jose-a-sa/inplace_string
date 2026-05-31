#include <qx/inplace_string.h>

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <list>
#include <vector>

static_assert(qx::intl::is_trivial_contiguous_iterator<std::vector<char>::iterator>::value);
static_assert(!qx::intl::is_trivial_contiguous_iterator<std::deque<char>::iterator>::value);
static_assert(!qx::intl::is_trivial_contiguous_iterator<std::list<char>::iterator>::value);
static_assert(qx::intl::is_trivial_contiguous_iterator<std::array<char, 10>::iterator>::value);
static_assert(qx::intl::is_trivial_contiguous_iterator<std::string::iterator>::value);
static_assert(qx::intl::is_trivial_contiguous_iterator<std::string_view::iterator>::value);

TEST(InplaceStringInitTest, DefaultConstructor)
{
    qx::inplace_string<10> const s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 10);
    EXPECT_EQ(s.max_size(), 10);
    EXPECT_STREQ(s.c_str(), "");
}

TEST(InplaceStringInitTest, CStringConstructor)
{
    qx::inplace_string<10> const s("hello");
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "hello");

    qx::inplace_string<20> const s2("hello world", 7);
    EXPECT_EQ(s2.size(), 7);
    EXPECT_STREQ(s2.c_str(), "hello w");

    auto const* str = "world";
    qx::inplace_string<20> const s3(str);
    EXPECT_EQ(s3.size(), 5);
    EXPECT_STREQ(s3.c_str(), "world");
}

TEST(InplaceStringInitTest, StringViewConstructor)
{
    constexpr std::string_view sv = "world";
    qx::inplace_string<10> const s(sv);
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "world");

    std::string const str = "my test str";
    qx::inplace_string<15> const s2(str);
    EXPECT_EQ(s2.size(), 11);
    EXPECT_STREQ(s2.c_str(), "my test str");

    std::string const str2 = "my_substring";
    qx::inplace_string<15> const s3(str2, 6, 6);
    EXPECT_EQ(s3.size(), 6);
    EXPECT_STREQ(s3.c_str(), "string");
}

TEST(InplaceStringInitTest, FillConstructor)
{
    qx::inplace_string<5> const s(4, 'x');
    EXPECT_EQ(s.size(), 4);
    EXPECT_STREQ(s.c_str(), "xxxx");

    EXPECT_THROW((qx::inplace_string<5>(6, 'x')), std::length_error);
}

TEST(InplaceStringInitTest, CopyConstructorAndSubstring)
{
    qx::inplace_string<10> const orig("abcdef");
    qx::inplace_string<10> const copy(orig, 2, 3);
    EXPECT_EQ(copy.size(), 3);
    EXPECT_STREQ(copy.c_str(), "cde");

    EXPECT_THROW((qx::inplace_string<10>(orig, 20, 1)), std::out_of_range);
}

TEST(InplaceStringInitTest, ElementAccess)
{
    qx::inplace_string<5> s("abc");
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s.at(1), 'b');
    EXPECT_EQ(s.front(), 'a');
    EXPECT_EQ(s.back(), 'c');

    EXPECT_THROW(s.at(5), std::out_of_range);
}

TEST(InplaceStringInitTest, Assignment)
{
    qx::inplace_string<10> s;
    s = "test";
    EXPECT_STREQ(s.c_str(), "test");
    s = 'X';
    EXPECT_STREQ(s.c_str(), "X");

    qx::inplace_string<10> s2;
    s2 = s;
    EXPECT_STREQ(s2.c_str(), "X");
}

TEST(InplaceStringInitTest, RangeConstructor)
{
    std::vector<char> v = {'a', 'b', 'c', 'd'};
    qx::inplace_string<10> const s(v.begin(), v.end());
    EXPECT_EQ(s.size(), 4);
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(InplaceStringInitTest, InitializerListConstructor)
{
    qx::inplace_string<10> const s = {'h', 'e', 'l', 'l', 'o'};
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "hello");
}

TEST(InplaceStringInitTest, MoveConstructor)
{
    qx::inplace_string<10> s1("move_me");
    qx::inplace_string<10> const s2(std::move(s1));
    EXPECT_STREQ(s2.c_str(), "move_me");
    EXPECT_EQ(s2.size(), 7);
    // Note: s1 is in a valid but unspecified state after move
}

TEST(InplaceStringInitTest, MoveAssignment)
{
    qx::inplace_string<10> s1("move_me");
    qx::inplace_string<10> s2;
    s2 = std::move(s1);
    EXPECT_STREQ(s2.c_str(), "move_me");
}

TEST(InplaceStringInitTest, IteratorRangeAssignment)
{
    qx::inplace_string<10> s("old");
    std::string new_data = "new";
    s.assign(new_data.begin(), new_data.end());
    EXPECT_STREQ(s.c_str(), "new");
}

TEST(InplaceStringInitTest, CStringAssignmentBounds)
{
    qx::inplace_string<5> s;
    s.assign("abcd");
    EXPECT_STREQ(s.c_str(), "abcd");

    // Assigning string longer than capacity should throw
    EXPECT_THROW(s.assign("abcdef"), std::length_error);
}
