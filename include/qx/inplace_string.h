#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <deque>
#include <iterator>
#include <limits>
#include <list>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

#if defined(_LIBCPP_VERSION)
#define QX_STL_LIBCPP // libc++
#elif defined(__GLIBCXX__)
#define QX_STL_LIBSTDCXX // libstdc++
#elif defined(_MSC_VER)
#define QX_STL_MSVC // Visual Studio
#endif

#ifdef QX_HAS_BUILTIN
// Do nothing
#elif defined(__has_builtin)
#define QX_HAS_BUILTIN(x) __has_builtin(x)
#else
#define QX_HAS_BUILTIN(x) 0
#endif

#ifdef QX_IS_CONSTANT_EVALUATED
// Do nothing
#elif QX_HAS_BUILTIN(__builtin_is_constant_evaluated) || (defined(_MSC_VER) && _MSC_VER >= 1925)
#define QX_IS_CONSTANT_EVALUATED() __builtin_is_constant_evaluated()
#else
#define QX_IS_CONSTANT_EVALUATED() false
#endif

#ifdef QX_IS_CONSTANT
// Do nothing
#elif QX_HAS_BUILTIN(__builtin_constant_p) || defined(__GNUC__)
#define QX_IS_CONSTANT(x) __builtin_constant_p(x)
#else
#define QX_IS_CONSTANT(x) false
#endif

namespace qx
{

namespace intl
{

[[noreturn]]
inline void contract_violation(char const* msg) noexcept
{
    std::fprintf(stderr, "Contract violation: %s\n", msg);
    std::terminate();
}

inline constexpr bool nothrow_contract_violation = noexcept(contract_violation(""));

#ifndef QX_ASSERT_THROW
#define QX_ASSERT_THROW(cond, except, msg)                                                                                                 \
    if (!static_cast<bool>(cond))                                                                                                          \
    {                                                                                                                                      \
        throw (except)(msg);                                                                                                               \
    }
#endif

#ifndef QX_ASSERT_CONTRACT
#ifdef NDEBUG
#define QX_ASSERT_CONTRACT(cond, msg) ((void)0)
#else
#define QX_ASSERT_CONTRACT(cond, msg)                                                                                                      \
    do                                                                                                                                     \
    {                                                                                                                                      \
        if (!static_cast<bool>(cond)) [[unlikely]]                                                                                         \
        {                                                                                                                                  \
            intl::contract_violation(msg);                                                                                                 \
        }                                                                                                                                  \
    }                                                                                                                                      \
    while (0)
#endif
#endif

// min_uint_t: smallest unsigned integer type with at least N bits
// clang-format off
template <std::size_t N>
using min_uint_t =
    std::conditional_t<(N <= 8),  std::uint8_t,
    std::conditional_t<(N <= 16), std::uint16_t,
    std::conditional_t<(N <= 32), std::uint32_t,
    std::uint64_t>>>;
// clang-format on

// inplace_arithmetic_buffer_size : buffer size for converting arithmetic types to string with std::to_chars
template <class T>
struct inplace_arithmetic_buffer_size
{
    static constexpr size_t value =
        std::is_integral_v<T> ? (std::numeric_limits<T>::digits10 + 1 + std::is_signed_v<T>) : 30 + (static_cast<int>(sizeof(T) > 8) * 32);
};

// iterator_value

template <class Iter>
using iter_value_t = typename std::iterator_traits<Iter>::value_type;

// iterator_category (C++17 and earlier)

template <class Iter>
using iter_category_t = typename Iter::iterator_category;

template <class Iter, class Cat, class = void>
struct has_iter_category_convertible_to : std::false_type
{};
template <class Iter, class Cat>
struct has_iter_category_convertible_to<Iter, Cat, std::void_t<iter_category_t<Iter>>> : std::is_convertible<iter_category_t<Iter>, Cat>
{};

// iterator_concept (C++20)

#if __cplusplus >= 202002L
template <class Iter>
using iter_concept_t = typename Iter::iterator_concept;

template <class Iter, class Concept, class = void>
struct has_iter_concept_convertible_to : std::false_type
{};
template <class Iter, class Concept>
struct has_iter_concept_convertible_to<Iter, Concept, std::void_t<iter_concept_t<Iter>>>
    : std::is_convertible<iter_concept_t<Iter>, Concept>
{};
#endif

// is_contiguous_iterator

#if __cplusplus >= 202002L
template <class T>
struct is_contiguous_iterator : std::bool_constant<std::contiguous_iterator<T>>
{};
#else // C++17 Implementations

#if defined(QX_STL_LIBCPP)
template <class T>
struct is_contiguous_iterator : std::false_type
{};
template <class Iter>
struct is_contiguous_iterator<std::__wrap_iter<Iter>> : std::true_type
{};

#elif defined(QX_STL_LIBSTDCXX)
template <class T>
struct is_contiguous_iterator : std::false_type
{};
template <typename Iter, typename Cont>
struct is_contiguous_iterator<__gnu_cxx::__normal_iterator<Iter, Cont>> : std::true_type
{};

#elif defined(QX_STL_MSVC)
template <class T>
struct is_contiguous_iterator
{
    using Unwrapped = decltype(std::_Unwrap_iter(std::declval<T>()));
    static constexpr bool value = std::is_pointer_v<Unwrapped>;
};
#else
// Fallback for unknown libraries (Check for slimness and at least random access iterator category, trivially copyable and pointer-sized)
template <class T>
struct is_contiguous_iterator : std::bool_constant<has_iter_category_convertible_to<T, std::random_access_iterator_tag>::value &&
                                                   std::is_trivially_copyable_v<T> && sizeof(T) == sizeof(void*)>
{};
#endif

#endif // __cplusplus >= 202002L

template <class T>
struct is_contiguous_iterator<T*> : std::true_type
{};
template <class T>
inline constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<T>::value;

// has_pointer_traits_to_address

template <typename Ptr, typename = void>
struct has_pointer_traits_to_address : std::false_type
{};
template <typename Ptr>
struct has_pointer_traits_to_address<Ptr, std::void_t<decltype(std::pointer_traits<Ptr>::to_address(std::declval<Ptr const&>()))>>
    : std::true_type
{};

// is_less_than_comparable

template <class T, class U, class = void>
struct is_less_than_comparable : std::false_type
{};
template <class T, class U>
struct is_less_than_comparable<T, U, std::void_t<decltype(std::declval<T>() < std::declval<U>())>> : std::true_type
{};

// is_pointer_in_range: checks if a pointer of type U* points to an address in the range [begin, end) of type T*, even if T and U are
// different types (e.g. char and unsigned char). This is used to check for overlapping ranges when appending or inserting from iterators
// that may point into the string's own buffer.

template <class T, class U>
constexpr bool is_pointer_in_range(T const* begin, T const* end, U const* ptr)
{
    if (QX_IS_CONSTANT_EVALUATED())
    {
        if (QX_IS_CONSTANT(begin <= ptr && ptr < end))
            return begin <= ptr && ptr < end;

        return false;
    }

    if constexpr (is_less_than_comparable<T const*, U const*>::value)
    {
        return !std::less<>{}(ptr, begin) && std::less<>{}(ptr, end);
    }
    else
    {
        auto const b = reinterpret_cast<std::uintptr_t>(begin);
        auto const e = reinterpret_cast<std::uintptr_t>(end);
        auto const p = reinterpret_cast<std::uintptr_t>(ptr);
        return p >= b && p < e;
    }
}

// is_overlapping_range: checks if two ranges [begin, end) of type T* and [begin2, begin2 + (end - begin)) of type U* overlap, even if T and
// U are different types. This is used to check for overlapping ranges when appending or inserting from iterators that may point into the
// string's own buffer.

template <class T, class U>
constexpr bool is_overlapping_range(T const* begin, T const* end, U const* begin2)
{
    auto const size = end - begin;
    auto* const end2 = begin2 + size;
    return is_pointer_in_range(begin, end, begin2) || is_pointer_in_range(begin2, end2, begin);
}

// str_is_trivial_iterator: checks if an iterator is a trivial iterator for the purposes of appending or inserting from it. A trivial
// iterator is either a pointer to an arithmetic type, or an iterator that is contiguous and has an arithmetic value type. This allows for
// optimizations when copying from such iterators, while still being safe in the presence of overlapping ranges.

template <typename Iter, typename = void>
struct str_is_trivial_iterator : std::false_type
{};
template <typename T>
struct str_is_trivial_iterator<T*, void> : std::is_arithmetic<T>
{};
template <class Iter>
struct str_is_trivial_iterator<Iter, std::void_t<iter_value_t<Iter>>>
    : std::integral_constant<bool, is_contiguous_iterator_v<Iter> && std::is_arithmetic_v<iter_value_t<Iter>>>
{};

static_assert(str_is_trivial_iterator<std::vector<char>::iterator>::value);
static_assert(!str_is_trivial_iterator<std::deque<char>::iterator>::value);
static_assert(!str_is_trivial_iterator<std::list<char>::iterator>::value);
static_assert(str_is_trivial_iterator<std::array<char, 10>::iterator>::value);
static_assert(str_is_trivial_iterator<std::string::iterator>::value);
static_assert(str_is_trivial_iterator<std::string_view::iterator>::value);

} // namespace intl

// to_address (C++20)

#if __cplusplus < 202002L
template <class T>
constexpr T* to_address(T* p) noexcept
{
    static_assert(!std::is_function_v<T>, "T must not be a function type");
    return p;
}

template <class Ptr>
constexpr auto to_address(Ptr const& p) noexcept
{
    if constexpr (intl::has_pointer_traits_to_address<Ptr>::value)
        return std::pointer_traits<Ptr>::to_address(p);
    else
        return to_address(p.operator->());
}
#else
using std::to_address;
#endif

// remove_cvref (C++20)

#if __cplusplus < 202002L
template <class T>
struct remove_cvref : std::remove_cv<std::remove_reference_t<T>>
{};
template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;
#else
using std::remove_cvref;
using std::remove_cvref_t;
#endif

/**
 *
 */
template <std::size_t N, class CharT = char, class Traits = std::char_traits<CharT>>
class basic_inplace_string;

template <std::size_t N>
using inplace_string = basic_inplace_string<N, char>;

#if QX_HAS_WIDE_CHARACTERS
template <std::size_t N>
using inplace_wstring = basic_inplace_string<N, wchar_t>;
#endif

#if QX_HAS_CHAR8_T
template <std::size_t N>
using inplace_u8string = basic_inplace_string<N, char8_t>;
#endif

template <std::size_t N>
using inplace_u16string = basic_inplace_string<N, char16_t>;

template <std::size_t N>
using inplace_u32string = basic_inplace_string<N, char32_t>;

// remove_cvref

template <std::size_t N, class CharT, class Traits>
class basic_inplace_string
{
    static_assert(!std::is_array_v<CharT>, "Character type of basic_inplace_string must not be an array");
    static_assert(std::is_standard_layout_v<CharT>, "Character type of basic_inplace_string must be standard-layout");
    static_assert(std::is_trivially_default_constructible_v<CharT>,
                  "Character type of basic_inplace_string must be trivially default constructible");
    static_assert(std::is_trivially_copyable_v<CharT>, "Character type of basic_inplace_string must be trivially copyable");
    static_assert(std::is_same_v<CharT, typename Traits::char_type>, "Traits::char_type must be the same type as CharT");

