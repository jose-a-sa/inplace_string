/*
 * Copyright (c) 2026 Jose Sa
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ios>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>

// contract hardening level (0: none, 1: all, default: debug-only)
#ifndef QX_HARDENING_MODE_NONE
#define QX_HARDENING_MODE_NONE 0
#define QX_HARDENING_MODE_ALL 1
#endif

#ifndef QX_HARDENING_MODE
#define QX_HARDENING_MODE QX_HARDENING_MODE_NONE
#endif

// contract assert behavior (default: IO-logging and trap)
#ifndef QX_ASSERT_MODE_NONE
#define QX_ASSERT_MODE_NONE 0
#define QX_ASSERT_MODE_TRAP 1
#define QX_ASSERT_MODE_LOG_TRAP 2
#define QX_ASSERT_MODE_ABORT 3
#define QX_ASSERT_MODE_LOG_ABORT 4
#endif

#ifndef QX_ASSERT_MODE
#define QX_ASSERT_MODE QX_ASSERT_MODE_LOG_TRAP // default assertion mode
#else
#if (QX_ASSERT_MODE > QX_ASSERT_MODE_LOG_ABORT)
#undef QX_ASSERT_MODE
#define QX_ASSERT_MODE QX_ASSERT_MODE_NONE
#endif
#endif

#if !defined(QX_STL_LIBCPP) && !defined(QX_STL_LIBSTDCXX) && !defined(QX_STL_MSVC)
#ifdef _LIBCPP_VERSION
#define QX_STL_LIBCPP
#elif defined(__GLIBCXX__)
#define QX_STL_LIBSTDCXX
#elif defined(_MSVC_STL_VERSION) || defined(_CPPLIB_VER)
#define QX_STL_MSVC
#else
#if __cplusplus < 202002L
#error "qx::inplace_string not supported: unknown STL in CXX17"
#endif
#endif
#endif

#ifndef QX_STRINGIFY
#define QX_STRINGIFY_IMPL(x) #x
#define QX_STRINGIFY(x) QX_STRINGIFY_IMPL(x)
#endif

// set __has_builtin when not defined
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef __has_include
#define __has_include(x) 0
#endif

// __builtin_constant_p is a GCC + Clang builtin
#ifndef QX_IS_CONSTANT
#if __has_builtin(__builtin_constant_p) || defined(__GNUC__)
#define QX_IS_CONSTANT(x) __builtin_constant_p(x)
#else
#define QX_IS_CONSTANT(x) false
#endif
#endif

// __builtin_expect is a GCC + Clang builtin
#ifndef QX_LIKELY
#if __has_builtin(__builtin_expect) || defined(__GNUC__)
#define QX_LIKELY(x) __builtin_expect(!!(x), 1)
#define QX_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define QX_LIKELY(x) (x)
#define QX_UNLIKELY(x) (x)
#endif
#endif

#ifndef QX_CONSTEXPR_CXX20
#if __cplusplus >= 202002L
#define QX_CONSTEXPR_CXX20 constexpr
#else
#define QX_CONSTEXPR_CXX20
#endif
#endif

#ifndef QX_FORCE_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define QX_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define QX_FORCE_INLINE __forceinline
#else
#define QX_FORCE_INLINE inline
#endif
#endif

#ifndef QX_COLD_NOINLINE
#if defined(__GNUC__) || defined(__clang__)
#define QX_COLD_NOINLINE __attribute__((cold, noinline))
#elif defined(_MSC_VER)
#define QX_COLD_NOINLINE __declspec(noinline)
#else
#define QX_COLD_NOINLINE
#endif
#endif

// __builtin_trap is a GCC + Clang builtin
#ifndef QX_TRAP
#if __has_builtin(__builtin_trap) || defined(__GNUC__)
#define QX_TRAP() __builtin_trap()
#elif defined(_MSC_VER)
// __debugbreak() alone doesn't mark the site as noreturn to the
// optimizer; __assume(false) tells MSVC control never continues.
#define QX_TRAP() (__debugbreak(), __assume(false))
#else
#define QX_TRAP() std::abort()
#endif
#endif

// __builtin_verbose_trap is Clang-only.
// AppleClang < 17 shipped a 1-arg version; upstream Clang 18+ is 2-arg.
#ifndef QX_TRAP_WITH_MSG
#if defined(__clang__) && __has_builtin(__builtin_verbose_trap)
#if defined(__apple_build_version__) && __apple_build_version__ < 17000000
#define QX_TRAP_WITH_MSG(tag, msg) __builtin_verbose_trap(tag ": " msg)
#else
#define QX_TRAP_WITH_MSG(tag, msg) __builtin_verbose_trap(tag, msg)
#endif
#else
#define QX_TRAP_WITH_MSG(tag, msg) QX_TRAP()
#endif
#endif

// char8_t (post portability fix)
#ifndef QX_HAS_CHAR8_T
#if defined(__cpp_char8_t) && __cpp_char8_t >= 202207L
#define QX_HAS_CHAR8_T 1
#else
#define QX_HAS_CHAR8_T 0
#endif
#endif

namespace qx
{

namespace intl
{

// std::remove_cvref (C++20)

#if __cplusplus >= 202002L
using std::remove_cvref;
using std::remove_cvref_t;
#else
template <class T>
struct remove_cvref : std::remove_cv<std::remove_reference_t<T>>
{};
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
#endif // __cplusplus >= 202002L

// std::is_constant_evaluated() (C++20)

constexpr bool is_constant_evaluated() noexcept
{
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L
    return std::is_constant_evaluated();
#elif __has_builtin(__builtin_is_constant_evaluated)
    return __builtin_is_constant_evaluated();
#else
    return false; // Fallback
#endif
}

inline QX_COLD_NOINLINE void v_contract_fail_handler(char const* msg)
{
    std::fputs(msg, stderr);
    std::fputc('\n', stderr);
    std::fflush(stderr);
}

#ifndef QX_CONTRACT_FAIL_HANDLER
#if QX_ASSERT_MODE == QX_ASSERT_MODE_NONE
#define QX_CONTRACT_FAIL_HANDLER(tag, loc, msg) ((void)0)
#elif QX_ASSERT_MODE == QX_ASSERT_MODE_TRAP
#define QX_CONTRACT_FAIL_HANDLER(tag, loc, msg) (::qx::intl::is_constant_evaluated() ? QX_TRAP() : QX_TRAP_WITH_MSG(tag, msg))
#elif QX_ASSERT_MODE == QX_ASSERT_MODE_LOG_TRAP
#define QX_CONTRACT_FAIL_HANDLER(tag, loc, msg)                                                                                            \
    (::qx::intl::is_constant_evaluated() ? QX_TRAP() : (::qx::intl::v_contract_fail_handler(loc ": " msg), QX_TRAP_WITH_MSG(tag, msg)))
#elif QX_ASSERT_MODE == QX_ASSERT_MODE_ABORT
#define QX_CONTRACT_FAIL_HANDLER(tag, loc, msg) (::qx::intl::is_constant_evaluated() ? QX_TRAP() : std::abort())
#elif QX_ASSERT_MODE == QX_ASSERT_MODE_LOG_ABORT
#define QX_CONTRACT_FAIL_HANDLER(tag, loc, msg)                                                                                            \
    (::qx::intl::is_constant_evaluated() ? QX_TRAP() : (::qx::intl::v_contract_fail_handler(loc ": " msg), std::abort()))
#else
#define QX_CONTRACT_FAIL_HANDLER(tag, loc, msg) ((void)0)
#endif
#endif

#ifndef QX_ASSERT_CONTRACT
#if (QX_HARDENING_MODE > QX_HARDENING_MODE_NONE) && (QX_ASSERT_MODE > QX_ASSERT_MODE_NONE)
#define QX_ASSERT_CONTRACT(cond, msg)                                                                                                      \
    (QX_LIKELY(cond) ? ((void)0)                                                                                                           \
                     : QX_CONTRACT_FAIL_HANDLER("qxlib", __FILE__ ":" QX_STRINGIFY(__LINE__), "contract violation '" #cond "': " msg))
#else
#define QX_ASSERT_CONTRACT(cond, msg) ((void)0)
#endif
#endif

// min_size_t: obtain the minimal size-like type that fits a given capacity
// clang-format off
template <std::size_t N>
using min_size_t = 
    std::conditional_t<(N <= std::numeric_limits<std::uint_least8_t>::max()),  std::uint_least8_t,
    std::conditional_t<(N <= std::numeric_limits<std::uint_least16_t>::max()), std::uint_least16_t,
    std::conditional_t<(N <= std::numeric_limits<std::uint_least32_t>::max()), std::uint_least32_t,
    std::conditional_t<(N <= std::numeric_limits<std::uint_least64_t>::max()), std::uint_least64_t, 
    std::size_t>>>>;
// clang-format on

// max_exponent_digits10_v helper

template <std::size_t N>
struct ceil_log10
{
    static constexpr int value = (N < 10) ? 1 : 1 + ceil_log10<N / 10>::value;
};
template <>
struct ceil_log10<0> : std::integral_constant<int, 1>
{};

template <class T>
inline constexpr auto max_exponent_digits10_v = ceil_log10<std::numeric_limits<T>::max_exponent10>::value;

// iterator_value

template <class Iter>
using iter_value_t = typename std::iterator_traits<Iter>::value_type;

// iterator_category (C++17 and earlier)

template <class Iter>
using iter_category_t = typename std::iterator_traits<Iter>::iterator_category;

template <class Iter, class Cat, class = void>
struct is_iter_with_category : std::false_type
{};
template <class Iter, class Cat>
struct is_iter_with_category<Iter, Cat, std::void_t<iter_category_t<Iter>>> : std::is_convertible<iter_category_t<Iter>, Cat>
{};
template <class Iter, class Cat>
inline constexpr bool is_iter_with_category_v = is_iter_with_category<Iter, Cat>::value;

// is_contiguous_iterator

#if __cplusplus >= 202002L

template <class T>
struct is_contiguous_iterator : std::bool_constant<std::contiguous_iterator<T>>
{};

#else // C++17 implementation

template <class T, class = void>
struct is_contiguous_iterator : std::false_type
{};
template <class T>
struct is_contiguous_iterator<T*, void> : std::is_object<T>
{};

#if defined(QX_STL_LIBCPP) // libc++ (LLVM)

#if __has_include(<__iterator/wrap_iter.h>)
#include <__iterator/wrap_iter.h>
template <class Iter>
struct is_contiguous_iterator<std::__wrap_iter<Iter>, void> : is_contiguous_iterator<Iter>
{};
#endif // __has_include(<__iterator/wrap_iter.h>)

#if __has_include(<__iterator/bounded_iter.h>)
#include <__iterator/bounded_iter.h>
template <class Iter>
struct is_contiguous_iterator<std::__bounded_iter<Iter>, void> : is_contiguous_iterator<Iter>
{};
#endif // __has_include(<__iterator/bounded_iter.h>)

#elif defined(QX_STL_LIBSTDCXX) // libstdc++ (GNU)

template <class T>
struct is_gnu_wrapped_iterator : std::false_type
{};
template <class T>
inline constexpr bool is_gnu_wrapped_iterator_v = is_gnu_wrapped_iterator<T>::value;

#if __has_include(<bits/stl_iterator.h>)
#include <bits/stl_iterator.h>
template <class Iter, class Cont>
struct is_gnu_wrapped_iterator<::__gnu_cxx::__normal_iterator<Iter, Cont>> : std::true_type
{};
template <class Iter, class Cont>
struct is_contiguous_iterator<::__gnu_cxx::__normal_iterator<Iter, Cont>, void> : is_contiguous_iterator<Iter>
{};
template <class Iter, class Cont, class Tag>
struct is_gnu_wrapped_iterator<::__gnu_debug::_Safe_iterator<Iter, Cont, Tag>> : std::true_type
{};
template <class Iter, class Cont, class Tag>
struct is_contiguous_iterator<::__gnu_debug::_Safe_iterator<Iter, Cont, Tag>, void> : is_contiguous_iterator<Iter>
{};
#endif // __has_include(<bits/stl_iterator.h>)

#elif defined(QX_STL_MSVC) // MSVC STL

template <class T, class = void>
struct is_msvc_wrapped_iterator : std::false_type
{};
template <class T>
inline constexpr bool is_msvc_wrapped_iterator_v = is_msvc_wrapped_iterator<T>::value;

#if __has_include(<xutility>)
#include <xutility>
template <class T>
struct is_msvc_wrapped_iterator<T, std::void_t<decltype(std::_Get_unwrapped(std::declval<T&>()))>> : std::true_type
{};
template <class T>
struct is_contiguous_iterator<T, std::enable_if_t<!std::is_pointer_v<T> && is_msvc_wrapped_iterator_v<T>>>
    : std::is_pointer<decltype(std::_Get_unwrapped(std::declval<T&>()))>
{};
#endif // __has_include(<xutility>)

#endif
#endif // __cplusplus >= 202002L

template <class T>
inline constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<T>::value;

// has_pointer_traits_to_address

template <class Ptr, class = void>
struct has_pointer_traits_to_address : std::false_type
{};
template <class Ptr>
struct has_pointer_traits_to_address<Ptr, std::void_t<decltype(std::pointer_traits<Ptr>::to_address(std::declval<Ptr&>()))>>
    : std::true_type
{};
template <class Ptr>
inline constexpr bool has_pointer_traits_to_address_v = has_pointer_traits_to_address<Ptr>::value;

// has_arrow_operator

template <class Ptr, class = void>
struct has_arrow_operator : std::false_type
{};
template <class Ptr>
struct has_arrow_operator<Ptr, std::void_t<decltype(std::declval<Ptr&>().operator->())>> : std::true_type
{};
template <class Ptr>
inline constexpr bool has_arrow_operator_v = has_arrow_operator<Ptr>::value;

// std::to_address (C++20)

#if __cplusplus >= 202002L
using std::to_address;
#else // __cplusplus < 202002L

template <class T>
constexpr T* to_address(T* p) noexcept
{
    static_assert(!std::is_function_v<T>, "T must not be a function type");
    return p;
}

#ifdef QX_STL_LIBSTDCXX
template <class T>
inline constexpr auto is_fancy_pointer_v =
    std::is_class_v<T> && (is_gnu_wrapped_iterator_v<T> || has_pointer_traits_to_address_v<T> || has_arrow_operator_v<T>);
#elif defined(QX_STL_MSVC)
template <class T>
inline constexpr auto is_fancy_pointer_v =
    std::is_class_v<T> && (is_msvc_wrapped_iterator_v<T> || has_arrow_operator_v<T> || has_pointer_traits_to_address_v<T>);
#else
template <class T>
inline constexpr auto is_fancy_pointer_v = std::is_class_v<T> && (has_arrow_operator_v<T> || has_pointer_traits_to_address_v<T>);
#endif

template <class Ptr, std::enable_if_t<is_fancy_pointer_v<std::remove_reference_t<Ptr>>, int> = 0>
constexpr auto to_address(Ptr&& ptr) noexcept -> decltype(auto)
{
    using pointer = std::remove_reference_t<Ptr>;
    if constexpr (has_pointer_traits_to_address_v<pointer>) // handlers the LLVM  unwrapping
        return std::pointer_traits<pointer>::to_address(std::forward<Ptr>(ptr));
#ifdef QX_STL_LIBSTDCXX
    if constexpr (is_gnu_wrapped_iterator_v<pointer>)
        return to_address(std::forward<Ptr>(ptr).base());
#elif defined(QX_STL_MSVC)
    if constexpr (is_msvc_wrapped_iterator_v<pointer>)
        return to_address(std::_Get_unwrapped(std::forward<Ptr>(ptr)));
#endif
    return to_address(std::forward<Ptr>(ptr).operator->());
}

#endif // __cplusplus >= 202002L

// is_trivial_contiguous_iterator: checks if an iterator is a trivial iterator for the purposes of appending or
// inserting from it. A trivial iterator is either a pointer to an arithmetic type, or an iterator that is contiguous
// and has an arithmetic value type. This allows for optimizations when copying from such iterators, while still being
// safe in the presence of overlapping ranges.
template <class Iter, class = void>
struct is_trivial_contiguous_iterator : std::false_type
{};
template <class T>
struct is_trivial_contiguous_iterator<T*, void> : std::is_arithmetic<T>
{};
template <class Iter>
struct is_trivial_contiguous_iterator<Iter, std::void_t<iter_value_t<Iter>>>
    : std::integral_constant<bool, is_contiguous_iterator_v<Iter> && std::is_arithmetic_v<iter_value_t<Iter>>>
{};
template <class Iter>
inline constexpr bool is_trivial_contiguous_iterator_v = is_trivial_contiguous_iterator<Iter>::value;

// is_less_than_comparable

template <class T, class U, class = void>
struct is_less_than_comparable : std::false_type
{};
template <class T, class U>
struct is_less_than_comparable<T, U, std::void_t<decltype(std::declval<T>() < std::declval<U>())>> : std::true_type
{};
template <class T, class U>
inline constexpr bool is_less_than_comparable_v = is_less_than_comparable<T, U>::value;

// convertible_to_string_view

template <class CharT, class Traits, class T>
struct convertible_to_string_view
    : std::integral_constant<bool,
          std::is_convertible_v<T const&, std::basic_string_view<CharT, Traits>> && !std::is_convertible_v<T const&, CharT const*>>
{};
template <class CharT, class Traits, class T>
inline constexpr bool convertible_to_string_view_v = convertible_to_string_view<CharT, Traits, T>::value;

// is_pointer_in_range: checks if a pointer of type U* points to an address in the range [begin, end) of type T*, even
// if T and U are different types (e.g. char and unsigned char). This is used to check for overlapping ranges when
// appending or inserting from iterators that may point into the string's own buffer.
template <class T, class U>
constexpr bool is_pointer_in_range(T const* begin, T const* end, U const* ptr)
{
    if (is_constant_evaluated())
    {
        if (QX_IS_CONSTANT(begin <= ptr && ptr < end))
            return begin <= ptr && ptr < end;

        return false;
    }

    if constexpr (is_less_than_comparable_v<T const*, U const*>)
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

// is_overlapping_range: checks if two ranges [begin, end) of type T* and [begin2, begin2 + (end - begin)) of type U*
// overlap, even if T and U are different types. This is used to check for overlapping ranges when appending or
// inserting from iterators that may point into the string's own buffer.
template <class T, class U>
constexpr bool is_overlapping_range(T const* begin, T const* end, U const* begin2)
{
    auto const size = end - begin;
    auto* const end2 = begin2 + size;
    return is_pointer_in_range(begin, end, begin2) || is_pointer_in_range(begin2, end2, begin);
}

} // namespace intl

template <std::size_t N, class CharT, class Traits = std::char_traits<CharT>>
class basic_inplace_string;

template <std::size_t N>
using inplace_string = basic_inplace_string<N, char>;

template <std::size_t N>
using inplace_wstring = basic_inplace_string<N, wchar_t>;

#if QX_HAS_CHAR8_T
template <std::size_t N>
using inplace_u8string = basic_inplace_string<N, char8_t>;
#endif

template <std::size_t N>
using inplace_u16string = basic_inplace_string<N, char16_t>;

template <std::size_t N>
using inplace_u32string = basic_inplace_string<N, char32_t>;

template <class CharT, std::size_t N>
basic_inplace_string(CharT const (&)[N]) -> basic_inplace_string<N - 1, CharT>; // NOLINT(*-avoid-c-arrays)

template <std::size_t N, class CharT, class Traits>
class basic_inplace_string
{
    static_assert(!std::is_array_v<CharT>, "Character type of basic_inplace_string must not be an array");
    static_assert(std::is_standard_layout_v<CharT>, "Character type of basic_inplace_string must be standard-layout");
    static_assert(
        std::is_trivially_default_constructible_v<CharT>, "Character type of basic_inplace_string must be trivially default constructible");
    static_assert(std::is_trivially_copyable_v<CharT>, "Character type of basic_inplace_string must be trivially copyable");
    static_assert(std::is_same_v<CharT, typename Traits::char_type>, "Traits::char_type must be the same type as CharT");

    using self = basic_inplace_string;
    using self_view = std::basic_string_view<CharT, Traits>;

    template <class U>
    using enable_if_string_like_t = std::enable_if_t<intl::convertible_to_string_view_v<CharT, Traits, U>, int>;

    template <class U>
    using enable_if_unsame_string_like_t = std::enable_if_t<intl::convertible_to_string_view_v<CharT, Traits, U> &&
            !std::is_same_v<intl::remove_cvref_t<U>, basic_inplace_string>,
        int>;

public:
    using traits_type = Traits;
    using value_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = value_type*;
    using const_pointer = value_type const*;

    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos = -1;

    constexpr basic_inplace_string() noexcept = default;

    QX_CONSTEXPR_CXX20 basic_inplace_string(basic_inplace_string const& str, size_type pos)
        : basic_inplace_string(str, pos, npos)
    {}

    QX_CONSTEXPR_CXX20 basic_inplace_string(basic_inplace_string const& str, size_type pos, size_type n)
        : basic_inplace_string()
    {
        size_type const str_sz = str.size();
        if (pos > str_sz)
            throw_out_of_range();
        init(str.data() + pos, std::min(n, str_sz - pos));
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    explicit QX_CONSTEXPR_CXX20 basic_inplace_string(StringLike const& str)
        : basic_inplace_string()
    {
        auto const str_view = self_view(str);
        init(str_view.data(), str_view.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string(StringLike const& str, size_type pos)
        : basic_inplace_string(str, pos, npos)
    {}

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string(StringLike const& str, size_type pos, size_type n)
        : basic_inplace_string()
    {
        self_view const str_view = self_view(str).substr(pos, n);
        init(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string(CharT const* str) // NOLINT(*-explicit-constructor, *-explicit-conversions)
        : basic_inplace_string()
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string(ptr) detected nullptr");
        init(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string(CharT const* str, size_type n)
        : basic_inplace_string()
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string(char const*, n) detected nullptr");
        init(str, n);
    }

    constexpr basic_inplace_string(std::nullptr_t) = delete; // C++23

    QX_CONSTEXPR_CXX20 basic_inplace_string(size_type n, CharT c)
        : basic_inplace_string()
    {
        init(n, c);
    }

    template <class Iterator, std::enable_if_t<intl::is_iter_with_category_v<Iterator, std::input_iterator_tag>, int> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string(Iterator begin, Iterator end)
        : basic_inplace_string()
    {
        init(begin, end);
    }

    // struct init_with_sentinel_tag
    // {};

    // template <class Iterator, class Sentinel>
    // constexpr basic_inplace_string(init_with_sentinel_tag /* tag */, Iterator
    // begin, Sentinel end) noexcept
    // {
    //     init_with_sentinel(begin, end);
    // }

    // template <ContainerCompatibleRange<CharT> R>
    // constexpr basic_inplace_string(std::from_range_t, R&& rg); // since C++23

    QX_CONSTEXPR_CXX20 basic_inplace_string(std::initializer_list<CharT> il)
        : basic_inplace_string()
    {
        init(il.begin(), il.end());
    }

    // NOLINTNEXTLINE(*-explicit-constructor,*-explicit-conversions)
    constexpr operator self_view() const noexcept { return self_view(data(), size()); }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    basic_inplace_string& operator=(StringLike const& str)
    {
        return assign(str);
    }

    basic_inplace_string& operator=(CharT const* str) { return assign(str); }

    constexpr basic_inplace_string& operator=(std::nullptr_t) = delete; // C++23

    constexpr basic_inplace_string& operator=(CharT c)
    {
        if (capacity() == 0)
            throw_length_error();
        traits_type::assign(*data(), c);
        set_size_and_null_terminate(1);
        return *this;
    }

    basic_inplace_string& operator=(std::initializer_list<CharT> il) { return assign(il.begin(), il.size()); }

    constexpr iterator begin() noexcept { return rep_.data; }
    constexpr const_iterator begin() const noexcept { return rep_.data; }
    constexpr iterator end() noexcept { return rep_.data + size(); }
    constexpr const_iterator end() const noexcept { return rep_.data + size(); }

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend() const noexcept { return end(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return rep_.size; }
    [[nodiscard]] constexpr size_type length() const noexcept { return rep_.size; }
    // ReSharper disable once CppMemberFunctionMayBeStatic
    [[nodiscard]] constexpr size_type max_size() const noexcept { return N; }
    // ReSharper disable once CppMemberFunctionMayBeStatic
    [[nodiscard]] constexpr size_type capacity() const noexcept { return N; }

    void resize(size_type n, CharT c)
    {
        if (n > max_size())
            throw_length_error();

        if (n > size())
            append(n - size(), c);
        else
            erase_to_end(n);
    }

    void resize(size_type n) { resize(n, value_type{}); }

    template <class Operation>
    void resize_and_overwrite(size_type n, Operation op)
    {
        using result_type = decltype(std::move(op)(data(), n));
        static_assert(std::is_integral_v<result_type>, "Operation return type must be integer-like");
        if (n > capacity())
            throw_length_error();

        set_size_and_null_terminate(n);
        erase_to_end(std::move(op)(data(), n));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void reserve(size_type n)
    {
        if (n > max_size())
            throw_length_error();
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void shrink_to_fit() noexcept { /* nop */ }

    QX_CONSTEXPR_CXX20 void clear() noexcept { set_size_and_null_terminate(0); }

    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    constexpr const_reference operator[](size_type pos) const noexcept
    {
        QX_ASSERT_CONTRACT(pos < size(), "inplace_string::operator[](pos): pos out of bounds");
        return rep_.data[pos];
    }
    constexpr reference operator[](size_type pos) noexcept
    {
        QX_ASSERT_CONTRACT(pos < size(), "inplace_string::operator[](pos): pos out of bounds");
        return rep_.data[pos];
    }

    constexpr const_reference at(size_type pos) const
    {
        if (pos >= size())
            throw_out_of_range();
        return rep_.data[pos];
    }
    constexpr reference at(size_type pos)
    {
        if (pos >= size())
            throw_out_of_range();
        return rep_.data[pos];
    }

    basic_inplace_string& operator+=(basic_inplace_string const& str) { return append(str); }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
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

    basic_inplace_string& operator+=(std::initializer_list<CharT> il) { return append(il.begin(), il.end()); }

    QX_CONSTEXPR_CXX20 basic_inplace_string& append(basic_inplace_string const& str) { return append(str.data(), str.size()); }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& append(StringLike const& str)
    {
        auto const str_view = self_view(str);
        return append(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& append(basic_inplace_string const& str, size_type pos, size_type n = npos)
    {
        size_type const str_sz = str.size();
        if (pos > str_sz)
            throw_out_of_range();
        return append(str.data() + pos, std::min(n, str_sz - pos));
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& append(StringLike const& str, size_type pos, size_type n = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_sz = str_view.size();
        if (pos > str_sz)
            throw_out_of_range();
        return append(str_view.data() + pos, std::min(n, str_sz - pos));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& append(CharT const* str, size_type n)
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::append(ptr, n) detected nullptr");
        size_type const sz = size();
        if (n > capacity() - sz)
            throw_length_error();

        if (n > 0)
        {
            pointer end = data() + sz;
            traits_type::copy(end, str, n);
            set_size_and_null_terminate(sz + n);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& append(CharT const* str)
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::append(ptr) detected nullptr");
        return append(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& append(size_type n, CharT c)
    {
        if (n > 0)
        {
            size_type const sz = size();
            if (n > capacity() - sz)
                throw_length_error();
            pointer end = data() + sz;
            traits_type::assign(end, n, c);
            set_size_and_null_terminate(n + sz);
        }
        return *this;
    }

    template <class Iterator, std::enable_if_t<intl::is_iter_with_category_v<Iterator, std::input_iterator_tag>, int> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& append(Iterator first, Iterator last)
    {
        if constexpr (intl::is_iter_with_category_v<Iterator, std::forward_iterator_tag>)
        {
            size_type const sz = size();
            auto const n = static_cast<size_type>(std::distance(first, last));
            if (n == 0)
                return *this;

            if (intl::is_trivial_contiguous_iterator_v<Iterator> && !address_in_range(*first))
            {
                if (n > capacity() - sz)
                    throw_length_error();
                copy_non_overlapping_range(first, last, data() + sz);
                set_size_and_null_terminate(sz + n);
                return *this;
            }
        }

        basic_inplace_string const tmp(first, last);
        append(tmp.data(), tmp.size());
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& append(std::initializer_list<CharT> il) { return append(il.begin(), il.size()); }

    // unchecked_append

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_append(basic_inplace_string const& str) noexcept
    {
        return unchecked_append(str.data(), str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_append(StringLike const& str) noexcept
    {
        auto const str_view = self_view(str);
        return unchecked_append(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_append(CharT const* str, size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::unchecked_append(ptr, n) detected nullptr");
        if (n > 0)
        {
            size_type const sz = size();
            pointer end = data() + sz;
            traits_type::copy(end, str, n);
            set_size_and_null_terminate(sz + n);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_append(CharT const* str) noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::unchecked_append(ptr) detected nullptr");
        return unchecked_append(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_append(size_type n, CharT c) noexcept
    {
        if (n > 0)
        {
            size_type const sz = size();
            pointer end = data() + sz;
            traits_type::assign(end, n, c);
            set_size_and_null_terminate(n + sz);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_append(std::initializer_list<CharT> il) noexcept
    {
        return unchecked_append(il.begin(), il.size());
    }

    // template <ContainerCompatibleRange<CharT> R>
    // constexpr basic_inplace_string* unchecked_append_range(R&& rg); // C++23

    // try_append

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_append(basic_inplace_string const& str) noexcept
    {
        return try_append(str.data(), str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string* try_append(StringLike const& str) noexcept
    {
        auto const str_view = self_view(str);
        return try_append(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_append(CharT const* str, size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::try_append(ptr, n) detected nullptr");
        size_type const sz = size();
        if (n > capacity() - sz)
            return nullptr;

        if (n > 0)
        {
            pointer end = data() + sz;
            traits_type::copy(end, str, n);
            set_size_and_null_terminate(sz + n);
        }
        return this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_append(CharT const* str) noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::try_append(ptr, n) detected nullptr");
        return try_append(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_append(size_type n, CharT c) noexcept
    {
        if (n > 0)
        {
            size_type const sz = size();
            if (n > capacity() - sz)
                return nullptr;
            pointer end = data() + sz;
            traits_type::assign(end, n, c);
            set_size_and_null_terminate(n + sz);
        }
        return this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_append(std::initializer_list<CharT> il) { return try_append(il.begin(), il.size()); }

    // push_back

    QX_CONSTEXPR_CXX20 basic_inplace_string& push_back(CharT c)
    {
        if (size() == capacity())
            throw_length_error();
        return unchecked_push_back(c);
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_push_back(CharT c)
    {
        pointer const ptr = data();
        size_type const sz = size();
        traits_type::assign(ptr[sz], c);
        set_size_and_null_terminate(sz + 1);
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_push_back(CharT c)
    {
        if (size() == capacity())
            return nullptr;
        return &unchecked_push_back(c);
    }

    QX_CONSTEXPR_CXX20 void pop_back()
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::pop_back(): called on empty string");
        set_size_and_null_terminate(size() - 1);
    }

    constexpr reference front() noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::front(): string is empty");
        return *data();
    }
    constexpr const_reference front() const noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::front(): string is empty");
        return *data();
    }
    constexpr reference back() noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::back(): string is empty");
        return *(data() + size() - 1);
    }
    constexpr const_reference back() const noexcept
    {
        QX_ASSERT_CONTRACT(!empty(), "inplace_string::back(): string is empty");
        return *(data() + size() - 1);
    }

    // assign

    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(basic_inplace_string const& str) noexcept { return *this = str; }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(StringLike const& str)
    {
        auto const str_view = self_view(str);
        return assign(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(basic_inplace_string const& str, size_type pos, size_type n = npos)
    {
        size_type const str_size = str.size();
        if (pos > str_size)
            throw_out_of_range();
        return assign(str.data() + pos, std::min(n, str_size - pos));
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(StringLike const& str, size_type pos, size_type n = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_size = str_view.size();
        if (pos > str_size)
            throw_out_of_range();
        return assign(str_view.data() + pos, std::min(n, str_size - pos));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(CharT const* str, size_type n)
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::assign(ptr, n) detected nullptr");
        if (n > capacity())
            throw_length_error();
        traits_type::move(data(), str, n);
        set_size_and_null_terminate(n);
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(CharT const* str)
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::assign(ptr) detected nullptr");
        return assign(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(size_type n, CharT c)
    {
        if (n > capacity())
            throw_length_error();
        traits_type::assign(data(), n, c);
        set_size_and_null_terminate(n);
        return *this;
    }

    template <class Iterator, std::enable_if_t<intl::is_iter_with_category_v<Iterator, std::input_iterator_tag>, int> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(Iterator first, Iterator last)
    {
        if constexpr (intl::is_iter_with_category_v<Iterator, std::forward_iterator_tag> &&
            intl::is_trivial_contiguous_iterator_v<Iterator>)
            assign_trivial(std::move(first), std::move(last));
        else
            assign_with_sentinel(std::move(first), std::move(last));
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& assign(std::initializer_list<CharT> il) { return assign(il.begin(), il.size()); }

    // template <ContainerCompatibleRange<CharT> R>
    // constexpr basic_inplace_string& assign_range(R&& rg);            // C++23

    // unchecked_assign

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_assign(basic_inplace_string const& str) noexcept { return *this = str; }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_assign(StringLike const& str) noexcept
    {
        auto const str_view = self_view(str);
        return unchecked_assign(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_assign(CharT const* str, size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::unchecked_assign(ptr, n) detected nullptr");
        traits_type::move(data(), str, n);
        set_size_and_null_terminate(n);
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_assign(CharT const* str) noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::unchecked_assign(ptr) detected nullptr");
        return unchecked_assign(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_assign(size_type n, CharT c) noexcept
    {
        traits_type::assign(data(), n, c);
        set_size_and_null_terminate(n);
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_assign(std::initializer_list<CharT> il)
    {
        return unchecked_assign(il.begin(), il.size());
    }

    // try_assign

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_assign(basic_inplace_string const& str) noexcept { return std::addressof(*this = str); }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string* try_assign(StringLike const& str) noexcept
    {
        auto const str_view = self_view(str);
        return try_assign(str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_assign(CharT const* str, size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::try_assign(ptr, n) detected nullptr");
        if (n > capacity())
            return nullptr;
        traits_type::move(data(), str, n);
        set_size_and_null_terminate(n);
        return this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_assign(CharT const* str) noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::try_assign(ptr) detected nullptr");
        return try_assign(str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_assign(size_type n, CharT c) noexcept
    {
        if (n > capacity())
            return nullptr;
        traits_type::assign(data(), n, c);
        set_size_and_null_terminate(n);
        return this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_assign(std::initializer_list<CharT> il) { return try_assign(il.begin(), il.size()); }

    // insert

    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos1, basic_inplace_string const& str)
    {
        return insert(pos1, str.data(), str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos1, StringLike const& str)
    {
        auto const str_view = self_view(str);
        return insert(pos1, str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos1, basic_inplace_string const& str, size_type pos2, size_type n2 = npos)
    {
        size_type const str_sz = str.size();
        if (pos2 > str_sz)
            throw_out_of_range();
        return insert(pos1, str.data() + pos2, std::min(n2, str_sz - pos2));
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos1, StringLike const& str, size_type pos2, size_type n2 = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_sz = str_view.size();
        if (pos2 > str_sz)
            throw_out_of_range();
        return insert(pos1, str_view.data() + pos2, std::min(n2, str_sz - pos2));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos, CharT const* str, size_type n)
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::insert(pos, ptr, n) detected nullptr");
        size_type sz = size();

        if (pos > sz)
            throw_out_of_range();
        if (n > capacity() - sz)
            throw_length_error();

        if (n > 0)
        {
            pointer ptr = data();
            size_type n_move = sz - pos;
            if (n_move != 0)
            {
                if (intl::is_pointer_in_range(ptr + pos, ptr + sz, str))
                    str += n;
                traits_type::move(ptr + pos + n, ptr + pos, n_move);
            }
            traits_type::move(ptr + pos, str, n);
            sz += n;
            set_size_and_null_terminate(sz);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos, CharT const* str)
    {
        QX_ASSERT_CONTRACT(str != nullptr, "string::insert(pos, ptr) detected nullptr");
        return insert(pos, str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& insert(size_type pos, size_type n, CharT c)
    {
        size_type sz = size();
        if (pos > sz)
            throw_out_of_range();

        if (n > 0)
        {
            if (n > capacity() - sz)
                throw_length_error();

            pointer p = data();
            size_type n_move = sz - pos;
            if (n_move != 0)
                traits_type::move(p + pos + n, p + pos, n_move);

            traits_type::assign(p + pos, n, c);
            sz += n;
            set_size_and_null_terminate(sz);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 iterator insert(const_iterator pos, CharT c) { return insert(pos, 1, c); }

    QX_CONSTEXPR_CXX20 iterator insert(const_iterator pos, size_type n, CharT c)
    {
        difference_type diff = pos - begin();
        insert(static_cast<size_type>(diff), n, c);
        return begin() + diff;
    }

    template <class Iterator, std::enable_if_t<intl::is_iter_with_category_v<Iterator, std::input_iterator_tag>, int> = 0>
    QX_CONSTEXPR_CXX20 iterator insert(const_iterator pos, Iterator first, Iterator last)
    {
        if constexpr (intl::is_iter_with_category_v<Iterator, std::forward_iterator_tag>)
        {
            auto const n = static_cast<size_type>(std::distance(first, last));
            return insert_with_size(pos, first, last, n);
        }

        basic_inplace_string const tmp(first, last);
        return insert(pos, tmp.data(), tmp.data() + tmp.size());
    }

    QX_CONSTEXPR_CXX20 iterator insert(const_iterator pos, std::initializer_list<CharT> il) { return insert(pos, il.begin(), il.end()); }

    // template <ContainerCompatibleRange<CharT> R>
    // constexpr iterator insert_range(const_iterator p, R&& rg);            // C++23

    // unchecked_insert

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_insert(size_type pos1, basic_inplace_string const& str) noexcept
    {
        return unchecked_insert(pos1, str.data(), str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_insert(size_type pos1, StringLike const& str) noexcept
    {
        auto const str_view = self_view{str};
        return unchecked_insert(pos1, str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_insert(size_type pos, CharT const* str, size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::unchecked_insert(pos, ptr, n) detected nullptr");
        if (n > 0)
        {
            size_type sz = size();
            pointer ptr = data();
            size_type n_move = sz - pos;
            if (n_move != 0)
            {
                if (intl::is_pointer_in_range(ptr + pos, ptr + sz, str))
                    str += n;
                traits_type::move(ptr + pos + n, ptr + pos, n_move);
            }
            traits_type::move(ptr + pos, str, n);
            sz += n;
            set_size_and_null_terminate(sz);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_insert(size_type pos, CharT const* str) noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "string::unchecked_insert(pos, ptr) detected nullptr");
        return unchecked_insert(pos, str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& unchecked_insert(size_type pos, size_type n, CharT c) noexcept
    {
        if (n > 0)
        {
            size_type sz = size();
            pointer p = data();
            size_type n_move = sz - pos;
            if (n_move != 0)
                traits_type::move(p + pos + n, p + pos, n_move);
            traits_type::assign(p + pos, n, c);
            sz += n;
            set_size_and_null_terminate(sz);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 iterator unchecked_insert(const_iterator pos, CharT c) noexcept { return unchecked_insert(pos, 1, c); }

    QX_CONSTEXPR_CXX20 iterator unchecked_insert(const_iterator pos, size_type n, CharT c) noexcept
    {
        difference_type diff = pos - begin();
        unchecked_insert(static_cast<size_type>(diff), n, c);
        return begin() + diff;
    }

    QX_CONSTEXPR_CXX20 iterator unchecked_insert(const_iterator pos, std::initializer_list<CharT> il)
    {
        return unchecked_insert(pos, il.begin(), il.ize());
    }

    // try_insert

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_insert(size_type pos1, basic_inplace_string const& str) noexcept
    {
        return try_insert(pos1, str.data(), str.size());
    }

    template <class StringLike, enable_if_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string* try_insert(size_type pos1, StringLike const& str) noexcept
    {
        auto const str_view = self_view(str);
        return try_insert(pos1, str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_insert(size_type pos, CharT const* str, size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::try_insert(pos, ptr, n) detected nullptr");
        size_type sz = size();
        if (pos > sz)
            return nullptr;

        if (n > 0)
        {
            if (n > capacity() - sz)
                return nullptr;

            pointer ptr = data();
            size_type n_move = sz - pos;
            if (n_move != 0)
            {
                if (intl::is_pointer_in_range(ptr + pos, ptr + sz, str))
                    str += n;
                traits_type::move(ptr + pos + n, ptr + pos, n_move);
            }
            traits_type::move(ptr + pos, str, n);
            sz += n;
            set_size_and_null_terminate(sz);
        }
        return this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_insert(size_type pos, CharT const* str) noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "string::try_insert(pos, str) detected nullptr");
        return try_insert(pos, str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_insert(size_type pos, size_type n, CharT c) noexcept
    {
        size_type sz = size();
        if (pos > sz)
            return nullptr;

        if (n > 0)
        {
            if (n > capacity() - sz)
                return nullptr;

            pointer p = data();
            size_type n_move = sz - pos;
            if (n_move != 0)
                traits_type::move(p + pos + n, p + pos, n_move);

            traits_type::assign(p + pos, n, c);
            sz += n;
            set_size_and_null_terminate(sz);
        }
        return this;
    }

    QX_CONSTEXPR_CXX20 std::optional<iterator> try_insert(const_iterator pos, CharT c) noexcept { return try_insert(pos, 1, c); }

    QX_CONSTEXPR_CXX20 std::optional<iterator> try_insert(const_iterator pos, size_type n, CharT c) noexcept
    {
        difference_type diff = pos - begin();
        if (!try_insert(static_cast<size_type>(diff), n, c))
            return std::nullopt;
        return begin() + diff;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string* try_insert(const_iterator pos, std::initializer_list<CharT> il)
    {
        return try_insert(pos, il.begin(), il.size());
    }

    // erase

    QX_CONSTEXPR_CXX20 basic_inplace_string& erase(size_type pos = 0, size_type n = npos)
    {
        if (pos > size())
            throw_out_of_range();

        if (n == npos)
        {
            set_size_and_null_terminate(pos);
            return *this;
        }

        if (n > 0)
        {
            size_type const sz = size();
            pointer const ptr = data();
            n = std::min(n, sz - pos);
            size_type n_move = sz - pos - n;
            if (n_move != 0)
                traits_type::move(ptr + pos, ptr + pos + n, n_move);
            set_size_and_null_terminate(sz - n);
        }
        return *this;
    }

    QX_CONSTEXPR_CXX20 iterator erase(const_iterator pos)
    {
        QX_ASSERT_CONTRACT(pos != end(), "inplace_string::erase(iterator) called with a non-dereferenceable iterator");
        iterator const start = begin();
        auto const offset = static_cast<size_type>(pos - start);
        erase(offset, 1);
        return start + static_cast<difference_type>(offset);
    }

    QX_CONSTEXPR_CXX20 iterator erase(const_iterator first, const_iterator last)
    {
        QX_ASSERT_CONTRACT(first <= last, "inplace_string::erase(first, last) called with invalid range");
        iterator const start = begin();
        auto const offset = static_cast<size_type>(first - start);
        erase(offset, static_cast<size_type>(last - first));
        return start + static_cast<difference_type>(offset);
    }

    // replace

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(size_type pos1, size_type n1, basic_inplace_string const& str)
    {
        return replace(pos1, n1, str.data(), str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(size_type pos1, size_type n1, StringLike const& str)
    {
        auto const str_view = self_view(str);
        return replace(pos1, n1, str_view.data(), str_view.size());
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(
        size_type pos1, size_type n1, basic_inplace_string const& str, size_type pos2, size_type n2 = npos)
    {
        size_type const str_sz = str.size();
        if (pos2 > str_sz)
            throw_out_of_range();
        return replace(pos1, n1, str.data() + pos2, std::min(n2, str_sz - pos2));
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(
        size_type pos1, size_type n1, StringLike const& str, size_type pos2, size_type n2 = npos)
    {
        auto const str_view = self_view(str);
        size_type const str_sz = str_view.size();
        if (pos2 > str_sz)
            throw_out_of_range();
        return replace(pos1, n1, str_view.data() + pos2, std::min(n2, str_sz - pos2));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(size_type pos, size_type n1, CharT const* str, size_type n2)
    {
        QX_ASSERT_CONTRACT(n2 == 0 || str != nullptr, "inplace_string::replace(pos, n1, ptr, n2) detected nullptr");

        size_type const sz = size();
        if (pos > sz)
            throw_out_of_range();

        n1 = std::min(n1, sz - pos);
        size_type const new_size = sz - n1 + n2;
        if (new_size > capacity())
            throw_length_error();

        pointer const ptr = data();
        pointer const dst = ptr + pos;
        if (n2 == n1)
        {
            traits_type::move(dst, str, n2);
        }
        else if (n2 < n1)
        {
            if (n2 > 0)
                traits_type::move(dst, str, n2);

            size_type const n_move = sz - pos - n1;
            if (n_move != 0)
                traits_type::move(dst + n2, dst + n1, n_move);
        }
        else
        {
            size_type const n_move = sz - pos - n1;
            if (n_move != 0)
                traits_type::move(dst + n2, dst + n1, n_move);

            if (n2 > 0)
                traits_type::move(dst, str, n2);
        }

        set_size_and_null_terminate(new_size);
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(size_type pos, size_type n1, CharT const* str)
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::replace(pos, n1, ptr) detected nullptr");
        return replace(pos, n1, str, traits_type::length(str));
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(size_type pos, size_type n1, size_type n2, CharT c)
    {
        size_type const sz = size();
        if (pos > sz)
            throw_out_of_range();

        n1 = std::min(n1, sz - pos);
        size_type const new_size = sz - n1 + n2;
        if (new_size > capacity())
            throw_length_error();

        pointer ptr = data();
        if (n1 != n2)
        {
            size_type n_move = sz - pos - n1;
            if (n_move != 0)
                traits_type::move(ptr + pos + n2, ptr + pos + n1, n_move);
        }

        traits_type::assign(ptr + pos, n2, c);
        set_size_and_null_terminate(sz - (n1 - n2));
        return *this;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator it1, const_iterator it2, basic_inplace_string const& str)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), str.data(), str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator it1, const_iterator it2, StringLike const& str)
    {
        auto const str_view = self_view(str);
        return replace(it1 - begin(), static_cast<size_type>(it2 - it1), str_view);
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator it1, const_iterator it2, CharT const* str, size_type n)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), str, n);
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator it1, const_iterator it2, CharT const* str)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), str);
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator it1, const_iterator it2, size_type n, CharT c)
    {
        return replace(static_cast<size_type>(it1 - begin()), static_cast<size_type>(it2 - it1), n, c);
    }

    template <class Iterator>
    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator i1, const_iterator i2, Iterator j1, Iterator j2)
    {
        basic_inplace_string const tmp(j1, j2);
        return replace(i1, i2, tmp);
    }

    // template <ContainerCompatibleRange<CharT> R>
    // constexpr basic_inplace_string& replace_with_range(const_iterator i1, const_iterator i2, R&& rg); // C++23

    QX_CONSTEXPR_CXX20 basic_inplace_string& replace(const_iterator it1, const_iterator it2, std::initializer_list<CharT> il)
    {
        return replace(it1, it2, il.begin(), il.end());
    }

    QX_CONSTEXPR_CXX20 size_type copy(CharT* str, size_type n, size_type pos = 0) const
    {
        size_type const sz = size();
        if (pos > sz)
            throw_out_of_range();
        size_type const rlen = std::min(n, sz - pos);
        traits_type::copy(str, data() + pos, rlen);
        return rlen;
    }

    QX_CONSTEXPR_CXX20 basic_inplace_string substr(size_type pos = 0, size_type n = npos) const
    {
        return basic_inplace_string(*this, pos, n);
    }

    template <size_type Pos = 0, size_type Count = npos>
    QX_CONSTEXPR_CXX20 auto substr() const noexcept -> basic_inplace_string<Count == npos ? (N - Pos) : Count, CharT, Traits>
    {
        static_assert(Pos <= N, "substr position out of range");
        constexpr size_type kSubCapacity = Count == npos ? (N - Pos) : Count;
        basic_inplace_string<kSubCapacity, value_type, traits_type> res;

        size_type const sz = size();
        if (Pos < sz)
        {
            size_type const n_copy = Count == npos ? sz - Pos : std::min(sz - Pos, Count);
            res.unchecked_assign(data() + Pos, n_copy);
        }
        return res;
    }

    QX_CONSTEXPR_CXX20 void swap(basic_inplace_string& other) noexcept
    {
        std::swap_ranges(rep_.data, rep_.data + N + 1, other.rep_.data);
        std::swap(rep_.size, other.rep_.size);
    }

    // c_str, data

    constexpr CharT const* c_str() const noexcept { return data(); }
    constexpr CharT const* data() const noexcept { return rep_.data; }
    constexpr CharT* data() noexcept { return rep_.data; }

    // find

    constexpr size_type find(basic_inplace_string const& str, size_type pos = 0) const noexcept
    {
        return find(str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr size_type find(StringLike const& str, size_type pos = 0) const noexcept
    {
        auto const str_view = self_view(str);
        return find(str_view.data(), pos, str_view.size());
    }

    constexpr size_type find(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find(ptr, pos, n) detected nullptr");

        const_pointer const ptr = data();
        size_type const sz = size();
        if (pos > sz)
            return npos;
        if (n == 0)
            return pos;

        // ReSharper disable once CppDFANullDereference
        const_pointer result = search_substring(ptr + pos, ptr + sz, str, str + n);
        if (result == ptr + sz)
            return npos;

        return static_cast<size_type>(result - ptr);
    }

    constexpr size_type find(CharT const* str, size_type pos = 0) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find(ptr, pos) detected nullptr");
        return find(str, pos, traits_type::length(str));
    }

    constexpr size_type find(CharT c, size_type pos = 0) const noexcept
    {
        const_pointer const ptr = data();
        size_type const sz = size();
        if (pos > sz)
            return npos;

        CharT const* r = traits_type::find(ptr + pos, sz - pos, c);
        if (r == nullptr)
            return npos;

        return static_cast<size_type>(r - ptr);
    }

    // rfind

    constexpr size_type rfind(basic_inplace_string const& str, size_type pos = npos) const noexcept
    {
        return rfind(str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr size_type rfind(StringLike const& str, size_type pos = npos) const noexcept
    {
        auto const str_view = self_view(str);
        return rfind(str_view.data(), pos, str_view.size());
    }

    constexpr size_type rfind(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::rfind(ptr, pos, n) detected nullptr");

        const_pointer const ptr = data();
        size_type const sz = size();

        pos = std::min(pos, sz);
        if (n < sz - pos)
            pos += n;
        else
            pos = sz;

        const_pointer const r = std::find_end(ptr, ptr + pos, str, str + n, traits_type::eq);
        if (n > 0 && r == ptr + pos)
            return npos;

        return static_cast<size_type>(r - ptr);
    }

    constexpr size_type rfind(CharT const* str, size_type pos = npos) const noexcept { return rfind(str, pos, traits_type::length(str)); }

    constexpr size_type rfind(CharT c, size_type pos = npos) const noexcept
    {
        const_pointer const ptr = data();
        size_type const sz = size();
        if (sz < 1)
            return npos;
        if (pos < sz)
            ++pos;
        else
            pos = sz;

        for (const_pointer ps = ptr + pos; ps != ptr;)
        {
            if (traits_type::eq(*--ps, c))
                return static_cast<size_type>(ps - ptr);
        }

        return npos;
    }

    // find_first_of

    constexpr size_type find_first_of(basic_inplace_string const& str, size_type pos = 0) const noexcept
    {
        return find_first_of(str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr size_type find_first_of(StringLike const& str, size_type pos = 0) const noexcept
    {
        auto const str_view = self_view(str);
        return find_first_of(str_view.data(), pos, str_view.size());
    }

    constexpr size_type find_first_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_first_of(ptr, pos, n) detected nullptr");
        const_pointer const ptr = data();
        size_type const sz = size();
        if (pos >= sz || n == 0)
            return npos;

        // ReSharper disable once CppDFANullDereference
        const_pointer r = find_first_of_ce(ptr + pos, ptr + sz, str, str + n);
        if (r == ptr + sz)
            return npos;
        return static_cast<size_type>(r - ptr);
    }

    constexpr size_type find_first_of(CharT const* str, size_type pos = 0) const noexcept
    {
        return find_first_of(str, pos, traits_type::length(str));
    }

    constexpr size_type find_first_of(CharT c, size_type pos = 0) const noexcept { return find(c, pos); }

    // find_last_of

    constexpr size_type find_last_of(basic_inplace_string const& str, size_type pos = npos) const noexcept
    {
        return find_last_of(str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr size_type find_last_of(StringLike const& str, size_type pos = npos) const noexcept
    {
        auto const str_view = self_view(str);
        return find_last_of(str_view.data(), pos, str_view.size());
    }

    constexpr size_type find_last_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_last_of(ptr, pos, n) detected nullptr");
        if (n > 0)
        {
            const_pointer const ptr = data();
            size_type const sz = size();
            if (pos < sz)
                ++pos;
            else
                pos = sz;

            for (const_pointer ps = ptr + pos; ps != ptr;)
            {
                if (traits_type::find(str, n, *--ps))
                    return static_cast<size_type>(ps - ptr);
            }
        }
        return npos;
    }

    constexpr size_type find_last_of(CharT const* str, size_type pos = npos) const noexcept
    {
        return find_last_of(str, pos, traits_type::length(str));
    }

    constexpr size_type find_last_of(CharT c, size_type pos = npos) const noexcept { return rfind(c, pos); }

    // find_first_not_of

    constexpr size_type find_first_not_of(basic_inplace_string const& str, size_type pos = 0) const noexcept
    {
        return find_first_not_of(str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr size_type find_first_not_of(StringLike const& str, size_type pos = 0) const noexcept
    {
        auto const str_view = self_view(str);
        return find_first_not_of(str_view.data(), pos, str_view.size());
    }

    constexpr size_type find_first_not_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_first_not_of(ptr, pos, n) detected nullptr");
        const_pointer const ptr = data();
        size_type const sz = size();
        if (pos < sz)
        {
            const_pointer const end = ptr + sz;
            for (const_pointer start = ptr + pos; start != end; ++start)
            {
                if (traits_type::find(str, n, *start) == nullptr)
                    return static_cast<size_type>(start - ptr);
            }
        }
        return npos;
    }

    constexpr size_type find_first_not_of(CharT const* str, size_type pos = 0) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find_first_not_of(ptr, pos, n) detected nullptr");
        return find_first_not_of(str, pos, traits_type::length(str));
    }

    constexpr size_type find_first_not_of(CharT c, size_type pos = 0) const noexcept
    {
        const_pointer const ptr = data();
        size_type const sz = size();
        if (pos < sz)
        {
            value_type const* pe = ptr + sz;
            for (value_type const* ps = ptr + pos; ps != pe; ++ps)
            {
                if (!traits_type::eq(*ps, c))
                    return static_cast<size_type>(ps - ptr);
            }
        }
        return npos;
    }

    // find_last_not_of

    constexpr size_type find_last_not_of(basic_inplace_string const& str, size_type pos = npos) const noexcept
    {
        return find_last_not_of(str.data(), pos, str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr size_type find_last_not_of(StringLike const& str, size_type pos = npos) const noexcept
    {
        auto const str_view = self_view(str);
        return find_last_not_of(str_view.data(), pos, str_view.size());
    }

    constexpr size_type find_last_not_of(CharT const* str, size_type pos, size_type n) const noexcept
    {
        QX_ASSERT_CONTRACT(n == 0 || str != nullptr, "inplace_string::find_last_not_of(ptr, pos, n) detected nullptr");
        const_pointer const ptr = data();
        size_type const sz = size();
        pos = std::min(pos, sz);
        if (pos < sz)
            ++pos;
        else
            pos = sz;

        for (value_type const* ps = ptr + pos; ps != ptr;)
        {
            if (traits_type::find(str, n, *--ps) == nullptr)
                return static_cast<size_type>(ps - ptr);
        }
        return npos;
    }

    constexpr size_type find_last_not_of(CharT const* str, size_type pos = npos) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::find_last_not_of(ptr, pos, n) detected nullptr");
        return find_last_not_of(str, pos, traits_type::length(str));
    }

    constexpr size_type find_last_not_of(CharT c, size_type pos = npos) const noexcept
    {
        const_pointer const ptr = data();
        size_type const sz = size();
        if (pos < sz)
            ++pos;
        else
            pos = sz;

        for (value_type const* ps = ptr + pos; ps != ptr;)
        {
            if (!traits_type::eq(*--ps, c))
                return static_cast<size_type>(ps - ptr);
        }
        return npos;
    }

    // compare

    constexpr int compare(basic_inplace_string const& str) const noexcept { return compare(self_view(str)); }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr int compare(StringLike const& str) const noexcept
    {
        auto const str_view = self_view(str);
        size_type const lhs_sz = size();
        size_type const rhs_sz = str_view.size();
        int const result = traits_type::compare(data(), str_view.data(), std::min(lhs_sz, rhs_sz));
        if (result != 0)
            return result;
        if (lhs_sz < rhs_sz)
            return -1;
        if (lhs_sz > rhs_sz)
            return 1;
        return 0;
    }

    constexpr int compare(size_type pos1, size_type n1, basic_inplace_string const& str) const
    {
        return compare(pos1, n1, str.data(), str.size());
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr int compare(size_type pos1, size_type n1, StringLike const& str) const
    {
        auto const str_view = self_view(str);
        return compare(pos1, n1, str_view.data(), str_view.size());
    }

    constexpr int compare(size_type pos1, size_type n1, basic_inplace_string const& str, size_type pos2, size_type n2 = npos) const
    {
        return compare(pos1, n1, self_view(str), pos2, n2);
    }

    template <class StringLike, enable_if_unsame_string_like_t<StringLike> = 0>
    constexpr int compare(size_type pos1, size_type n1, StringLike const& str, size_type pos2, size_type n2 = npos) const
    {
        auto const str_view = self_view(str);
        return self_view(*this).substr(pos1, n1).compare(str_view.substr(pos2, n2));
    }

    constexpr int compare(CharT const* str) const noexcept
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::compare(ptr) detected nullptr");
        return compare(0, npos, str, traits_type::length(str));
    }

    constexpr int compare(size_type pos1, size_type n1, CharT const* str) const
    {
        QX_ASSERT_CONTRACT(str != nullptr, "inplace_string::compare(pos1, n1, ptr) detected nullptr");
        return compare(pos1, n1, str, traits_type::length(str));
    }

    constexpr int compare(size_type pos1, size_type n1, CharT const* str, size_type n2) const
    {
        QX_ASSERT_CONTRACT(n2 == 0 || str != nullptr, "inplace_string::compare(pos1, n1, ptr, n2) detected nullptr");
        size_type const sz = size();

        if (pos1 > sz)
            throw_out_of_range();

        size_type const rlen = std::min(n1, sz - pos1);

        int r = traits_type::compare(data() + pos1, str, std::min(rlen, n2));
        if (r == 0)
        {
            if (rlen < n2)
                r = -1;
            else if (rlen > n2)
                r = 1;
        }
        return r;
    }

#if __cplusplus >= 202002L

    // starts_with

    constexpr bool starts_with(std::basic_string_view<CharT, Traits> sv) const noexcept
    {
        return self_view(data(), size()).starts_with(sv);
    }

    constexpr bool starts_with(CharT c) const noexcept { return !empty() && traits_type::eq(front(), c); }

    constexpr bool starts_with(CharT const* str) const { return starts_with(self_view(str)); }

    // ends_with

    constexpr bool ends_with(std::basic_string_view<CharT, Traits> sv) const noexcept { return self_view(data(), size()).ends_with(sv); }

    constexpr bool ends_with(CharT c) const noexcept { return !empty() && traits_type::eq(back(), c); }

    constexpr bool ends_with(CharT const* str) const { return ends_with(self_view(str)); }

#endif

#if __cplusplus > 202302L

    // contains

    constexpr bool contains(std::basic_string_view<CharT, Traits> sv) const noexcept { return self_view(data(), size()).contains(sv); }

    constexpr bool contains(CharT c) const noexcept { return self_view(data(), size()).contains(c); }

    constexpr bool contains(CharT const* str) const { return self_view(data(), size()).contains(str); }

#endif

private:
    // The actual size type used for storing the size of the string. It is chosen based on the maximum size of the
    // string (N) to save space. It is guaranteed to be large enough to store any size up to N, and it is an unsigned
    // integer type for simplicity of implementation.
    template <class SizeT, class ChT, std::size_t M>
    struct alignas(size_type) inplace_string_storage
    {
        union // NOLINT(*-non-private-member-variables-in-classes)
        {
            SizeT size{};
            ChT pad; //< ensures similar layout to libc++ short representation
        };
        ChT data[M + 1]; // NOLINT(*-avoid-c-arrays, *-non-private-member-variables-in-classes)

        constexpr inplace_string_storage() noexcept { data[0] = ChT{}; } // NOTE: to avoid full buffer init
    };

    // internal type used to store the size information, automatically changes between capacities
    using compressed_size_type = intl::min_size_t<N>;

    // inplace_string representation
    inplace_string_storage<compressed_size_type, CharT, N> rep_{};

    // exception handling

    [[noreturn]] static QX_COLD_NOINLINE void throw_out_of_range() { throw std::out_of_range{"basic_inplace_string"}; }

    [[noreturn]] static QX_COLD_NOINLINE void throw_length_error() { throw std::length_error{"basic_inplace_string"}; }

    // size and null termination as single operation

    QX_CONSTEXPR_CXX20 void set_size_and_null_terminate(size_type n) noexcept
    {
        QX_ASSERT_CONTRACT(
            n <= std::numeric_limits<compressed_size_type>::max(), "inplace_string::set_size_and_null_terminate(n) size overflow");
        traits_type::assign(rep_.data[n], value_type{});
        rep_.size = static_cast<compressed_size_type>(n);
    }

    QX_CONSTEXPR_CXX20 void init(value_type const* str, size_type n)
    {
        if (n > max_size())
            throw_length_error();
        traits_type::copy(data(), str, n);
        set_size_and_null_terminate(n);
    }

    QX_CONSTEXPR_CXX20 void init(size_type n, value_type c)
    {
        if (n > max_size())
            throw_length_error();
        traits_type::assign(data(), n, c);
        set_size_and_null_terminate(n);
    }

    template <class Iterator>
    QX_CONSTEXPR_CXX20 void init(Iterator first, Iterator last)
    {
        if constexpr (intl::is_iter_with_category_v<Iterator, std::forward_iterator_tag>)
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
    QX_CONSTEXPR_CXX20 void assign_with_sentinel(Iterator first, Sentinel last)
    {
        basic_inplace_string tmp;
        tmp.init_with_sentinel(std::move(first), std::move(last));
        assign(tmp.data(), tmp.size());
    }

    template <class Iterator, class Sentinel>
    QX_CONSTEXPR_CXX20 void assign_trivial(Iterator first, Sentinel last)
    {
        QX_ASSERT_CONTRACT(intl::is_trivial_contiguous_iterator_v<Iterator>,
            "inplace_string::assign_trivial(first, last): the iterator type must be trivial");

        auto const n = static_cast<size_type>(std::distance(first, last));
        if (n > capacity())
            throw_length_error();

        const_pointer const src = intl::to_address(first);
        pointer const ptr = data();

        if (intl::is_overlapping_range(ptr, ptr + n, src))
            traits_type::move(ptr, src, n);
        else
            traits_type::copy(ptr, src, n);

        set_size_and_null_terminate(n);
    }

    template <class Iterator, class Sentinel>
    QX_CONSTEXPR_CXX20 void init_with_sentinel(Iterator first, Sentinel last)
    {
        // strong exception guarantee: the string is left in a valid empty state if throws during initialization
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

    template <class Iterator, class Sentinel>
    QX_CONSTEXPR_CXX20 void init_with_size(Iterator first, Sentinel last, size_type sz)
    {
        if (sz > max_size())
            throw_length_error();

        // strong exception guarantee: the string is left in a valid empty state if throws during initialization
        try
        {
            pointer const begin = data();
            pointer const end = copy_non_overlapping_range(std::move(first), std::move(last), begin);
            set_size_and_null_terminate(end - begin);
        }
        catch (...)
        {
            set_size_and_null_terminate(0);
            throw;
        }
    }

    template <class Iterator, class Sentinel>
    QX_CONSTEXPR_CXX20 iterator insert_with_size(const_iterator pos, Iterator first, Sentinel last, size_type n)
    {
        auto const ip = static_cast<size_type>(pos - begin());
        if (n == 0)
            return begin() + ip;

        if (intl::is_trivial_contiguous_iterator_v<Iterator> && !address_in_range(*first))
        {
            return insert_from_safe_copy(n, ip, std::move(first), std::move(last));
        }

        basic_inplace_string tmp;
        tmp.init_with_sentinel(std::move(first), std::move(last));
        return insert_from_safe_copy(n, ip, tmp.begin(), tmp.end());
    }

    template <class ForwardIterator, class Sentinel>
    QX_CONSTEXPR_CXX20 iterator insert_from_safe_copy(size_type n, size_type ip, ForwardIterator first, Sentinel last)
    {
        size_type sz = size();
        if (n > capacity() - sz)
            throw_length_error();
        pointer ptr = data();

        size_type n_move = sz - ip;
        if (n_move != 0)
            traits_type::move(ptr + ip + n, ptr + ip, n_move);

        sz += n;
        set_size_and_null_terminate(sz);
        copy_non_overlapping_range(std::move(first), std::move(last), ptr + ip);

        return begin() + ip;
    }

    QX_CONSTEXPR_CXX20 void erase_to_end(size_type pos)
    {
        QX_ASSERT_CONTRACT(pos <= capacity(), "inplace_string::erase_to_end(pos) trying to erase positions outside the capacity");
        set_size_and_null_terminate(pos);
    }

    template <class T>
    constexpr bool address_in_range(T const& c) const
    {
        return intl::is_pointer_in_range(data(), data() + size() + 1, std::addressof(c));
    }

    template <class Iterator, class Sentinel>
    static QX_CONSTEXPR_CXX20 pointer copy_non_overlapping_range(Iterator first, Sentinel last, pointer dest)
    {
        if constexpr (intl::is_contiguous_iterator_v<Iterator> && std::is_same_v<value_type, intl::iter_value_t<Iterator>> &&
            std::is_same_v<Iterator, Sentinel>)
        {
            const_pointer const first_addr = intl::to_address(std::move(first));
            const_pointer const last_addr = intl::to_address(std::move(last));
            QX_ASSERT_CONTRACT(!intl::is_overlapping_range(first_addr, last_addr, dest),
                "inplace_string::copy_non_overlapping_range(first, last, dest) called with an overlapping range!");
            auto const n_copy = last_addr - first_addr;
            traits_type::copy(dest, first_addr, n_copy);
            return dest + n_copy;
        }

        for (; first != last; ++first)
            traits_type::assign(*dest++, *first);
        return dest;
    }

    static constexpr const_pointer find_first_of_ce(const_pointer first1, const_pointer last1, const_pointer first2, const_pointer last2)
    {
        for (; first1 != last1; ++first1)
        {
            for (const_pointer j = first2; j != last2; ++j)
            {
                if (traits_type::eq(*first1, *j))
                    return first1;
            }
        }
        return last1;
    }

    static constexpr value_type const* search_substring(
        value_type const* first1, value_type const* last1, value_type const* first2, value_type const* last2) noexcept
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

            first1 = traits_type::find(first1, len1 - len2 + 1, f2);
            if (first1 == nullptr)
                return last1;

            if (traits_type::compare(first1, first2, len2) == 0)
                return first1;

            ++first1;
        }
    }

    template <std::size_t M, class T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
    friend inplace_string<M> to_inplace_string(T);

    template <std::size_t M, class T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
    friend std::optional<inplace_string<M>> try_to_inplace_string(T) noexcept;

    template <std::size_t M, class T, std::enable_if_t<std::is_arithmetic_v<T>, int>>
    friend inplace_string<M> unchecked_to_inplace_string(T) noexcept;
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
    QX_ASSERT_CONTRACT(rhs != nullptr, "inplace_string::operator==(inplace_string, char*) detected nullptr");

    std::size_t const rhs_len = Traits::length(rhs);
    if (rhs_len != lhs.size())
        return false;

    return lhs.compare(0, basic_inplace_string<N, CharT, Traits>::npos, rhs, rhs_len) == 0;
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
 * NOTE: operator+ is intentionally omitted for basic_inplace_string<N, CharT,
 * Traits>.
 *
 * Rationale:
 *  - Memory Safety: Unlike std::string, inplace_string<N> does not have a
 *    dynamic allocator. Implementing operator+ would require an arbitrary
 *    choice of result capacity (e.g., N, M, or N+M).
 *
 *  - Stack Protection: Returning inplace_string<N+M> can lead to "stack
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

template <std::size_t N, class CharT, class Traits>
inline QX_CONSTEXPR_CXX20 void swap(basic_inplace_string<N, CharT, Traits>& lhs, basic_inplace_string<N, CharT, Traits>& rhs) noexcept
{
    lhs.swap(rhs);
}

// int stoi(string const& __str, size_t* __idx = nullptr, int __base = 10);
// long stol(string const& __str, size_t* __idx = nullptr, int __base = 10);
// unsigned long stoul(string const& __str, size_t* __idx = nullptr, int __base
// = 10); long long stoll(string const& __str, size_t* __idx = nullptr, int
// __base = 10); unsigned long long stoull(string const& __str, size_t* __idx =
// nullptr, int __base = 10);

// float stof(string const& __str, size_t* __idx = nullptr);
// double stod(string const& __str, size_t* __idx = nullptr);
// long double stold(string const& __str, size_t* __idx = nullptr);

// Constexpr-friendly integer log10 ceil (replaces the broken recursive lambda)

template <std::size_t N, class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
inplace_string<N> unchecked_to_inplace_string(T val) noexcept
{
    inplace_string<N> res;
    auto const begin = res.data();
    auto const [end, ec] = std::to_chars(begin, begin + N, val);
    QX_ASSERT_CONTRACT(ec == std::errc{}, "unchecked_to_inplace_string(val): value exceeds buffer capacity");
    res.set_size_and_null_terminate(static_cast<std::size_t>(end - begin));
    return res;
}

template <std::size_t N, class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
std::optional<inplace_string<N>> try_to_inplace_string(T val) noexcept
{
    inplace_string<N> res;
    auto const begin = res.data();
    auto const [end, ec] = std::to_chars(begin, begin + N, val);
    if (ec != std::errc())
        return std::nullopt;
    res.set_size_and_null_terminate(static_cast<std::size_t>(end - begin));
    return res;
}

template <std::size_t N, class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
inplace_string<N> to_inplace_string(T val)
{
    inplace_string<N> res;
    auto const begin = res.data();
    auto const [end, ec] = std::to_chars(begin, begin + N, val);
    if (ec != std::errc())
        inplace_string<N>::throw_length_error();
    res.set_size_and_null_terminate(static_cast<std::size_t>(end - begin));
    return res;
}

template <class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
auto to_inplace_string(T val) noexcept
{
    static constexpr std::size_t kRequiredN = std::is_integral_v<T>
        ? 2 + std::numeric_limits<T>::digits10 // integral types
        : 4 + std::numeric_limits<T>::max_digits10 + std::max(2, intl::max_exponent_digits10_v<T>);

    using SizeT = intl::min_size_t<kRequiredN>;
    using ReqStrT = inplace_string<kRequiredN>;
    static constexpr std::size_t kSizeSize = std::max(sizeof(SizeT), sizeof(char));
    static constexpr std::size_t kOptimalN = ((sizeof(ReqStrT) - kSizeSize) / sizeof(char)) - 1;

    return unchecked_to_inplace_string<kOptimalN>(val);
}

} // namespace qx

namespace std
{

template <std::size_t N>
struct hash<qx::basic_inplace_string<N, char>> : hash<basic_string_view<char>>
{};
#if QX_HAS_CHAR8_T
template <std::size_t N>
struct hash<qx::basic_inplace_string<N, char8_t>> : hash<basic_string_view<char8_t>>
{};
#endif
template <std::size_t N>
struct hash<qx::basic_inplace_string<N, char16_t>> : hash<basic_string_view<char16_t>>
{};
template <std::size_t N>
struct hash<qx::basic_inplace_string<N, char32_t>> : hash<basic_string_view<char32_t>>
{};
template <std::size_t N>
struct hash<qx::basic_inplace_string<N, wchar_t>> : hash<basic_string_view<wchar_t>>
{};

template <class CharT, class Traits, std::size_t N>
basic_ostream<CharT, Traits>& operator<<(basic_ostream<CharT, Traits>& os, qx::basic_inplace_string<N, CharT, Traits> const& str)
{
    return os << std::basic_string_view<CharT, Traits>(str);
}

template <class CharT, class Traits, std::size_t N>
basic_istream<CharT, Traits>& operator>>(basic_istream<CharT, Traits>& is, qx::basic_inplace_string<N, CharT, Traits>& str)
{
    ios_base::iostate state = ios_base::goodbit;
    typename basic_istream<CharT, Traits>::sentry sentry(is);
    if (sentry)
    {
        streamsize const width = is.width();
        is.width(0); // reset unconditionally, regardless of what follows

        try
        {
            str.clear();
            auto const& ct = use_facet<ctype<CharT>>(is.getloc());
            auto* buf = is.rdbuf();
            while (width <= 0 || static_cast<streamsize>(str.size()) < width)
            {
                auto const c = buf->sgetc();
                if (Traits::eq_int_type(c, Traits::eof()))
                {
                    state |= ios_base::eofbit;
                    break;
                }

                CharT const ch = Traits::to_char_type(c);
                if (ct.is(ctype_base::space, ch))
                    break;

                str.push_back(ch); // throws length_error if the word needs more than N chars
                buf->sbumpc();
            }

            if (str.empty())
                state |= ios_base::failbit;
        }
        catch (...)
        {
            state |= ios_base::badbit;
            auto const mask = is.exceptions();
            try
            {
                is.exceptions(ios_base::goodbit);
                is.setstate(state);
                is.exceptions(mask);
            }
            catch (...)
            {}

            if (mask & ios_base::badbit)
                throw; // still resolves to the ORIGINAL exception, not the swallowed one above
        }

        is.setstate(state);
    }
    else
    {
        is.setstate(ios_base::failbit);
    }
    return is;
}

} // namespace std