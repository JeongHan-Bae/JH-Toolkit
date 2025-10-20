# 🧊 **JH Toolkit — POD Module Overview**

📁 **Module:** `<jh/pod>`  
📦 **Namespace:** `jh::pod`  
📍 **Location:** `jh/pods/`  
📅 **Version:** 1.3.3+ → 1.4.0-dev (2025)  
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

| Component                       | Header                    |     Status      | Description                                                                                                                             |
|---------------------------------|---------------------------|:---------------:|-----------------------------------------------------------------------------------------------------------------------------------------|
| [`array<T, N>`](array.md)       | `<jh/pods/array.h>`       |    ✅ Stable     | Fixed-size POD array — layout-stable and ABI transparent.                                                                               |
| [`bitflags<T>`](bits.md)        | `<jh/pods/bits.h>`        |    ⚙️ Minor     | POD-compatible fixed-size bitfield. A minor internal refactor is planned in the next version (no API changes).                          |
| [`bytes_view`](bytes_view.md)   | `<jh/pods/bytes_view.h>`  |    ✅ Stable     | Zero-copy proxy for trivial memory regions.                                                                                             |
| [`optional<T>`](optional.md)    | `<jh/pods/optional.h>`    |    ✅ Stable     | POD-safe optional value wrapper without hidden state or RTTI.                                                                           |
| [`pair<T1, T2>`](pair.md)       | `<jh/pods/pair.h>`        |    ✅ Stable     | Lightweight POD-compatible pair container.                                                                                              |
| [`pod_like`](pod_like.md)       | `<jh/pods/pod_like.h>`    |    ✅ Stable     | Concept defining trivial, standard-layout types — equivalent to the formal POD rule.                                                    |
| [`span<T>`](span.md)            | `<jh/pods/span.h>`        |    ✅ Stable     | POD-compatible non-owning view over contiguous memory.                                                                                  |
| [`string_view`](string_view.md) | `<jh/pods/string_view.h>` |    ✅ Stable     | POD-safe, constexpr-compatible UTF-8 string view.                                                                                       |
| [`stringify`](stringify.md)     | `<jh/pods/stringify.h>`   |    ✅ Stable     | Submodule for human-readable debug printing of POD structures.                                                                          |
| [`tools`](tools.md)             | `<jh/pods/tools.h>`       | ⚠️ Transitional | Contains compile-time POD macros and transitional helpers. `tuple` part is under redesign and may move to `tuple.h` in a later version. |

---

## ⚙️ Initialization Model

The following table summarizes which `jh::pod` types can be **aggregate-initialized directly**,
and which should be constructed using **factory helpers or static functions** for clarity and safety.

| Type           | Aggregate Initialization | Recommended Constructor                                                                                    | Notes                                                                                   |
|----------------|--------------------------|------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------|
| `array<T, N>`  | ✅                        | —                                                                                                          | Fully aggregate-initializable (`array<int, 3> a{1,2,3};`).                              |
| `bitflags<N>`  | ⚠️                       | `jh::pod::from_bytes(array<std::uint8_t, N / 8> arr)`                                                      | Aggregate init possible but not recommended — use byte source for clarity.              |
| `bytes_view`   | ⚠️                       | `jh::pod::bytes_view::from(const T &obj)`<br>`jh::pod::bytes_view::from(const T *arr, std::uint64_t size)` | Aggregate init legal but discouraged; semantic meaning requires explicit `from()` call. |
| `optional<T>`  | ⚠️ (empty only)          | `jh::pod::make_optional(const T &value)`                                                                   | Empty optional can be aggregate-initialized; prefer `make_optional()` for values.       |
| `pair<T1, T2>` | ✅                        | —                                                                                                          | Pure aggregate type (`pair<int,int> p{1,2};`).                                          |
| `span<T>`      | ✅                        | —                                                                                                          | Aggregate (`{ptr, size}`) or constructed via `to_span()` for containers.                |
| `string_view`  | ✅                        | —                                                                                                          | Aggregate init valid; prefer `from_literal()` for string literals.                      |

> 🧩 **Note:**
> Aggregate initialization (`{...}`) is supported only for types that represent **plain data views**
> or statically sized objects. Types involving runtime semantics (`bytes_view`, `optional<T>`)
> have factory helpers to express ownership and lifetime intent more clearly.

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

---

> 📌 **Target platform:** 64-bit only — `std::size_t` is avoided in favor of fixed-size types (`std::uint*_t`) for layout
> clarity and deterministic memory modeling.  
> 💡 `jh::pod` types use fixed-size integers like `uint32_t` / `uint64_t` in place of `size_t`.  
> This ensures consistent layout across all platforms and reflects our 64-bit-only design.  
> All buffer sizes are explicitly bounded — oversize instantiations fail at compile time via concept checks.
