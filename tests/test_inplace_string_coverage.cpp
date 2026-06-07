/**
 * test_inplace_string_coverage.cpp
 *
 * Targets coverage gaps not exercised by the existing test files:
 *   - unchecked_append / unchecked_assign / unchecked_to_inplace_string
 *   - try_append / try_assign / try_to_inplace_string (edge paths)
 *   - try_insert / try_insert with overflow / out-of-range
 *   - operator+= (char, string_view, c-string overloads)
 *   - operator= for char and zero-capacity guard
 *   - push_back at capacity (throws)
 *   - replace iterator-pair overloads & initializer_list overload
 *   - replace with same-size / shrink / grow paths
 *   - insert(const_iterator, char) and insert(const_iterator, n, char)
 *   - insert(const_iterator, initializer_list)
 *   - rfind(string), rfind(string, pos), rfind(c-string, pos, n)
 *   - find(string), find(c-string, pos, n), find(char, out-of-range)
 *   - find_first_of / find_last_of char overloads
 *   - find_first_not_of(char) edge cases, find_last_not_of(char) & n==0 path
 *   - compare(pos1, n1, inplace_string, pos2, n2) and string_view overload
 *   - compare pos1 > sz throws, n2==npos throws
 *   - operator< / > / <= / >= with c-string on both sides
 *   - global qx::swap
 *   - resize(n) default-char fill
 *   - resize_and_overwrite
 *   - length() (alias for size())
 *   - inplace_wstring / deduction guide
 *   - different-N equality (operator==, operator!=)
 *   - append(string_view-like, pos, n)
 *   - assign(string_view-like, pos, n) & out-of-range
 *   - insert(pos, inplace_string) and insert(pos, inplace_string, pos2, n2)
 *   - insert(pos, string_view-like)
 *   - replace(pos, n, inplace_string, pos2, n2)
 *   - replace(iterator, iterator, ...) variants
 *   - substr() return type is inplace_string (copy)
 *   - data() non-const
 *   - find_last_not_of with pos < sz branch
 *   - find_first_not_of n==0 returns npos
 */

#include "string_api_tester.h"
#include <qx/inplace_string.h>

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// ===========================================================================
// unchecked_append
// ===========================================================================

TEST(UncheckedAppend, InplaceString)
{
    qx::inplace_string<20> s("hello");
    qx::inplace_string<10> s2(" world");
    s.unchecked_append(s2);
    EXPECT_STREQ(s.c_str(), "hello world");
}

TEST(UncheckedAppend, StringView)
{
    qx::inplace_string<20> s("foo");
    std::string_view sv(" bar");
    s.unchecked_append(sv);
    EXPECT_STREQ(s.c_str(), "foo bar");
}

TEST(UncheckedAppend, CStringWithN)
{
    qx::inplace_string<10> s("ab");
    s.unchecked_append("cde", 3);
    EXPECT_STREQ(s.c_str(), "abcde");
}

TEST(UncheckedAppend, CString)
{
    qx::inplace_string<10> s("hi");
    s.unchecked_append("!");
    EXPECT_STREQ(s.c_str(), "hi!");
}

TEST(UncheckedAppend, FillChar)
{
    qx::inplace_string<10> s("x");
    s.unchecked_append(3, 'y');
    EXPECT_STREQ(s.c_str(), "xyyy");
}

TEST(UncheckedAppend, SubstringForm)
{
    qx::inplace_string<20> s("pre-");
    qx::inplace_string<10> src("abcdef");
    s.unchecked_append(src, 2, 3); // "cde"
    EXPECT_STREQ(s.c_str(), "pre-cde");
}

TEST(UncheckedAppend, SubstringFormStringView)
{
    qx::inplace_string<20> s("pre-");
    std::string_view sv("abcdef");
    s.unchecked_append(sv, 2, 3); // "cde"
    EXPECT_STREQ(s.c_str(), "pre-cde");
}

TEST(UncheckedAppend, ZeroN_IsNoop)
{
    qx::inplace_string<10> s("abc");
    s.unchecked_append("xyz", 0);
    EXPECT_STREQ(s.c_str(), "abc");
    EXPECT_EQ(s.size(), 3);
}

// ===========================================================================
// unchecked_assign
// ===========================================================================

TEST(UncheckedAssign, InplaceString)
{
    qx::inplace_string<10> s("old");
    qx::inplace_string<10> const s2("new");
    s.unchecked_assign(s2);
    EXPECT_STREQ(s.c_str(), "new");
}

TEST(UncheckedAssign, StringView)
{
    qx::inplace_string<10> s("old");
    std::string_view sv("replaced");
    s.unchecked_assign(sv);
    EXPECT_STREQ(s.c_str(), "replaced");
}

TEST(UncheckedAssign, CStringWithN)
{
    qx::inplace_string<10> s("old");
    s.unchecked_assign("hello", 3);
    EXPECT_STREQ(s.c_str(), "hel");
}

TEST(UncheckedAssign, CString)
{
    qx::inplace_string<10> s("old");
    s.unchecked_assign("brand");
    EXPECT_STREQ(s.c_str(), "brand");
}

TEST(UncheckedAssign, FillChar)
{
    qx::inplace_string<10> s("old");
    s.unchecked_assign(4, 'z');
    EXPECT_STREQ(s.c_str(), "zzzz");
    EXPECT_EQ(s.size(), 4);
}

