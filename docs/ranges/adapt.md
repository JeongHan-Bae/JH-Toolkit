# 🌗 **JH Toolkit — `jh::ranges::adapt` API Reference**

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
bridging user-defined or non-standard “sequence-like” objects with
the C++ standard ranges infrastructure.

This adaptor provides **uniform entry semantics** for any sequence
into a standard-compliant, viewable, and reentrant range form.

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
constexpr auto adapt(Seq&& seq);
```

### 2. Pipe form

```cpp
constexpr auto adapt();
```

### Equivalent usage

```cpp
auto r1 = jh::ranges::adapt(seq);
auto r2 = seq | jh::ranges::adapt();
```

---

## 🔹 Description

`adapt()` promotes an arbitrary `jh::concepts::sequence`
into a range type that satisfies `std::ranges::viewable_range`.

It performs **no eager operations**, **no allocations**,
and preserves both the ownership and traversal semantics of the original object.

Internally, it delegates to [`jh::to_range()`](../conceptual/sequence.md#jhto_rangeseq),
which selects the minimal and semantically transparent transformation required
to make the input range-compliant.

---

## 🔹 Behavior

The behavior of `adapt()` exactly mirrors [`jh::to_range()`](../conceptual/sequence.md#jhto_rangeseq),
which classifies sequences into three categories:

| Case | Input Category                                                       | Transformation                | Description                                                                                                                          |
|------|----------------------------------------------------------------------|-------------------------------|--------------------------------------------------------------------------------------------------------------------------------------|
| ①    | **Standard reentrant range (copy/move capable)**                     | Transparent forwarding        | Already models `std::ranges::range` and is viewable; forwarded directly without wrapping.                                            |
| ②    | **Standard reentrant range (restricted: move-only / stable-holder)** | `std::ranges::subrange` proxy | Iterator/sentinel pair is valid, but type cannot satisfy `viewable_range` due to ownership or lifetime; wrapped as `subrange`.       |
| ③    | **Sequence-only type (non-range)**                                   | `jh::ranges::range_adaptor`   | Object models `jh::concepts::sequence` but not `std::ranges::range`; promoted to a compliant wrapper preserving iteration semantics. |

Every valid `jh::concepts::sequence` falls into one of these three paths —
there is **no undefined case** or runtime overhead.

---

## 🔹 Design Semantics

`jh::ranges::adapt` provides **promotion semantics** only —
it guarantees that all `sequence` types can enter the standard range pipeline
without type conflicts or reentrancy loss.

* Delegates dispatch and structure detection entirely to `jh::to_range()`.
* Introduces pipe syntax (`seq | adapt()`) for uniform pipeline integration.
* Performs compile-time classification of sequence traits — no runtime penalty.
* Restores `viewable_range` compliance for non-copyable or holder-based types.
* Fully transparent for already conforming reentrant ranges.

`adapt()` thus acts as the **gateway** into the entire JH Ranges system,
ensuring any conforming sequence becomes a valid, lazily traversable range.

---

## 🔹 Integration Notes

* `adapt()` is the pipeline equivalent of `jh::to_range()`.
* Required by higher-level adaptors like `views::common`, `views::zip`, etc.
* Fully idempotent — applying it multiple times yields the same result.
* Included automatically by `<jh/ranges_ext>` or via `<jh/ranges/adapt.h>`.

---

## 🧩 Summary

`jh::ranges::adapt` is a **fundamental promotion adaptor**
that bridges JH `sequence` semantics to standard `range` semantics.  
It transparently wraps or forwards its input to ensure compliance with
`std::ranges::viewable_range`, preserving all observable behavior.

> **In short:**  
> `jh::ranges::adapt()` = `jh::to_range()` + pipe support
> → safely normalizes any `sequence` into a viewable, reentrant range.