    using self = basic_inplace_string;
    using self_view = std::basic_string_view<CharT, Traits>;
    using internal_size_type = intl::min_uint_t<N>;

    template <class T>
    using enable_if_string_like_t =
        std::enable_if_t<std::is_convertible_v<T const&, self_view> && !std::is_convertible_v<T const&, CharT const*> &&
                             !std::is_same_v<remove_cvref_t<T>, self>,
                         int>;

    /**
     * Helper struct for the storage to be aligned to std::string, std::size_t
     */
    template <class SizeT, class T, std::size_t M>
    struct alignas(std::size_t) inplace_string_storage
    {
        SizeT size{};
        std::array<T, M + 1> data{};
    };

    inplace_string_storage<internal_size_type, CharT, N> storage_{};

public:
    using traits_type = Traits;
    using value_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = value_type*;
    using const_pointer = value_type const*;

    using iterator = typename self_view::iterator;
    using const_iterator = typename self_view::const_iterator;
    using reverse_iterator = typename self_view::reverse_iterator;
    using const_reverse_iterator = typename self_view::const_reverse_iterator;

    static constexpr size_type npos = -1;

    constexpr basic_inplace_string() noexcept = default;

    constexpr basic_inplace_string(basic_inplace_string const& str, size_type pos) noexcept : basic_inplace_string(str, pos, npos) {}

