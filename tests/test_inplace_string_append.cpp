#include <qx/inplace_string.h>

#include <gtest/gtest.h>

#include <vector>
#include <list>

// 1. Appending another inplace_string
TEST(InplaceStringAppendTest, AppendInplaceString)
{
    qx::inplace_string<15> s("hello ");
    qx::inplace_string<10> s2("world");
    
    s.append(s2);
    EXPECT_STREQ(s.c_str(), "hello world");
    
    qx::inplace_string<15> s3("!");
    s3 += s2; // operator+= overload
    EXPECT_STREQ(s3.c_str(), "!world");
}

// 2. Appending Substrings (with Out-of-Bounds checks)
TEST(InplaceStringAppendTest, AppendSubstring)
{
    qx::inplace_string<20> s("start-");
    qx::inplace_string<10> src("abcdef");
    
    // Append from pos 2, length 3 ("cde")
    s.append(src, 2, 3);
    EXPECT_STREQ(s.c_str(), "start-cde");

    // Append StringLike (string_view) substring
    std::string_view sv = "xyz";
    s.append(sv, 1, 1); // Append 'y'
    EXPECT_STREQ(s.c_str(), "start-cdey");

    // Out of bounds throw
    EXPECT_THROW(s.append(src, 15, 1), std::out_of_range);
    EXPECT_THROW(s.append(sv, 5, 1), std::out_of_range);
}

// 3. Appending via Iterators (Hits both branches of the if constexpr)
TEST(InplaceStringAppendTest, AppendIterators)
{
    qx::inplace_string<20> s1("vec:");
    std::vector<char> vec{'a', 'b', 'c'};
    
    // Branch 1: Trivial Contiguous Iterator (uses optimized memory copy)
    s1.append(vec.begin(), vec.end());
    EXPECT_STREQ(s1.c_str(), "vec:abc");

    qx::inplace_string<20> s2("list:");
    std::list<char> lst{'x', 'y', 'z'};
    
    // Branch 2: Non-contiguous Iterator (falls back to temporary string copy)
    s2.append(lst.begin(), lst.end());
    EXPECT_STREQ(s2.c_str(), "list:xyz");
}

// 4. Appending Initializer Lists
TEST(InplaceStringAppendTest, AppendInitializerList)
{
    qx::inplace_string<10> s("foo");
    s.append({'b', 'a', 'r'});
    EXPECT_STREQ(s.c_str(), "foobar");

    qx::inplace_string<10> s2("foo");
    s2 += {'b', 'a', 'z'};
    EXPECT_STREQ(s2.c_str(), "foobaz");
}

// 5. Empty Appends (Short-circuiting)
TEST(InplaceStringAppendTest, AppendEmpty)
{
    qx::inplace_string<10> s("test");
    
    s.append(""); // Empty C-string
    EXPECT_EQ(s.size(), 4);
    
    s.append(0, 'x'); // 0 fill characters
    EXPECT_EQ(s.size(), 4);
    
    std::vector<char> empty_vec;
    s.append(empty_vec.begin(), empty_vec.end()); // Empty iterator range
    EXPECT_EQ(s.size(), 4);
    
    EXPECT_STREQ(s.c_str(), "test");
}
