#include <gmock/gmock.h>

#include <iomanip>
#include <ios>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <streambuf>

#include <qx/inplace_string.h>
#include <string>

// operator<<

TEST(InplaceStringInsertion, Basic)
{
    qx::inplace_string<16> s = "hello";
    std::ostringstream oss;
    oss << s;
    EXPECT_EQ(oss.str(), "hello");
}

TEST(InplaceStringInsertion, Empty)
{
    qx::inplace_string<16> s;
    std::ostringstream oss;
    oss << s;
    EXPECT_TRUE(oss.str().empty());
}

TEST(InplaceStringInsertion, FullCapacity)
{
    qx::inplace_string<5> s = "abcde";
    std::ostringstream oss;
    oss << s;
    EXPECT_EQ(oss.str(), "abcde");
}

TEST(InplaceStringInsertion, EmbeddedNul)
{
    qx::inplace_string<8> s;
    s.append("ab", 2);
    s.push_back('\0');
    s.append("cd", 2);
    std::ostringstream oss;
    oss << s;
    ASSERT_EQ(oss.str().size(), 5u);
    EXPECT_EQ(oss.str(), std::string("ab\0cd", 5));
}

TEST(InplaceStringInsertion, Chaining)
{
    qx::inplace_string<8> a = "foo";
    qx::inplace_string<8> b = "bar";
    std::ostringstream oss;
    oss << a << ' ' << b;
    EXPECT_EQ(oss.str(), "foo bar");
}

TEST(InplaceStringInsertion, RespectsSetwPaddingRightJustify)
{
    qx::inplace_string<8> s = "hi";
    std::ostringstream oss;
    oss << std::setw(5) << std::setfill('*') << s;
    EXPECT_EQ(oss.str(), "***hi");
}

TEST(InplaceStringInsertion, RespectsSetwPaddingLeftJustify)
{
    qx::inplace_string<8> s = "hi";
    std::ostringstream oss;
    oss << std::left << std::setw(5) << std::setfill('*') << s;
    EXPECT_EQ(oss.str(), "hi***");
}

TEST(InplaceStringInsertion, WidthResetAfterUse)
{
    qx::inplace_string<8> s = "hi";
    std::ostringstream oss;
    oss << std::setw(5) << std::setfill('*') << s << s;
    EXPECT_EQ(oss.str(), "***hihi");
}

TEST(InplaceStringInsertion, Wchar)
{
    qx::inplace_wstring<16> s = L"wide";
    std::wostringstream oss;
    oss << s;
    EXPECT_EQ(oss.str(), L"wide");
}

TEST(InplaceStringInsertion, WidthNarrowerThanContentDoesNotTruncate)
{
    qx::inplace_string<16> s = "hello";
    std::ostringstream oss;
    oss << std::setw(3) << s;
    EXPECT_EQ(oss.str(), "hello");
}

TEST(InplaceStringInsertion, WidthEqualToContentAddsNoPadding)
{
    qx::inplace_string<16> s = "hello";
    std::ostringstream oss;
    oss << std::setw(5) << s;
    EXPECT_EQ(oss.str(), "hello");
}

TEST(InplaceStringInsertion, DefaultFillIsSpace)
{
    qx::inplace_string<16> s = "hi";
    std::ostringstream oss;
    oss << std::setw(5) << s;
    EXPECT_EQ(oss.str(), "   hi");
}

TEST(InplaceStringInsertion, ExplicitRightJustifyMatchesDefault)
{
    qx::inplace_string<16> s = "hi";
    std::ostringstream oss;
    oss << std::right << std::setw(5) << std::setfill('-') << s;
    EXPECT_EQ(oss.str(), "---hi");
}

TEST(InplaceStringInsertion, NoOutputWhenStreamAlreadyFailed)
{
    qx::inplace_string<16> s = "hello";
    std::ostringstream oss;
    oss.setstate(std::ios_base::failbit);
    oss << s;
    EXPECT_TRUE(oss.str().empty());
    EXPECT_TRUE(oss.fail());
}