TEST(UncheckedAssign, InitializerList)
{
    qx::inplace_string<10> s("old");
    s.unchecked_assign({'n', 'e', 'w'});
    EXPECT_STREQ(s.c_str(), "new");
}

TEST(UncheckedAssign, SubstringForm)
{
    qx::inplace_string<10> s("xxxxxx");
    qx::inplace_string<10> const src("abcdef");
    s.unchecked_assign(src, 1, 3); // "bcd"
    EXPECT_STREQ(s.c_str(), "bcd");
}

TEST(UncheckedAssign, SubstringFormStringView)
{
    qx::inplace_string<10> s("xxxxxx");
    std::string_view sv("abcdef");
    s.unchecked_assign(sv, 1, 3); // "bcd"
    EXPECT_STREQ(s.c_str(), "bcd");
}

// ===========================================================================
// try_append
// ===========================================================================

TEST(TryAppend, SuccessReturnsThis)
{
    qx::inplace_string<10> s("ab");
    auto* ret = s.try_append("cd");
    EXPECT_EQ(ret, &s);
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(TryAppend, OverflowReturnsNull)
{
    qx::inplace_string<5> s("abcd");
    auto* ret = s.try_append("ef"); // would need 6, capacity 5
    EXPECT_EQ(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abcd"); // unchanged
}

TEST(TryAppend, FillChar_Success)
{
    qx::inplace_string<10> s("ab");
    auto* ret = s.try_append(3, 'x');
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abxxx");
}

TEST(TryAppend, FillChar_Overflow)
{
    qx::inplace_string<5> s("abc");
    auto* ret = s.try_append(5, 'x'); // 3+5=8 > 5
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(s.size(), 3); // unchanged
}

TEST(TryAppend, InplaceString_Success)
{
    qx::inplace_string<10> s("hi");
    qx::inplace_string<10> s2("!");
    auto* ret = s.try_append(s2);
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "hi!");
}

TEST(TryAppend, InplaceString_Overflow)
{
    qx::inplace_string<5> s("abcd");
    qx::inplace_string<10> s2("xyz");
    auto* ret = s.try_append(s2);
    EXPECT_EQ(ret, nullptr);
}

TEST(TryAppend, StringViewLike)
{
    qx::inplace_string<10> s("go");
    std::string_view sv("lang");
    auto* ret = s.try_append(sv);
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "golang");
}

TEST(TryAppend, SubstringForm_OutOfRange)
{
    qx::inplace_string<10> s("hello");
    qx::inplace_string<10> const src("world");
    auto* ret = s.try_append(src, 99, 1); // pos > src.size()
    EXPECT_EQ(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "hello"); // unchanged
}

TEST(TryAppend, SubstringFormStringView_OutOfRange)
{
    qx::inplace_string<10> s("hello");
    std::string_view sv("world");
    auto* ret = s.try_append(sv, 99, 1);
    EXPECT_EQ(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "hello");
}

TEST(TryAppend, InitializerList_Success)
{
    qx::inplace_string<10> s("abc");
    auto* ret = s.try_append({'d', 'e'});
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abcde");
}

// ===========================================================================
// try_assign
// ===========================================================================

TEST(TryAssign, SuccessReturnsThis)
{
    qx::inplace_string<10> s("old");
    auto* ret = s.try_assign("new");
    EXPECT_EQ(ret, &s);
    EXPECT_STREQ(s.c_str(), "new");
}

TEST(TryAssign, OverflowReturnsNull)
{
    qx::inplace_string<5> s("abc");
    auto* ret = s.try_assign("toolongstring");
    EXPECT_EQ(ret, nullptr);
}

TEST(TryAssign, FillChar_Success)
{
    qx::inplace_string<10> s("abc");
    auto* ret = s.try_assign(4, 'z');
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "zzzz");
}

TEST(TryAssign, FillChar_Overflow)
{
    qx::inplace_string<5> s("abc");
    auto* ret = s.try_assign(10, 'z');
    EXPECT_EQ(ret, nullptr);
}

TEST(TryAssign, InplaceString)
{
    qx::inplace_string<10> s("old");
    qx::inplace_string<10> const s2("brand");
    auto* ret = s.try_assign(s2);
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "brand");
}

TEST(TryAssign, SubstringForm_Success)
{
    qx::inplace_string<10> s("old");
    qx::inplace_string<10> const src("abcdef");
    auto* ret = s.try_assign(src, 2, 3); // "cde"
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "cde");
}

TEST(TryAssign, SubstringForm_OutOfRange)
{
    qx::inplace_string<10> s("old");
    qx::inplace_string<10> const src("abc");
    auto* ret = s.try_assign(src, 99, 1);
    EXPECT_EQ(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "old"); // unchanged
}

TEST(TryAssign, StringViewLike_SubstringOutOfRange)
{
    qx::inplace_string<10> s("old");
    std::string_view sv("abc");
    auto* ret = s.try_assign(sv, 99, 1);
    EXPECT_EQ(ret, nullptr);
}

TEST(TryAssign, InitializerList)
{
    qx::inplace_string<10> s("old");
    auto* ret = s.try_assign({'n', 'e', 'w'});
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "new");
}

// ===========================================================================
// try_insert
// ===========================================================================

