# 🗂️ **JH Toolkit — `jh::ranges::range_wrapper` API Reference**

📁 **Header:** `<jh/ranges/range_wrapper.h>`  
📦 **Namespace:** `jh::ranges`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::range_wrapper` is a **duck-typed sequence adaptor**,
converting any [`jh::concepts::sequence`](../conceptual/sequence.md)
into a valid `std::ranges::range` object.

It acts as the **universal bridge** between JH’s behavioral sequence model
and the C++ standard ranges system — wrapping any iterable object
that supports `begin()` / `end()` traversal into a compliant, lazy range.

---

## 🔹 Definition

```cpp
template <jh::concepts::sequence Seq>
class range_wrapper;
```

A lightweight, header-only type that satisfies `std::ranges::range`.  
All operations are constexpr and incur zero runtime overhead.

---

## 🔹 Behavior

| Aspect                | Description                                                                                                                    |
|-----------------------|--------------------------------------------------------------------------------------------------------------------------------|
| **Purpose**           | Converts any duck-typed `jh::concepts::sequence` into a valid `std::ranges::range`.                                            |
| **Iterator model**    | Uses `std::begin()` / `std::end()` deduction; compatible with STL, arrays, pointers, and custom containers.                    |
| **Laziness**          | Provides lazy access — no copying or allocation of underlying elements.                                                        |
| **Const propagation** | If the original sequence supports const iteration, `range_wrapper<const Seq&>` is transparently deduced.                       |
| **Move semantics**    | Fully moveable and forwardable.                                                                                                |
| **Idempotency**       | Wrapping an object that is already a `std::ranges::range` or another `range_wrapper` produces a transparent pass-through type. |

---

## 🔹 Usage

`range_wrapper` is **not** meant for direct use —  
it is invoked automatically by [`jh::to_range()`](../conceptual/sequence.md#jhto_rangeseq)
to normalize arbitrary sequence-like objects into range-compatible forms.  

Example (recommended usage via `jh::to_range`):

```cpp
#include <jh/conceptual/sequence.h>
#include <vector>

int main() {
    std::vector<int> xs = {1, 2, 3};

    // Recommended: use jh::to_range() to let the system deduce the correct wrapper.
    auto rng = jh::to_range(xs);

    for (auto v : rng)
        std::cout << v << ' ';
}
```

*Output:*

```
1 2 3
```

> ⚙️ **Note:**
> You can construct `range_wrapper` manually,
> but for standard containers (which are already `std::ranges::range`),
> this only rewraps their iterators unnecessarily.  
> `jh::to_range()` automatically detects and avoids redundant wrapping.

---

## 🔹 Integration Notes

* `range_wrapper` is an **internal bridging type**,
  not intended for direct user-level instantiation.  
* It is the **primary mechanism** by which `jh::to_range()` adapts duck-typed sequences.  
* All `jh::ranges::views` adaptors (e.g. [`zip`](views/zip.md), [`enumerate`](views/enumerate.md))
  depend on `range_wrapper` to ensure consistent interoperability.  
* The adaptor is **idempotent** — repeated wrapping automatically collapses into a pass-through type.  
* Avoid explicit `range_wrapper` construction unless writing low-level integration code.

---

### 🧩 **Summary**

`jh::ranges::range_wrapper` defines the **canonical bridge**
between `jh::concepts::sequence` and `std::ranges::range`.  
It is automatically applied through `jh::to_range()`
and ensures that every iterable — STL or custom — can participate
in range pipelines with predictable, zero-overhead behavior.
