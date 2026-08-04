#include <gtest/gtest.h>
#include <qx/inplace_string.h>

#include <string>
#include <string_view>

using S = qx::inplace_string<30>;

TEST(InplaceStringConcat, InplaceConcat)
{
    S a = "abc";
    S b = "def";

    // Value combinations
    EXPECT_EQ(a + b, "abcdef");
    EXPECT_EQ(S{"abc"} + b, "abcdef");
    EXPECT_EQ(a + S{"def"}, "abcdef");
    EXPECT_EQ(S{"abc"} + S{"def"}, "abcdef");

    // Self addition
    EXPECT_EQ(a + a, "abcabc");

    // Chaining
    EXPECT_EQ(S{"a"} + S{"b"} + S{"c"}, "abc");
}

TEST(InplaceStringConcat, InplaceEdgeCases)
{
    S a = "abc";
    S empty;

    // Empty strings
    EXPECT_EQ(empty + a, "abc");
    EXPECT_EQ(a + empty, "abc");
    EXPECT_EQ(S{} + S{}, "");

    // Capacity boundaries and mismatches
    qx::inplace_string<6> c = "abc";
    qx::inplace_string<6> d = "def";
    EXPECT_EQ(c + d, "abcdef");

    qx::inplace_string<16> e = "hello ";
    qx::inplace_string<32> f = "world";
    EXPECT_EQ(e + f, "hello world");

    // Embedded nulls
    S null_str1("a\0b", 3);
    S null_str2("c\0d", 3);
    EXPECT_EQ(null_str1 + null_str2, S("a\0bc\0d", 6));
}

TEST(InplaceStringConcat, WithStdString)
{
    S inplace_val = "abc";
    std::string std_val = "def";

    EXPECT_EQ(inplace_val + std_val, "abcdef");
    EXPECT_EQ(std_val + inplace_val, "defabc");

    // Move semantics
    EXPECT_EQ(std::string{"abc"} + inplace_val, "abcabc");
    EXPECT_EQ(inplace_val + std::string{"def"}, "abcdef");
}

TEST(InplaceStringConcat, WithStringView)
{
    S inplace_val = "abc";
    std::string_view view_val = "def";

    EXPECT_EQ(inplace_val + view_val, "abcdef");
    EXPECT_EQ(view_val + inplace_val, "defabc");
}

TEST(InplaceStringConcat, WithCString)
{
    S a = "abc";

    // Standard literals
    EXPECT_EQ(a + "def", "abcdef");
    EXPECT_EQ("def" + a, "defabc");

    // Empty literals
    EXPECT_EQ(a + "", "abc");
    EXPECT_EQ("" + a, "abc");
}

TEST(InplaceStringConcat, WithChar)
{
    S a = "abc";

    EXPECT_EQ(a + 'd', "abcd");
    EXPECT_EQ('X' + a, "Xabc");

    // Embedded nulls with chars
    S with_null("a\0b", 3);
    EXPECT_EQ(with_null + 'x', S("a\0bx", 4));
}

TEST(MixedTypes, StdStringChaining)
{
    S a = "a";
    std::string b = "b";
    std::string_view c = "c";

    EXPECT_EQ(a + c + b + "d" + 'e', "acbde");
    EXPECT_EQ(a + "b" + 'c' + S{"d"}, "abcd");
}