TEST(TryInsert, SuccessString)
{
    qx::inplace_string<10> s("ac");
    qx::inplace_string<10> const ins("b");
    auto* ret = s.try_insert(1, ins);
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(TryInsert, OverflowReturnsNull)
{
    qx::inplace_string<5> s("abcd");
    auto* ret = s.try_insert(2, "XY"); // 4+2=6 > 5
    EXPECT_EQ(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(TryInsert, OutOfRangeReturnsNull)
{
    qx::inplace_string<10> s("abc");
    auto* ret = s.try_insert(99, "x");
    EXPECT_EQ(ret, nullptr);
}

TEST(TryInsert, FillChar_Success)
{
    qx::inplace_string<10> s("ad");
    auto* ret = s.try_insert(1, 2, 'b');
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abbd");
}

TEST(TryInsert, FillChar_Overflow)
{
    qx::inplace_string<5> s("abcd");
    auto* ret = s.try_insert(1, 5, 'x');
    EXPECT_EQ(ret, nullptr);
}

TEST(TryInsert, FillChar_OutOfRange)
{
    qx::inplace_string<10> s("abc");
    auto* ret = s.try_insert(99, 1, 'x');
    EXPECT_EQ(ret, nullptr);
}

TEST(TryInsert, SubstringForm_OutOfRange)
{
    qx::inplace_string<10> s("abc");
    qx::inplace_string<10> const src("xyz");
    auto* ret = s.try_insert(0, src, 99, 1); // pos2 > src.size()
    EXPECT_EQ(ret, nullptr);
}

TEST(TryInsert, StringViewLike)
{
    qx::inplace_string<10> s("ac");
    std::string_view sv("b");
    // try_insert with StringLike delegates to try_insert(pos1, ptr, n)
    auto* ret = s.try_insert(1, sv);
    EXPECT_NE(ret, nullptr);
    EXPECT_STREQ(s.c_str(), "abc");
}

// ===========================================================================
// operator+= overloads not yet hit
// ===========================================================================

TEST(OperatorPlusEq, Char)
{
    qx::inplace_string<10> s("abc");
    s += 'd';
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(OperatorPlusEq, CString)
{
    qx::inplace_string<20> s("hello");
    s += " world";
    EXPECT_STREQ(s.c_str(), "hello world");
}

TEST(OperatorPlusEq, StringViewLike)
{
    qx::inplace_string<20> s("foo");
    std::string bar = " bar";
    s += bar; // std::string satisfies StringLike
    EXPECT_STREQ(s.c_str(), "foo bar");
}

// ===========================================================================
// operator= edge cases
// ===========================================================================

TEST(OperatorAssign, CharOnZeroCapacity)
{
    // Assigning a char to a zero-capacity string must throw
    EXPECT_THROW((qx::inplace_string<0>{} = 'x'), std::length_error);
}

TEST(OperatorAssign, InitializerList)
{
    qx::inplace_string<10> s;
    s = {'a', 'b', 'c'};
    EXPECT_STREQ(s.c_str(), "abc");
}

// ===========================================================================
// push_back at capacity throws
// ===========================================================================

TEST(PushBack, AtCapacityThrows)
{
    qx::inplace_string<5> s("abcde");
    EXPECT_THROW(s.push_back('f'), std::length_error);
}

// ===========================================================================
// insert: iterator and initializer_list overloads
// ===========================================================================

TEST(InsertIterator, SingleChar)
{
    qx::inplace_string<10> s("ac");
    auto it = s.insert(s.begin() + 1, 'b');
    EXPECT_EQ(*it, 'b');
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InsertIterator, NChars)
{
    qx::inplace_string<10> s("ad");
    auto it = s.insert(s.begin() + 1, 2, 'b');
    EXPECT_EQ(*it, 'b');
    EXPECT_STREQ(s.c_str(), "abbd");
}

TEST(InsertIterator, InitializerList)
{
    qx::inplace_string<10> s("ad");
    s.insert(s.begin() + 1, {'b', 'c'});
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(InsertPos, InplaceString)
{
    qx::inplace_string<10> s("ac");
    qx::inplace_string<10> const ins("b");
    s.insert(1, ins);
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InsertPos, InplaceStringSubstring)
{
    qx::inplace_string<10> s("ad");
    qx::inplace_string<10> const src("xbc");
    s.insert(1, src, 1, 2); // insert "bc" at pos 1
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(InsertPos, InplaceStringSubstring_OutOfRange)
{
    qx::inplace_string<10> s("ad");
    qx::inplace_string<10> const src("xbc");
    EXPECT_THROW(s.insert(1, src, 99, 1), std::out_of_range);
}

TEST(InsertPos, StringViewLike)
{
    qx::inplace_string<10> s("ac");
    std::string_view sv("b");
    s.insert(1, sv);
    EXPECT_STREQ(s.c_str(), "abc");
}

TEST(InsertPos, StringViewLikeSubstring)
{
    qx::inplace_string<10> s("ad");
    std::string_view sv("xbcy");
    s.insert(1, sv, 1, 2); // "bc"
    EXPECT_STREQ(s.c_str(), "abcd");
}

TEST(InsertPos, StringViewLikeSubstring_OutOfRange)
{
    qx::inplace_string<10> s("ad");
    std::string_view sv("xbc");
    EXPECT_THROW(s.insert(1, sv, 99, 1), std::out_of_range);
}

// ===========================================================================
// replace: iterator-pair overloads
// ===========================================================================

TEST(ReplaceIterator, InplaceString)
{
    qx::inplace_string<20> s("the quick fox");
    qx::inplace_string<10> const rep("slow");
    s.replace(s.begin() + 4, s.begin() + 9, rep);
    EXPECT_STREQ(s.c_str(), "the slow fox");
}

TEST(ReplaceIterator, StringViewLike)
{
    qx::inplace_string<20> s("the quick fox");
    std::string_view sv("lazy");
    s.replace(s.begin() + 4, s.begin() + 9, sv);
    EXPECT_STREQ(s.c_str(), "the lazy fox");
}

TEST(ReplaceIterator, CStringWithN)
{
    qx::inplace_string<20> s("hello world");
    s.replace(s.begin() + 6, s.end(), "there", 5);
    EXPECT_STREQ(s.c_str(), "hello there");
}

TEST(ReplaceIterator, CString)
{
    qx::inplace_string<20> s("hello world");
    s.replace(s.begin() + 6, s.end(), "C++");
    EXPECT_STREQ(s.c_str(), "hello C++");
}

TEST(ReplaceIterator, FillChar)
{
    qx::inplace_string<20> s("abcde");
    s.replace(s.begin() + 1, s.begin() + 4, 2, 'X');
    EXPECT_STREQ(s.c_str(), "aXXe");
}

TEST(ReplaceIterator, InitializerList)
{
    qx::inplace_string<20> s("abcde");
    s.replace(s.begin() + 1, s.begin() + 3, {'X', 'Y'});
    EXPECT_STREQ(s.c_str(), "aXYde");
}

TEST(ReplaceIterator, IteratorRange)
{
    qx::inplace_string<20> s("abcde");
    std::vector<char> v = {'X', 'Y', 'Z'};
    s.replace(s.begin() + 1, s.begin() + 3, v.begin(), v.end());
    EXPECT_STREQ(s.c_str(), "aXYZde");
}

// replace with equal-length (no-shift path)
TEST(ReplacePosN, EqualLength)
{
    qx::inplace_string<10> s("abcde");
    s.replace(1, 3, "XYZ"); // replace "bcd" with "XYZ" (same length)
    EXPECT_STREQ(s.c_str(), "aXYZe");
}

// replace with shrink path (n2 < n1)
TEST(ReplacePosN, Shrink)
{
    qx::inplace_string<10> s("abcde");
    s.replace(1, 3, "X"); // replace "bcd" with "X"
    EXPECT_STREQ(s.c_str(), "aXe");
}

// replace with grow path (n2 > n1)
TEST(ReplacePosN, Grow)
{
    qx::inplace_string<20> s("abcde");
    s.replace(1, 1, "XXXXX"); // replace "b" with "XXXXX"
    EXPECT_STREQ(s.c_str(), "aXXXXXcde");
}

// replace(pos, n, inplace_string)
TEST(ReplacePosN, InplaceString)
{
    qx::inplace_string<20> s("the quick fox");
    qx::inplace_string<10> const rep("lazy");
    s.replace(4, 5, rep);
    EXPECT_STREQ(s.c_str(), "the lazy fox");
}

// replace(pos, n1, inplace_string, pos2, n2)
TEST(ReplacePosN, InplaceStringSubstring)
{
    qx::inplace_string<20> s("the quick fox");
    qx::inplace_string<10> const src("laziness");
    s.replace(4, 5, src, 0, 4); // "lazy"
    EXPECT_STREQ(s.c_str(), "the lazi fox");
}

TEST(ReplacePosN, InplaceStringSubstring_OutOfRange)
{
    qx::inplace_string<20> s("hello");
    qx::inplace_string<10> const src("world");
    EXPECT_THROW(s.replace(0, 1, src, 99, 1), std::out_of_range);
}

// replace(pos, n1, string_view, pos2, n2)
TEST(ReplacePosN, StringViewLikeSubstring)
{
    qx::inplace_string<20> s("the quick fox");
    std::string_view sv("laziness");
    s.replace(4, 5, sv, 0, 4); // "lazy"
    EXPECT_STREQ(s.c_str(), "the lazi fox");
}

TEST(ReplacePosN, StringViewLikeSubstring_OutOfRange)
{
    qx::inplace_string<20> s("hello");
    std::string_view sv("world");
    EXPECT_THROW(s.replace(0, 1, sv, 99, 1), std::out_of_range);
}

// replace that would exceed capacity
TEST(ReplacePosN, ExceedsCapacityThrows)
{
    qx::inplace_string<5> s("ab");
    EXPECT_THROW(s.replace(0, 1, "XXXXXXXXXXX"), std::length_error);
}

// replace out-of-range pos throws
TEST(ReplacePosN, OutOfRangePosThrows)
{
    qx::inplace_string<10> s("abc");
    EXPECT_THROW(s.replace(99, 1, "X"), std::out_of_range);
}

// ===========================================================================
// rfind variants
// ===========================================================================

TEST(RFind, InplaceString)
{
    qx::inplace_string<20> const s("abcabc");
    qx::inplace_string<10> const pat("bc");
    EXPECT_EQ(s.rfind(pat), 4);
    EXPECT_EQ(s.rfind(pat, 3), 1);
}

TEST(RFind, StringViewLike)
{
    qx::inplace_string<20> const s("abcabc");
    std::string_view sv("bc");
    EXPECT_EQ(s.rfind(sv), 4);
}

TEST(RFind, CStringWithPosN)
{
    qx::inplace_string<20> const s("abcabc");
    EXPECT_EQ(s.rfind("bc", qx::inplace_string<20>::npos, 2), 4);
    EXPECT_EQ(s.rfind("bc", 3, 2), 1);
}

TEST(RFind, CStringFromPos)
{
    qx::inplace_string<20> const s("abcabc");
    EXPECT_EQ(s.rfind("abc", 2), 0);
}

TEST(RFind, CharFromPos)
{
    qx::inplace_string<20> const s("aabba");
    EXPECT_EQ(s.rfind('b', 3), 3);
    EXPECT_EQ(s.rfind('b', 2), 2);
    EXPECT_EQ(s.rfind('z'), qx::inplace_string<20>::npos);
}

TEST(RFind, EmptyString)
{
    qx::inplace_string<10> const s;
    EXPECT_EQ(s.rfind('a'), qx::inplace_string<10>::npos);
}

// ===========================================================================
// find variants
// ===========================================================================

TEST(Find, InplaceString)
{
    qx::inplace_string<20> const s("hello world");
    qx::inplace_string<10> const pat("world");
    EXPECT_EQ(s.find(pat), 6);
    EXPECT_EQ(s.find(pat, 7), qx::inplace_string<20>::npos);
}

TEST(Find, StringViewLike)
{
    qx::inplace_string<20> const s("hello world");
    std::string sv("world");
    EXPECT_EQ(s.find(sv), 6);
}

TEST(Find, CStringWithPosN)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find("world", 0, 5), 6);
    EXPECT_EQ(s.find("world", 7, 5), qx::inplace_string<20>::npos);
}

TEST(Find, EmptyPatternReturnsPos)
{
    // find(ptr, pos, n==0) returns pos immediately
    qx::inplace_string<10> const s("abc");
    EXPECT_EQ(s.find("", 2, 0), 2);
}

TEST(Find, PosExceedsSizeReturnsNpos)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_EQ(s.find('a', 99), qx::inplace_string<10>::npos);
}

// ===========================================================================
// find_first_of / find_last_of: char overloads
// ===========================================================================

TEST(FindFirstOf, CharOverload)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find_first_of('o'), 4);
    EXPECT_EQ(s.find_first_of('z'), qx::inplace_string<20>::npos);
}

TEST(FindFirstOf, PosExceedsSizeReturnsNpos)
{
    // pos >= sz or n == 0 both return npos
    qx::inplace_string<10> const s("abc");
    EXPECT_EQ(s.find_first_of("abc", 99, 3), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_first_of("abc", 0, 0), qx::inplace_string<10>::npos);
}

TEST(FindLastOf, CharOverload)
{
    qx::inplace_string<20> const s("hello world");
    EXPECT_EQ(s.find_last_of('o'), 7);
    EXPECT_EQ(s.find_last_of('z'), qx::inplace_string<20>::npos);
}

TEST(FindLastOf, EmptyNReturnsNpos)
{
    qx::inplace_string<10> const s("abc");
    // n == 0 branch in find_last_of(ptr, pos, n)
    EXPECT_EQ(s.find_last_of("abc", qx::inplace_string<10>::npos, 0), qx::inplace_string<10>::npos);
}

TEST(FindLastOf, InplaceString)
{
    qx::inplace_string<20> const s("hello world");
    qx::inplace_string<10> const vowels("aeiou");
    EXPECT_EQ(s.find_last_of(vowels), 7);
}

TEST(FindLastOf, StringViewLike)
{
    qx::inplace_string<20> const s("hello world");
    std::string vowels("aeiou");
    EXPECT_EQ(s.find_last_of(vowels), 7);
}

// ===========================================================================
// find_first_not_of edge cases
// ===========================================================================

TEST(FindFirstNotOf, CharOverload)
{
    qx::inplace_string<20> const s("aaabcd");
    EXPECT_EQ(s.find_first_not_of('a'), 3);
    // All same → returns npos via the loop falling off the end but
    // note: implementation returns pos when pos >= sz, so test carefully.
    qx::inplace_string<10> const same("aaaa");
    // pos (0) < sz (4), loop finds no mismatch → falls through to return npos
    EXPECT_EQ(same.find_first_not_of('a'), qx::inplace_string<10>::npos);
}

TEST(FindFirstNotOf, NEqualsZeroReturnsNpos)
{
    qx::inplace_string<10> const s("abc");
    // auto cpy = std::string(s);
    // cpy.find_first_not_of("abc", 0, 0)
    // n == 0 branch
    EXPECT_EQ(s.find_first_not_of("abc", 0, 0), 0);
}

TEST(FindFirstNotOf, PosExceedsSizeReturnsNpos)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_EQ(s.find_first_not_of("x", 99), qx::inplace_string<10>::npos);
}

