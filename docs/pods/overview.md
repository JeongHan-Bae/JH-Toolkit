# 🧊 **JH Toolkit — POD Module Overview**

📁 **Module:** `<jh/pod>`  
📦 **Namespace:** `jh::pod`  
📍 **Location:** `jh/pods/`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::pod` defines the **Plain Old Data layer** of the JH Toolkit —  
a collection of **layout-stable, trivially copyable** types designed
for predictable memory modeling and zero-overhead data handling.

Unlike STL's higher-level abstractions (`std::array`, `std::pair`, etc.),
`jh::pod` types provide full transparency and ABI determinism.  
They are **plain data types** — normally stack-allocated by default,
yet fully compatible with heap-based storage via STL containers or shared pointers.

`jh::pod` ensures:

* Trivial construction, destruction, and copying
* Standard layout and cross-platform binary stability
* No RTTI, metadata, or hidden allocation overhead
* Safe reinterpretation via [`jh::pod::bytes_view`](bytes_view.md) from trivial memory sources

It forms the foundation for **safe binary access**, **memory mapping**,
and **bare-metal compatible structures** with complete compiler-level optimization.

---

## 🌍 Overview

The `jh::pod` module defines a complete system for working with **Plain Old Data (POD)-like types**,  
optimized for **raw memory containers**, **placement-new**, and **low-overhead computation**.

This module enforces memory-safe layout and guarantees:

* Zero-cost construction
* `memcpy`-safe semantics
* Standard layout
* High-performance compatibility with `std::stack`, `arena`, `mmap`, etc.

---

## 🧠 POD Design Philosophy

* 🧱 **POD means full control over memory layout**
* 🧼 No constructors, destructors, virtual tables, or hidden heap allocations
* 🔒 Enables safe serialization, memory mapping, and bare-metal operation
* 💡 All fields known at compile time → layout-stable and tooling-friendly

In essence, `jh::pod` types behave as **modernized C structs** —  
optimized for binary safety, deterministic layout, and predictable performance.

---

## 🔹 Core Components

| Component                       | Header                    |  Status  | Description                                                                                                                          |
|---------------------------------|---------------------------|:--------:|--------------------------------------------------------------------------------------------------------------------------------------|
| [`array<T, N>`](array.md)       | `<jh/pods/array.h>`       | ✅ Stable | Fixed-size POD array — layout-stable and ABI transparent.                                                                            |
| [`bitflags<T>`](bits.md)        | `<jh/pods/bits.h>`        | ✅ Stable | POD-compatible fixed-size bitfield. Provides deterministic bit operations and constexpr-safe manipulation.                           |
| [`bytes_view`](bytes_view.md)   | `<jh/pods/bytes_view.h>`  | ✅ Stable | Zero-copy proxy for trivial memory regions.                                                                                          |
| [`optional<T>`](optional.md)    | `<jh/pods/optional.h>`    | ✅ Stable | POD-safe optional value wrapper without hidden state or RTTI.                                                                        |
| [`pair<T1, T2>`](pair.md)       | `<jh/pods/pair.h>`        | ✅ Stable | Lightweight POD-compatible pair container.                                                                                           |
| [`pod_like`](pod_like.md)       | `<jh/pods/pod_like.h>`    | ✅ Stable | Concept defining trivial, standard-layout types — equivalent to the formal POD rule.                                                 |
| [`span<T>`](span.md)            | `<jh/pods/span.h>`        | ✅ Stable | POD-compatible non-owning view over contiguous memory.                                                                               |
| [`string_view`](string_view.md) | `<jh/pods/string_view.h>` | ✅ Stable | POD-safe, constexpr-compatible UTF-8 string view.                                                                                    |
| [`stringify`](stringify.md)     | `<jh/pods/stringify.h>`   | ✅ Stable | Submodule for human-readable debug printing of POD structures.                                                                       |
| [`tools`](tools.md)             | `<jh/pods/tools.h>`       | ✅ Stable | Contains compile-time macros like `JH_POD_STRUCT` and `JH_ASSERT_POD_LIKE`. <br>Transitional `tuple` has been removed since v1.3.4+. |
| [`tuple<Ts...>`](tuple.md)      | `<jh/pods/tuple.h>`       | ✅ Stable | Variadic compositional POD tuple — supports `make_tuple()` and structured bindings (Clang 15+).                                      |

## ⚙️ Initialization Model

The following table summarizes which `jh::pod` types can be **aggregate-initialized directly**,
and which should be constructed using **factory helpers (`make_*`)** for clarity and portability.

| Type           | Aggregate Initialization     | `make_*` Support  | Recommended Constructor                               | Notes                                                                                           |
|----------------|------------------------------|-------------------|-------------------------------------------------------|-------------------------------------------------------------------------------------------------|
| `array<T, N>`  | ✅                            | —                 | —                                                     | Fully aggregate-initializable (`array<int, 3> a{1,2,3};`).                                      |
| `bitflags<N>`  | ⚠️ (empty only)              | —                 | `jh::pod::from_bytes(array<std::uint8_t, N / 8> arr)` | Aggregate init possible but not recommended — use byte source for clarity.                      |
| `bytes_view`   | ⚠️                           | —                 | `jh::pod::bytes_view::from(const T &obj)`             | Aggregate init legal but discouraged; prefer explicit `from()` for lifetime clarity.            |
| `optional<T>`  | ⚠️ (empty only)              | ✅ `make_optional` | `jh::pod::make_optional(const T &value)`              | Empty optional can be aggregate-initialized; prefer `make_optional()` for value initialization. |
| `pair<T1, T2>` | ✅                            | ✅ `make_pair`     | — / `jh::pod::make_pair(a, b)`                        | Pure aggregate type; `make_pair()` provides readable construction.                              |
| `tuple<Ts...>` | ✅ (Clang 15+)  ⚠️ (GCC ≤ 13) | ✅ `make_tuple`    | `jh::pod::make_tuple(v1, v2, ...)`                    | Clang 15+ supports direct `{}` initialization; GCC ≤ 13 may require `make_tuple()`.             |
| `span<T>`      | ✅                            | —                 | —                                                     | Aggregate (`{ptr, size}`) or constructed via `to_span()` for containers.                        |
| `string_view`  | ✅                            | —                 | — / `from_literal()`                                  | Aggregate init valid; `from_literal()` preferred for string literals.                           |

> 🧩 **Notes**
>
> * `make_*` helpers exist purely for **clarity and cross-compiler consistency** —
>   they are `constexpr` and add no runtime cost.  
> * For compilers older than Clang 15 or GCC 13,
>   `jh::pod::make_tuple()` ensures portable initialization syntax.  
> * `pair` and `optional` helpers match STL naming for familiarity,
>   but remain pure POD factories — zero constructors or hidden state. 

---

## 🧩 Module Summary

* `jh::pod` provides a **layout-deterministic, ABI-stable** system
  for memory-safe Plain Old Data objects.  
* It is the **foundation for binary I/O and low-level data handling**
  throughout the JH Toolkit.  
* The module is **functionally complete** —  
  no major structural extensions are planned beyond 1.4.0.

---

## 🧭 Navigation

|          Resource          |                                                                    Link                                                                     |
|:--------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------:|
|    🏠**Back to README**    |                [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                 |
|    📘 **Go to `array`**    |           [![Go to array Reference](https://img.shields.io/badge/Go%20to%20Array%20Reference-green?style=flat-square)](array.md)            |
|  📗 **Go to `bitflags`**   |           [![Go to bits Reference](https://img.shields.io/badge/Go%20to%20Bitflags%20Reference-green?style=flat-square)](bits.md)           |
| 📙 **Go to `bytes_view`**  |  [![Go to bytes\_view Reference](https://img.shields.io/badge/Go%20to%20Bytes%20View%20Reference-green?style=flat-square)](bytes_view.md)   |
|  📘 **Go to `optional`**   |       [![Go to optional Reference](https://img.shields.io/badge/Go%20to%20Optional%20Reference-green?style=flat-square)](optional.md)       |
|    📗 **Go to `pair`**     |             [![Go to pair Reference](https://img.shields.io/badge/Go%20to%20Pair%20Reference-green?style=flat-square)](pair.md)             |
|  📙 **Go to `pod_like`**   |     [![Go to pod\_like Reference](https://img.shields.io/badge/Go%20to%20Pod%20Like%20Reference-green?style=flat-square)](pod_like.md)      |
|    📘 **Go to `span`**     |             [![Go to span Reference](https://img.shields.io/badge/Go%20to%20Span%20Reference-green?style=flat-square)](span.md)             |
| 📗 **Go to `string_view`** | [![Go to string\_view Reference](https://img.shields.io/badge/Go%20to%20String%20View%20Reference-green?style=flat-square)](string_view.md) |
|  📙 **Go to `stringify`**  |     [![Go to stringify Reference](https://img.shields.io/badge/Go%20to%20Stringify%20Reference-green?style=flat-square)](stringify.md)      |
|    📘 **Go to `tools`**    |           [![Go to tools Reference](https://img.shields.io/badge/Go%20to%20Tools%20Reference-green?style=flat-square)](tools.md)            |
|    📗 **Go to `tuple`**    |           [![Go to tuple Reference](https://img.shields.io/badge/Go%20to%20Tuple%20Reference-green?style=flat-square)](tuple.md)            |

---

> 📌 **Target platform:** 64-bit only — all POD types use fixed-width integers (`std::uint*_t`)
> for layout determinism and cross-platform ABI stability.  
>
> 💡 In practice, **two conventions coexist by design**:
>
> * For **actual data length and storage fields**, JH Toolkit uses
>   fixed-size integers (`uint32_t`, `uint64_t`) to ensure binary stability
>   and precise control over serialized or mapped layouts.  
> * For **generic interfaces, structured bindings, and STL interop**,
>   types such as `jh::pod::tuple` and related utilities
>   use `std::size_t` to match the expectations of standard algorithms and
>   trait-based deduction (`tuple_size`, `tuple_element`, ranges, etc.).  
>
> Although `std::size_t` and `uint64_t` are both 64-bit on target platforms,
> they have distinct type identities (`unsigned long` vs `unsigned long long`),
> which affects only overload resolution — not layout or ABI.  
>
> All buffer sizes and POD aggregates remain **statically bounded**;
> any oversize or non-trivial instantiation triggers a compile-time concept failure.