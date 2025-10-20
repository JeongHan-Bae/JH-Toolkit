# 🔭 **JH Toolkit — `jh::ranges::views::zip` API Reference**

📁 **Header:** `<jh/ranges/views/zip.h>`  
📦 **Namespace:** `jh::ranges::views`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::views::zip` is a **C++20-compatible adaptor** equivalent to `std::views::zip` (C++23).  
It constructs a [`jh::ranges::zip_view`](../zip_view.md) after converting all input objects
into valid `std::ranges::range` instances through [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq).

This adaptor provides early `zip` functionality for all **duck-typed sequences** recognized by
[`jh::concepts::sequence`](../../conceptual/sequence.md),
bridging STL containers, raw arrays, pointers, and user-defined iterables.

---

## 🔹 Definition

Creates a [`jh::ranges::zip_view`](../zip_view.md) over one or more sequences.
Each argument is normalized via `jh::to_range()`, then wrapped by `std::views::all`.
All underlying ranges are traversed in lockstep, producing tuple-like element proxies.

```cpp
template <jh::concepts::sequence... Seq>
constexpr auto zip(Seq&&... seqs);
```

---

## 🔹 Description

* Accepts any number of arguments satisfying `jh::concepts::sequence`.
* Each input is transformed into a valid `std::ranges::range` using `jh::to_range()`.
* Constructs a `jh::ranges::zip_view` that provides synchronized iteration across all ranges.
* Iteration stops at the shortest range's end.
* Operates entirely lazily — no allocation or data duplication.
* Fully compatible with structured bindings and standard algorithms.

---

## 🔹 Behavior

| Aspect                  | Description                                                                                |
|-------------------------|--------------------------------------------------------------------------------------------|
| **Input conversion**    | Every input is passed through `jh::to_range()` to ensure it models `std::ranges::range`.   |
| **Iteration model**     | Elements from all ranges are accessed in lockstep; iteration ends when any range exhausts. |
| **Reference semantics** | Dereferencing yields tuple-like proxies of element references — no copying.                |
| **Compatibility**       | Works with STL containers, raw arrays, C-style pointers, and custom duck-typed types.      |
| **Fallback layer**      | Provides pre-C++23 availability of `std::views::zip`.                                      |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/zip.h>
#include <vector>
#include <array>

int main() {
    std::vector<int> xs = {1, 2, 3};
    std::array<char, 3> ys = {'A', 'B', 'C'};

    for (auto [a, b] : jh::ranges::views::zip(xs, ys)) {
        std::cout << a << " -> " << b << '\n';
    }
}
```

*Output:*

```
1 -> A
2 -> B
3 -> C
```

---

## 🔹 Integration Notes

* `jh::to_range()` ensures idempotent wrapping —
  once an object becomes a valid `std::ranges::range`, subsequent wrapping is a compile-time no-op.
* Behaves as a **transparent adaptor** when called on existing ranges.
* Serves as a **bridge** between JH duck-typed sequences and the STL range system.
* Included automatically by `<jh/views>` (1.3.3+) or via direct include `<jh/ranges/views/zip.h>`.

---

### 🧩 **Summary**

`jh::ranges::views::zip` provides a **lazy, lockstep traversal adaptor** fully compatible with
`std::views::zip`. It extends range-based composition to all JH Toolkit sequences,
supporting seamless iteration across heterogeneous sources with no runtime overhead.