// operator>>

TEST(InplaceStringExtraction, Basic)
{
    std::istringstream iss("hello");
    qx::inplace_string<16> s;
    iss >> s;
    EXPECT_EQ(s, "hello");
    EXPECT_TRUE(iss.good() || iss.eof()); // extraction succeeded
    EXPECT_FALSE(iss.fail());
}

TEST(InplaceStringExtraction, SkipsLeadingWhitespace)
{
    std::istringstream iss("   \t\n  hello");
    qx::inplace_string<16> s;
    iss >> s;
    EXPECT_EQ(s, "hello");
    EXPECT_FALSE(iss.fail());
}

TEST(InplaceStringExtraction, StopsAtWhitespace)
{
    std::istringstream iss("hello world");
    qx::inplace_string<16> s;
    iss >> s;
    EXPECT_EQ(s, "hello");
    // "world" must still be extractable afterward
    iss >> s;
    EXPECT_EQ(s, "world");
}

TEST(InplaceStringExtraction, MultipleChained)
{
    std::istringstream iss("one two three");
    qx::inplace_string<16> a;
    qx::inplace_string<16> b;
    qx::inplace_string<16> c;
    iss >> a >> b >> c;
    EXPECT_EQ(a, "one");
    EXPECT_EQ(b, "two");
    EXPECT_EQ(c, "three");
    EXPECT_FALSE(iss.fail());
}

TEST(InplaceStringExtraction, SetwResetsAfterUse)
{
    std::istringstream iss("abcdef ghijkl");
    qx::inplace_string<32> a;
    qx::inplace_string<32> b;
    qx::inplace_string<32> c;
    iss >> std::setw(3) >> a; // limited to 3 chars: consumes "abc", leaves "def" in the stream
    EXPECT_EQ(a, "abc");
    iss >> b; // NOT limited to 3 anymore (width resets); reads the leftover "def"
    EXPECT_EQ(b, "def");
    iss >> c; // unaffected by the earlier setw, reads the second word whole
    EXPECT_EQ(c, "ghijkl");
}

TEST(InplaceStringExtraction, EofMidTokenSucceedsWithEofbit)
{
    std::istringstream iss("hi");
    qx::inplace_string<16> s;
    iss >> s;
    EXPECT_EQ(s, "hi");
    EXPECT_TRUE(iss.eof());
    EXPECT_FALSE(iss.fail());
}

TEST(InplaceStringExtraction, AllWhitespaceFailsAndLeavesStrUntouched)
{
    std::istringstream iss("    ");
    qx::inplace_string<16> s = "unchanged";
    iss >> s;
    EXPECT_TRUE(iss.fail());
    EXPECT_EQ(s, "unchanged"); // sentry fails before str is ever cleared
}

TEST(InplaceStringExtraction, EmptyStreamFails)
{
    std::istringstream iss("");
    qx::inplace_string<16> s;
    iss >> s;
    EXPECT_TRUE(iss.fail());
    EXPECT_TRUE(s.empty());
}

TEST(InplaceStringExtraction, ExactCapacityFitDetectsEof)
{
    std::istringstream iss("abcde");
    qx::inplace_string<5> s;
    iss >> s;
    EXPECT_EQ(s, "abcde");
    EXPECT_TRUE(iss.eof());
    EXPECT_FALSE(iss.fail());
}

TEST(InplaceStringExtraction, Wchar)
{
    std::wistringstream iss(L"wide chars");
    qx::inplace_wstring<16> s;
    iss >> s;
    EXPECT_EQ(s, L"wide");
}

TEST(InplaceStringExtractionOverflow, SwallowedByDefaultMask)
{
    std::istringstream iss("abcdefghij");
    qx::inplace_string<5> s;
    iss >> s;
    EXPECT_EQ(s, "abcde");
    EXPECT_TRUE(iss.bad());
    EXPECT_TRUE(iss.fail());
}

