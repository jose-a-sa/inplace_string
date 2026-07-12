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

TEST(InplaceStringReplace, SelfReferentialAdditional)
{
    qx::inplace_string<32> s1("ABCDEF");
    std::string_view sv(s1.data() + 1, 3); // points to "BCD"
    s1.replace(2, 2, sv); // Replaces "CD" with "BCD" -> "ABBCDEF"
    EXPECT_STREQ(s1.c_str(), "ABBCDEF");

    qx::inplace_string<32> s2("012345");
    s2.replace(1, 4, s2, 1, 4); // Replaces "1234" with "1234"
    EXPECT_STREQ(s2.c_str(), "012345");

    qx::inplace_string<32> s3("abcdefgh");
    s3.replace(2, 3, s3, 4, 3); // Replaces "cde" with "efg"
    EXPECT_STREQ(s3.c_str(), "abefgfgh");
    
    qx::inplace_string<32> s4("xyz");
    s4.replace(1, 1, s4.data() + 2, 1); // Replaces 'y' with 'z' -> "xzz"
    EXPECT_STREQ(s4.c_str(), "xzz");
}

TEST(InplaceStringReplace, SelfReferentialGrow)
{
    qx::inplace_string<32> s = "ABCDEFGH";
    s.replace(0, 1, s, 3, 3); // replace "A" with substr(3,3) == "DEF"
    EXPECT_STREQ(s.c_str(), "DEFBCDEFGH");
}

TEST(InplaceStringReplace, SelfReferentialGrowCorrupts)
{
    qx::inplace_string<32> s = "0123456789";
    s.replace(2, 2, s, 5, 4); // replace "23" with substr(5,4) == "5678"
    EXPECT_STREQ(s.c_str(), "015678456789"); // currently produces "015456456789"
}

TEST(InplaceStringReplace, SelfReferentialShrink)
{
    qx::inplace_string<32> s = "ABCDEFGH";
    s.replace(1, 4, s, 6, 1); // replace "BCDE" with substr(6,1) == "G"
    EXPECT_STREQ(s.c_str(), "AGFGH");
}

TEST(InplaceStringReplace, SelfReferentialEqualSize)
{
    qx::inplace_string<32> s = "ABCDEFGH";
    s.replace(2, 2, s, 5, 2); // replace "CD" with substr(5,2) == "FG"
    EXPECT_STREQ(s.c_str(), "ABFGEFGH");
}

TEST(InplaceStringReplace, SelfReferentialGrowSourceBeforePos)
{
    qx::inplace_string<32> s = "ABCDEFGH";
    s.replace(4, 1, s, 0, 3); // replace "E" with substr(0,3) == "ABC"; source untouched by the shift
    EXPECT_STREQ(s.c_str(), "ABCDABCFGH");
}

TEST(InplaceStringReplace, ReplacePosCountChar)
{
    qx::inplace_string<20> s("hello world");
    
    // replace(pos, n1, n2, c)
    s.replace(6, 5, 4, 'X'); // replace 5 chars ("world") with 4 'X's
    EXPECT_STREQ(s.c_str(), "hello XXXX");
    
    // replace expanding
    s.replace(0, 5, 6, 'Y'); // replace "hello" with "YYYYYY"
    EXPECT_STREQ(s.c_str(), "YYYYYY XXXX");
}

TEST(InplaceStringReplace, ReplaceIteratorWithPointersAndList)
{
    qx::inplace_string<20> s("abcdef");
    
    // replace(it1, it2, ptr, n)
    s.replace(s.begin() + 1, s.begin() + 3, "XYZW", 3); // "bc" -> "XYZ"
    EXPECT_STREQ(s.c_str(), "aXYZdef");

    // replace(it1, it2, ptr)
    s.replace(s.begin() + 1, s.begin() + 4, "B"); // "XYZ" -> "B"
    EXPECT_STREQ(s.c_str(), "aBdef");

    // replace(it1, it2, initializer_list)
    s.replace(s.begin() + 1, s.begin() + 2, {'1', '2'}); // "B" -> "12"
    EXPECT_STREQ(s.c_str(), "a12def");
    
    // replace(it1, it2, n, c)
    s.replace(s.begin() + 1, s.begin() + 3, 3, '9'); // "12" -> "999"
    EXPECT_STREQ(s.c_str(), "a999def");
}
