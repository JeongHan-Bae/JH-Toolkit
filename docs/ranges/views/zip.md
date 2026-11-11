# 🔭 **JH-Toolkit — `jh::ranges::views::zip` API Reference**

📁 **Header:** `<jh/ranges/views/zip.h>`  
📦 **Namespace:** `jh::ranges::views`  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::views::zip` is the JH-Toolkit's sequence-aware Zip adaptor.
It accepts types modeling [`jh::concepts::sequence`](../../conceptual/sequence.md),
converts them to standard ranges through `jh::to_range()`, and combines them
with `jh::ranges::zip_view`. This gives callers a non-consuming Sequence input
contract and supports custom duck-typed sequences.

The resulting [`jh::ranges::zip_view`](../../ranges/zip_view.md) models
`std::ranges::view`. Its traversal category depends on the converted inputs
and on whether the standard or C++20 fallback implementation is active.

The companion `jh::ranges::views::zip_pipe` provides multi-argument pipe syntax
such as `a | zip_pipe(b, c, d)`.

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

`zip(a, b, ...)` returns a [`jh::ranges::zip_view`](../../ranges/zip_view.md)
combining the converted input sequences.

---

### 2. **Single-sequence pipe form**

`zip(b)` returns a closure, enabling:

```cpp
seq1 | jh::ranges::views::zip(seq2)
```

This one-argument return type is a JH-Toolkit adaptor choice. The C++23
`std::views::zip(b)` expression returns a view containing one range.

---

### 3. **Multi-sequence pipe extension**

`zip_pipe(b, c, ...)` returns a closure that supports multi-argument pipe syntax:

```cpp
seq1 | jh::ranges::views::zip_pipe(seq2, seq3, seq4)
```

`zip_pipe` explicitly captures the right-hand sequences for a later pipe call.

---

## 🔹 Description

`jh::ranges::views::zip` lazily combines multiple sequences into a single range of tuples,
where each tuple element corresponds to the same position across all input sequences.  
Iteration stops at the shortest sequence, matching standard `zip` semantics.

Each input is converted via [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq).
The resulting standard views are passed to `jh::ranges::zip_view`.
Lvalue containers are held through reference views; rvalue containers are
moved into owning views. A closure that captures an rvalue container is
consumed when piped as a temporary, or can be used with `std::move(closure)`.
The source of a borrowed lvalue view must outlive the Zip view.
Passing a non-owning view by value retains that view's dependency on its
external element storage.

---

## 🔹 Behavior

| Aspect                  | Description                                                                |
|-------------------------|----------------------------------------------------------------------------|
| **Input normalization** | Each sequence is passed through `jh::to_range()` before zipping.           |
| **Input ownership**     | Lvalues are borrowed; rvalues are owned by their converted views.         |
| **Underlying type**     | Produces a [`jh::ranges::zip_view`](../../ranges/zip_view.md).             |
| **Iteration model**     | Advances all underlying iterators in lockstep; ends at the shortest range. |
| **Element access**      | Lvalue source elements remain linked to their originals; prvalue elements are stored by value in the proxy. |
| **Compatibility**       | Accepts standard containers and custom types modeling `jh::concepts::sequence`. |
| **Pipe compatibility**  | Supports both standard single-argument and extended multi-argument forms.  |
| **Semantic extension**  | `zip_pipe` extends `zip` pipeline semantics for multiple RHS sequences.    |
| **Laziness**            | Iteration and element access occur on demand; source views determine any underlying allocation. |

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

The JH-Toolkit separates two caller intentions: `zip(a, b, ...)` constructs
a view immediately, while `zip(b)` captures one sequence for a later pipe
expression. `zip_pipe(b, c, ...)` captures multiple sequences for the same
purpose. These forms make sequence conversion and pipe composition explicit
at the call site.

| Call | Result | Typical use |
|------|--------|-------------|
| `zip(a, b, ...)` | `zip_view` | Iterate several sequences directly |
| `zip(b)` | Closure | `a \| zip(b)` |
| `zip_pipe(b, c, ...)` | Closure | `a \| zip_pipe(b, c)` |

The [C++23 standard adaptor](https://eel.is/c++draft/range.zip.overview)
returns a view for `std::views::zip(b)`. JH's single-argument closure is an
intentional interface choice for its sequence-oriented pipelines.

For example:

```cpp
range1 | jh::ranges::views::zip_pipe(range2, range3, range4);
```

constructs a Zip view when the left-hand sequence is supplied.

With one argument, `zip_pipe(b)` has the same closure behavior as `zip(b)`.

---

## 🔹 Compatibility & Concepts

Both `zip` and `zip_pipe` operate on any type that satisfies
[`jh::concepts::sequence`](../../conceptual/sequence.md).   
All input sequences are converted through
[`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq),
which supplies standard range inputs to `zip_view`.

The caller's sequence type must uphold the non-consuming iteration contract;
the concept checks its required operations at compile time.

---

## 🔹 Integration Notes

* `jh::to_range()` promotes supported custom sequences into standard ranges.
* `zip_pipe` provides a closure for multiple right-hand sequences.
* Both adaptors construct views that iterate on demand.
* Include `<jh/views>` or `<jh/ranges/views/zip.h>`.

---

## 🧩 Summary

`jh::ranges::views::zip` gives non-consuming sequences a direct Zip call and a
single-sequence pipe form. `zip_pipe` adds a multi-sequence pipe form. Both
convert their inputs to standard ranges before constructing `zip_view`.