TEST(FindFirstNotOf, InplaceString)
{
    qx::inplace_string<20> const s("hello world");
    qx::inplace_string<10> const chars("helo");
    EXPECT_EQ(s.find_first_not_of(chars), 5); // ' '
}

TEST(FindFirstNotOf, StringViewLike)
{
    qx::inplace_string<20> const s("hello world");
    std::string sv("helo");
    EXPECT_EQ(s.find_first_not_of(sv), 5);
}

// ===========================================================================
// find_last_not_of edge cases
// ===========================================================================

TEST(FindLastNotOf, CharOverload)
{
    qx::inplace_string<20> const s("aabbcc");
    EXPECT_EQ(s.find_last_not_of('c'), 3); // last 'b'
    qx::inplace_string<10> const same("cccc");
    EXPECT_EQ(same.find_last_not_of('c'),  qx::inplace_string<10>::npos);
}

TEST(FindLastNotOf, NEqualsZeroPath)
{
    // n == 0 in find_last_not_of(ptr, pos, n): the loop body has
    // traits::find(str, 0, *ps) which always returns nullptr → always matches
    // "not of", so first character from the back should be returned.
    qx::inplace_string<10> const s("abc");
    EXPECT_EQ(s.find_last_not_of("", qx::inplace_string<10>::npos, 0), 2u); // last 'c'
}

