#include <gmock/gmock.h>

#include <qx/inplace_string.h>

#include <array>
#include <deque>
#include <list>
#include <type_traits>
#include <vector>

static_assert(qx::intl::is_trivial_contiguous_iterator_v<std::vector<char>::iterator>);
static_assert(!qx::intl::is_trivial_contiguous_iterator_v<std::deque<char>::iterator>);
static_assert(!qx::intl::is_trivial_contiguous_iterator_v<std::list<char>::iterator>);
static_assert(qx::intl::is_trivial_contiguous_iterator_v<std::array<char, 10>::iterator>);
static_assert(qx::intl::is_trivial_contiguous_iterator_v<std::string::iterator>);
static_assert(qx::intl::is_trivial_contiguous_iterator_v<std::string_view::iterator>);

static_assert(std::is_trivially_copyable_v<qx::inplace_string<30>>);
static_assert(std::is_trivially_copyable_v<qx::inplace_wstring<30>>);
static_assert(std::is_trivially_copyable_v<qx::inplace_string<256>>);
static_assert(std::is_trivially_copyable_v<qx::inplace_wstring<256>>);

// ===========================================================================
// Constructors & Initialization
// ===========================================================================

TEST(InplaceStringInit, DefaultConstructor)
{
    qx::inplace_string<10> const s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.capacity(), 10);
    EXPECT_EQ(s.max_size(), 10);
    EXPECT_STREQ(s.c_str(), "");
}

TEST(InplaceStringInit, CStringConstructor)
{
    qx::inplace_string<10> const s("hello");
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "hello");

    qx::inplace_string<20> const s2("hello world", 7);
    EXPECT_EQ(s2.size(), 7);
    EXPECT_STREQ(s2.c_str(), "hello w");

    auto const* str = "world";
    qx::inplace_string<20> const s3(str);
    EXPECT_EQ(s3.size(), 5);
    EXPECT_STREQ(s3.c_str(), "world");
}

TEST(InplaceStringInit, StringViewConstructor)
{
    constexpr std::string_view sv = "world";
    qx::inplace_string<10> const s(sv);
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "world");

    std::string const str = "my test str";
    qx::inplace_string<15> const s2(str);
    EXPECT_EQ(s2.size(), 11);
    EXPECT_STREQ(s2.c_str(), "my test str");

    std::string const str2 = "my_substring";
    qx::inplace_string<15> const s3(str2, 6, 6);
    EXPECT_EQ(s3.size(), 6);
    EXPECT_STREQ(s3.c_str(), "string");
}

TEST(InplaceStringInit, FillConstructor)
{
    qx::inplace_string<5> const s(4, 'x');
    EXPECT_EQ(s.size(), 4);
    EXPECT_STREQ(s.c_str(), "xxxx");
    EXPECT_THROW((qx::inplace_string<5>(6, 'x')), std::length_error);
}

TEST(InplaceStringInit, CopyConstructorAndSubstring)
{
    qx::inplace_string<10> const orig("abcdef");
    qx::inplace_string<10> const copy(orig, 2, 3);
    EXPECT_EQ(copy.size(), 3);
    EXPECT_STREQ(copy.c_str(), "cde");
    EXPECT_THROW((qx::inplace_string<10>(orig, 20, 1)), std::out_of_range);
}

TEST(InplaceStringInit, RangeConstructor)
{
    std::vector<char> v = {'a', 'b', 'c', 'd'};
    qx::inplace_string<10> const s(v.begin(), v.end());
    EXPECT_EQ(s.size(), 4);
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(InplaceStringInit, InitializerListConstructor)
{
    qx::inplace_string<10> const s = {'h', 'e', 'l', 'l', 'o'};
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "hello");
}

TEST(InplaceStringInit, MoveConstructor)
{
    qx::inplace_string<10> s1("move_me");
    qx::inplace_string<10> const s2(std::move(s1));
    EXPECT_STREQ(s2.c_str(), "move_me");
    EXPECT_EQ(s2.size(), 7);
}

TEST(InplaceStringInit, PureInputIterator)
{
    std::istringstream stream("inputiter");
    std::istream_iterator<char> start(stream);
    std::istream_iterator<char> end;

    qx::inplace_string<20> s(start, end);
    EXPECT_STREQ(s.c_str(), "inputiter");
    EXPECT_EQ(s.size(), 9);
}

TEST(InplaceStringInit, DeductionGuideFromStringLiteral)
{
    qx::basic_inplace_string s("hello");
    static_assert(s.capacity() == 5);
    EXPECT_STREQ(s.c_str(), "hello");

    qx::basic_inplace_string ws(L"hi");
    static_assert(ws.capacity() == 2);
    EXPECT_TRUE(ws == L"hi");
}

TEST(InplaceStringInit, NullPointerConstructorDeath)
{
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str);
        },
        "contract violation"
    );
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str, 5);
        },
        "contract violation"
    );
}