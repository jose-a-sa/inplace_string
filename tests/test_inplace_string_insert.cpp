#include <gtest/gtest.h>
#include <qx/inplace_string.h>

// TEST(InplaceStringInsertTest, InsertCString)
// {
//     qx::inplace_string<10> s("ac");
//     s.insert(1, "b");
//     EXPECT_STREQ(s.c_str(), "abc");
// }

// TEST(InplaceStringInsertTest, InsertExceedsCapacityThrows)
// {
//     qx::inplace_string<5> s("ab");
//     EXPECT_THROW(s.insert(1, "cdef"), std::bad_alloc);
// }

// TEST(InplaceStringInsertTest, InsertOutOfRangeThrows)
// {
//     qx::inplace_string<5> s("ab");
//     EXPECT_THROW(s.insert(5, "c"), std::out_of_range);
// }

// TEST(InplaceStringInsertTest, InsertFill)
// {
//     qx::inplace_string<10> s("ad");
//     s.insert(1, 2, 'b');
//     EXPECT_STREQ(s.c_str(), "abbd");
// }

// TEST(InplaceStringInsertTest, InsertOverlapping)
// {
//     qx::inplace_string<10> s("abc");
//     // Insert a substring of itself into itself
//     s.insert(1, s.data(), 2); // Insert "ab" at pos 1
//     EXPECT_STREQ(s.c_str(), "aabbc");
// }

// TEST(InplaceStringInsertTest, InsertIterator)
// {
//     qx::inplace_string<10> s("ac");
//     std::string to_insert = "b";
//     s.insert(s.begin() + 1, to_insert.begin(), to_insert.end());
//     EXPECT_STREQ(s.c_str(), "abc");
// }