TEST(InplaceStringExtractionOverflow, PropagatesOriginalExceptionWhenMaskSet)
{
    std::istringstream iss("abcdefghij");
    iss.exceptions(std::ios_base::badbit);
    qx::inplace_string<5> s;

    ASSERT_THROW(iss >> s, std::length_error);
    EXPECT_TRUE(iss.bad());
    EXPECT_EQ(s, "abcde"); // partial prefix still visible after the throw
}

TEST(InplaceStringExtractionOverflow, StreamUsableAgainAfterClear)
{
    std::istringstream iss("abcdefghij klm");
    qx::inplace_string<5> s1;
    qx::inplace_string<5> s2;
    iss >> s1; // overflows, badbit set, swallowed
    EXPECT_TRUE(iss.fail());
    iss >> s2; // stream already failed; sentry fails, s2 untouched
    EXPECT_TRUE(s2.empty());

    iss.clear();
    iss >> s2;
    EXPECT_EQ(s2, "fghij");
}

TEST(InplaceStringExtractionOverflow, NarrowerSetwAvoidsOverflow)
{
    std::istringstream iss("abcdefghij");
    qx::inplace_string<32> big;
    iss >> std::setw(4) >> big;
    EXPECT_EQ(big, "abcd");
}

TEST(InplaceStringExtractionOverflow, WiderSetwDoesNotProtectAgainstOverflow)
{
    std::istringstream iss("abcdefghij");
    iss.exceptions(std::ios_base::badbit);
    qx::inplace_string<5> small;

    ASSERT_THROW(iss >> std::setw(100) >> small, std::length_error);
    EXPECT_EQ(small, "abcde");
}

// round-tripping

TEST(InplaceStringRoundtrip, WriteThenRead)
{
    qx::inplace_string<16> src = "roundtrip";
    std::stringstream ss;
    ss << src;
    qx::inplace_string<16> dst;
    ss >> dst;
    EXPECT_EQ(dst, src);
}

TEST(InplaceStringRoundtrip, MultipleTokensPreserveOrder)
{
    qx::inplace_string<8> a = "alpha";
    qx::inplace_string<8> b = "beta";
    qx::inplace_string<8> c = "gamma";
    std::stringstream ss;
    ss << a << ' ' << b << ' ' << c;

    qx::inplace_string<8> ra;
    qx::inplace_string<8> rb;
    qx::inplace_string<8> rc;
    ss >> ra >> rb >> rc;
    EXPECT_EQ(ra, a);
    EXPECT_EQ(rb, b);
    EXPECT_EQ(rc, c);
}

// exception safety

namespace
{

struct poison_char_marker
{}; // distinguishes our thrown exception from any other

struct throwing_streambuf : std::streambuf
{
    std::string data;
    std::size_t pos = 0;
    char poison;

    throwing_streambuf(std::string d, char p)
        : data(std::move(d))
        , poison(p)
    {}

    int_type underflow() override
    {
        if (pos >= data.size())
            return traits_type::eof();
        char const c = data[pos];
        if (c == poison)
            throw poison_char_marker{};
        return traits_type::to_int_type(c);
    }

    int_type uflow() override
    {
        int_type const c = underflow();
        if (!traits_type::eq_int_type(c, traits_type::eof()))
            ++pos;
        return c;
    }
};

} // namespace

TEST(InplaceStringExtractionExceptionSafety, SwallowedByDefaultMask)
{
    throwing_streambuf buf("ab#cd", '#');
    std::istream is(&buf);
    // default exception mask is goodbit: the exception must NOT escape.
    qx::inplace_string<16> s = "sentinel";
    is >> s;
    EXPECT_TRUE(is.bad());
    EXPECT_EQ(s, "ab");
}

TEST(InplaceStringExtractionExceptionSafety, RethrowsOriginalWhenMaskIncludesBadbit)
{
    throwing_streambuf buf("ab#cd", '#');
    std::istream is(&buf);
    is.exceptions(std::ios_base::badbit);

    qx::inplace_string<16> s;
    ASSERT_THROW(is >> s, poison_char_marker); // must be the ORIGINAL type, not ios_base::failure
    EXPECT_TRUE(is.bad());
}