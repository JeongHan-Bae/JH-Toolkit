# 🔭 **JH-Toolkit — `jh::ranges::views::enumerate` API Reference**

📁 **Header:** `<jh/ranges/views/enumerate.h>`  
📦 **Namespace:** `jh::ranges::views`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::views::enumerate` is a **sequence-aware range adaptor** that lazily pairs
each element of a sequence with its corresponding index.  
It provides a C++20-compatible alternative to the proposed `std::views::enumerate` (C++23).

This adaptor is implemented via [`jh::ranges::views::zip`](zip.md) and
[`std::views::iota`](https://en.cppreference.com/w/cpp/ranges/iota_view).  
It can be used in both **direct-call** and **pipe** forms, and supports
any index type convertible to the sequence's `difference_type`.

---

## 🔹 Definition

```cpp
namespace jh::ranges::views {

inline constexpr detail::enumerate_fn enumerate{};

}
```

---

## 🔹 Interface

The public interface supports two forms:

1. **Direct call form**

   ```cpp
   template <jh::concepts::sequence Seq, typename Int>
   constexpr auto enumerate(Seq&& seq, Int start = 0);
   ```

2. **Pipe form**

   ```cpp
   template <typename Int = std::ptrdiff_t>
   constexpr auto enumerate(Int start = 0);
   ```

The second form returns a closure object, enabling syntax like:

```cpp
seq | jh::ranges::views::enumerate(5)
```

---

## 🔹 Description

`enumerate()` constructs a **lazy zipped range** by pairing
an arithmetic index view with the given sequence.

Internally, it combines:

```cpp
std::views::iota(static_cast<diff_t>(start))
```

with the input range using `jh::ranges::views::zip`.  
Here `diff_t` is deduced as:

```cpp
jh::concepts::sequence_difference_t<std::remove_cvref_t<Seq>>
```

The function supports any `start` value that can be `static_cast`
to `diff_t`. Invalid casts result in ill-formed code at compile time.

---

## 🔹 Behavior

| Aspect                  | Description                                                               |
|-------------------------|---------------------------------------------------------------------------|
| **Index generation**    | Uses `std::views::iota(start)` to generate an index sequence.             |
| **Type deduction**      | Index type deduced via `jh::concepts::sequence_difference_t<Seq>`.        |
| **Input handling**      | Lvalue inputs are borrowed and rvalue inputs are owned through `zip`.     |
| **Element access**      | Yields an index value and source element access through Zip's proxy.     |
| **Pipe compatibility**  | Callable in both direct and pipe forms.                                   |
| **Type constraints**    | `start` must be convertible to the deduced difference type.               |
| **Composition**         | Uses `zip` to combine the index view with the source sequence.          |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/enumerate.h>
#include <vector>
#include <iostream>

int main() {
    std::vector<std::string> words = {"alpha", "beta", "gamma"};

    // Direct call
    for (auto [i, w] : jh::ranges::views::enumerate(words, 1))
        std::cout << i << ": " << w << '\n';

    // Pipe form
    for (auto [i, w] : words | jh::ranges::views::enumerate(10))
        std::cout << i << ": " << w << '\n';
}
```

**Output:**

```
1: alpha
2: beta
3: gamma
10: alpha
11: beta
12: gamma
```

---

## 🔹 Index Type (`diff_t`)

The index type is deduced as `jh::concepts::sequence_difference_t<Seq>`,
resolved through the following rules:

1. If the iterator defines a valid `difference_type`, it is used.
2. If subtraction (`it - it`) is available, its result type is used.
3. If neither exists, it falls back to `std::ptrdiff_t`.
4. Mismatched definitions of the above invalidate the sequence concept.
5. `start` is `static_cast` to this type; invalid conversions are ill-formed.

---

## 🔹 Integration Notes

* Uses [`jh::ranges::views::zip`](zip.md) to combine indices with source elements.
* `enumerate()` acts as syntactic sugar for pairing elements with indices.
* Works with standard containers and JH duck-typed sequences that model `sequence`.
* Included automatically by `<jh/views>` (since **v1.3.3+**) or via `<jh/ranges/views/enumerate.h>`.

---

## 🧩 Summary

`jh::ranges::views::enumerate` provides a lazy index-value adaptor
compatible with the full C++20 ranges pipeline.  
It offers the same ergonomics as Python's `enumerate`, while remaining fully generic
and type-safe across all sequence-like objects modeled by JH's sequence concept.
