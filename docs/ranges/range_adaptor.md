# 🗂️ **JH Toolkit — `jh::ranges::range_adaptor` API Reference**

📁 **Header:** `<jh/ranges/range_adaptor.h>`  
📦 **Namespace:** `jh::ranges`  
📅 **Version:** 1.4.0 (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::range_adaptor` is a **duck-typed sequence adaptor**,
converting any [`jh::concepts::sequence`](../conceptual/sequence.md)
into a valid `std::ranges::range` object (more precisely, a `viewable_range`).

It acts as the **universal bridge** between JH's tolerance duck-typing for third-party or user-defined containers 
and the C++ standard ranges system — wrapping any iterable object
that supports `begin()` / `end()` traversal into a compliant, lazy range.

> ⚙️ **Note:**
> Only the types explicitly promoted via `jh::ranges::range_adaptor` is guaranteed
> as a `std::ranges::viewable_range`.  
> Using `jh::to_range()` will only wrap types that are not already compliant,
> so the resulting type is expected as:
> - If the input is a movable and copyable `std::ranges::range`, it is forwarded as-is.
> - If the input is already a `std::ranges::range`, but non-movable or non-copyable,
>   It is wrapped in `std::ranges::subrange` to ensure viewability.  
> - Otherwise, it is wrapped in `jh::ranges::range_adaptor` to provide range compliance.  
> 
> In summary, `jh::to_range(seq)` guarantees three things:
>   1. seq must satisfy `jh::concepts::sequence`, meaning it can be iterated, and iterating does not consume it.
>   2. The return value is at least a range, but not necessarily a view.
>   3. If the input is a lvalue, the return value can always be safely passed into `std::views::all()`.

---

## 🔹 Definition

```cpp
template <typename Seq>
class range_adaptor;
```

A lightweight, header-only type that satisfies `std::ranges::range`.  
All operations are constexpr and incur zero runtime overhead.

**Note:**
In practice, `range_adaptor` is a delegate type for `jh::to_range(seq)`.  
This means it does not actually check the type's validity within the template.  
This is why we recommend using `jh::to_range(seq)` directly (in [`<jh/concepts>`](../conceptual/overview.md)).  
`range_adaptor` is also not exported through any user-facing header.

---

## 🔹 Behavior

| Aspect                | Description                                                                                                                    |
|-----------------------|--------------------------------------------------------------------------------------------------------------------------------|
| **Purpose**           | Converts any duck-typed `jh::concepts::sequence` into a valid `std::ranges::range`.                                            |
| **Iterator model**    | Uses `std::begin()` / `std::end()` deduction; compatible with STL, arrays, pointers, and custom containers.                    |
| **Laziness**          | Provides lazy access — no copying or allocation of underlying elements.                                                        |
| **Const propagation** | If the original sequence supports const iteration, `range_adaptor<const Seq&>` is transparently deduced.                       |
| **Move semantics**    | Fully moveable and forwardable.                                                                                                |
| **Idempotency**       | Wrapping an object that is already a `std::ranges::range` or another `range_adaptor` produces a transparent pass-through type. |

---

## 🔹 Usage

`range_adaptor` is **not** meant for direct use —  
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
> You can construct `range_adaptor` manually,
> but for standard containers (which are already `std::ranges::range`),
> this only rewraps their iterators unnecessarily.  
> `jh::to_range()` automatically detects and avoids redundant wrapping.

---

## 🔹 Integration Notes

* `range_adaptor` is an **internal bridging type**,
  not intended for direct user-level instantiation.  
* It is the **primary mechanism** by which `jh::to_range()` adapts duck-typed sequences.  
* All `jh::ranges::views` adaptors (e.g. [`zip`](views/zip.md), [`enumerate`](views/enumerate.md))
  depend on `range_adaptor` to ensure consistent interoperability.  
* The adaptor is **idempotent** — repeated wrapping automatically collapses into a pass-through type.  
* Avoid explicit `range_adaptor` construction unless writing low-level integration code.

---

### 🧩 **Summary**

`jh::ranges::range_adaptor` defines the **canonical bridge**
between `jh::concepts::sequence` and `std::ranges::range`.  
It is automatically applied through `jh::to_range()`
and ensures that every iterable — STL or custom — can participate
in range pipelines with predictable, zero-overhead behavior.
