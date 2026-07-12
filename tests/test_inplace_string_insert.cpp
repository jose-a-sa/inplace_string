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

TEST(InplaceStringInsert, IteratorCharacterInsert)
{
    qx::inplace_string<10> s("ac");
    
    // insert(const_iterator, c)
    auto it1 = s.insert(s.begin() + 1, 'b');
    EXPECT_EQ(*it1, 'b');
    EXPECT_STREQ(s.c_str(), "abc");

    // insert(const_iterator, n, c)
    auto it2 = s.insert(s.begin() + 3, 2, 'X');
    EXPECT_EQ(*it2, 'X');
    EXPECT_STREQ(s.c_str(), "abcXX");
}

TEST(InplaceStringInsert, UncheckedIteratorInsert)
{
    qx::inplace_string<10> s("14");
    
    // unchecked_insert(const_iterator, c)
    auto it1 = s.unchecked_insert(s.begin() + 1, '2');
    EXPECT_EQ(*it1, '2');
    EXPECT_STREQ(s.c_str(), "124");

    // unchecked_insert(const_iterator, n, c)
    auto it2 = s.unchecked_insert(s.begin() + 2, 1, '3');
    EXPECT_EQ(*it2, '3');
    EXPECT_STREQ(s.c_str(), "1234");
    
    // unchecked_insert(const_iterator, initializer_list)
    auto it3 = s.unchecked_insert(s.end(), {'5', '6'});
    EXPECT_EQ(*it3, '5');
    EXPECT_STREQ(s.c_str(), "123456");
}

TEST(InplaceStringInsert, TryInsertOptionalIterators)
{
    qx::inplace_string<10> s("abc");
    
    // try_insert(const_iterator, c) -> returns std::optional<iterator>
    auto opt_it = s.try_insert(s.begin() + 1, 'X');
    ASSERT_TRUE(opt_it.has_value());
    EXPECT_EQ(**opt_it, 'X');
    EXPECT_STREQ(s.c_str(), "aXbc");

    // try_insert(const_iterator, n, c) -> returns std::optional<iterator>
    auto opt_it2 = s.try_insert(s.begin() + 4, 2, 'Y');
    ASSERT_TRUE(opt_it2.has_value());
    EXPECT_STREQ(s.c_str(), "aXbcYY");
    
    // try_insert(const_iterator, initializer_list)
    auto opt_it3 = s.try_insert(s.end(), {'Z'});
    ASSERT_TRUE(opt_it3.has_value());
    EXPECT_STREQ(s.c_str(), "aXbcYYZ");

    // Capacity failure returns nullopt
    qx::inplace_string<3> full("123");
    EXPECT_FALSE(full.try_insert(full.begin(), 'X').has_value());
    EXPECT_FALSE(full.try_insert(full.begin(), 1, 'X').has_value());
    EXPECT_FALSE(full.try_insert(full.begin(), {'X'}).has_value());
}
