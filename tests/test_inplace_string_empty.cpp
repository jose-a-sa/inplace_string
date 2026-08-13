#include <gtest/gtest.h>

#include <stdexcept>
#include <string_view>
#include <type_traits>

#include <qx/inplace_string.h>

using S = qx::inplace_string<0>;
constexpr auto npos = S::npos;

TEST(InplaceStringEmpty, IsEmptyType)
{
    EXPECT_TRUE(std::is_empty_v<S>);
}

TEST(InplaceStringEmpty, CapacityAndState)
{
    S s;
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.length(), 0);
    EXPECT_EQ(s.capacity(), 0);
    EXPECT_EQ(s.max_size(), 0);
}

TEST(InplaceStringEmpty, AssignmentAllowsEmpty)
{
    S s;

    s = "";
    EXPECT_EQ(s.size(), 0);

    s = std::string_view("");
    EXPECT_EQ(s.size(), 0);

    S s2;
    s = s2;
    EXPECT_EQ(s.size(), 0);

    s.assign("");
    EXPECT_EQ(s.size(), 0);

    s.assign(std::string_view(""));
    EXPECT_EQ(s.size(), 0);

    s.assign(0, 'a'); // Assigning zero copies of 'a' is valid
    EXPECT_EQ(s.size(), 0);
}

TEST(InplaceStringEmpty, AssignmentFailsOnNonEmpty)
{
    S s;
    EXPECT_THROW(s = "a", std::length_error);
    EXPECT_THROW(s.assign("a"), std::length_error);
    EXPECT_THROW(s.assign(1, 'a'), std::length_error);
}

TEST(InplaceStringEmpty, DataAndNullTermination)
{
    S s;
    S const& cs = s;

    ASSERT_NE(s.data(), nullptr);
    ASSERT_NE(s.c_str(), nullptr);
    EXPECT_EQ(s.data()[0], '\0');
    EXPECT_EQ(s.c_str()[0], '\0');

    ASSERT_NE(cs.data(), nullptr);
    ASSERT_NE(cs.c_str(), nullptr);
    EXPECT_EQ(cs.data()[0], '\0');
    EXPECT_EQ(cs.c_str()[0], '\0');
}

TEST(InplaceStringEmpty, AtThrowsOutOfRange)
{
    S s;
    S const& cs = s;

    EXPECT_THROW(s.at(0), std::out_of_range);
    EXPECT_THROW(cs.at(0), std::out_of_range);
}

TEST(InplaceStringEmpty, Iterators)
{
    S s;
    S const& cs = s;

    EXPECT_EQ(s.begin(), s.end());
    EXPECT_EQ(cs.begin(), cs.end());
    EXPECT_EQ(s.cbegin(), s.cend());

    EXPECT_EQ(s.rbegin(), s.rend());
    EXPECT_EQ(cs.rbegin(), cs.rend());
    EXPECT_EQ(s.crbegin(), s.crend());
}

TEST(InplaceStringEmpty, ModifiersOnEmpty)
{
    S s;

    s.clear();
    EXPECT_TRUE(s.empty());

    s.erase(std::size_t{0}, std::size_t{0});
    EXPECT_TRUE(s.empty());

    s.resize(0);
    EXPECT_TRUE(s.empty());
    s.resize(0, 'a');
    EXPECT_TRUE(s.empty());

    S s2;
    s.swap(s2);
    EXPECT_TRUE(s.empty());
    EXPECT_TRUE(s2.empty());
}

TEST(InplaceStringEmpty, ModifiersFailOutOfBounds)
{
    S s;
    EXPECT_THROW(s.resize(1), std::length_error);
    EXPECT_THROW(s.push_back('a'), std::length_error);
    EXPECT_THROW(s.append("a"), std::length_error);
    EXPECT_THROW(s.insert(0, "a"), std::length_error);
    EXPECT_THROW(s.erase(1, 1), std::out_of_range);
}

TEST(InplaceStringEmpty, FindOperations)
{
    S s;

    EXPECT_EQ(s.find(""), 0);
    EXPECT_EQ(s.find(std::string_view("")), 0);
    EXPECT_EQ(s.find(S{}), 0);

    EXPECT_EQ(s.find("a"), npos);
    EXPECT_EQ(s.find('a'), npos);

    EXPECT_EQ(s.rfind(""), 0);
    EXPECT_EQ(s.rfind("a"), npos);
    EXPECT_EQ(s.rfind('a'), npos);

    EXPECT_EQ(s.find_first_of(""), npos);
    EXPECT_EQ(s.find_first_of("a"), npos);
    EXPECT_EQ(s.find_first_of('a'), npos);

    EXPECT_EQ(s.find_last_of(""), npos);
    EXPECT_EQ(s.find_last_of("a"), npos);
    EXPECT_EQ(s.find_last_of('a'), npos);

    EXPECT_EQ(s.find_first_not_of(""), npos);
    EXPECT_EQ(s.find_first_not_of("a"), npos);
    EXPECT_EQ(s.find_first_not_of('a'), npos);

    EXPECT_EQ(s.find_last_not_of(""), npos);
    EXPECT_EQ(s.find_last_not_of("a"), npos);
    EXPECT_EQ(s.find_last_not_of('a'), npos);
}

TEST(InplaceStringEmpty, Compare)
{
    S s;
    S s2;

    EXPECT_EQ(s.compare(s2), 0);
    EXPECT_EQ(s.compare(""), 0);
    EXPECT_EQ(s.compare(std::string_view("")), 0);

    EXPECT_LT(s.compare("a"), 0);
    EXPECT_LT(s.compare(std::string_view("a")), 0);
}

TEST(InplaceStringEmpty, Operators)
{
    S s;
    S s2;

    EXPECT_TRUE(s == s2);
    EXPECT_TRUE(s == "");
    EXPECT_TRUE(s == std::string_view(""));

    EXPECT_FALSE(s != s2);
    EXPECT_TRUE(s != "a");

    EXPECT_FALSE(s < s2);
    EXPECT_TRUE(s <= s2);
    EXPECT_FALSE(s > s2);
    EXPECT_TRUE(s >= s2);

    EXPECT_TRUE(s < "a");
    EXPECT_TRUE(s <= "a");
    EXPECT_FALSE(s > "a");
    EXPECT_FALSE(s >= "a");
}