#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// ===========================================================================
// Erase (erase, clear, pop_back)
// ===========================================================================

TEST(InplaceStringErase, EraseRanges)
{
    qx::inplace_string<10> s("abcdef");
    s.erase(2, 2);
    EXPECT_STREQ(s.c_str(), "abef");

    qx::inplace_string<10> s2("abcdef");
    s2.erase(3);
    EXPECT_STREQ(s2.c_str(), "abc");

    qx::inplace_string<10> s3("abcdef");
    s3.erase(2, 0);
    EXPECT_STREQ(s3.c_str(), "abcdef");
}

TEST(InplaceStringErase, ClearAndPopBack)
{
    qx::inplace_string<10> s("abc");
    s.pop_back();
    EXPECT_EQ(s.size(), 2);
    EXPECT_STREQ(s.c_str(), "ab");

    s.clear();
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

TEST(InplaceStringErase, ExceptionsAndContracts)
{
    qx::inplace_string<10> s("abc");
    EXPECT_THROW(s.erase(5, 1), std::out_of_range);

    qx::inplace_string<10> s_empty;
    EXPECT_DEATH(s_empty.pop_back(), "contract violation");
    EXPECT_DEATH(s.erase(s.end()), "contract violation");
    EXPECT_DEATH(s.erase(s.begin() + 2, s.begin() + 1), "contract violation");
}