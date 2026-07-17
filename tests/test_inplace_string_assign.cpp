#include <gmock/gmock.h>

#include <qx/inplace_string.h>

#include <list>
#include <string_view>

// Assignment (assign, try_assign, unchecked_assign, operator=)

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

TEST(InplaceStringAssign, AssignSubstringNSaturation)
{
    qx::inplace_string<10> s;
    qx::inplace_string<10> const src("abcdef");
    s.assign(src, 2, qx::inplace_string<10>::npos);
    EXPECT_STREQ(s.c_str(), "cdef");

    std::string const str_src("abcdef");
    s.assign(str_src, 2, 100);
    EXPECT_STREQ(s.c_str(), "cdef");

    s.assign(src, src.size(), 5);
    EXPECT_TRUE(s.empty());
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

    EXPECT_STREQ(s.try_assign({'a', 'b'})->c_str(), "ab");
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

TEST(InplaceStringAssign, OperatorAssignStringLike)
{
    qx::inplace_string<10> s;
    std::string const src("stringlike");
    s = src;
    EXPECT_STREQ(s.c_str(), "stringlike");
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
    EXPECT_THROW(s.assign(6, 'x'), std::length_error); // fill-assign capacity check
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.assign(null_str, 5);
        },
        "contract violation");
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.assign(null_str);
        },
        "contract violation");
}

TEST(InplaceStringAssign, UncheckedAssignExceptionsAndContracts)
{
    qx::inplace_string<5> s;
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.unchecked_assign(null_str, 5);
        },
        "contract violation");
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.unchecked_assign(null_str);
        },
        "contract violation");
}

TEST(InplaceStringAssign, TryAssignExceptionsAndContracts)
{
    qx::inplace_string<5> s;
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            (void)s.try_assign(null_str, 5);
        },
        "contract violation");
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            (void)s.try_assign(null_str);
        },
        "contract violation");
}

TEST(InplaceStringAssign, SelfReferentialMutations)
{
    qx::inplace_string<15> s1("abc");
    EXPECT_NE(s1.try_assign(s1), nullptr);
    EXPECT_STREQ(s1.c_str(), "abc");

    s1.unchecked_assign(s1);
    EXPECT_STREQ(s1.c_str(), "abc");

    qx::inplace_string<15> s2("abcdef");
    s2.assign(s2.data() + 2, 3); // Assigns "cde"
    EXPECT_STREQ(s2.c_str(), "cde");

    qx::inplace_string<15> s3("abcdef");
    s3.unchecked_assign(s3.data() + 1, 4); // Assigns "bcde"
    EXPECT_STREQ(s3.c_str(), "bcde");
}

TEST(InplaceStringAssign, AssignIteratorPairOverlappingBuffer)
{
    qx::inplace_string<15> s("abcdef");
    s.assign(s.data() + 1, s.data() + 5); // "bcde", overlapping source range
    EXPECT_STREQ(s.c_str(), "bcde");
}

TEST(InplaceStringAssign, AssignIteratorPairNonContiguousForward)
{
    qx::inplace_string<15> s("old");
    std::list<char> lst{'l', 'i', 's', 't'};
    s.assign(lst.begin(), lst.end());
    EXPECT_STREQ(s.c_str(), "list");
}

TEST(InplaceStringAssign, AssignInitializerList)
{
    qx::inplace_string<10> s("old");

    s.assign({'a', 'b', 'c'});
    EXPECT_STREQ(s.c_str(), "abc");

    EXPECT_NE(s.try_assign({'x', 'y'}), nullptr);
    EXPECT_STREQ(s.c_str(), "xy");

    qx::inplace_string<2> small;
    EXPECT_EQ(small.try_assign({'1', '2', '3'}), nullptr); // Overflow check

    s.unchecked_assign({'z'});
    EXPECT_STREQ(s.c_str(), "z");
}
