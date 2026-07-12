#include <gmock/gmock.h>

#include <qx/inplace_string.h>

#include <string_view>

// ===========================================================================
// Assignment (assign, try_assign, unchecked_assign, operator=)
// ===========================================================================

TEST(InplaceStringAssign, BasicAssign)
{
    qx::inplace_string<10> s1("hello");
    qx::inplace_string<10> const s2("world");
    s1.assign(s2);
    EXPECT_STREQ(s1.c_str(), "world");

    s1.assign("test");
    EXPECT_STREQ(s1.c_str(), "test");

    s1.assign(5, 'a');
    EXPECT_STREQ(s1.c_str(), "aaaaa");

    s1.assign({'a', 'b', 'c'});
    EXPECT_STREQ(s1.c_str(), "abc");

    std::string source = "range";
    s1.assign(source.begin(), source.end());
    EXPECT_STREQ(s1.c_str(), "range");
}

TEST(InplaceStringAssign, AssignSubstring)
{
    qx::inplace_string<10> s("old");
    std::string src("abcdef");
    s.assign(src, 2, 3);
    EXPECT_STREQ(s.c_str(), "cde");

    EXPECT_THROW(s.assign(src, 99, 1), std::out_of_range);
    qx::inplace_string<10> const s_src("abc");
    EXPECT_THROW(s.assign(s_src, 99, 1), std::out_of_range);
}

TEST(InplaceStringAssign, UncheckedAssign)
{
    qx::inplace_string<10> s("old");
    s.unchecked_assign("new");
    EXPECT_STREQ(s.c_str(), "new");

    std::string_view sv("replaced");
    s.unchecked_assign(sv);
    EXPECT_STREQ(s.c_str(), "replaced");

    s.unchecked_assign("hello", 3);
    EXPECT_STREQ(s.c_str(), "hel");

    s.unchecked_assign(4, 'z');
    EXPECT_STREQ(s.c_str(), "zzzz");

    s.unchecked_assign({'n', 'e', 'w'});
    EXPECT_STREQ(s.c_str(), "new");
}

TEST(InplaceStringAssign, TryAssign)
{
    qx::inplace_string<10> s("old");
    EXPECT_EQ(s.try_assign("new"), &s);
    EXPECT_STREQ(s.c_str(), "new");

    EXPECT_EQ(s.try_assign("toolongstring"), nullptr);
    EXPECT_STREQ(s.c_str(), "new"); // unchanged

    EXPECT_NE(s.try_assign(4, 'z'), nullptr);
    EXPECT_EQ(s.try_assign(11, 'z'), nullptr);

    EXPECT_STREQ(s.try_append(std::string("new"))->c_str(), "zzzznew");
}

TEST(InplaceStringAssign, OperatorAssign)
{
    qx::inplace_string<10> s;
    s = "test";
    EXPECT_STREQ(s.c_str(), "test");
    s = 'X';
    EXPECT_STREQ(s.c_str(), "X");
    s = {'a', 'b', 'c'};
    EXPECT_STREQ(s.c_str(), "abc");

    EXPECT_THROW((qx::inplace_string<0>{} = 'x'), std::length_error);
}

TEST(InplaceStringAssign, SelfAssignment)
{
    qx::inplace_string<15> s("self");
    s = s;
    EXPECT_STREQ(s.c_str(), "self");
    s = std::move(s);
    EXPECT_STREQ(s.c_str(), "self");
    s.assign(s);
    EXPECT_STREQ(s.c_str(), "self");

    qx::inplace_string<15> s1("abcdef");
    s1.assign(s1, 2, 3);
    EXPECT_STREQ(s1.c_str(), "cde");
}

TEST(InplaceStringAssign, ExceptionsAndContracts)
{
    qx::inplace_string<5> s;
    EXPECT_THROW(s.assign("abcdef"), std::length_error);
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.assign(null_str, 5);
        },
        "contract violation"
    );
}

TEST(InplaceStringAssign, SelfReferentialMutations)
{
    // Try assign with itself
    qx::inplace_string<15> s1("abc");
    EXPECT_NE(s1.try_assign(s1), nullptr);
    EXPECT_STREQ(s1.c_str(), "abc");

    // Unchecked assign with itself
    s1.unchecked_assign(s1);
    EXPECT_STREQ(s1.c_str(), "abc");

    // Assigning a raw pointer pointing into its own buffer (shrinking)
    qx::inplace_string<15> s2("abcdef");
    s2.assign(s2.data() + 2, 3); // Assigns "cde"
    EXPECT_STREQ(s2.c_str(), "cde");

    // Unchecked assignment from an internal pointer
    qx::inplace_string<15> s3("abcdef");
    s3.unchecked_assign(s3.data() + 1, 4); // Assigns "bcde"
    EXPECT_STREQ(s3.c_str(), "bcde");
}
