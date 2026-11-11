# 🗂️ **JH-Toolkit — `jh::ranges::zip_view` API Reference**

📁 **Header:** `<jh/ranges/zip_view.h>`  
📦 **Namespace:** `jh::ranges`  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::zip_view` aliases `std::ranges::zip_view` when the standard version
is available. Its C++20 fallback iterates over input views in lockstep and
produces tuple-like element proxies without materializing the zipped sequence.
The fallback aims to follow the C++23 standard view's behavior where supported.

This header provides the view used by
[`jh::ranges::views::zip`](views/zip.md) and [`jh::ranges::views::enumerate`](views/enumerate.md).

---

## 🔹 Definition

```cpp
template <typename... Views>
    requires ((std::ranges::view<Views> && std::ranges::input_range<Views>) && ...)
class zip_view;
```

The C++20 fallback supports input views and retains forward traversal when
all underlying views are forward ranges. Its current iterator operations stop
at forward traversal; the standard view also supports bidirectional and random
access traversal when its inputs allow them. The specialized `jh::views::zip`
adaptor has a separate, non-consuming `jh::concepts::sequence` input contract.
`zip_view` stores its component views by value. A reference view borrows its
source; an owning view retains the source object. A copied non-owning view may
still refer to external element storage that must outlive iteration.

Selection follows the condition in the header:

```cpp
#if defined(__cpp_lib_ranges_zip) && __cpp_lib_ranges_zip >= 202110L
#define JH_HAS_STD_ZIP_VIEW 1
#elif defined(__cplusplus) && __cplusplus > 202302L
#define JH_HAS_STD_ZIP_VIEW 1
#else
#define JH_HAS_STD_ZIP_VIEW 0
#endif
```

The feature macro selects the standard implementation in C++23 when the
library provides it. The later-language branch assumes a standard Zip
implementation is present.

The fallback comprises:

* `zip_iterator` — synchronized iterator tuple  
* `zip_sentinel` — termination marker  
* `zip_reference_proxy` — structured binding–friendly reference aggregator

These components provide synchronized input and forward traversal in the fallback.

---

## 🔹 Behavior

| Aspect                  | Description                                                                                      |
|:------------------------|:-------------------------------------------------------------------------------------------------|
| **Purpose**             | Provides C++20-compatible synchronized traversal of standard input views.                       |
| **Input model**         | Accepts standard input views, including adapted sequences and single-pass input views.          |
| **Iteration**           | Yields proxies holding references or values according to each source element.                    |
| **Termination**         | Ends when any underlying range is exhausted (shortest-range rule).                               |
| **Const compatibility** | Supports const iteration if all subranges allow it.                                              |
| **Laziness**            | Fully lazy; elements are accessed only on demand.                                                |
| **Fallback behavior**   | Automatically replaced by `std::ranges::zip_view` when supported.                                |

---

## 🔹 Internal Types (for reference only)

These are **implementation details** used internally by the fallback version:

| Type                  | Role                                                               | Availability            |
|-----------------------|--------------------------------------------------------------------|-------------------------|
| `zip_reference_proxy` | Represents the tuple-like dereference result; supports ADL-based `get`. | Fallback only |
| `zip_iterator`        | Iterates through multiple subranges synchronously.                       | Fallback only |
| `zip_sentinel`        | Marks the unified end position across all subranges.                     | Fallback only |

These implementation types are absent when `jh::ranges::zip_view` aliases the
standard type.

---

## 🔹 Example

```cpp
#include <jh/ranges/zip_view.h>
#include <vector>
#include <array>
#include <iostream>

int main() {
    std::vector<int> a = {1, 2, 3};
    std::array<char, 3> b = {'x', 'y', 'z'};

    jh::ranges::zip_view view(a, b);

    for (auto [x, y] : view)
        std::cout << x << " : " << y << '\n';
}
```

*Output:*

```
1 : x
2 : y
3 : z
```

---

## 🔹 Integration Notes

* Used internally by [`jh::ranges::views::zip`](views/zip.md)
  and [`jh::ranges::views::enumerate`](views/enumerate.md).
* Automatically included when using `<jh/views>` or `<jh/ranges/views/zip.h>`.

---

### 🧩 **Summary**

`jh::ranges::zip_view` provides lazy, synchronized iteration across standard
input views. The C++20 fallback retains forward traversal when every input
view is forward.