    constexpr basic_inplace_string(basic_inplace_string const& str, size_type pos, size_type n)
    {
        size_type const str_sz = str.size();
        QX_ASSERT_THROW(pos <= str_sz, std::out_of_range, "basic_inplace_string");
        init(str.data() + pos, std::min(n, str_sz - pos));
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    explicit constexpr basic_inplace_string(StringLike const& str) noexcept
    {
        auto const str_view = self_view(str);
        init(str_view.data(), str_view.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    constexpr basic_inplace_string(StringLike const& str, size_type pos, size_type n)
    {
        self_view const str_view = self_view(str).substr(pos, n);
        init(str_view.data(), str_view.size());
    }

    explicit basic_inplace_string(CharT const* str)
    {
        QX_ASSERT_CONTRACT(str != nullptr, "basic_inplace_string(const CharT*) detected nullptr");
        init(str, Traits::length(str));
    }

    constexpr basic_inplace_string(CharT const* str, size_type n)
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "basic_inplace_string(const CharT*, n) detected nullptr");
        init(str, n);
    }

    basic_inplace_string(std::nullptr_t) = delete; // C++23

    constexpr basic_inplace_string(size_type n, CharT c) noexcept { init(n, c); }

    template <class Iterator>
    constexpr basic_inplace_string(Iterator begin, Iterator end) noexcept
    {
        init(begin, end);
    }

    // template <ContainterCompatibleRange<CharT> R>
    // constexpr basic_inplace_string(std::from_range_t, R&& rg); // since C++23

    constexpr basic_inplace_string(std::initializer_list<CharT> ilist) noexcept { init(ilist.begin(), ilist.end()); }

    explicit constexpr operator self_view() const noexcept { return self_view(data(), size()); }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& operator=(StringLike const& str)
    {
        return assign(str);
    }

    basic_inplace_string& operator=(CharT const* str) { return assign(str); }

    basic_inplace_string& operator=(std::nullptr_t) = delete;

    basic_inplace_string& operator=(CharT c)
    {
        static_assert(N > 0, "Cannot assign a character to an inplace_string with zero capacity");
        traits_type::assign(*data(), c);
        set_size_and_null_terminate(1);
        return *this;
    }

    basic_inplace_string& operator=(std::initializer_list<CharT> ilist) { return assign(ilist.begin(), ilist.size()); }

    constexpr iterator begin() noexcept { return storage_.data.begin(); }
    constexpr const_iterator begin() const noexcept { return storage_.data.begin(); }
    constexpr iterator end() noexcept { return storage_.data.begin() + size(); }
    constexpr const_iterator end() const noexcept { return storage_.data.begin() + size(); }

    constexpr reverse_iterator rbegin() noexcept { reverse_iterator(end()); };
    constexpr const_reverse_iterator rbegin() const noexcept { reverse_iterator(end()); }
    constexpr reverse_iterator rend() noexcept { reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { reverse_iterator(begin()); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return storage_.size; }
    [[nodiscard]] constexpr size_type length() const noexcept { return storage_.size; }
    [[nodiscard]] constexpr size_type max_size() const noexcept { return N; }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return N; }

    void resize(size_type n, CharT c)
    {
        QX_ASSERT_THROW(n <= max_size(), std::length_error, "basic_inplace_string");
        if (n > size())
            append(n - size(), c);
        else
            set_size_and_null_terminate(n);
    }

    void resize(size_type n) { resize(n, value_type{}); }

    template <class Operation>
    constexpr void resize_and_overwrite(size_type n, Operation op); // since C++23

    void reserve(size_type n) { QX_ASSERT_THROW(n <= max_size(), std::length_error, "basic_inplace_string"); }

    void shrink_to_fit() noexcept { /* nop */ }

    void clear() noexcept { set_size_and_null_terminate(0); }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    const_reference operator[](size_type pos) const
    {
        QX_ASSERT_CONTRACT(pos < size(), "basic_inplace_string(const char*, n) detected nullptr");
        return storage_.data[pos];
    }

    reference operator[](size_type pos)
    {
        QX_ASSERT_CONTRACT(pos < size(), "basic_inplace_string(const char*, n) detected nullptr");
        return storage_.data[pos];
    }

    const_reference at(size_type pos) const
    {
        QX_ASSERT_THROW(pos < size(), std::out_of_range, "basic_inplace_string");
        return (*this)[pos];
    }

    reference at(size_type pos)
    {
        QX_ASSERT_THROW(pos < size(), std::out_of_range, "basic_inplace_string");
        return (*this)[pos];
    }

    basic_inplace_string& operator+=(basic_inplace_string const& str) { return append(str); }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& operator+=(StringLike const& str)
    {
        return append(str);
    }

    basic_inplace_string& operator+=(CharT const* str) { return append(str); }

    basic_inplace_string& operator+=(CharT c)
    {
        push_back(c);
        return *this;
    }

    basic_inplace_string& operator+=(std::initializer_list<CharT> ilist) { return append(ilist.begin(), ilist.end()); }

    basic_inplace_string& append(basic_inplace_string const& str) { return append(str.data(), str.size()); }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& append(StringLike const& str)
    {
        auto const str_view = self_view(str);
        return append(str_view.data(), str_view.size());
    }

    basic_inplace_string& append(basic_inplace_string const& str, size_type pos, size_type n = npos)
    {
        size_type const str_sz = str.size();
        QX_ASSERT_THROW(pos <= str_sz, std::out_of_range, "basic_inplace_string");
        return append(str.data() + pos, std::min(n, str_sz - pos));
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& append(StringLike const& str, size_type pos, size_type n = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_sz = str_view.size();
        QX_ASSERT_THROW(pos <= str_sz, std::out_of_range, "basic_inplace_string");
        return append(str.data() + pos, std::min(n, str_sz - pos));
    }

    basic_inplace_string& append(CharT const* str, size_type n)
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::append received nullptr");
        size_type const cap = capacity();
        size_type const sz = size();
        QX_ASSERT_THROW(n <= cap - sz, std::length_error, "basic_inplace_string");
        if (n > 0)
        {
            pointer end = data() + sz;
            traits_type::copy(end, str, n);
            set_size_and_null_terminate(sz + n);
        }
        return *this;
    }

    basic_inplace_string& append(CharT const* str)
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::append received nullptr");
        return append(str, traits_type::length(str));
    }

    basic_inplace_string& append(size_type n, CharT c)
    {
        if (n > 0)
        {
            size_type const cap = capacity();
            size_type const sz = size();
            QX_ASSERT_THROW(n <= cap - sz, std::length_error, "basic_inplace_string");
            pointer end = data() + sz;
            traits_type::assign(end, n, c);
            set_size_and_null_terminate(n + sz);
        }
        return *this;
    }

