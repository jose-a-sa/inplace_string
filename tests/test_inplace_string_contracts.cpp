#include <cstddef>
#include <qx/inplace_string.h>

#include <gtest/gtest.h>

// ---------------------------------------------------------
// Custom Macro for Safe Death Testing
// ---------------------------------------------------------
#ifndef NDEBUG
#define EXPECT_DEATH_IF_NOT_DEBUG(statement, regex) EXPECT_DEATH(statement, regex)
#else
#define EXPECT_DEATH_IF_NOT_DEBUG(statement, regex) GTEST_SKIP() << "Skipping contract death test in Release mode to prevent UB."
#endif

// ---------------------------------------------------------
// Construction & Initialization Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, NullPointerConstructorDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, NullPointerConstructorWithSizeDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str, 5);
        },
        "contract violation");
}

// ---------------------------------------------------------
// Element Access Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, OperatorBracketOutOfBoundsDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s("abc");
            char const volatile c = s[5]; // 5 >= size()
            (void)c;
        },
        "contract violation");
}

TEST(InplaceStringContractTest, FrontOnEmptyDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.front();
            (void)c;
        },
        "contract violation");
}

TEST(InplaceStringContractTest, BackOnEmptyDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.back();
            (void)c;
        },
        "contract violation");
}

// ---------------------------------------------------------
// Modifier Contracts (Append, Assign, Insert, Replace)
// ---------------------------------------------------------
TEST(InplaceStringContractTest, PopBackEmptyDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s;
            s.pop_back();
        },
        "contract violation");
}

TEST(InplaceStringContractTest, AppendNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.append(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, AssignNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s;
            char const* null_str = nullptr;
            s.assign(null_str, 5);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, InsertNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("ac");
            char const* null_str = nullptr;
            s.insert(1, null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, ReplaceNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.replace(0, 1, null_str);
        },
        "contract violation");
}

// ---------------------------------------------------------
// Erase & Iterator Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, EraseEndIteratorDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            s.erase(s.end());
        },
        "contract violation");
}

TEST(InplaceStringContractTest, EraseInvalidIteratorRangeDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            // first > last
            s.erase(s.begin() + 2, s.begin() + 1);
        },
        "contract violation");
}

// ---------------------------------------------------------
// Search & Compare Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, FindNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s("abc");
            char const* null_str = nullptr;
            s.find(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, CompareNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.compare(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, OperatorEqualsNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            bool b = (s == null_str);
            (void)b;
        },
        "contract violation");
}