#include <gmock/gmock.h>

#include <qx/inplace_string.h>
#include <string_view>

// ===========================================================================
// Search (find, rfind, find_first_of, etc.)
// ===========================================================================

TEST(InplaceStringFind, FindSubstring)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find("world"), 6);
    EXPECT_EQ(s.find("planet"), qx::inplace_string<20>::npos);
    EXPECT_EQ(s.find("world", 7, 5), qx::inplace_string<20>::npos);

    qx::inplace_string<10> const s2("abc");
    EXPECT_EQ(s2.find("", 2, 0), 2);
    EXPECT_EQ(s2.find('a', 99), qx::inplace_string<10>::npos);
}

TEST(InplaceStringFind, RFind)
{
    qx::inplace_string<20> const s("abcefabc");
    EXPECT_EQ(s.rfind("bc"), 6);
    EXPECT_EQ(s.rfind("bc", 3, 2), 1);
    EXPECT_EQ(s.rfind('b', 3), 1);
    EXPECT_EQ(std::string_view{s}.rfind('b', 3), 1);
    EXPECT_EQ(s.rfind('z'), qx::inplace_string<20>::npos);
}

TEST(InplaceStringFind, FindFirstOf)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find_first_of("aeiou"), 1);
    EXPECT_EQ(s.find_first_of('o'), 4);
    EXPECT_EQ(s.find_first_of('z'), qx::inplace_string<20>::npos);

    qx::inplace_string<10> const s2("abc");
    EXPECT_EQ(s2.find_first_of("abc", 99, 3), qx::inplace_string<10>::npos);
}

TEST(InplaceStringFind, FindLastOf)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find_last_of('o'), 7);
    EXPECT_EQ(s.find_last_of("aeiou"), 7);
    EXPECT_EQ(s.find_last_of('z'), qx::inplace_string<20>::npos);
}

TEST(InplaceStringFind, FindFirstNotOf)
{
    qx::inplace_string<20> const s("aaabcd");
    EXPECT_EQ(s.find_first_not_of('a'), 3);

    qx::inplace_string<20> const s2("hello world");
    EXPECT_EQ(s2.find_first_not_of("helo"), 5); // ' '

    qx::inplace_string<10> const same("aaaa");
    EXPECT_EQ(same.find_first_not_of('a'), qx::inplace_string<10>::npos);
}

TEST(InplaceStringFind, FindLastNotOf)
{
    qx::inplace_string<20> const s("aabbcc");
    EXPECT_EQ(s.find_last_not_of('c'), 3); // last 'b'

    qx::inplace_string<20> const s2("hello world!!!");
    EXPECT_EQ(s2.find_last_not_of("!"), 10); // 'd'

    qx::inplace_string<10> const same("cccc");
    EXPECT_EQ(same.find_last_not_of('c'), qx::inplace_string<10>::npos);
}

TEST(InplaceStringFind, EmptyStringEdgeCases)
{
    qx::inplace_string<10> const s;
    EXPECT_EQ(s.find("anything"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.rfind("anything"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_first_of("abc"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_last_of("abc"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_first_not_of("abc"), qx::inplace_string<10>::npos);
}

TEST(InplaceStringFind, ExceptionsAndContracts)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.find(null_str);
        },
        "contract violation"
    );
}