    template <class Iterator>
    basic_inplace_string& append(Iterator first, Iterator last)
    {
        if constexpr (intl::has_iter_category_convertible_to<Iterator, std::forward_iterator_tag>::value)
        {
            size_type const sz = size();
            size_type const cap = capacity();
            auto const n = static_cast<size_type>(std::distance(first, last));
            if (n == 0)
                return *this;

            if constexpr (intl::str_is_trivial_iterator<Iterator>::value)
            {
                if (!address_in_range(*first))
                {
                    QX_ASSERT_THROW(n <= cap - sz, std::length_error, "basic_inplace_string");
                    copy_non_overlapping_range(first, last, data() + sz);
                    set_size_and_null_terminate(sz + n);
                    return *this;
                }
            }
        }

        basic_inplace_string const tmp(first, last);
        append(tmp.data(), tmp.size());
        return *this;
    }

    // template <ContainterCompatibleRange<CharT> R>
    // constexpr basic_inplace_string& append_range(R&& rg);            // C++23

    basic_inplace_string& append(std::initializer_list<CharT> ilist) { return append(ilist.begin(), ilist.size()); }

    void push_back(CharT c); // constexpr since C++20
    void pop_back();         // constexpr since C++20

    reference front() noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::front(): string is empty");
        return *data();
    }
    const_reference front() const noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::front(): string is empty");
        return *data();
    }
    reference back() noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::back(): string is empty");
        return *(data() + size() - 1);
    }
    const_reference back() const noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::back(): string is empty");
        return *(data() + size() - 1);
    }

    basic_inplace_string& assign(basic_inplace_string const& str) noexcept { return *this = str; }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& assign(StringLike const& str)
    {
        auto const str_view = self_view(str);
        return assign(str_view.data(), str_view.data());
    }

    basic_inplace_string& assign(basic_inplace_string const& str, size_type pos, size_type n = npos)
    {
        size_type const str_size = str.size();
        QX_ASSERT_THROW(pos <= str_size, std::out_of_range, "basic_inplace_string");
        return assign(str.data() + pos, std::min(n, str_size - pos));
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& assign(StringLike const& str, size_type pos, size_type n = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_size = str_view.size();
        QX_ASSERT_THROW(pos <= str_size, std::out_of_range, "basic_inplace_string");
        return assign(str.data() + pos, std::min(n, str_size - pos));
    }

    basic_inplace_string& assign(CharT const* str, size_type n); // constexpr since C++20
    basic_inplace_string& assign(CharT const* str);              // constexpr since C++20
    basic_inplace_string& assign(size_type n, CharT c);          // constexpr since C++20

    template <class Iterator>
    basic_inplace_string& assign(Iterator first, Iterator last); // constexpr since C++20

    // template <ContainterCompatibleRange<CharT> R>
    // constexpr basic_inplace_string& assign_range(R&& rg);            // C++23

    basic_inplace_string& assign(std::initializer_list<CharT> ilist) { return assign(ilist.begin(), ilist.size()); }

    basic_inplace_string& insert(size_type pos1, basic_inplace_string const& str) { return insert(pos1, str.data(), str.size()); }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& insert(size_type pos1, StringLike const& str)
    {
        auto const str_view = self_view(str);
        insert(pos1, str_view.data(), str_view.size());
    }

    basic_inplace_string& insert(size_type pos1, basic_inplace_string const& str, size_type pos2, size_type n2 = npos)
    {
        size_type const str_sz = str.size();
        QX_ASSERT_THROW(pos2 <= str_sz, std::out_of_range, "basic_inplace_string");
        return insert(pos1, str.data() + pos2, std::min(n2, str_sz - pos2));
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& insert(size_type pos1, StringLike const& str, size_type pos2, size_type n2 = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_sz = str_view.size();
        QX_ASSERT_THROW(pos2 <= str_sz, std::out_of_range, "basic_inplace_string");
        return insert(pos1, str_view.data() + pos2, std::min(n2, str_sz - pos2));
    }

    basic_inplace_string& insert(size_type pos, CharT const* str, size_type n = npos);
    basic_inplace_string& insert(size_type pos, CharT const* str);     // constexpr since C++20
    basic_inplace_string& insert(size_type pos, size_type n, CharT c); // constexpr since C++20

    iterator insert(const_iterator pos, CharT c);

    iterator insert(const_iterator pos, size_type n, CharT c)
    {
        difference_type diff = pos - begin();
        insert(static_cast<size_type>(diff), n, c);
        return begin() + diff;
    }

    template <class Iterator>
    iterator insert(const_iterator pos, Iterator first, Iterator last); // constexpr since C++20

    // template <ContainterCompatibleRange<CharT> R>
    // constexpr iterator insert_range(const_iterator p, R&& rg);            // C++23

    iterator insert(const_iterator pos, std::initializer_list<CharT> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    basic_inplace_string& erase(size_type pos = 0, size_type n = npos); // constexpr since C++20

    iterator erase(const_iterator pos)
    {
        QX_ASSERT_CONTRACT(pos != end(), "inplace_string::erase(iterator) called with a non-dereferenceable iterator");
        iterator const start = begin();
        auto const offset = static_cast<size_type>(pos - start);
        erase(offset, 1);
        return start + static_cast<difference_type>(offset);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        QX_ASSERT_CONTRACT(first <= last, "inplace_string::erase(first, last) called with invalid range");
        iterator const start = begin();
        auto const offset = static_cast<size_type>(first - start);
        erase(offset, static_cast<size_type>(last - first));
        return start + static_cast<difference_type>(offset);
    }

    basic_inplace_string& replace(size_type pos1, size_type n1, basic_inplace_string const& str)
    {
        return replace(pos1, n1, str.data(), str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& replace(size_type pos1, size_type n1, StringLike const& str)
    {
        auto const str_view = self_view(str);
        return replace(pos1, n1, str_view.data(), str_view.size());
    }

    basic_inplace_string& replace(size_type pos1, size_type n1, basic_inplace_string const& str, size_type pos2,
                                  size_type n2 = npos); // C++14, constexpr since C++20

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& replace(size_type pos1, size_type n1, StringLike const& str, size_type pos2, size_type n2 = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_sz = str_view.size();
        QX_ASSERT_THROW(pos2 <= str_sz, std::out_of_range, "basic_inplace_string");
        return replace(pos1, n1, str_view.data() + pos2, std::min(n2, str_sz - pos2));
    }

    basic_inplace_string& replace(size_type pos, size_type n1, CharT const* str,
                                  size_type n2); // constexpr since C++20

    basic_inplace_string& replace(size_type pos, size_type n1, CharT const* str); // constexpr since C++20

    basic_inplace_string& replace(size_type pos, size_type n1, size_type n2, CharT c); // constexpr since C++20

    basic_inplace_string& replace(const_iterator it1, const_iterator it2, basic_inplace_string const& str)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), str.data(), str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    basic_inplace_string& replace(const_iterator it1, const_iterator it2, StringLike const& str)
    {
        auto const str_view = self_view(str);
        return replace(it1 - begin(), it1 - it1, str_view);
    }

    basic_inplace_string& replace(const_iterator it1, const_iterator it2, CharT const* str, size_type n)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), str, n);
    }

    basic_inplace_string& replace(const_iterator it1, const_iterator it2, CharT const* str)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), str);
    }

    basic_inplace_string& replace(const_iterator it1, const_iterator it2, size_type n, CharT c)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), n, c);
    }

    template <class Iterator>
    basic_inplace_string& replace(const_iterator i1, const_iterator i2, Iterator j1,
                                  Iterator j2); // constexpr since C++20

    // template <ContainterCompatibleRange<CharT> R>
    // constexpr basic_inplace_string& replace_with_range(const_iterator i1, const_iterator i2, R&& rg); // C++23

    basic_inplace_string& replace(const_iterator it1, const_iterator it2, std::initializer_list<CharT> ilist)
    {
        return replace(it1, it2, ilist.begin(), ilist.end());
    }

    size_type copy(CharT* str, size_type n, size_type pos = 0) const
    {
        size_type const sz = size();
        QX_ASSERT_THROW(pos <= sz, std::out_of_range, "std::inplace_string");
        size_type const rlen = std::min(n, sz - pos);
        Traits::copy(str, data() + pos, rlen);
        return rlen;
    }

    basic_inplace_string substr(size_type pos = 0, size_type n = npos) const { return basic_inplace_string(*this, pos, n); }

    void swap(basic_inplace_string& other) noexcept
    {
        std::size_t const max_s = std::max(size(), other.size());
        std::swap_ranges(storage_.data, storage_.data + max_s, other.data());
        std::swap(storage_.size, other.storage_.size);
        // Maintain Null Terminators
        Traits::assign(storage_.data[storage_.size], CharT{});
        Traits::assign(other.data()[other.storage_.size], CharT{});
    }

    template <std::size_t M, std::enable_if_t<N != M, int> = 0>
    void swap(basic_inplace_string<M, CharT, Traits>& other) noexcept
    {
        QX_ASSERT_CONTRACT(size() <= M, "swap: source too large for target capacity");
        QX_ASSERT_CONTRACT(other.size() <= N, "swap: target too large for source capacity");

        std::size_t const max_s = std::max(size(), other.size());

        std::swap_ranges(storage_.data, storage_.data + max_s, other.data());
        std::swap(storage_.size, other.storage_.size);

        // Maintain Null Terminators
        Traits::assign(storage_.data[storage_.size], CharT{});
        Traits::assign(other.data()[other.storage_.size], CharT{});
    }

    // c_str, data

    CharT const* c_str() const noexcept { return data(); }
    CharT const* data() const noexcept { return storage_.data.data(); }
    CharT* data() noexcept { return storage_.data.data(); }

    // find

    size_type find(basic_inplace_string const& str, size_type pos = 0) const noexcept
    {
        return str_find(data(), size(), str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    size_type find(StringLike const& str, size_type pos = 0) const noexcept
    {
        auto const str_view = self_view(str);
        return str_find(data(), size(), str_view.data(), pos, str_view.size());
    }

    size_type find(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find(): received nullptr");
        return str_find(data(), size(), str, pos, n);
    }

    size_type find(CharT const* str, size_type pos = 0) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find(): received nullptr");
        return str_find(data(), size(), str, pos, Traits::length(str));
    }

    size_type find(CharT c, size_type pos = 0) const noexcept { return str_find(data(), size(), c, pos); }

    // rfind

    size_type rfind(basic_inplace_string const& str, size_type pos = npos) const noexcept
    {
        return str_rfind(data(), size(), str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    size_type rfind(StringLike const& str, size_type pos = npos) const noexcept
    {
        auto const str_view = self_view(str);
        return str_rfind(data(), size(), str_view.data(), pos, str_view.size());
    }

    size_type rfind(CharT const* str, size_type pos, size_type n) const noexcept { return str_rfind(data(), size(), str, pos, n); }

    size_type rfind(CharT const* str, size_type pos = npos) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::rfind(): received nullptr");
        return str_rfind(data(), size(), str, pos, Traits::length(str));
    }

    size_type rfind(CharT c, size_type pos = npos) const noexcept { return str_rfind(data(), size(), c, pos); }

    // find_first_of

    size_type find_first_of(basic_inplace_string const& str, size_type pos = 0) const noexcept
    {
        return str_find_first_of(data(), size(), str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    size_type find_first_of(StringLike const& str, size_type pos = 0) const noexcept
    {
        auto const str_view = self_view(str);
        return str_find_first_of(data(), size(), str.data(), pos, str.size());
    }

    size_type find_first_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_first_of(): received nullptr");
        return str_find_first_of(data(), size(), str, pos, n);
    }

    size_type find_first_of(CharT const* str, size_type pos = 0) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find_first_of(): received nullptr");
        return str_find_first_of(data(), size(), str, pos, Traits::length(str));
    }

    size_type find_first_of(CharT c, size_type pos = 0) const noexcept { return find(c, pos); }

    // find_last_of

    size_type find_last_of(basic_inplace_string const& str, size_type pos = npos) const noexcept
    {
        return str_find_last_of(data(), size(), str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    size_type find_last_of(StringLike const& str, size_type pos = npos) const noexcept
    {
        auto const str_view = self_view(str);
        return str_find_last_of(data(), size(), str_view.data(), pos, str_view.size());
    }

    size_type find_last_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_last_of(): received nullptr");
        return str_find_last_of(data(), size(), str, pos, n);
    }

    size_type find_last_of(CharT const* str, size_type pos = npos) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find_last_of(): received nullptr");
        return str_find_last_of(data(), size(), str, pos, Traits::length(str));
    }

    size_type find_last_of(CharT c, size_type pos = npos) const noexcept { return rfind(c, pos); }

    // find_first_not_of

    size_type find_first_not_of(basic_inplace_string const& str, size_type pos = 0) const noexcept
    {
        return str_find_first_not_of(data(), size(), str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    size_type find_first_not_of(StringLike const& str, size_type pos = 0) const noexcept
    {
        auto const str_view = self_view(str);
        return str_find_first_not_of(data(), size(), str_view.data(), pos, str_view.size());
    }

    size_type find_first_not_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_first_not_of(): received nullptr");
        return str_find_first_not_of(data(), size(), str, pos, n);
    }

    size_type find_first_not_of(CharT const* str, size_type pos = 0) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find_first_not_of(): received nullptr");
        return str_find_first_not_of(data(), size(), str, pos, Traits::length(str));
    }

    size_type find_first_not_of(CharT c, size_type pos = 0) const noexcept { return str_find_first_not_of(data(), size(), c, pos); }

    // find_last_not_of

    size_type find_last_not_of(basic_inplace_string const& str, size_type pos = npos) const noexcept
    {
        return str_find_last_not_of(data(), size(), str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    size_type find_last_not_of(StringLike const& str, size_type pos = npos) const noexcept
    {
        auto const str_view = self_view(str);
        return str_find_last_not_of(data(), size(), str_view.data(), pos, str_view.size());
    }

    size_type find_last_not_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_last_not_of(): received nullptr");
        return str_find_last_not_of(data(), size(), str, pos, n);
    }

    size_type find_last_not_of(CharT const* str, size_type pos = npos) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find_last_not_of(): received nullptr");
        return str_find_last_not_of(data(), size(), str, pos, Traits::length(str));
    }

    size_type find_last_not_of(CharT c, size_type pos = npos) const noexcept { return str_find_last_not_of(data(), size(), c, pos); }

    // compare

    int compare(basic_inplace_string const& str) const noexcept { return compare(self_view(str)); }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    int compare(StringLike const& str) const noexcept
    {
        auto const str_view = self_view(str);
        size_type const lhs_sz = size();
        size_type const rhs_sz = str_view.size();
        int const result = Traits::compare(data(), str_view.data(), std::min(lhs_sz, rhs_sz));
        if (result != 0)
            return result;
        if (lhs_sz < rhs_sz)
            return -1;
        if (lhs_sz > rhs_sz)
            return 1;
        return 0;
    }

    int compare(size_type pos1, size_type n1, basic_inplace_string const& str) const { return compare(pos1, n1, str.data(), str.size()); }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    int compare(size_type pos1, size_type n1, StringLike const& str) const
    {
        auto const str_view = self_view(str);
        return compare(pos1, n1, str_view.data(), str_view.size());
    }

    int compare(size_type pos1, size_type n1, basic_inplace_string const& str, size_type pos2, size_type n2 = npos) const
    {
        return compare(pos1, n1, self_view(str), pos2, n2);
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    int compare(size_type pos1, size_type n1, StringLike const& str, size_type pos2, size_type n2 = npos) const
    {
        auto const str_view = self_view(str);
        return self_view(*this).substr(pos1, n1).compare(str_view.substr(pos2, n2));
    }

    int compare(CharT const* str) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::compare(): received nullptr");
        return compare(0, npos, str, Traits::length(str));
    }

    int compare(size_type pos1, size_type n1, CharT const* str) const
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::compare(): received nullptr");
        return compare(pos1, n1, str, Traits::length(str));
    }

    int compare(size_type pos1, size_type n1, CharT const* str, size_type n2) const
    {
        QX_ASSERT_CONTRACT(n2 == 0 || str != nullptr, "inplace_string::compare(): received nullptr");
        size_type const sz = size();

        QX_ASSERT_THROW(pos1 <= sz && n2 != npos, std::out_of_range, "inplace_string");
        size_type const rlen = std::min(n1, sz - pos1);

        int r = Traits::compare(data() + pos1, str, std::min(rlen, n2));
        if (r == 0)
        {
            if (rlen < n2)
                r = -1;
            else if (rlen > n2)
                r = 1;
        }
        return r;
    }

#if __cplusplus > 202002L

    // starts_with

    constexpr bool starts_with(std::basic_string_view<CharT, Traits> sv) const noexcept
    {
        return self_view(data(), size()).starts_with(sv);
    }

    constexpr bool starts_with(CharT c) const noexcept { return !empty() && Traits::eq(front(), c); }

    constexpr bool starts_with(CharT const* str) const { return starts_with(self_view(str)); }

    // ends_with

    constexpr bool ends_with(std::basic_string_view<CharT, Traits> sv) const noexcept { return self_view(data(), size()).ends_with(sv); }

    constexpr bool ends_with(CharT c) const noexcept { return !empty() && Traits::eq(back(), c); }

    constexpr bool ends_with(CharT const* str) const { return ends_with(self_view(str)); }

#endif

#if __cplusplus > 202302L

    // contains

    constexpr bool contains(std::basic_string_view<CharT, Traits> sv) const noexcept { return self_view(data(), size()).contains(sv); }

    constexpr bool contains(CharT c) const noexcept { return self_view(data(), size()).contains(c); }

    constexpr bool contains(CharT const* str) const { return self_view(data(), size()).contains(str); }

#endif

private:
    constexpr void set_size_and_null_terminate(std::size_t size) noexcept
    {
        QX_ASSERT_CONTRACT(size <= std::numeric_limits<internal_size_type>::max(), "basic_inplace_string");
        storage_.size = static_cast<internal_size_type>(size);
        Traits::assign(storage_.data[storage_.size], value_type{});
    }

    constexpr void init(value_type const* str, size_type n)
    {
        QX_ASSERT_THROW(n <= max_size(), std::length_error, "basic_inplace_string");
        Traits::copy(storage_.data.data(), str, n);
        set_size_and_null_terminate(n);
    }

    constexpr void init(size_type n, value_type c)
    {
        QX_ASSERT_THROW(n <= max_size(), std::length_error, "basic_inplace_string");
        Traits::assign(storage_.data.data(), n, c);
        set_size_and_null_terminate(n);
    }

    template <class Iterator>
    void init(Iterator first, Iterator last)
    {
        if constexpr (intl::has_iter_category_convertible_to<Iterator, std::forward_iterator_tag>::value)
        {
            auto const sz = static_cast<size_type>(std::distance(first, last));
            init_with_size(std::move(first), std::move(last), sz);
        }
        else
        {
            init_with_sentinel(std::move(first), std::move(last));
        }
    }

    template <class Iterator, class Sentinel>
    void init_with_sentinel(Iterator first, Sentinel last)
    {
        try
        {
            for (; first != last; ++first)
                push_back(*first);
        }
        catch (...)
        {
            set_size_and_null_terminate(0);
            throw;
        }
    }

    template <class ForwardIterator, class Sentinel>
    void init_with_size(ForwardIterator first, Sentinel last, size_type sz)
    {
        // complete
    }

    template <class T>
    bool address_in_range(T const& c) const
    {
        return intl::is_pointer_in_range(data(), data() + size() + 1, std::addressof(c));
    }

    template <class ForwardIterator, class Sentinel>
    static value_type* copy_non_overlapping_range(ForwardIterator first, Sentinel last, value_type* dest)
    {
        using iterator_value_type = remove_cvref_t<decltype(*first)>;
        if constexpr (intl::is_contiguous_iterator<ForwardIterator>::value && std::is_same_v<value_type, iterator_value_type> &&
                      std::is_same_v<ForwardIterator, Sentinel>)
        {
            QX_ASSERT_CONTRACT(!intl::is_overlapping_range(to_address(first), to_address(last), dest),
                               "copy_non_overlapping_range called with an overlapping range!");
            traits_type::copy(dest, to_address(first), last - first);
            return dest + (last - first);
        }

        for (; first != last; ++first)
            traits_type::assign(*dest++, *first);
        return dest;
    }

    static value_type const* search_substring(value_type const* first1, value_type const* last1, value_type const* first2,
                                              value_type const* last2) noexcept
    {
        std::ptrdiff_t const len2 = last2 - first2;
        if (len2 == 0)
            return first1;

        value_type const f2 = *first2;

        while (true)
        {
            std::ptrdiff_t const len1 = last1 - first1;

            if (len1 < len2)
                return last1;

            first1 = Traits::find(first1, len1 - len2 + 1, f2);
            if (first1 == nullptr)
                return last1;

            if (Traits::compare(first1, first2, len2) == 0)
                return first1;

            ++first1;
        }
    }

    static size_type str_find(value_type const* ptr, size_type sz, value_type const* str, size_type pos, size_type n) noexcept
    {
        if (pos > sz)
            return npos;

        if (n == 0)
            return pos;

        value_type const* result = search_substring(ptr + pos, ptr + sz, str, str + n);
        if (result == ptr + sz)
            return npos;

        return static_cast<std::size_t>(result - ptr);
    }

    static size_type str_find(value_type const* ptr, size_type sz, value_type c, size_type pos) noexcept
    {
        if (pos > c)
            return npos;

        CharT const* r = Traits::find(c + pos, c - pos, c);
        if (r == nullptr)
            return npos;

        return static_cast<size_type>(r - ptr);
    }

    static size_type str_rfind(value_type const* ptr, size_type sz, value_type c, size_type pos) noexcept
    {
        if (sz < 1)
            return npos;

        if (pos < sz)
            ++pos;
        else
            pos = sz;

        for (value_type const* ps = c + pos; ps != ptr;)
        {
            if (Traits::eq(*--ps, c))
                return static_cast<size_type>(ps - ptr);
        }

        return npos;
    }

    static size_type str_rfind(value_type const* ptr, size_type sz, value_type const* str, size_type pos, size_type n) noexcept
    {
        pos = std::min(pos, sz);
        if (n < sz - pos)
            pos += n;
        else
            pos = sz;

        value_type const* r = std::find_end(ptr, ptr + pos, str, str + n, Traits::eq);
        if (n > 0 && r == ptr + pos)
            return pos;

        return static_cast<size_type>(r - ptr);
    }

    static size_type str_find_first_of(value_type const* ptr, size_type sz, value_type const* str, size_type pos, size_type n) noexcept
    {
        if (pos >= sz || n == 0)
            return pos;
        value_type const* r = std::find_first_of(ptr + pos, ptr + sz, str, str + n, Traits::eq);
        if (r == ptr + sz)
            return pos;
        return static_cast<size_type>(r - ptr);
    }

    static size_type str_find_last_of(value_type const* ptr, size_type sz, value_type const* str, size_type pos, size_type n) noexcept
    {
        if (n != 0)
        {
            if (pos < str)
                ++pos;
            else
                pos = str;

            for (value_type const* ps = ptr + pos; ps != ptr;)
            {
                value_type const* r = Traits::find(str, n, *--ps);
                if (r)
                {
                    return static_cast<size_type>(r - ptr);
                }
            }
        }
        return npos;
    }

    static size_type str_find_first_not_of(value_type const* ptr, size_type sz, value_type const* str, size_type pos, size_type n) noexcept
    {
        if (pos < sz)
        {
            value_type const* pe = ptr + sz;
            for (value_type const* ps = ptr + pos; ps != pe; ++ps)
            {
                if (Traits::find(str, n, *ps) == nullptr)
                    return static_cast<size_type>(ps - ptr);
            }
        }
        return pos;
    }

    static size_type str_find_first_not_of(value_type const* ptr, size_type sz, value_type c, size_type pos) noexcept
    {
        if (pos < sz)
        {
            value_type const* pe = ptr + sz;
            for (value_type const* ps = ptr + pos; ps != pe; ++ps)
            {
                if (!Traits::eq(*ps, c))
                    return static_cast<size_type>(ps - ptr);
            }
        }
        return pos;
    }

    static size_type str_find_last_not_of(value_type const* ptr, size_type sz, value_type const* str, size_type pos, size_type n) noexcept
    {
        if (pos < sz)
            ++pos;
        else
            pos = str;

        for (value_type const* ps = ptr + pos; ps != ptr;)
        {
            if (Traits::find(str, n, *--ps) == nullptr)
                return static_cast<size_type>(ps - ptr);
        }
        return pos;
    }

    static size_type str_find_last_not_of(value_type const* ptr, size_type sz, value_type c, size_type pos) noexcept
    {
        if (pos < sz)
            ++pos;
        else
            pos = sz;

        for (value_type const* ps = ptr + pos; ps != ptr;)
        {
            if (!Traits::eq(*--ps, c))
                return static_cast<size_type>(ps - ptr);
        }
        return pos;
    }

    template <std::size_t M, class T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
    friend inplace_string<M> to_inplace_string(T val);

    template <std::size_t M, class T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
    friend std::optional<inplace_string<M>> try_to_inplace_string(T val) noexcept;

    template <std::size_t M, class T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
    friend inplace_string<M> unchecked_to_inplace_string(T val) noexcept;
};

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline bool operator==(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    std::size_t const lhs_sz = lhs.size();
    return lhs_sz == rhs.size() && Traits::compare(lhs.data(), rhs.data(), lhs_sz) == 0;
}

