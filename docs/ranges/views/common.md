# 🔭 **JH Toolkit — `jh::ranges::views::common` API Reference**

📁 **Header:** `<jh/ranges/views/common.h>`  
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

`jh::ranges::views::common` is a **unified normalization adaptor**
for both [`std::ranges::range`](https://en.cppreference.com/w/cpp/ranges/range)
and [`jh::concepts::sequence`](../../conceptual/sequence.md) types.

It extends `std::views::common` by providing
a complete bridge across **standard**, **third-party**,
and **semantically constrained** iterable types.

Its purpose is to guarantee that all pipeline components
operate on **common ranges** — i.e., those with identical iterator and sentinel types —
while preserving each type's native ownership and traversal semantics.

No copies, allocations, or eager transformations are performed.

---

## 🔹 Definition

```cpp
namespace jh::ranges::views {

inline constexpr detail::common_fn common{};

}
```

---

## 🔹 Interface

1. **Direct form**

   ```cpp
   template <typename R>
   requires (jh::concepts::sequence<R> || std::ranges::range<R>)
   constexpr auto common(R&& range);
   ```

2. **Pipe form**

   ```cpp
   constexpr auto common();
   ```

Equivalent usage:

```cpp
auto v1 = jh::ranges::views::common(r);
auto v2 = r | jh::ranges::views::common();
```

---

## 🔹 Dispatch Model

`common()` inspects the structural and semantic traits of its input `R`,
and dispatches to one of **five** normalization paths:

|   | Category                                             | Description                                                                        | Action                                                                                                                                                                              |
|---|------------------------------------------------------|------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| ① | Already `common_range`                               | Iterator and sentinel types are identical.                                         | Returned directly (transparent pass-through).                                                                                                                                       |
| ② | Fully reentrant and `std::views::all`–compatible     | Copy/move–capable reentrant ranges or sequences.                                   | Forwarded transparently via `jh::to_range()` → `std::views::all` → `std::ranges::common_view`.                                                                                      |
| ③ | Reentrant but restricted (move-only / stable-holder) | Copy/move incomplete types that must not transfer ownership.                       | Wrapped into `std::ranges::subrange`, then passed through `std::views::all` → `std::ranges::common_view`.                                                                           |
| ④ | Sequence-only (not a range)                          | Duck-typed iterable meeting `jh::concepts::sequence` but not `std::ranges::range`. | Promoted to compliant range via [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq) <br> → `jh::ranges::range_wrapper` → `std::views::all` → `std::ranges::common_view`. |
| ⑤ | Consuming range (non-reentrant)                      | Single-pass generators such as `iota`, `enumerate`, etc.                           | Forwarded directly to `std::views::common()` (standard consumptive path).                                                                                                           |

Each normalization path guarantees **iterator compatibility**,
**no lifetime disruption**, and **semantic transparency**.

---

## 🔹 Design Semantics

`jh::ranges::views::common` acts as a **semantic bridge**,
not merely a wrapper around `std::views::common`.

It provides:

* Compatibility with **standard ranges** and all **custom or third-party types**
  that model [`jh::concepts::sequence`](../../conceptual/sequence.md).
* Automatic **promotion** or **proxy wrapping** for structurally incomplete or restricted types.
* Preservation of **non-consuming** reentrancy semantics when applicable.
* Full **pipeline-form support** under **libc++**,
  which does not provide a pipe overload for `std::views::common()`.

In contrast to the standard adaptor,
which unconditionally wraps its input in a `common_view` —
even when the source already models `common_range` —
the JH implementation performs **compile-time transparent forwarding**.  
This eliminates redundant view layers,
reduces template instantiation depth,
and ensures that already-conforming types remain type-identical after adaptation.

Hence, `common()` serves as a **unifying and compilation-friendly adaptor**
that allows all iterable types — whether STL-compliant or user-defined —
to enter the same range pipeline
without semantic noise or performance loss.

---

## 🔹 Description

Normalization logic proceeds as follows:

1. **Already common**  
   If `R` models `std::ranges::common_range`, it is returned unchanged.

2. **Fully reentrant (copy/move complete)**  
   If `R` is compatible with `std::views::all`,
   it is transparently forwarded through `jh::to_range()` to `std::views::all`.

3. **Reentrant but restricted**  
   If the type is stable-holding, move-only, or semantically non-transferable,
   it is proxied as `std::ranges::subrange(begin(r), end(r))`
   and then passed through `std::views::all`.

4. **Sequence-only types**  
   Non-range iterables are promoted to standard-compliant ranges via `jh::to_range()`,
   which produces a `jh::ranges::range_wrapper` preserving all observable semantics.

5. **Consuming ranges**  
   One-shot streams (like `std::views::iota`, `enumerate`, `istream_view`)
   are directly normalized via `std::views::common()`.

No copies or allocations are made unless required for iterator unification.

---

## 🔹 Example

```cpp
#include <jh/ranges/views/common.h>
#include <jh/ranges/views/enumerate.h>
#include <jh/ranges/views/zip.h>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {10, 20, 30};

    // Case ①+②: zip of two vectors — already common
    auto zipped = a | jh::ranges::views::zip_pipe(b);
    static_assert(std::ranges::common_range<decltype(zipped)>);

    // Case ⑤: enumerate produces non-common view (iterator != sentinel)
    auto enumerated = jh::ranges::views::enumerate(a, 1);
    static_assert(!std::ranges::common_range<decltype(enumerated)>);

    // Normalize enumerate() output
    auto normalized = enumerated | jh::ranges::views::common();
    static_assert(std::ranges::common_range<decltype(normalized)>);

    for (auto [i, v] : normalized)
        std::cout << i << ": " << v << "\n";
}
```

**Output:**

```
1: 1
2: 2
3: 3
```

---

## 🔹 Integration with `adapt` and `to_range`

`jh::ranges::views::common()` is a **superset adaptor** of
[`jh::ranges::adapt`](../adapt.md) and [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq).

While all three adaptors serve to normalize input objects into range-compatible forms,
their scopes differ:

| Adaptor                                                        | Role                              | Input Domain                                                                  | Output Guarantee            | Behavior                                                                                         |
|----------------------------------------------------------------|-----------------------------------|-------------------------------------------------------------------------------|-----------------------------|--------------------------------------------------------------------------------------------------|
| [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq) | **Fundamental promotion**         | Types modeling `jh::concepts::sequence` — including reentrant standard ranges | `std::ranges::view`         | Converts a sequence to a valid non-consuming range representation.                               |
| [`jh::ranges::adapt`](../adapt.md)                             | **Pipeline form of `to_range()`** | Same as `to_range()` (duck-typed or standard reentrant sequences)             | `std::ranges::view`         | Provides pipe syntax for promotion (<code>adapt(seq)</code> or <code>seq &#124; adapt()</code>). |
| `jh::ranges::views::common()`                                  | **Upgraded unifying adaptor**     | All sequences **plus** consuming (single-pass) ranges                         | `std::ranges::common_range` | Accepts both reentrant and consuming ranges, preserving or enforcing common-range semantics.     |

Unlike `adapt()`, which only *elevates* a `sequence` into a range,
`common()` extends that behavior to cover **all iterable categories**,
including *consuming* ranges.

It automatically chooses the minimal transformation necessary:

* Directly forwards already compliant `common_range`s.
* Promotes plain `sequence`s via `jh::to_range()`.
* Proxies restricted non-copyable or non-movable ranges as `std::ranges::subrange`.
* Invokes `std::views::common()` for consuming or single-pass sources.

In other words:

> `adapt()` → performs **promotion** (sequence → range)
>
> `common()` → performs **promotion + normalization** (sequence ∣ range → common_range)

This makes `jh::ranges::views::common()` the canonical adaptor
for entering the JH Toolkit range pipeline with complete semantic coverage.


---

## 🔹 Integration Notes

* Transparent adaptor — does not alter ownership or lifetime.
* Fully idempotent: applying `common()` repeatedly yields identical results.
* Supports all view adaptors (`zip`, `enumerate`, `flatten`, etc.).
* Provides missing pipe overload for `std::views::common()` in libc++.
* Included by `<jh/views>` or directly via `<jh/ranges/views/common.h>`.

---

## 🧩 Summary

`jh::ranges::views::common` provides a **complete and precise normalization layer**
between C++ standard ranges and extended JH Toolkit `sequence` concepts.

It covers five distinct input categories — from STL-compliant ranges
to constrained holders and duck-typed iterables —
bridging them into a unified, reentrant, and pipeline-ready model.

This adaptor also corrects the standard library's portability gap
by restoring pipe syntax under libc++,
ensuring consistent behavior and syntax across all toolchains.
