#include <gtest/gtest.h>
#include <qx/inplace_string.h>

TEST(InplaceStringEraseTest, EraseMiddle)
{
    qx::inplace_string<10> s("abcdef");
    s.erase(2, 2); // erase "cd"
    EXPECT_STREQ(s.c_str(), "abef");
}

TEST(InplaceStringEraseTest, EraseEndNpos)
{
    qx::inplace_string<10> s("abcdef");
    s.erase(3); // erase from pos 3 to end
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InplaceStringEraseTest, EraseOutOfRangeThrows)
{
    qx::inplace_string<10> s("abc");
    EXPECT_THROW(s.erase(5, 1), std::length_error);
}

TEST(InplaceStringEraseTest, Clear)
{
    qx::inplace_string<10> s("hello");
    s.clear();
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

TEST(InplaceStringReplaceTest, ReplaceCString)
{
    qx::inplace_string<15> s("the quick fox");
    s.replace(4, 5, "lazy");
    EXPECT_STREQ(s.c_str(), "the lazy fox");
}

TEST(InplaceStringReplaceTest, ReplaceExceedsCapacity)
{
    qx::inplace_string<10> s("hello");
    EXPECT_THROW(s.replace(0, 1, "reallylongstring"), std::length_error);
}

TEST(InplaceStringReplaceTest, ReplaceFill)
{
    qx::inplace_string<10> s("abc");
    s.replace(1, 1, 3, 'x'); // replace 'b' with 'xxx'
    EXPECT_STREQ(s.c_str(), "axxxc");
}