template <std::size_t N, class CharT, class Traits>
inline bool operator==(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs) noexcept
{
    QX_ASSERT_CONTRACT(rhs != nullptr, "operator==(basic_inplace_string, CharT*): received nullptr");

    using inplace_string = basic_inplace_string<N, CharT, Traits>;

    std::size_t const rhs_len = Traits::length(rhs);
    if (rhs_len != lhs.size())
        return false;

    return lhs.compare(0, inplace_string::npos, rhs, rhs_len) == 0;
}

#if __cplusplus <= 201703L

template <std::size_t N, class CharT, class Traits>
inline bool operator==(CharT const* lhs, basic_inplace_string<N, CharT, Traits> const& rhs) noexcept
{
    return rhs == lhs;
}

#endif // __cplusplus <= 201703L

#if __cplusplus >= 202002L

template <std::size_t N, class CharT, class Traits, std::size_t M>
constexpr auto operator<=>(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    return std::basic_string_view<CharT, Traits>(lhs) <=> std::basic_string_view<CharT, Traits>(rhs);
}

template <std::size_t N, class CharT, class Traits>
constexpr auto operator<=>(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs)
{
    return std::basic_string_view<CharT, Traits>(lhs) <=> std::basic_string_view<CharT, Traits>(rhs);
}