TEST(FindLastNotOf, InplaceString)
{
    qx::inplace_string<20> const s("hello world!!!");
    qx::inplace_string<10> const chars("!");
    EXPECT_EQ(s.find_last_not_of(chars), 10); // 'd'
}

TEST(FindLastNotOf, StringViewLike)
{
    qx::inplace_string<20> const s("hello world!!!");
    std::string sv("!");
    EXPECT_EQ(s.find_last_not_of(sv), 10);
}

TEST(FindLastNotOf, PosLessThanSz)
{
    // Tests the `if (pos < sz) ++pos; else pos = sz;` branch when pos < sz
    qx::inplace_string<20> const s("hello!!!");
    EXPECT_EQ(s.find_last_not_of("!", 4), 4); // 'o' at index 4
}

// ===========================================================================
// compare: various overloads
// ===========================================================================

TEST(Compare, PosN_InplaceStringSubstring)
{
    qx::inplace_string<20> const s("prefix_abc_suffix");
    qx::inplace_string<10> const sub("abc");
    EXPECT_EQ(s.compare(7, 3, sub, 0, 3), 0);
    EXPECT_GT(s.compare(7, 3, sub, 0, 2), 0); 
    EXPECT_LT(s.compare(7, 2, sub, 0, 3), 0);
}

