#include <gmock/gmock.h>

#include <qx/inplace_string.h>

#include <list>
#include <vector>

// ===========================================================================
// Append (append, try_append, unchecked_append, push_back, operator+=)
// ===========================================================================

TEST(InplaceStringAppend, BasicAppend)
{
    qx::inplace_string<15> s("hello ");
    qx::inplace_string<10> s2("world");
    s.append(s2);
    EXPECT_STREQ(s.c_str(), "hello world");

    qx::inplace_string<20> s3("start-");
    qx::inplace_string<10> src("abcdef");
    s3.append(src, 2, 3);
    EXPECT_STREQ(s3.c_str(), "start-cde");

    s3.append("");
    EXPECT_STREQ(s3.c_str(), "start-cde");

    s3.append({'b', 'a', 'r'});
    EXPECT_STREQ(s3.c_str(), "start-cdebar");
}

TEST(InplaceStringAppend, AppendIterators)
{
    qx::inplace_string<20> s1("vec:");
    std::vector<char> vec{'a', 'b', 'c'};
    s1.append(vec.begin(), vec.end());
    EXPECT_STREQ(s1.c_str(), "vec:abc");

    qx::inplace_string<20> s2("list:");
    std::list<char> lst{'x', 'y', 'z'};
    s2.append(lst.begin(), lst.end());
    EXPECT_STREQ(s2.c_str(), "list:xyz");
}

TEST(InplaceStringAppend, UncheckedAppend)
{
    qx::inplace_string<20> s("hello");
    s.unchecked_append(" world", 6);
    EXPECT_STREQ(s.c_str(), "hello world");

    s.unchecked_append("!");
    EXPECT_STREQ(s.c_str(), "hello world!");

    s.unchecked_append(3, 'y');
    EXPECT_STREQ(s.c_str(), "hello world!yyy");
}

TEST(InplaceStringAppend, TryAppend)
{
    qx::inplace_string<10> s("ab");
    EXPECT_EQ(s.try_append("cd"), &s);
    EXPECT_STREQ(s.c_str(), "abcd");

    EXPECT_EQ(s.try_append("efghijk"), nullptr);
    EXPECT_STREQ(s.c_str(), "abcd");

    EXPECT_NE(s.try_append(3, 'x'), nullptr);
    EXPECT_STREQ(s.c_str(), "abcdxxx");

    EXPECT_NE(s.try_append({'y', 'z'}), nullptr);
    EXPECT_STREQ(s.c_str(), "abcdxxxyz");
}

TEST(InplaceStringAppend, OperatorPlusEq)
{
    qx::inplace_string<10> s("abc");
    s += 'd';
    EXPECT_STREQ(s.c_str(), "abcd");
    s += "ef";
    EXPECT_STREQ(s.c_str(), "abcdef");

    std::string bar = "g";
    s += bar;
    EXPECT_STREQ(s.c_str(), "abcdefg");
}

TEST(InplaceStringAppend, PushBack)
{
    qx::inplace_string<10> s;
    s.push_back('a');
    s.push_back('b');
    EXPECT_STREQ(s.c_str(), "ab");

    qx::inplace_string<5> full("abcde");
    EXPECT_THROW(full.push_back('f'), std::length_error);
}

TEST(InplaceStringAppend, SelfOverlapping)
{
    qx::inplace_string<20> s("loop");
    std::string_view sv(s.data(), s.size());
    s.append(sv);
    EXPECT_STREQ(s.c_str(), "looploop");

    qx::inplace_string<20> s3("loop");
    s3.append(s3, 0, 4);
    EXPECT_STREQ(s3.c_str(), "looploop");
}

TEST(InplaceStringAppend, ExceptionsAndContracts)
{
    qx::inplace_string<5> s("abcd");
    EXPECT_THROW(s.append("ef"), std::length_error);
    EXPECT_THROW(s.append(5, 'x'), std::length_error);
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.append(null_str);
        },
        "contract violation"
    );
}

TEST(InplaceStringAppend, SelfReferentialMutations)
{
    // Full self-appending (s.append(s))
    qx::inplace_string<20> s1("abc");
    s1.append(s1);
    EXPECT_STREQ(s1.c_str(), "abcabc");

    // Substring self-appending (s.append(s, pos, count))
    qx::inplace_string<20> s2("abcdef");
    s2.append(s2, 1, 3); // Appends "bcd"
    EXPECT_STREQ(s2.c_str(), "abcdefbcd");

    // Unchecked self-appending
    qx::inplace_string<20> s3("xyz");
    s3.unchecked_append(s3);
    EXPECT_STREQ(s3.c_str(), "xyzxyz");

    // Try self-appending
    qx::inplace_string<20> s4("123");
    EXPECT_NE(s4.try_append(s4), nullptr);
    EXPECT_STREQ(s4.c_str(), "123123");

    // Appending a raw pointer pointing inside its own internal buffer
    qx::inplace_string<20> s5("hello");
    s5.append(s5.data() + 1, 3); // Appends "ell"
    EXPECT_STREQ(s5.c_str(), "helloell");
}
