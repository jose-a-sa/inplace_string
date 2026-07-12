#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// ===========================================================================
// Compare (compare, ==, !=, <, >, <=, >=)
// ===========================================================================

TEST(InplaceStringCompare, CompareMember)
{
    qx::inplace_string<10> const s1("abc");
    qx::inplace_string<10> const s2("def");
    EXPECT_LT(s1.compare(s2), 0);
    EXPECT_GT(s2.compare(s1), 0);

    qx::inplace_string<20> const s3("prefix_abc_suffix");
    EXPECT_EQ(s3.compare(7, 3, "abc"), 0);
    EXPECT_LT(s3.compare(7, 2, "abc", 3), 0);
}

TEST(InplaceStringCompare, EqualityOperators)
{
    qx::inplace_string<10> const s1("abc");
    qx::inplace_string<10> const s2("abc");
    qx::inplace_string<10> const s3("def");

    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);
    EXPECT_TRUE(s1 != s3);
    EXPECT_TRUE(s1 == "abc");
}

TEST(InplaceStringCompare, RelationalOperators)
{
    qx::inplace_string<10> const s1("apple");
    qx::inplace_string<10> const s2("banana");

    EXPECT_TRUE(s1 < s2);
    EXPECT_TRUE(s2 > s1);
    EXPECT_TRUE(s1 <= s2);

    qx::inplace_string<5> const cap1("abc");
    qx::inplace_string<10> const cap2("abc");
    EXPECT_TRUE(cap1 == cap2);
}

TEST(InplaceStringCompare, EmptyStringEdgeCases)
{
    qx::inplace_string<10> const s1;
    qx::inplace_string<10> const s2;
    EXPECT_EQ(s1.compare(s2), 0);
    EXPECT_TRUE(s1 == s2);
}

TEST(InplaceStringCompare, ExceptionsAndContracts)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_THROW(s.compare(99, 1, "x", 1), std::out_of_range);
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.compare(null_str);
        },
        "contract violation"
    );
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            bool b = (s == null_str);
            (void)b;
        },
        "contract violation"
    );
}

#if __cplusplus > 202002L

TEST(InplaceStringQueries, StartsWith)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_TRUE(s.starts_with("hello"));
    EXPECT_TRUE(s.starts_with('h'));
    EXPECT_FALSE(s.starts_with("world"));
    EXPECT_FALSE(s.starts_with('e'));
}

TEST(InplaceStringQueries, EndsWith)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_TRUE(s.ends_with("world"));
    EXPECT_TRUE(s.ends_with('d'));
    EXPECT_FALSE(s.ends_with("hello"));
    EXPECT_FALSE(s.ends_with('l'));
}

#endif

#if __cplusplus > 202302L

TEST(InplaceStringQueries, Contains)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_TRUE(s.contains("lo wo"));
    EXPECT_TRUE(s.contains(' '));
    EXPECT_FALSE(s.contains("planet"));
    EXPECT_FALSE(s.contains('z'));
}

#endif
