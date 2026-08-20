# `qx::inplace_string`

A fixed-capacity, stack-backed string for modern C++ with zero dynamic memory allocation and a `std::string`-compatible interface.

## Compatibility

* **Language Standard:** C++20 (fully supported with C++17 backwards compatibility).
  * *Note:* `starts_with`/`ends_with` require C++20; `contains` requires C++23. They conditionally compile depending on the active standard.
* **Compilers Tested:**
  * Clang 14, 18, 20
  * GCC 11, 14
  * Apple Clang 15, 17, 21
  * MSVC 14.2 (VS 2019), 14.4 (VS 2022), 14.5

## Integration

This library is header-only and can be copied directly into your project or fetched via CMake.

### CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  qx_inplace_string
  GIT_REPOSITORY https://github.com/jose-a-sa/inplace_string.git
  GIT_TAG        v1.0.1
)
FetchContent_MakeAvailable(qx_inplace_string)

target_link_libraries(your_target PRIVATE qx::inplace_string)
```

## Quick Start

```cpp
#include <qx/inplace_string.h>
#include <iostream>

int main() {
    // Basic usage
    qx::inplace_string<32> text = "Hello";
    text.append(" World");
    text += "!";

    // Number conversion
    auto num_str = qx::to_inplace_string<8>(12345);

    // Substring with exact compile-time capacity sizing
    auto sub = text.substr<0, 5>(); // Returns inplace_string<5> ("Hello")

    std::cout << text << " | Sub: " << sub << '\n';
}
```

## Configuration Macros

`qx::inplace_string` offers compile-time toggles to tune safety, binary footprint, and copy semantics.

### Trivial Copyability Threshold (`QX_INPLACE_STRING_TRIVIAL_COPY_THRESHOLD`)

By default, `inplace_string` maintains non-trivial copy operations to closely mirror standard string semantics. Defining `QX_INPLACE_STRING_TRIVIAL_COPY_THRESHOLD` sets the maximum buffer capacity (in bytes) for which `inplace_string<N>` is made trivially copyable.

```cpp
#define QX_INPLACE_STRING_TRIVIAL_COPY_THRESHOLD 64
#include <qx/inplace_string.h>

static_assert(std::is_trivially_copyable_v<qx::inplace_string<30>>); // 32 bytes
static_assert(std::is_trivially_copyable_v<qx::inplace_string<62>>); // 64 bytes
```

Controls the balance between copying all bytes via a trivial versus copying only active bytes. For small capacities below the threshold, trivially copying the entire `qx::inplace_string<N>` layout via bitwise copy is fast and enables POD/C-struct compatibility. For larger capacities above the threshold, non-trivial copy operations are preferred so that only active characters up to `size()` are copied via `std::char_traits::copy`, avoiding unnecessary overhead from copying unused trailing capacity.

### Disabling `operator+` Stack Expansion (`QX_INPLACE_STRING_NO_OPERATOR_PLUS`)

By default, adding two `inplace_string` instances (`a + b`) creates a new `inplace_string<N + M>` at compile time. If you wish to prevent implicit stack growth or template code bloat from returning value types of capacity `N+M`, define `QX_INPLACE_STRING_NO_OPERATOR_PLUS`.

```cpp
#define QX_INPLACE_STRING_NO_OPERATOR_PLUS
#include <qx/inplace_string.h>
```

* **What stays enabled:** Overloads returning `std::string` (e.g., `inplace_string + std::string` or `inplace_string + std::string_view`) remain available for interoperability.
* **What is disabled:** Overloads returning `inplace_string<N + M>` (e.g., `inplace_string + inplace_string`, `inplace_string + char`, `inplace_string + &char[M]`). This encourages using explicit `.append()`, `.try_append()`, or `+=` operations.

### Hardening & Contract Checks

Runtime precondition checks (null pointers, out-of-bounds indices, invalid iterator ranges) can be configured globally:

| Macro               | Options / Default                                                                                                                             | Description                                                 |
| :------------------ | :-------------------------------------------------------------------------------------------------------------------------------------------- | :---------------------------------------------------------- |
| `QX_HARDENING_MODE` | `QX_HARDENING_MODE_OFF` (Default)<br>`QX_HARDENING_MODE_ALL`                                                                                  | Enables precondition checks across all API boundaries.      |
| `QX_ASSERT_MODE`    | `QX_ASSERT_MODE_LOG_TRAP` (Default)<br>`QX_ASSERT_MODE_NONE`<br>`QX_ASSERT_MODE_TRAP`<br>`QX_ASSERT_MODE_ABORT`<br>`QX_ASSERT_MODE_LOG_ABORT` | Defines failure action when a hardening assertion triggers. |

```cpp
// Example: Hardened Debug Build
#define QX_HARDENING_MODE QX_HARDENING_MODE_ALL
#define QX_ASSERT_MODE    QX_ASSERT_MODE_LOG_TRAP
#include <qx/inplace_string.h>
```

When hardening is turned off (`QX_HARDENING_MODE_OFF`), all checks compile away entirely with zero runtime overhead.

## API Design & Differences from `std::string`

### Three-tiered mutation API

For methods whose **only** failure mode is running out of capacity (such as `append`, `assign`, `insert`, and `push_back`), three distinct calling patterns are provided:

```cpp
qx::inplace_string<8> str = "abc";

// Throwing tier (Standard behavior)
str.append("def");           // Throws std::length_error on capacity overflow

// Non-Throwing / Checked tier
if (!str.try_append("ghi")) { // Returns `this` pointer on success, nullptr on overflow
    // Handle overflow gracefully without exceptions
}

// Unchecked tier (Fast path)
str.unchecked_append("xyz"); // Precondition: caller guarantees space. Skips bounds checks.
```

> *Note:* Methods like `replace` do not offer `try_*` / `unchecked_*` variants because they can also fail with `std::out_of_range` for invalid positions. Tiered calls are restricted to capacity-only failures to eliminate error-code ambiguity.

### Substring capacity sizing

Standard `std::string::substr` requires dynamic allocation. `qx::inplace_string` provides two options:
* `substr(pos, n)`: Runtime overload returning `inplace_string<N>` (preserves original capacity).
* `substr<Pos, Count>()`: Compile-time template overload returning `inplace_string<Count>`, minimizing stack footprint.

## Feature comparison

| Feature                        | `qx::inplace_string`                                         | `boost::static_string`                | `etl::string`                        |
| :----------------------------- | :----------------------------------------------------------- | :------------------------------------ | :----------------------------------- |
| **Overflow Behavior**          | Throws `std::length_error`                                   | Throws `std::length_error`            | **Truncates** by default (sets flag) |
| **Checked / Unchecked API**    | Per-call (`try_*` / `unchecked_*`)                           | Throwing only                         | Global compile-time policy           |
| **Trivial Copyability Toggle** | Threshold-based (`QX_INPLACE_STRING_TRIVIAL_COPY_THRESHOLD`) | No                                    | No                                   |
| **Disable `operator+` Toggle** | Yes (`QX_INPLACE_STRING_NO_OPERATOR_PLUS`)                   | N/A (Only `+=` / `append`)            | No `operator+`                       |
| **Compile-Time Substring**     | Yes (`substr<Pos, Count>()`)                                 | View-based (`subview()`)              | View-based (`etl::string_view`)      |
| **`std::hash` Compatibility**  | Matches `std::hash<std::string_view>`                        | Matches `std::hash<std::string_view>` | Custom ETL hash                      |


## License

Distributed under the MIT License. See `LICENSE` for details.