TEST(Compare, PosN_StringViewLikeSubstring)
{
    qx::inplace_string<20> const s("prefix_abc_suffix");
    std::string sv("abc");
    EXPECT_EQ(s.compare(7, 3, sv, 0, 3), 0);
}

TEST(Compare, PosN_CStringWithN2)
{
    qx::inplace_string<10> const s("abcdef");
    EXPECT_EQ(s.compare(0, 3, "abc", 3), 0);
    EXPECT_LT(s.compare(0, 2, "abc", 3), 0);
}

TEST(Compare, PosGreaterThanSzThrows)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_THROW(s.compare(99, 1, "x", 1), std::out_of_range);
}

TEST(Compare, N2NposThrows)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_THROW(s.compare(0, 1, "x", qx::inplace_string<10>::npos), std::out_of_range);
}

TEST(Compare, CStringFromPos)
{
    qx::inplace_string<20> const s("prefix_abc");
    EXPECT_EQ(s.compare(7, 3, "abc"), 0);
}

TEST(Compare, LengthGreater)
{
    qx::inplace_string<10> const s1("abcd");
    qx::inplace_string<10> const s2("abc");
    EXPECT_GT(s1.compare(s2), 0);
}

// ===========================================================================
// Relational operators with c-string on left-hand side (pre-C++20 only,
// but we test them through the non-member functions which are always present)
// ===========================================================================

TEST(RelationalOperators, DifferentCapacity)
{
    qx::inplace_string<5> const s1("abc");
    qx::inplace_string<10> const s2("abc");
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 != s2);

    qx::inplace_string<10> const s3("abd");
    EXPECT_TRUE(s1 < s3);
    EXPECT_TRUE(s3 > s1);
    EXPECT_TRUE(s1 <= s2);
    EXPECT_TRUE(s1 >= s2);
}

TEST(RelationalOperators, CStringLhsEquals)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE("abc" == s);
    EXPECT_FALSE("xyz" == s);
}

#if __cplusplus <= 201703L
// These free functions are only defined for C++17 and earlier
TEST(RelationalOperators, CStringLhsNotEquals)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE("xyz" != s);
    EXPECT_FALSE("abc" != s);
}

TEST(RelationalOperators, CStringLhsLess)
{
    qx::inplace_string<10> const s("bcd");
    EXPECT_TRUE("abc" < s);
    EXPECT_FALSE("bcd" < s);
}

TEST(RelationalOperators, CStringLhsGreater)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE("bcd" > s);
    EXPECT_FALSE("abc" > s);
}

TEST(RelationalOperators, CStringLhsLessEq)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE("abc" <= s);
    EXPECT_TRUE("aaa" <= s);
    EXPECT_FALSE("bcd" <= s);
}

TEST(RelationalOperators, CStringLhsGreaterEq)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE("abc" >= s);
    EXPECT_TRUE("bcd" >= s);
    EXPECT_FALSE("aaa" >= s);
}
#endif

TEST(RelationalOperators, CStringRhsLessEq)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE(s <= "abc");
    EXPECT_TRUE(s <= "bcd");
    EXPECT_FALSE(s <= "aaa");
}

