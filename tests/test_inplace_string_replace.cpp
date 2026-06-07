#include <gmock/gmock.h>

#include <qx/inplace_string.h>

// ===========================================================================
// Replace
// ===========================================================================

TEST(InplaceStringReplace, BasicReplace)
{
    qx::inplace_string<20> s("the quick fox");
    s.replace(4, 5, "lazy");
    EXPECT_STREQ(s.c_str(), "the lazy fox");

    qx::inplace_string<10> s2("abcde");
    s2.replace(1, 3, "XYZ");
    EXPECT_STREQ(s2.c_str(), "aXYZe");

    qx::inplace_string<10> s3("abcde");
    s3.replace(1, 3, "X");
    EXPECT_STREQ(s3.c_str(), "aXe");

    qx::inplace_string<20> s4("abcde");
    s4.replace(1, 1, "XXXXX");
    EXPECT_STREQ(s4.c_str(), "aXXXXXcde");
}

TEST(InplaceStringReplace, ReplaceIterators)
{
    qx::inplace_string<20> s("the quick fox");
    qx::inplace_string<10> const rep("slow");
    s.replace(s.begin() + 4, s.begin() + 9, rep);
    EXPECT_STREQ(s.c_str(), "the slow fox");

    qx::inplace_string<20> s2("abcde");
    s2.replace(s2.begin() + 1, s2.begin() + 4, 2, 'X');
    EXPECT_STREQ(s2.c_str(), "aXXe");

    std::vector<char> v = {'X', 'Y', 'Z'};
    s2.replace(s2.begin() + 1, s2.begin() + 3, v.begin(), v.end());
    EXPECT_STREQ(s2.c_str(), "aXYZe");
}

TEST(InplaceStringReplace, ReplaceSubstring)
{
    qx::inplace_string<20> s("the quick fox");
    qx::inplace_string<10> const src("laziness");
    s.replace(4, 5, src, 0, 4);
    EXPECT_STREQ(s.c_str(), "the lazi fox");
}

TEST(InplaceStringReplace, ExceptionsAndContracts)
{
    qx::inplace_string<5> s("ab");
    EXPECT_THROW(s.replace(0, 1, "XXXXXXXXXXX"), std::length_error);
    EXPECT_THROW(s.replace(99, 1, "X"), std::out_of_range);
    EXPECT_DEATH(
        {
            char const* null_str = nullptr;
            s.replace(0, 1, null_str);
        },
        "contract violation"
    );
}