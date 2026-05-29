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
// 1. Construction & Initialization Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, NullPointerConstructorDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str);
        },
        "Contract violation");
}

TEST(InplaceStringContractTest, NullPointerConstructorWithSizeDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            char const* null_str = nullptr;
            qx::inplace_string<10> const s(null_str, 5);
        },
        "Contract violation");
}

// ---------------------------------------------------------
// 2. Element Access Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, OperatorBracketOutOfBoundsDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s("abc");
            char const volatile c = s[5]; // 5 >= size()
            (void)c;
        },
        "Contract violation");
}

TEST(InplaceStringContractTest, FrontOnEmptyDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.front();
            (void)c;
        },
        "Contract violation");
}

TEST(InplaceStringContractTest, BackOnEmptyDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> const s;
            char const volatile c = s.back();
            (void)c;
        },
        "Contract violation");
}

// ---------------------------------------------------------
// 3. Modifier Contracts (Append, Assign, Insert, Replace)
// ---------------------------------------------------------
TEST(InplaceStringContractTest, PopBackEmptyDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s;
            s.pop_back();
        },
        "Contract violation");
}

TEST(InplaceStringContractTest, AppendNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.append(null_str);
        },
        "Contract violation");
}

TEST(InplaceStringContractTest, AssignNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s;
            char const* null_str = nullptr;
            s.assign(null_str, 5);
        },
        "Contract violation");
}

// TEST(InplaceStringContractTest, InsertNullPointerDeath)
// {
//     EXPECT_DEATH_IF_NOT_DEBUG({
//                                   qx::inplace_string<10> s("ac");
//                                   char const* null_str = nullptr;
//                                   s.insert(1, null_str);
//                               },
//                               "Contract violation");
// }

TEST(InplaceStringContractTest, ReplaceNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.replace(0, 1, null_str);
        },
        "Contract violation");
}

// ---------------------------------------------------------
// 4. Erase & Iterator Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, EraseEndIteratorDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            s.erase(s.end());
        },
        "Contract violation");
}

TEST(InplaceStringContractTest, EraseInvalidIteratorRangeDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            // first > last
            s.erase(s.begin() + 2, s.begin() + 1);
        },
        "Contract violation");
}

// ---------------------------------------------------------
// 5. Cross-Capacity Swapping Contracts
// ---------------------------------------------------------
// TEST(InplaceStringContractTest, SwapSourceTooLargeDeath)
// {
//     EXPECT_DEATH_IF_NOT_DEBUG({
//                                   qx::inplace_string<10> large("123456789");
//                                   qx::inplace_string<5> small;
//                                   // large.size() (9) > small.capacity() (5)
//                                   large.swap(small);
//                               },
//                               "Contract violation");
// }

// TEST(InplaceStringContractTest, SwapTargetTooLargeDeath)
// {
//     EXPECT_DEATH_IF_NOT_DEBUG({
//                                   qx::inplace_string<5> small;
//                                   qx::inplace_string<10> large("123456789");
//                                   // large.size() (9) > small.capacity() (5)
//                                   small.swap(large);
//                               },
//                               "Contract violation");
// }

// ---------------------------------------------------------
// 6. Search & Compare Contracts
// ---------------------------------------------------------
TEST(InplaceStringContractTest, FindNullPointerDeath)
{
      qx::inplace_string<10> const s("abc");
            char const* null_str = nullptr;
            s.find(null_str);
    // EXPECT_DEATH_IF_NOT_DEBUG(
    //     {
    //         qx::inplace_string<10> const s("abc");
    //         char const* null_str = nullptr;
    //         s.find(null_str);
    //     },
    //     "Contract violation");
}

TEST(InplaceStringContractTest, CompareNullPointerDeath)
{
    EXPECT_DEATH_IF_NOT_DEBUG(
        {
            qx::inplace_string<10> s("abc");
            char const* null_str = nullptr;
            s.compare(null_str);
        },
        "Contract violation");
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
        "Contract violation");
}