
#include <qx/inplace_string.h>

#include <gtest/gtest.h>

#include "string_api_tester.h"

TEST(InplaceStringSearchTest, FindSubstring)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find("world"), 6);
    EXPECT_EQ(s.find("planet"), qx::inplace_string<20>::npos);
}

TEST(InplaceStringSearchTest, RFindChar)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.rfind('o'), 7);
    EXPECT_EQ(s.rfind('z'), qx::inplace_string<20>::npos);
}

TEST(InplaceStringSearchTest, FindFirstOf)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find_first_of("aeiou"), 1); // 'e'
}

TEST(InplaceStringSearchTest, FindLastNotOf)
{
    qx::inplace_string<20> const s("hello world!!!");
    EXPECT_EQ(s.find_last_not_of("!"), 10); // 'd'
}

TEST(InplaceStringCompareTest, EqualityOperators)
{
    qx::inplace_string<10> const s1("abc");
    qx::inplace_string<10> const s2("abc");
    qx::inplace_string<10> const s3("def");

    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);
    EXPECT_TRUE(s1 != s3);

    // Test against raw string
    EXPECT_TRUE(s1 == "abc");
}

TEST(InplaceStringCompareTest, RelationalOperators)
{
    qx::inplace_string<10> const s1("apple");
    qx::inplace_string<10> const s2("banana");

    EXPECT_TRUE(s1 < s2);
    EXPECT_TRUE(s2 > s1);
    EXPECT_TRUE(s1 <= s2);
}

TEST(InplaceStringSearchGapsTest, FindFirstNotOfAndFindLastOf)
{
    qx::inplace_string<20> const s("hello world");
    auto const tester = qx::string_api_tester(s);

    // find_first_not_of matches first char not present in target set
    tester.result_eq([](auto& s) { return s.find_first_not_of("helo"); }, 5)
        .result_eq([](auto& s) { return s.find_first_not_of("hello world"); }, qx::inplace_string<20>::npos)
        .result_eq([](auto& s) { return s.find_last_of("aeiou"); }, 7)
        .result_eq([](auto& s) { return s.find_last_of("xyz"); }, qx::inplace_string<20>::npos);
}
