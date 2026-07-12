#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// ===========================================================================
// Element Access & Copying
// ===========================================================================

TEST(InplaceStringAccess, ElementAccess)
{
    qx::inplace_string<5> s("abc");
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s.at(1), 'b');
    EXPECT_EQ(s.front(), 'a');
    EXPECT_EQ(s.back(), 'c');

    EXPECT_THROW(s.at(5), std::out_of_range);
}

TEST(InplaceStringAccess, DataMutation)
{
    qx::inplace_string<10> s("abc");
    char* ptr = s.data();
    ptr[0] = 'X';
    EXPECT_STREQ(s.c_str(), "Xbc");
}

TEST(InplaceStringAccess, SubstringExtraction)
{
    qx::inplace_string<20> const s("hello world");
    auto sub = s.substr(6, 5);
    EXPECT_EQ(sub.capacity(), 20);
    EXPECT_STREQ(sub.c_str(), "world");

    EXPECT_THROW(s.substr(20, 1), std::out_of_range);
}

TEST(InplaceStringAccess, SubstringTemplatedExtraction)
{
    qx::inplace_string<20> const s("hello world");

    auto sub1 = s.substr<6>();
    EXPECT_EQ(sub1.max_size(), 14);
    EXPECT_STREQ(sub1.c_str(), "world");

    auto sub2 = s.substr<6, 5>();
    EXPECT_EQ(sub2.max_size(), 5);
    EXPECT_STREQ(sub2.c_str(), "world");
}

TEST(InplaceStringAccess, CopyToRawBuffer)
{
    qx::inplace_string<10> const s("abcdef");
    char buffer[8] = {0};

    size_t written = s.copy(buffer, 3, 1);
    EXPECT_EQ(written, 3);
    EXPECT_EQ(buffer[0], 'b');

    written = s.copy(buffer, 20, 4);
    EXPECT_EQ(written, 2);

    EXPECT_THROW(s.copy(buffer, 2, 15), std::out_of_range);
}

TEST(InplaceStringAccess, ExceptionsAndContracts)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s("abc");
            char const volatile c = s[5];
            (void)c;
        },
        "contract violation"
    );
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.front();
            (void)c;
        },
        "contract violation"
    );
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.back();
            (void)c;
        },
        "contract violation"
    );
}
