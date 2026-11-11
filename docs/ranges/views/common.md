# 🔭 **JH-Toolkit — `jh::ranges::views::common` API Reference**

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

Its result models **common_range**: begin and end have the same type.
The adaptor may borrow an lvalue source or own an rvalue source according to
the selected view construction path. It does not materialize range elements.

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

The implementation has four dispatch branches:

| Input | Conversion | Result |
|-------|------------|--------|
| `jh::concepts::sequence` whose converted view is already common | `jh::to_range()` then `std::views::all` | The converted view |
| `jh::concepts::sequence` whose converted view has a distinct sentinel | `jh::to_range()` then `std::views::all` | `std::ranges::common_view` |
| Standard range outside the Sequence concept accepted by `std::views::common` | `std::views::common()` | The standard adaptor's result |
| Single-pass standard range with a non-copyable iterator | JH shared cursor wrapper | A common input range whose iterator copies share one position |

`jh::to_range()` may use a borrowed subrange for a restricted lvalue source.
The source must remain alive while that view is used. The shared cursor path
allocates cursor state on each `begin()` call; it keeps single-pass semantics
and does not make the source reentrant.

---

## 🔹 Design Semantics

`jh::ranges::views::common` accepts JH sequences and standard ranges through
one entry point.

It provides:

* Compatibility with **standard ranges** and all **custom or third-party types**
  that model [`jh::concepts::sequence`](../../conceptual/sequence.md).
* Automatic **promotion** or **proxy wrapping** for structurally incomplete or restricted types.
* Preservation of the source's repeatability contract when the conversion
  and caller-supplied sequence satisfy it.
* Full **pipeline-form support** under **libc++**,
  which does not provide a pipe overload for `std::views::common()`.

For a converted Sequence view that is already common, JH returns the view
selected by `std::views::all`. The standard adaptor also avoids adding a
`common_view` when its input is already common. The JH extension is the
Sequence conversion before that standard normalization.

---

## 🔹 Description

An `istream_view` is an example of a single-pass standard range outside the
Sequence contract; its move-only iterator uses the JH shared cursor path.
An `iota_view` supports repeatable iteration and is not a one-shot stream.

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

    // Zip of two vectors is already common in the C++20 fallback
    auto zipped = a | jh::ranges::views::zip_pipe(b);
    static_assert(std::ranges::common_range<decltype(zipped)>);

    // This enumerate view has a distinct end sentinel
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

`common()` uses [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq)
for Sequence inputs and then supplies a common range. The
[`adapt`](../adapt.md) adaptor exposes `to_range()` without the common-range
normalization step.

While all three adaptors serve to normalize input objects into range-compatible forms,
their scopes differ:

| Adaptor                                                        | Role                              | Input Domain                                                                  | Output Guarantee            | Behavior                                                                                         |
|----------------------------------------------------------------|-----------------------------------|-------------------------------------------------------------------------------|-----------------------------|--------------------------------------------------------------------------------------------------|
| [`jh::to_range()`](../../conceptual/sequence.md#jhto_rangeseq) | **Sequence conversion** | Types modeling `jh::concepts::sequence` | `std::ranges::range` | Converts a Sequence to a standard range representation. |
| [`jh::ranges::adapt`](../adapt.md) | **Direct and pipe entry point** | Same Sequence domain | `std::ranges::viewable_range` | Exposes conversion for direct and pipe calls. |
| `jh::ranges::views::common()` | **Common-range normalization** | Sequences and standard ranges | `std::ranges::common_range` | Uses Sequence conversion, the standard adaptor, or a shared single-pass cursor. |

For a standard input range that does not model Sequence, `common()` selects
the standard adaptor when it can construct the result and otherwise uses the
shared cursor wrapper. For a Sequence, it applies `jh::to_range()` and adds
`common_view` only when the converted view needs it.

In other words:

> `adapt()` → performs **promotion** (sequence → range)
>
> `common()` → performs **promotion + normalization** (sequence ∣ range → common_range)

This makes `common()` the entry point when a pipeline specifically needs
matching begin and end types.


---

## 🔹 Integration Notes

* Lvalue-backed results require the source to outlive the view.
* An already-common converted view does not gain another `common_view` layer.
* Supports all view adaptors (`zip`, `enumerate`, `flatten`, etc.).
* Provides missing pipe overload for `std::views::common()` in libc++.
* Included by `<jh/views>` or directly via `<jh/ranges/views/common.h>`.

---

## 🧩 Summary

`jh::ranges::views::common` accepts Sequence and standard range inputs and
returns a common range. Its Sequence branch uses `jh::to_range()` before
normalizing the end type.

This adaptor also corrects the standard library's portability gap
by restoring pipe syntax under libc++,
ensuring consistent behavior and syntax across all toolchains.
