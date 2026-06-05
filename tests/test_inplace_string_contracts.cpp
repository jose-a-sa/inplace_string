#define QX_HARDENING_MODE QX_HARDENING_MODE_ALL
#define QX_ASSERT_MODE QX_ASSERT_MODE_LOG_TRAP

#include <qx/inplace_string.h>

#include <gtest/gtest.h>

// ---------------------------------------------------------
// Construction & Initialization Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, NullPointerConstructorDeath)
{
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, NullPointerConstructorWithSizeDeath)
{
    EXPECT_DEATH(
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
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s("abc");
            char const volatile c = s[5]; // 5 >= size()
            (void)c;
        },
        "contract violation");
}

TEST(InplaceStringContractTest, FrontOnEmptyDeath)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.front();
            (void)c;
        },
        "contract violation");
}

TEST(InplaceStringContractTest, BackOnEmptyDeath)
{
    EXPECT_DEATH(
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
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s;
            s.pop_back();
        },
        "contract violation");
}

TEST(InplaceStringContractTest, AppendNullPointerDeath)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.append(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, AssignNullPointerDeath)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s;
            char const* null_str = nullptr;
            s.assign(null_str, 5);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, InsertNullPointerDeath)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s("ac");
            char const* null_str = nullptr;
            s.insert(1, null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, ReplaceNullPointerDeath)
{
    EXPECT_DEATH(
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
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s("abc");
            s.erase(s.end());
        },
        "contract violation");
}

TEST(InplaceStringContractTest, EraseInvalidIteratorRangeDeath)
{
    EXPECT_DEATH(
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
    EXPECT_DEATH(
        {
            qx::inplace_string<10> const s("abc");
            char const* null_str = nullptr;
            s.find(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, CompareNullPointerDeath)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.compare(null_str);
        },
        "contract violation");
}

TEST(InplaceStringContractTest, OperatorEqualsNullPointerDeath)
{
    EXPECT_DEATH(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            bool b = (s == null_str);
            (void)b;
        },
        "contract violation");
}