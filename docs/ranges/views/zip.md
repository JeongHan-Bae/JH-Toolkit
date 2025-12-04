# 🔭 **JH Toolkit — `jh::ranges::views::zip` API Reference**

📁 **Header:** `<jh/ranges/views/zip.h>`  
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

`jh::ranges::views::zip` is a **generalized, sequence-aware implementation** of the C++23 `std::views::zip` adaptor.  
It extends the semantics of `zip` to all types that model [`jh::concepts::sequence`](../../conceptual/sequence.md),
not just standard `std::ranges::range`s.

The resulting type, [`jh::ranges::zip_view`](../../ranges/zip_view.md), models both
`std::ranges::view` and `jh::concepts::sequence`, enabling seamless interoperability
between **STL ranges** and **JH sequence abstractions**.

This header also introduces an **extended adaptor**, `jh::ranges::views::zip_pipe`,
which allows **multi-argument pipe syntax** (`a | zip_pipe(b, c, d)`),
providing a more complete and expressive semantic model than the C++23 `std::views::zip`.

---

## 🔹 Definition

```cpp
namespace jh::ranges::views {

inline constexpr detail::zip_fn zip{};
inline constexpr detail::zip_pipe_fn zip_pipe{};

}
```

---

## 🔹 Interface

### 1. **Direct call form**

```cpp
template <jh::concepts::sequence... Seq>
constexpr auto zip(Seq&&... seqs);
```

Returns a [`jh::ranges::zip_view`](../../ranges/zip_view.md) combining all input sequences.  
Equivalent to `std::views::zip` in C++23.

---

### 2. **Single-sequence pipe form**

```cpp
template <jh::concepts::sequence Seq>
constexpr auto zip(Seq&& seq);
```

Returns a closure object, enabling expressions like:

```cpp
seq1 | jh::ranges::views::zip(seq2)
```

This form is equivalent to the standard C++23 `zip` pipeline semantics.

---

### 3. **Multi-sequence pipe extension**

```cpp
template <jh::concepts::sequence... Seq>
constexpr auto zip_pipe(Seq&&... seqs);
```

Returns a closure that supports multi-argument pipeline syntax:

```cpp
seq1 | jh::ranges::views::zip_pipe(seq2, seq3, seq4)
```

This is an **intentional semantic extension** beyond C++23:  
`zip_pipe` restores closure semantics for multiple right-hand sequences,
while remaining fully compatible with the C++ range adaptor model.

---

## 🔹 Description

`jh::ranges::views::zip` lazily combines multiple sequences into a single range of tuples,
where each tuple element corresponds to the same position across all input sequences.  
Iteration stops at the shortest sequence, matching standard `zip` semantics.

Each input is normalized via [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq),
ensuring compatibility with non-standard, duck-typed sequence types that still model
`jh::concepts::sequence`.

This adaptor behaves identically to `std::views::zip` when used with standard ranges,
but automatically promotes non-range sequence-like types to safe view wrappers.

---

## 🔹 Behavior

