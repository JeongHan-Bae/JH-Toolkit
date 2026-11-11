# 🌗 **JH-Toolkit — `jh::ranges::adapt` API Reference**

📁 **Header:** `<jh/ranges/adapt.h>`  
📦 **Namespace:** `jh::ranges`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](range_ext.md)

</div>

---

## 🧭 Introduction

`jh::ranges::adapt` is a **promotion adaptor** that converts any object
modeling [`jh::concepts::sequence`](../conceptual/sequence.md)
into a valid [`std::ranges::viewable_range`](https://en.cppreference.com/w/cpp/ranges/viewable_range).

It serves as a **pipeline-friendly wrapper** around
[`jh::to_range()`](../conceptual/sequence.md#jhto_rangeseq),
bridging user-defined or non-standard "sequence-like" objects with
the C++ standard ranges infrastructure.

The result is usable in range pipelines. A directly usable lvalue range stays
borrowed; an rvalue produces an owned result. Repeatability depends on the
source sequence honoring its non-consuming contract.

---

## 🔹 Definition

```cpp
namespace jh::ranges {

inline constexpr detail::adapt_fn adapt{};

}
```

---

## 🔹 Interface

### 1. Direct form

```cpp
template <jh::concepts::sequence Seq>
constexpr decltype(auto) adapt(Seq&& seq);
```

### 2. Pipe form

```cpp
constexpr auto adapt();
```

### Equivalent usage

```cpp
auto&& r1 = jh::ranges::adapt(seq);
auto&& r2 = seq | jh::ranges::adapt();
```

---

## 🔹 Description

`adapt()` promotes an arbitrary `jh::concepts::sequence`
into a range type that satisfies `std::ranges::viewable_range`.

`adapt()` does not iterate over elements during conversion. Its public call
operators preserve the reference returned for a directly usable lvalue range.
For an rvalue, the result is materialized as an owned value so the adaptor does
not return a reference to an expired argument. A non-movable lvalue source can
instead produce a `subrange` of its iterators; that source must outlive the
subrange.

Temporary `std::initializer_list` inputs are rejected by `jh::to_range()`;
their backing array would not survive a copied initializer-list handle.
A copied or moved non-owning view still refers to its original storage; that
storage must remain alive while the adapted result is used.

Internally, it delegates to [`jh::to_range()`](../conceptual/sequence.md#jhto_rangeseq),
which selects the minimal and semantically transparent transformation required
to make the input range-compliant.

---

## 🔹 Behavior

The behavior of `adapt()` exactly mirrors [`jh::to_range()`](../conceptual/sequence.md#jhto_rangeseq),
which classifies sequences into three categories:

| Case | Input Category                                                       | Transformation                | Description                                                                                                                          |
|------|----------------------------------------------------------------------|-------------------------------|--------------------------------------------------------------------------------------------------------------------------------------|
| ①    | **Standard sequence range (copy/move capable)**                      | Reference or owned value      | An lvalue remains borrowed; an rvalue is moved or copied into the result.                                                               |
| ②    | **Standard reentrant range (restricted: move-only / stable-holder)** | `std::ranges::subrange` proxy | Iterator/sentinel pair is valid, but type cannot satisfy `viewable_range` due to ownership or lifetime; wrapped as `subrange`.       |
| ③    | **Sequence-only type (non-range)**                                   | `jh::ranges::range_adaptor`   | Object models `jh::concepts::sequence` but not `std::ranges::range`; promoted to a compliant wrapper preserving iteration semantics. |

The branch is selected at compile time. An owned result can still incur a move
or copy, depending on the input type.

---

## 🔹 Design Semantics

`jh::ranges::adapt` exposes `jh::to_range()` through direct and pipe calls.
Callers must account for borrowed lifetimes and rvalue ownership according to
the selected branch.

* Delegates dispatch and structure detection entirely to `jh::to_range()`.
* Introduces pipe syntax (`seq | adapt()`) for uniform pipeline integration.
* Performs compile-time classification of sequence traits.
* Restores `viewable_range` compliance for non-copyable or holder-based types.
* Preserves identity for a directly usable lvalue range.

`adapt()` thus acts as the **gateway** into the entire JH Ranges system,
ensuring any conforming sequence becomes a valid, lazily traversable range.

---

## 🔹 Integration Notes

* `adapt()` is the pipeline equivalent of `jh::to_range()`.
* Higher-level adaptors such as `views::zip` may call `jh::to_range()` directly.
* Retaining an adapted lvalue result requires the source to remain alive.
* Included automatically by `<jh/ranges_ext>` or via `<jh/ranges/adapt.h>`.

---

## 🧩 Summary

`jh::ranges::adapt` is a **fundamental promotion adaptor**
that bridges JH `sequence` semantics to standard `range` semantics.  
It transparently wraps or forwards its input to ensure compliance with
`std::ranges::viewable_range`, preserving lvalue source identity where the
underlying range can be borrowed directly.

> **In short:**  
> `jh::ranges::adapt()` = `jh::to_range()` + pipe support
> → supplies a viewable range under the sequence's repeatability contract.
