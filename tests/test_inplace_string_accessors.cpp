#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// Element Access & Copying

TEST(InplaceStringAccess, ElementAccess)
{
    qx::inplace_string<5> s("abc");
    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s.at(1), 'b');
    EXPECT_EQ(s.front(), 'a');
    EXPECT_EQ(s.back(), 'c');

    EXPECT_THROW(s.at(5), std::out_of_range);
}

TEST(InplaceStringAccess, OperatorBracketAtNullTerminator)
{
    qx::inplace_string<5> const s("abc");
    EXPECT_EQ(s[3], '\0');

    qx::inplace_string<5> mutable_s("abc");
    EXPECT_EQ(mutable_s[3], '\0');
}

TEST(InplaceStringAccess, AtDoesNotExemptNullTerminator)
{
    qx::inplace_string<5> const s("abc");
    EXPECT_THROW(s.at(3), std::out_of_range);
    EXPECT_EQ(s.at(2), 'c'); // one below the boundary: succeeds

    qx::inplace_string<5> mutable_s("abc");
    EXPECT_THROW(mutable_s.at(3), std::out_of_range);
    mutable_s.at(0) = 'X';
    EXPECT_STREQ(mutable_s.c_str(), "Xbc");
}

TEST(InplaceStringAccess, FrontBackMutable)
{
    qx::inplace_string<5> s("abc");
    s.front() = 'X';
    s.back() = 'Z';
    EXPECT_STREQ(s.c_str(), "XbZ");
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

TEST(InplaceStringAccess, SubstringDefaultsAndSaturation)
{
    qx::inplace_string<20> const s("hello world");

    EXPECT_STREQ(s.substr().c_str(), "hello world");

    EXPECT_TRUE(s.substr(s.size()).empty());

    EXPECT_STREQ(s.substr(0, 5).c_str(), "hello");
    EXPECT_STREQ(s.substr(6, 100).c_str(), "world");
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

TEST(InplaceStringAccess, SubstringTemplatedPastEnd)
{
    qx::inplace_string<20> const s("hi");
    auto const sub = s.substr<10>();
    EXPECT_TRUE(sub.empty());

    auto const sub_exact = s.substr<2>(); // Pos == sz exactly
    EXPECT_TRUE(sub_exact.empty());
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

TEST(InplaceStringAccess, CopyBoundaryCases)
{
    qx::inplace_string<10> const s("abcdef");
    char buffer[8] = {0};

    EXPECT_EQ(s.copy(buffer, 5, s.size()), 0U);

    EXPECT_EQ(s.copy(buffer, 2, 0), 2U);
    EXPECT_EQ(buffer[0], 'a');
    EXPECT_EQ(buffer[1], 'b');
}

TEST(InplaceStringAccess, ExceptionsAndContracts)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s("abc");
            char const volatile c = s[5];
            (void)c;
        },
        "contract violation");
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s("abc");
            s[5] = 'x';
        },
        "contract violation");
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.front();
            (void)c;
        },
        "contract violation");
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s;
            s.front() = 'x';
        },
        "contract violation");
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.back();
            (void)c;
        },
        "contract violation");
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s;
            s.back() = 'x';
        },
        "contract violation");
}