| Aspect                  | Description                                                                |
|-------------------------|----------------------------------------------------------------------------|
| **Input normalization** | Each sequence is passed through `jh::to_range()` before zipping.           |
| **Underlying type**     | Produces a [`jh::ranges::zip_view`](../../ranges/zip_view.md).             |
| **Iteration model**     | Advances all underlying iterators in lockstep; ends at the shortest range. |
| **Reference semantics** | Elements are referenced, not copied; fully lazy evaluation.                |
| **Compatibility**       | Works with STL containers, arrays, pointers, and all JH sequence types.    |
| **Pipe compatibility**  | Supports both standard single-argument and extended multi-argument forms.  |
| **Semantic extension**  | `zip_pipe` extends `zip` pipeline semantics for multiple RHS sequences.    |
| **Laziness**            | No allocations; iteration is performed on demand.                          |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/zip.h>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<int> ids = {1, 2, 3};
    std::vector<std::string> names = {"alpha", "beta", "gamma"};
    std::vector<double> weights = {1.1, 2.2, 3.3};

    // Direct call form
    for (auto [id, name] : jh::ranges::views::zip(ids, names))
        std::cout << id << ": " << name << '\n';

    // Standard single-sequence pipe form
    for (auto [id, name] : ids | jh::ranges::views::zip(names))
        std::cout << id << ": " << name << '\n';

    // Extended multi-sequence pipe form
    for (auto [id, name, w] : ids | jh::ranges::views::zip_pipe(names, weights))
        std::cout << id << ": " << name << " (" << w << ")\n";
}
```

**Output:**

```
1: alpha
2: beta
3: gamma
1: alpha
2: beta
3: gamma
1: alpha (1.1)
2: beta (2.2)
3: gamma (3.3)
```

---

## 🔹 Design Rationale

In the C++23 standard, `std::views::zip` accepts **two or more ranges** and always returns
a concrete `zip_view`.  
This design is intentional: since an expression like `zip(a, b)` is already complete,
it cannot serve as a deferred pipeline adaptor — the compiler must treat it as a finished view.

Consequently, the standard `zip` cannot be used in a pipeline when more than one sequence
is supplied. The adaptor must choose between two distinct semantics:

| Case             | Argument count | Return type | Meaning                               |
|------------------|----------------|-------------|---------------------------------------|
| `zip(x)`         | One argument   | Closure     | Used in pipeline (e.g. `a \| zip(x)`) |
| `zip(x, y, ...)` | Two or more    | `zip_view`  | Complete expression returning a range |

Because these two forms share the same name but different roles,
the adaptor must decide **at the call site** whether to return a closure or a view.  
This choice is unambiguous in standard `zip`, but it also limits expressiveness:  
multi-sequence pipelines cannot exist, since the adaptor cannot "guess"
whether it appears in a pipeline context.

To resolve this semantic asymmetry, the JH Toolkit introduces
`jh::ranges::views::zip_pipe` — a dedicated **pipe-oriented** variant of `zip`.

`zip_pipe` always produces a **closure**, regardless of argument count**.  
This ensures that expressions like:

```cpp
range1 | jh::ranges::views::zip_pipe(range2, range3, range4);
```

are always well-defined, lazy, and composition-safe.

When used with a single argument, `zip_pipe` behaves identically to `zip`,
but for multiple arguments it extends the pipeline semantics in a
**clear, unambiguous, and logically complete** way,
providing expressive range composition without conflicting with the standard `zip` contract.

---

## 🔹 Compatibility & Concepts

Both `zip` and `zip_pipe` operate on any type that satisfies
[`jh::concepts::sequence`](../../conceptual/sequence.md).   
All input sequences are internally normalized through
[`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq),
which guarantees range compliance and consistent iterator behavior.

This ensures seamless interoperability between standard STL ranges
and all duck-typed sequence types recognized by the JH Toolkit.

---

## 🔹 Integration Notes

* The behavior of `zip` exactly matches C++23's `std::views::zip` when used with standard ranges.
* `jh::to_range()` automatically promotes non-standard types into compatible range wrappers.
* `zip_pipe` extends the standard pipeline semantics safely and orthogonally.
* Both adaptors are fully lazy and composable within range pipelines.
* Included automatically by `<jh/views>` (since **v1.3.3+**) or directly via `<jh/ranges/views/zip.h>`.

---

## 🧩 Summary

`jh::ranges::views::zip` provides a **generalized, sequence-aware zip adaptor**
that unifies standard and custom range types under a single interface.
Together with `jh::ranges::views::zip_pipe`, it forms a more **semantically complete**
version of the standard `zip`, offering consistent syntax for both direct and pipe forms —
even when multiple right-hand sequences are involved.

It is a **drop-in superset** of `std::views::zip`, extending its reach to
duck-typed sequences and resolving its pipeline ambiguity in a clean, consistent way.