TEST(RelationalOperators, CStringRhsGreaterEq)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE(s >= "abc");
    EXPECT_TRUE(s >= "aaa");
    EXPECT_FALSE(s >= "bcd");
}

TEST(RelationalOperators, CStringRhsLess)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE(s < "bcd");
    EXPECT_FALSE(s < "abc");
}

TEST(RelationalOperators, CStringRhsGreater)
{
    qx::inplace_string<10> const s("bcd");
    EXPECT_TRUE(s > "abc");
    EXPECT_FALSE(s > "bcd");
}

TEST(RelationalOperators, CStringRhsNotEquals)
{
    qx::inplace_string<10> const s("abc");
    EXPECT_TRUE(s != "xyz");
    EXPECT_FALSE(s != "abc");
}

// ===========================================================================
// Global qx::swap
// ===========================================================================

TEST(GlobalSwap, SameCapacity)
{
    qx::inplace_string<10> s1("alpha");
    qx::inplace_string<10> s2("beta");
    qx::swap(s1, s2);
    EXPECT_STREQ(s1.c_str(), "beta");
    EXPECT_STREQ(s2.c_str(), "alpha");
}

// ===========================================================================
// resize(n) with default char (zero-fill)
// ===========================================================================

TEST(Resize, DefaultCharFill)
{
    qx::inplace_string<10> s("abc");
    s.resize(6); // extend with '\0' chars
    EXPECT_EQ(s.size(), 6);
    // The three extra chars should be '\0'
    EXPECT_EQ(s[3], '\0');
    EXPECT_EQ(s[4], '\0');
    EXPECT_EQ(s[5], '\0');
}

TEST(Resize, SameSize_IsNoop)
{
    qx::inplace_string<10> s("abc");
    s.resize(3, 'x');
    EXPECT_STREQ(s.c_str(), "abc");
}

// ===========================================================================
// resize_and_overwrite
// ===========================================================================

TEST(ResizeAndOverwrite, TruncatePath)
{
    qx::inplace_string<15> s("hello world");
    // op returns a new size smaller than n → erase_to_end(n) is then called
    // with the return value, not n itself.
    s.resize_and_overwrite(5, [](char* buf, std::size_t n) {
        (void)buf;
        (void)n;
        return std::size_t{3};
    });
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "hel");
}

TEST(ResizeAndOverwrite, ExtendPath)
{
    qx::inplace_string<20> s("hi");
    s.resize_and_overwrite(5, [](char* buf, std::size_t n) {
        for (std::size_t i = 2; i < n; ++i)
            buf[i] = 'x';
        return n; // keep full size
    });
    EXPECT_EQ(s.size(), 5);
    EXPECT_STREQ(s.c_str(), "hixxx");
}

TEST(ResizeAndOverwrite, ShrinkBelowCurrentSize)
{
    qx::inplace_string<20> s("longstring");
    s.resize_and_overwrite(3, [](char* buf, std::size_t n) {
        (void)buf;
        return n; // keep the 3 chars already there
    });
    EXPECT_EQ(s.size(), 3);
    EXPECT_STREQ(s.c_str(), "lon");
}

// ===========================================================================
// length() alias
// ===========================================================================

TEST(Length, AliasForSize)
{
    qx::inplace_string<10> const s("hello");
    EXPECT_EQ(s.length(), s.size());
    EXPECT_EQ(s.length(), 5);
}

// ===========================================================================
// inplace_wstring
// ===========================================================================

TEST(WideString, BasicOps)
{
    qx::inplace_wstring<10> ws(L"hello");
    EXPECT_EQ(ws.size(), 5);
    EXPECT_TRUE(ws == L"hello");
    ws.append(L" w");
    EXPECT_EQ(ws.size(), 7);
}

// ===========================================================================
// Deduction guide: basic_inplace_string(CharT const (&)[N]) -> <N-1, CharT>
// ===========================================================================

TEST(DeductionGuide, FromStringLiteral)
{
    qx::basic_inplace_string s("hello"); // deduces basic_inplace_string<5, char>
    static_assert(s.capacity() == 5);
    EXPECT_STREQ(s.c_str(), "hello");

    qx::basic_inplace_string ws(L"hi"); // deduces basic_inplace_string<2, wchar_t>
    static_assert(ws.capacity() == 2);
    EXPECT_TRUE(ws == L"hi");
}

// ===========================================================================
// append(StringLike, pos, n)
// ===========================================================================

TEST(Append, StringViewLikeSubstring)
{
    qx::inplace_string<20> s("start-");
    std::string src("abcdef");
    s.append(src, 2, 3); // "cde"
    EXPECT_STREQ(s.c_str(), "start-cde");
}

TEST(Append, StringViewLikeSubstring_OutOfRange)
{
    qx::inplace_string<10> s("abc");
    std::string src("xyz");
    EXPECT_THROW(s.append(src, 99, 1), std::out_of_range);
}

TEST(Append, InplaceStringSubstring_OutOfRange)
{
    qx::inplace_string<10> s("abc");
    qx::inplace_string<10> const src("xyz");
    EXPECT_THROW(s.append(src, 99, 1), std::out_of_range);
}

TEST(Append, ExceedsCapacityThrows)
{
    qx::inplace_string<5> s("abcd");
    EXPECT_THROW(s.append("ef"), std::length_error);
}