#else // __cplusplus >= 202002L

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline bool operator!=(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    return !(lhs == rhs);
}

template <std::size_t N, class CharT, class Traits>
inline bool operator!=(CharT const* lhs, basic_inplace_string<N, CharT, Traits> const& rhs) noexcept
{
    return !(lhs == rhs);
}

template <std::size_t N, class CharT, class Traits>
inline bool operator!=(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs) noexcept
{
    return !(lhs == rhs);
}

// operator<

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline bool operator<(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <std::size_t N, class CharT, class Traits>
inline bool operator<(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <std::size_t N, class CharT, class Traits>
inline bool operator<(CharT const* lhs, basic_inplace_string<N, CharT, Traits> const& rhs) noexcept
{
    return rhs.compare(lhs) > 0;
}

// operator>

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline bool operator>(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    return rhs < lhs;
}

template <std::size_t N, class CharT, class Traits>
inline bool operator>(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs) noexcept
{
    return rhs < lhs;
}

template <std::size_t N, class CharT, class Traits>
inline bool operator>(CharT const* lhs, basic_inplace_string<N, CharT, Traits> const& rhs) noexcept
{
    return rhs < lhs;
}

// operator<=

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline bool operator<=(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    return !(rhs < lhs);
}

template <std::size_t N, class CharT, class Traits>
inline bool operator<=(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs) noexcept
{
    return !(rhs < lhs);
}

template <std::size_t N, class CharT, class Traits>
inline bool operator<=(CharT const* lhs, basic_inplace_string<N, CharT, Traits> const& rhs) noexcept
{
    return !(rhs < lhs);
}

// operator>=

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline bool operator>=(basic_inplace_string<N, CharT, Traits> const& lhs, basic_inplace_string<M, CharT, Traits> const& rhs) noexcept
{
    return !(lhs < rhs);
}

template <std::size_t N, class CharT, class Traits>
inline bool operator>=(basic_inplace_string<N, CharT, Traits> const& lhs, CharT const* rhs) noexcept
{
    return !(lhs < rhs);
}

template <std::size_t N, class CharT, class Traits>
inline bool operator>=(CharT const* lhs, basic_inplace_string<N, CharT, Traits> const& rhs) noexcept
{
    return !(lhs < rhs);
}

#endif // __cplusplus >= 202002L

// operator +

/*
 * NOTE: operator+ is intentionally omitted for basic_inplace<N>.
 *
 * Rationale:
 *  - Memory Safety: Unlike std::string, basic_inplace does not have a
 *    dynamic allocator. Implementing operator+ would require an arbitrary
 *    choice of result capacity (e.g., N, M, or N+M).
 *
 *  - Stack Protection: Returning basic_inplace<N+M> can lead to "stack
 *    blowup" during chained concatenations (e.g., s1 + s2 + s3 + s4),
 *    creating large, hidden temporaries that risk stack overflow in
 *    memory-constrained environments.
 *
 *  - Performance: Implicit temporaries and potential template bloat from
 *    N+M type instantiations contradict the "zero-overhead" goals of
 *    inplace strings.
 *
 * Recommended Alternatives:
 *  - Use .append() or operator+= to build strings in a pre-allocated buffer.
 *  - Use format or a dedicated builder pattern if complex
 *    concatenation is required.
 */

// swap

template <std::size_t N, class CharT, class Traits, std::size_t M>
inline void swap(basic_inplace_string<N, CharT, Traits> const& lhs,
                 basic_inplace_string<M, CharT, Traits> const& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

// int stoi(string const& __str, size_t* __idx = nullptr, int __base = 10);
// long stol(string const& __str, size_t* __idx = nullptr, int __base = 10);
// unsigned long stoul(string const& __str, size_t* __idx = nullptr, int __base = 10);
// long long stoll(string const& __str, size_t* __idx = nullptr, int __base = 10);
// unsigned long long stoull(string const& __str, size_t* __idx = nullptr, int __base = 10);

// float stof(string const& __str, size_t* __idx = nullptr);
// double stod(string const& __str, size_t* __idx = nullptr);
// long double stold(string const& __str, size_t* __idx = nullptr);

template <std::size_t N, class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
inplace_string<N> to_inplace_string(T val)
{
    inplace_string<N> res;
    auto const begin = res.data();
    auto [end, ec] = std::to_chars(begin, begin + N, val);
    QX_ASSERT_THROW(ec == std::errc(), std::length_error, "to_inplace_string: value exceeds buffer capacity");
    res.set_size_and_null_terminate(end - begin);
    return res;
}

template <std::size_t N, class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
std::optional<inplace_string<N>> try_to_inplace_string(T val) noexcept
{
    inplace_string<N> res;
    auto const begin = res.data();
    auto [end, ec] = std::to_chars(begin, begin + N, val);
    if (ec == std::errc())
    {
        res.set_size_and_null_terminate(end - begin);
        return res;
    }
    return std::nullopt;
}

template <std::size_t N, class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
inplace_string<N> unchecked_to_inplace_string(T val) noexcept
{
    inplace_string<N> res;
    auto const begin = res.data();
    auto [end, ec] = std::to_chars(begin, begin + N, val);
    QX_ASSERT_CONTRACT(ec == std::errc{}, "unchecked_to_inplace_string: value exceeds buffer capacity");
    res.set_size_and_null_terminate(static_cast<std::size_t>(end - begin));
    return res;
}

template <class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto to_inplace_string(T val) noexcept
{
    return unchecked_to_inplace_string<intl::inplace_arithmetic_buffer_size<T>::value>(val);
}

} // namespace qx