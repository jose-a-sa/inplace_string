#include <qx/inplace_string.h>

#include <gtest/gtest.h>

TEST(InplaceStringInsertTest, InsertCString)
{
    qx::inplace_string<10> s("ac");
    s.insert(1, "b");
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InplaceStringInsertTest, InsertExceedsCapacityThrows)
{
    qx::inplace_string<5> s("ab");
    EXPECT_THROW(s.insert(1, "cdef"), std::length_error);
}

TEST(InplaceStringInsertTest, InsertOutOfRangeThrows)
{
    qx::inplace_string<5> s("ab");
    EXPECT_THROW(s.insert(5, "c"), std::out_of_range);
}

TEST(InplaceStringInsertTest, InsertFill)
{
    qx::inplace_string<10> s("ad");
    s.insert(1, 2, 'b');
    EXPECT_STREQ(s.c_str(), "abbd");
}

TEST(InplaceStringInsertTest, InsertOverlapping)
{
    qx::inplace_string<10> s("abc");
    // Insert a substring of itself into itself
    s.insert(1, s.data(), 2); // Insert "ab" at pos 1
    EXPECT_STREQ(s.c_str(), "aabbc");
}

TEST(InplaceStringInsertTest, InsertIterator)
{
    qx::inplace_string<10> s("ac");
    std::string to_insert = "b";
    s.insert(s.begin() + 1, to_insert.begin(), to_insert.end());
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InplaceStringModifiersTest, PushBackAndPopBack)
{
    qx::inplace_string<10> s;
    
    // Test push_back on empty and populated string
    s.push_back('a');
    s.push_back('b');
    s.push_back('c');
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "abc");

    // Test pop_back happy path
    s.pop_back();
    EXPECT_EQ(s.size(), 2);
    EXPECT_STREQ(s.c_str(), "ab");

    s.pop_back();
    s.pop_back();
    EXPECT_TRUE(s.empty());
    EXPECT_STREQ(s.c_str(), "");
}

TEST(InplaceStringIteratorTest, PureInputIterator)
{
    std::istringstream stream("inputiter");
    std::istream_iterator<char> start(stream);
    std::istream_iterator<char> end;

    // Test constructor with single-pass input iterator
    qx::inplace_string<20> s(start, end);
    EXPECT_STREQ(s.c_str(), "inputiter");
    EXPECT_EQ(s.size(), 9);

    // Test append with single-pass input iterator
    std::istringstream stream2("more");
    std::istream_iterator<char> start2(stream2);
    s.append(start2, end);
    EXPECT_STREQ(s.c_str(), "inputitermore");

    // Test capacity enforcement (must throw std::length_error if it overflows)
    std::istringstream long_stream("this_is_a_very_long_stream_that_exceeds_capacity");
    std::istream_iterator<char> start3(long_stream);
    qx::inplace_string<10> short_s;
    EXPECT_THROW(short_s.assign(start3, end), std::length_error);
}

TEST(InplaceStringSelfTest, SelfAssignmentAndCopy)
{
    qx::inplace_string<15> s("self");

    // 1. Copy self-assignment
    s = s; 
    EXPECT_STREQ(s.c_str(), "self");

    // 2. Move self-assignment
    // (Using a volatile pointer or manual std::move invocation to bypass compiler warnings)
    s = std::move(s);
    EXPECT_STREQ(s.c_str(), "self");

    // 3. Whole string self-assign method
    s.assign(s);
    EXPECT_STREQ(s.c_str(), "self");
}

TEST(InplaceStringSelfTest, SelfSubstrOverlapping)
{
    // 1. Self Substring Assignment
    qx::inplace_string<15> s1("abcdef");
    s1.assign(s1, 2, 3); // Assign "cde" to itself
    EXPECT_STREQ(s1.c_str(), "cde");
    EXPECT_EQ(s1.size(), 3);

    // 2. Complex Overlapping Insertion (Source overlaps shifted region)
    qx::inplace_string<20> s2("abcdef");
    // Insert "bcd" (from s2.data() + 1) right into index 2
    // 'ab' + 'bcd' + 'cdef' = 'abbcdcdef'
    s2.insert(2, s2.data() + 1, 3);
    EXPECT_STREQ(s2.c_str(), "abbcdcdef");

    // 3. Appending a substring of itself
    qx::inplace_string<20> s3("loop");
    s3.append(s3, 0, 4); // Append "loop" to "loop"
    EXPECT_STREQ(s3.c_str(), "looploop");
}