TEST(Append, FillCharExceedsCapacityThrows)
{
    qx::inplace_string<5> s("abc");
    EXPECT_THROW(s.append(5, 'x'), std::length_error);
}

// ===========================================================================
// assign(StringLike, pos, n)
// ===========================================================================

TEST(Assign, StringViewLikeSubstring)
{
    qx::inplace_string<10> s("old");
    std::string src("abcdef");
    s.assign(src, 2, 3); // "cde"
    EXPECT_STREQ(s.c_str(), "cde");
}

TEST(Assign, StringViewLikeSubstring_OutOfRange)
{
    qx::inplace_string<10> s("old");
    std::string src("abc");
    EXPECT_THROW(s.assign(src, 99, 1), std::out_of_range);
}

TEST(Assign, InplaceStringSubstring_OutOfRange)
{
    qx::inplace_string<10> s("old");
    qx::inplace_string<10> const src("abc");
    EXPECT_THROW(s.assign(src, 99, 1), std::out_of_range);
}

// ===========================================================================
// data() non-const mutation
// ===========================================================================

TEST(Data, NonConstMutation)
{
    qx::inplace_string<10> s("abc");
    char* ptr = s.data();
    ptr[0] = 'X';
    EXPECT_STREQ(s.c_str(), "Xbc");
}

// ===========================================================================
// substr copies correctly
// ===========================================================================

TEST(Substr, IsACopy)
{
    qx::inplace_string<20> const s("hello world");
    auto sub = s.substr(6, 5);
    static_assert(std::is_same_v<decltype(sub), qx::inplace_string<20>>, "substr must return same type");
    EXPECT_STREQ(sub.c_str(), "world");
    // Modifying sub must not affect original
    sub[0] = 'W';
    EXPECT_STREQ(s.c_str(), "hello world");
}

// ===========================================================================
// unchecked_to_inplace_string
// ===========================================================================

TEST(UncheckedToInplaceString, BasicInt)
{
    auto s = qx::unchecked_to_inplace_string<10>(42);
    EXPECT_STREQ(s.c_str(), "42");
}

TEST(UncheckedToInplaceString, NegativeInt)
{
    auto s = qx::unchecked_to_inplace_string<12>(-9876);
    EXPECT_STREQ(s.c_str(), "-9876");
}

TEST(UncheckedToInplaceString, Float)
{
    auto s = qx::unchecked_to_inplace_string<20>(3.14f);
    EXPECT_EQ(s.size(), std::string_view(s.c_str()).size());
    EXPECT_GT(s.size(), 0);
}

// ===========================================================================
// to_inplace_string (sized overload) success path
// ===========================================================================

TEST(ToInplaceStringSized, Success)
{
    auto s = qx::to_inplace_string<5>(123);
    EXPECT_STREQ(s.c_str(), "123");
}

// ===========================================================================
// Edge: erase(pos, 0) is a no-op
// ===========================================================================

TEST(Erase, ZeroLengthIsNoop)
{
    qx::inplace_string<10> s("abcdef");
    s.erase(2, 0);
    EXPECT_STREQ(s.c_str(), "abcdef");
}

// ===========================================================================
// Edge: insert at end (pos == size)
// ===========================================================================

TEST(Insert, AtEnd)
{
    qx::inplace_string<10> s("abc");
    s.insert(3, "def");
    EXPECT_STREQ(s.c_str(), "abcdef");
}

// ===========================================================================
// Edge: insert at position 0
// ===========================================================================

TEST(Insert, AtBegin)
{
    qx::inplace_string<10> s("def");
    s.insert(0, "abc");
    EXPECT_STREQ(s.c_str(), "abcdef");
}

// ===========================================================================
// Empty string edge cases
// ===========================================================================

TEST(EmptyString, FindOnEmpty)
{
    qx::inplace_string<10> const s;
    EXPECT_EQ(s.find("anything"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.rfind("anything"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_first_of("abc"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_last_of("abc"), qx::inplace_string<10>::npos);
    EXPECT_EQ(s.find_first_not_of("abc"), qx::inplace_string<10>::npos);
}

TEST(EmptyString, CompareWithEmpty)
{
    qx::inplace_string<10> const s1;
    qx::inplace_string<10> const s2;
    EXPECT_EQ(s1.compare(s2), 0);
    EXPECT_TRUE(s1 == s2);
}

// ===========================================================================
// append via overlapping self data (exercises the tmp-copy fallback path)
// ===========================================================================

TEST(AppendOverlapping, SelfSubstring)
{
    qx::inplace_string<20> s("loop");
    // append a range that points into s itself (not trivial-contiguous-safe)
    // This forces the `basic_inplace_string const tmp(first, last)` branch
    // inside append(InputIterator, InputIterator).
    std::string_view sv(s.data(), s.size()); // not a contiguous iterator of s
    s.append(sv);
    EXPECT_STREQ(s.c_str(), "looploop");
}

// ===========================================================================
// Append: length_error via iterator path
// ===========================================================================

TEST(AppendIterator, ExceedsCapacityThrows)
{
    qx::inplace_string<5> s("ab");
    std::vector<char> v = {'c', 'd', 'e', 'f'}; // 4 chars, 2+4=6 > 5
    EXPECT_THROW(s.append(v.begin(), v.end()), std::length_error);
}