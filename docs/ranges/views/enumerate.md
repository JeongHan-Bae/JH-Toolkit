# 🔭 **JH Toolkit — `jh::ranges::views::enumerate` API Reference**

📁 **Header:** `<jh/ranges/views/enumerate.h>`  
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

`jh::ranges::views::enumerate` is a **C++20-compatible adaptor** that pairs each element of a sequence
with its corresponding index, equivalent to `std::views::enumerate` (C++23 proposal).
It is implemented using [`jh::ranges::views::zip`](zip.md) and [`std::views::iota`](https://en.cppreference.com/w/cpp/ranges/iota_view).

Each element is first normalized through [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq),
ensuring compatibility with all **duck-typed sequences** recognized by
[`jh::concepts::sequence`](../../conceptual/sequence.md).

---

## 🔹 Definition

Creates a zipped range of `(index, element)` pairs by combining an index sequence and the input sequence.

```cpp
template <jh::concepts::sequence Seq>
constexpr auto enumerate(
    Seq&& seq,
    jh::concepts::sequence_difference_t<Seq> start = 0
);
```

---

## 🔹 Description

`enumerate()` behaves like Python's `enumerate`, but implemented as a **lazy range adaptor**.
It takes any sequence, normalizes it via [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq),
and pairs each element with a sequential index starting from `start` (default `0`).

Internally, it works by zipping:

```cpp
std::views::iota(start)
```

with the input range, effectively creating a range of tuples `(index, value)`.

This allows you to iterate with indices without manual counters, while maintaining
full compatibility with the C++20 ranges pipeline and all **JH duck-typed sequences**.

---

## 🔹 Behavior

| Aspect                  | Description                                                             |
| ----------------------- | ----------------------------------------------------------------------- |
| **Index generation**    | Uses `std::views::iota(start)` to produce an arithmetic index sequence. |
| **Input normalization** | The input is passed through `jh::to_range()` for range compliance.      |
| **Iteration model**     | Elements are traversed in lockstep with their index.                    |
| **Reference semantics** | Dereferencing yields `(index, reference)` pairs — no copying.           |
| **Compatibility**       | Works with STL containers, arrays, pointers, and JH duck-typed types.   |
| **Laziness**            | Entirely lazy, zero allocation or eager evaluation.                     |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/enumerate.h>
#include <vector>
#include <iostream>

int main() {
    std::vector<std::string> words = {"alpha", "beta", "gamma"};

    for (auto [i, w] : jh::ranges::views::enumerate(words, 1)) {
        std::cout << i << ": " << w << '\n';
    }
}
```

*Output:*

```
1: alpha
2: beta
3: gamma
```

---

## 🔹 Integration Notes

* `enumerate()` acts as syntactic sugar for pairing indices with elements.
* Relies on `jh::ranges::views::zip` — thus inherits all its range semantics.
* `jh::to_range()` ensures seamless support for STL containers, arrays, and custom sequence-like objects.
* Can be combined with other adaptors in pipelines, e.g.:

  ```cpp
  seq | jh::views::enumerate() | std::views::filter(...)
  ```
* Included automatically by `<jh/views>` (1.3.3+) or via direct include `<jh/ranges/views/enumerate.h>`.

---

### 🧩 **Summary**

`jh::ranges::views::enumerate` provides a **lazy, index-value pairing adaptor**
that emulates Python-style enumeration while maintaining full C++20 range compatibility.
It bridges the gap between JH Toolkit's sequence abstraction and standard range pipelines,
offering a zero-overhead way to iterate with explicit indices.
