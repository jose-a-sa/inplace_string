#include <gtest/gtest.h>
#include <iterator>
#include <qx/inplace_string.h>

TEST(InplaceStringTest, AssignString)
{
    qx::inplace_string<10> s1("hello");
    qx::inplace_string<10> const s2("world");

    s1.assign(s2);
    EXPECT_EQ(s1.size(), 5);
    EXPECT_STREQ(s1.c_str(), "world");
}

TEST(InplaceStringTest, AssignCString)
{
    qx::inplace_string<10> s;
    s.assign("test");
    EXPECT_EQ(s.size(), 4);
    EXPECT_STREQ(s.c_str(), "test");
}

TEST(InplaceStringTest, AssignSizeAndChar)
{
    qx::inplace_string<10> s;
    s.assign(5, 'a');
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "aaaaa");
}

TEST(InplaceStringTest, AssignInitializerList)
{
    qx::inplace_string<10> s;
    s.assign({'a', 'b', 'c'});
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InplaceStringTest, AssignIterators)
{
    std::string source = "range";
    qx::inplace_string<10> s;
    s.assign(source.begin(), source.end());
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "range");
}

TEST(InplaceStringTest, AssignOutOfBoundsThrows)
{
    qx::inplace_string<5> s;
    // Attempting to assign more than capacity (5) should throw std::length_error
    EXPECT_THROW(s.assign("too long string"), std::length_error);
}

TEST(InplaceStringTest, AssignSubstring)
{
    qx::inplace_string<10> const s1("hello");
    qx::inplace_string<10> s2;
    s2.assign(s1, 1, 3); // "ell"
    EXPECT_EQ(s2.size(), 3);
    EXPECT_STREQ(s2.c_str(), "ell");
}