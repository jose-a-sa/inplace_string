# inplace_string

`qx::inplace_string` is a fixed-capacity, stack-backed string type that offers a `std::string`-like API without requiring dynamic allocation.

It is designed for code that needs predictable storage, bounded stack usage, and explicit handling of overflow.

## Why use it?

`qx::inplace_string` is appropriate when the dominant requirement is bounded storage and predictable behavior rather than dynamic growth.

Use it when you need:

- a compile-time maximum size
- no heap allocation
- predictable stack usage and object size
- a `std::string`-like interface for common operations
- explicit handling of capacity overflow

## Motivation and design philosophy

The design objective is the same as for `qx::inplace_vector`: provide a fixed-capacity, allocation-free container for code that values predictable memory behavior over dynamic growth.

`qx::inplace_string` applies that model to character sequences:

- capacity is part of the type
- storage is local and bounded
- overflow is explicit rather than implicit
- the interface remains ergonomic for common use cases

## Support and compatibility

The library is designed around the ISO C++20 model as the primary modern baseline. It also preserves compatibility with C++17 mode for broader adoption and portability.

The repository’s test coverage reflects that support across the main compiler toolchains and STL implementations:

- Ubuntu: Clang/LLVM 14, 18, 20 with libc++/libstdc++, GCC 11, 14 with libstdc++
- macOS: Apple Clang 15, 17, 21 with libc++
- Windows: MSVC 14.2, 14.4, and 14.5

In practice, this means the implementation aims to be both modern and portable, while still remaining usable in C++17 mode. It also includes explicit detection of contiguous iterators so that range-based append and insert operations can take the most efficient path on the tested standard library implementations.

## Basic usage

```cpp
#include <qx/inplace_string.h>

qx::inplace_string<32> text = "hello";
text.append(" world");
text += "!";

auto value = qx::to_inplace_string<8>(12345);
```

## API overview

### Support level compared to `std::string`

`qx::inplace_string` is intended to provide the same API surface as `std::string` for the common case, but any operation that would exceed the fixed capacity is handled explicitly. In the throwing interface, such operations raise `std::length_error("qx::inplace_string")`.

### Intentional differences

Compared with `std::string`, `qx::inplace_string` differs in a few important ways:

- fixed capacity: the maximum size is known at compile time
- no dynamic growth: operations that would exceed capacity are handled explicitly
- no allocator model: there is no heap growth or allocator customization
- no general-purpose `std::string` feature set: it targets the common core rather than every corner case
- explicit overflow behavior: the API exposes throwing, checked, and unchecked variants where appropriate
- `operator+` is intentionally omitted because its result capacity would be ambiguous and potentially undesirable in a bounded container

### Checked and unchecked mutation APIs

Beyond the usual string operations, the library also provides checked variants for mutating operations that may exceed the fixed capacity. These are the intended counterparts to the standard mutators when the caller wants to avoid silent overflow:

```cpp
qx::inplace_string<16> s = "abc";

s.try_append("def");      // returns `this` on success, nullptr on failure
s.try_insert(3, "xyz");
s.try_assign("hello");
s.try_push_back('!');
```

The pattern is the same throughout the API:

- the regular mutating functions perform the operation directly and rely on the caller to ensure the requested growth fits
- the `try_*` variants report whether the operation could be completed without exceeding capacity
- the `unchecked_*` variants are for trusted contexts where the caller already knows the target buffer is large enough and wants to avoid the extra checking cost

In the current header, the checked and unchecked mutation family covers append, assign, insert, and push-back operations. There are no `try_replace` or `unchecked_replace` overloads in the present implementation.

## Conversion helpers

A particularly useful part of the API is the family of numeric conversion helpers:

```cpp
auto s1 = qx::to_inplace_string(42);
auto s2 = qx::to_inplace_string<16>(123456);
auto s3 = qx::try_to_inplace_string<4>(1000);      // returns std::nullopt if it does not fit
auto s4 = qx::unchecked_to_inplace_string<8>(42); // assumes capacity is sufficient
```

These helpers are intentionally differentiated:

- `to_inplace_string<Capacity>(value)` throws `std::length_error` if the formatted value does not fit in the requested capacity
- `try_to_inplace_string<Capacity>(value)` returns `std::optional<inplace_string<Capacity>>` and is the safe choice when the caller wants to test fit without throwing
- `unchecked_to_inplace_string<Capacity>(value)` is for trusted contexts where the caller already knows the buffer is large enough; it avoids the overhead of checking and is useful in performance-sensitive code paths
