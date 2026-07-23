# inplace_string

A fixed-capacity, stack-backed string with a `std::string`-like API and no dynamic allocation. Capacity is a template parameter; anything that would exceed it is handled explicitly instead of triggering a reallocation.

Use it where you want a compile-time size cap, predictable stack usage, and no heap traffic, but still want `append`/`find`/`substr`/etc. to feel like `std::string`.

## Compatibility

Targets C++20, works in C++17 mode too. Tested against Clang 14/18/20, GCC 11/14, Apple Clang 15/17/21, and MSVC 14.2/14.4/14.5. `starts_with`/`ends_with` need C++20, `contains` needs C++23 — both just disappear on older standards rather than failing to compile.

Contiguous iterators (raw pointers, `vector<CharT>`, `array<CharT,_>`, `string`/`string_view` iterators, ...) are detected in C++17 mode and given a `std::memcpy` fast path in append/insert/assign.

## Adding it to your project

This project is header only and can be simply copied into your project. For easy CMake integration, add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
  qx_inplace_string
  GIT_REPOSITORY https://github.com/jose-a-sa/inplace_string.git
  GIT_TAG        v1.0.0 # pin a tag or commit
)
FetchContent_MakeAvailable(qx_inplace_string)

target_link_libraries(your_target PRIVATE qx::inplace_string) # adjust to your actual target/component name
```

## Usage

```cpp
#include <qx/inplace_string.h>

qx::inplace_string<32> text = "hello";
text.append(" world");
text += "!";

auto value = qx::to_inplace_string<8>(12345);
```

## Where it differs from std::string

- **No `operator+`.** For two capacities `N1`/`N2` there's no result capacity that isn't either wasteful (`N1+N2`) or too small for some inputs (`max(N1,N2)`). Due to the ambiguity and to protect the stack from overflowing, prefer `append` and `try_append`.
- **No allocator parameter.** Nothing to configure since nothing is allocated.
- **Three way API to mutate the string**, but only on overloads whose sole failure mode is capacity (i.e. `append`/`assign`/`insert`/`push_back`):
  ```cpp
  s.append("def");           // throws std::length_error if it doesn't fit
  s.try_append("def");       // returns `this`, or nullptr on failure
  s.unchecked_append("def"); // caller guarantees it fits, skips the check
  ```
It means that `replace(pos, ...)` has no `try_replace`/`unchecked_replace`, as it can also fail with `std::out_of_range` for a bad `pos`, and a single sentinel return can't tell you which of the two actually happened. try_*/unchecked_* only exist where that ambiguity can't arise.
- **`substr<Pos, Count>()`** — a compile-time-indexed overload alongside the usual runtime `substr(pos, n)`. Returns a string whose capacity is `Count` (or `N - Pos`), not the full `N` of the source.
- **`std::hash`** matches `std::hash<string_view>` for equal contents, so it's interchangeable with `string`/`string_view` as a map key.
- Size is stored in the smallest unsigned type that fits `N`; layout follows ordinary struct rules with no forced alignment.

The rest of the API matches `std::string`. Self-aliasing is supported, not UB.  `s.append(s)`, inserting from a pointer into `s`'s own buffer, replacing with a source range that overlaps the destination, is all handled correctly.

## Hardening

Precondition checks — null pointers, out-of-range indices, invalid iterator pairs — are gated behind two macros, both off by default:

```cpp
#define QX_HARDENING_MODE QX_HARDENING_MODE_ALL   // turns contract checks on at all
#define QX_ASSERT_MODE    QX_ASSERT_MODE_LOG_TRAP // what happens when one fails (this is the default once hardening is on)
#include <qx/inplace_string.h>
```

or from the command line:

```
-DQX_HARDENING_MODE=QX_HARDENING_MODE_ALL -DQX_ASSERT_MODE=QX_ASSERT_MODE_LOG_TRAP
```

`QX_ASSERT_MODE` picks the failure behavior: `NONE` (0), `TRAP` (1, silent trap), `LOG_TRAP` (2, log to stderr then trap — the default), `ABORT` (3), `LOG_ABORT` (4). With hardening off, all of this compiles away to nothing, so there's no reason not to turn it on for debug/test builds.

This is unrelated to `try_*`/`unchecked_*` above — that's about capacity, which is data-dependent; hardening is about catching caller bugs.

## Comparing to other fixed-capacity strings

|                         | inplace_string                          | Boost.StaticString                        | ETL string                                                   |
| ----------------------- | --------------------------------------- | ----------------------------------------- | ------------------------------------------------------------ |
| overflow                | throws                                  | throws                                    | **truncates** by default, with a propagating truncation flag |
| checked/unchecked calls | `try_*`/`unchecked_*` per call          | throwing only                             | truncate-vs-error is a compile-time policy, not per-call     |
| `operator+`             | no                                      | not a member (append/`+=` instead)        | no                                                           |
| substring               | `substr(pos,n)` + `substr<Pos,Count>()` | `substr()` + `subview()` (returns a view) | view-based, via `etl::string_view`                           |

Boost.StaticString is the closer `std::string` clone — broader API, no capacity-checking tier of its own. ETL truncates by default, which fits environments where exceptions aren't available at all. This library keeps throwing as the default (so overflow never passes silently) but makes checking a per-call choice via `try_*`/`unchecked_*`, rather than an exceptions-on/exceptions-off global switch.
