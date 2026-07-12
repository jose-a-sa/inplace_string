#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// ===========================================================================
// Insert (insert, try_insert)
// ===========================================================================

TEST(InplaceStringInsert, BasicInsert)
{
    qx::inplace_string<10> s("ac");
    s.insert(1, "b");
    EXPECT_STREQ(s.c_str(), "abc");

    qx::inplace_string<10> s2("ad");
    s2.insert(1, 2, 'b');
    EXPECT_STREQ(s2.c_str(), "abbd");

    qx::inplace_string<10> s3("def");
    s3.insert(0, "abc");
    EXPECT_STREQ(s3.c_str(), "abcdef");

    qx::inplace_string<10> s4("abc");
    s4.insert(3, "def");
    EXPECT_STREQ(s4.c_str(), "abcdef");
}

TEST(InplaceStringInsert, InsertSubstring)
{
    qx::inplace_string<10> s("ad");
    qx::inplace_string<10> const src("xbc");
    s.insert(1, src, 1, 2);
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(InplaceStringInsert, InsertIterators)
{
    qx::inplace_string<10> s("ac");
    std::string to_insert = "b";
    s.insert(s.begin() + 1, to_insert.begin(), to_insert.end());
    EXPECT_STREQ(s.c_str(), "abc");

    s.insert(s.begin() + 1, {'X', 'Y'});
    EXPECT_STREQ(s.c_str(), "aXYbc");
}

TEST(InplaceStringInsert, TryInsert)
{
    qx::inplace_string<10> s("ac");
    EXPECT_NE(s.try_insert(1, "b"), nullptr);
    EXPECT_STREQ(s.c_str(), "abc");

    EXPECT_EQ(s.try_insert(99, "x"), nullptr);

    qx::inplace_string<5> s_full("abcd");
    EXPECT_EQ(s_full.try_insert(2, "XY"), nullptr);
}

TEST(InplaceStringInsert, Overlapping)
{
    qx::inplace_string<10> s("abc");
    s.insert(1, s.data(), 2);
    EXPECT_STREQ(s.c_str(), "aabbc");

    qx::inplace_string<20> s2("abcdef");
    s2.insert(2, s2.data() + 1, 3);
    EXPECT_STREQ(s2.c_str(), "abbcdcdef");
}

TEST(InplaceStringInsert, ExceptionsAndContracts)
{
    qx::inplace_string<5> s("ab");
    EXPECT_THROW(s.insert(1, "cdef"), std::length_error);
    EXPECT_THROW(s.insert(5, "c"), std::out_of_range);
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.insert(1, null_str);
        },
        "contract violation"
    );
}

TEST(InplaceStringInsert, SelfReferentialMutations)
{
    qx::inplace_string<15> s1("abc");
    s1.insert(1, s1); // Inserts "abc" at index 1 -> "aabcbc"
    EXPECT_STREQ(s1.c_str(), "aabcbc");

    qx::inplace_string<15> s2("abcdef");
    s2.insert(2, s2, 1, 3); // Inserts "bcd" at index 2 -> "abbcdcdef"
    EXPECT_STREQ(s2.c_str(), "abbcdcdef");

    qx::inplace_string<15> s3("xyz");
    EXPECT_NE(s3.try_insert(0, s3), nullptr);
    EXPECT_STREQ(s3.c_str(), "xyzxyz");

    qx::inplace_string<15> s4("123");
    s4.unchecked_insert(3, s4);
    EXPECT_STREQ(s4.c_str(), "123123");
}
