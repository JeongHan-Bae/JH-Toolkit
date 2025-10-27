# 🗂️ **JH Toolkit — `jh::ranges::zip_view` API Reference**

📁 **Header:** `<jh/ranges/zip_view.h>`  
📦 **Namespace:** `jh::ranges`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::zip_view` provides a **C++20 fallback implementation** of
`std::ranges::zip_view` (C++23 and later).  
It allows *lockstep iteration* over multiple ranges, producing tuple-like element proxies
without copying or allocating data.

This header belongs to the **`ranges` repository** and serves as an internal dependency of
[`jh::ranges::views::zip`](views/zip.md) and [`jh::ranges::views::enumerate`](views/enumerate.md).  
It is **not intended for direct inclusion** by user code.

---

## 🔹 Definition

```cpp
template <std::ranges::input_range... Ranges>
class zip_view;
```

`jh::ranges::zip_view` emulates the observable behavior and API of
`std::ranges::zip_view`, transparently aliasing it when available.  

The fallback activation is controlled by **feature detection**, not just the language standard:

```cpp
#if defined(__cpp_lib_ranges_zip) && __cpp_lib_ranges_zip >= 202110L
#define JH_HAS_STD_ZIP_VIEW 1
#elif defined(__cplusplus) && __cplusplus > 202302L
#define JH_HAS_STD_ZIP_VIEW 1
#else
#define JH_HAS_STD_ZIP_VIEW 0
#endif
```

> ⚙️ **Note:**
> Using `-std=c++23` or higher is completely **safe** —  
> the compiler will automatically enable C++23 semantics where supported.  
> If a particular implementation does **not** yet provide
> `std::ranges::zip_view`, it simply won't define
> `__cpp_lib_ranges_zip` or expose a `__cplusplus` value above `202302L`.  
> In that case, the JH Toolkit fallback remains active automatically.
>
> What is **not safe**, however, is manually faking feature macros —
> e.g. defining `-D__cpp_lib_ranges_zip=202110L` by hand.  
> Doing so can desynchronize the detection logic and result in
> undefined behavior or partial symbol exposure.

When the standard version is absent, this header provides a
fully portable implementation composed of:

* `zip_iterator` — synchronized iterator tuple  
* `zip_sentinel` — termination marker  
* `zip_reference_proxy` — structured binding–friendly reference aggregator

These components together reproduce the complete semantics of `std::ranges::zip_view`.

---

## 🔹 Behavior

| Aspect                  | Description                                                                                      |
|:------------------------|:-------------------------------------------------------------------------------------------------|
| **Purpose**             | Provides C++20-compatible `zip_view` equivalent to the C++23 standard version.                   |
| **Input model**         | Accepts any `std::ranges::input_range` and works transparently with `jh::ranges::range_wrapper`. |
| **Iteration**           | Yields tuples of element references aggregated via proxy.                                        |
| **Termination**         | Ends when any underlying range is exhausted (shortest-range rule).                               |
| **Const compatibility** | Supports const iteration if all subranges allow it.                                              |
| **Laziness**            | Fully lazy; elements are accessed only on demand.                                                |
| **Fallback behavior**   | Automatically replaced by `std::ranges::zip_view` when supported.                                |

---

## 🔹 Internal Types (for reference only)

These are **implementation details** used internally by the fallback version:

| Type                  | Role                                                               | Availability            |
|-----------------------|--------------------------------------------------------------------|-------------------------|
| `zip_reference_proxy` | Represents the tuple-like dereference result; supports `std::get`. | Removed in C++23 builds |
| `zip_iterator`        | Iterates through multiple subranges synchronously.                 | Removed in C++23 builds |
| `zip_sentinel`        | Marks the unified end position across all subranges.               | Removed in C++23 builds |

> ⚠️ **Do not depend on these types directly.**  
> In C++23 or later, they will no longer exist —  
> `jh::ranges::zip_view` becomes a simple alias to `std::ranges::zip_view`.

---

## 🔹 Example

```cpp
#include <jh/ranges/zip_view.h>
#include <vector>
#include <array>

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

## ⚠️ Integration Notes (update)

* Used internally by [`jh::ranges::views::zip`](views/zip.md)
  and [`jh::ranges::views::enumerate`](views/enumerate.md).
* Designed to be **ABI-compatible** with the standard `std::ranges::zip_view` once available.
* Avoids any dependency on experimental or vendor-specific range extensions.
* Automatically included when using `<jh/views>` or `<jh/ranges/views/zip.h>`.

> ⚠️ **Known limitation (1.3.4 freeze note):**  
> The current implementation of `jh::ranges::zip_view` is **not yet fully recognized**
> by `jh::concepts::sequence`.  
> It only satisfies the *range-binding semantics* through structural matching
> rather than full duck-type conformance.  
> This limitation affects concept resolution in generic pipelines,
> though the view itself functions correctly within `jh::views` and STL range contexts.

> 💡 **Planned improvement (1.3.5):**  
> The upcoming **1.3.5** release will extend the internal trait detection and
> concept resolution rules to fully register `jh::ranges::zip_view`
> as a valid `jh::concepts::sequence` type.  
> This update will introduce stronger duck-typing semantics
> and ensure that all range adaptors—`zip`, `enumerate`, and future ones—
> participate seamlessly in generic pipelines and meta-level type deduction.

---

### 🧩 **Summary**

`jh::ranges::zip_view` is a **pre-standard, C++20-compatible bridge** to
`std::ranges::zip_view`.  
It provides lazy, synchronized iteration across multiple ranges,
offering identical semantics to the C++23 standard view while remaining
safe, lightweight, and forward-compatible.
