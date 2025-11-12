# ⚗️ **JH Toolkit — Meta Module Overview**

📁 **Module:** `<jh/meta>`  
📦 **Namespace:** `jh::meta`  
📍 **Location:** `jh/metax/`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::meta` defines the **Meta-Programming Layer** of JH Toolkit —
a cohesive set of **constexpr-safe compile-time utilities**
for type reflection, tuple manipulation, and deterministic hashing.

All components are header-only, `noexcept`, and **usable in `constexpr` / `consteval` contexts**.  
They form the foundation of JH Toolkit’s **compile-time computation system**,
bridging low-level concepts (type constraints) and high-level reflection mechanisms.

---

## 🌍 Overview

The Meta module provides tools for **static evaluation and type-safe metaprogramming**,  
enabling tuple flattening, ADL-based apply-invocation, and deterministic hashing —
all without relying on the STL at runtime.

| Capability              | Purpose                                                                     |
|-------------------------|-----------------------------------------------------------------------------|
| **Character semantics** | Compile-time ASCII classification and concept constraints (`any_char`).     |
| **Hashing layer**       | Deterministic compile-time string and type hash utilities.                  |
| **Tuple manipulation**  | Flattening and structural proxy interfaces for nested `tuple_like` objects. |
| **ADL apply**           | Universal tuple invocation compatible with user-defined tuple-likes.        |

---

## 🧮 Design Philosophy

* 🔍 **Compile-time determinism** — All operations fully `constexpr` and side-effect-free.
* 💡 **Structural metaprogramming** — Relies on concepts and ADL rather than inheritance.
* 🧱 **Zero overhead** — No heap usage or RTTI; entirely inlined templates.
* ⚙️ **Namespace clarity** — Public headers reside in `jh/metax/`, re-exported through `<jh/meta>`.

---

## 🔹 Core Components

| Component       | Header                       |  Status  | Description                                                                                |
|-----------------|------------------------------|:--------:|--------------------------------------------------------------------------------------------|
| `char`          | `<jh/metax/char.h>`          | ✅ Stable | Character classification and concept `any_char`; foundation for constexpr hashing.         |
| `hash`          | `<jh/metax/hash.h>`          | ✅ Stable | Deterministic compile-time hash algorithms (`fnv1a64`, `xxhash64`, etc.).                  |
| `flatten_proxy` | `<jh/metax/flatten_proxy.h>` | ✅ Stable | Compile-time tuple flattening and proxy wrapper for nested `tuple_like` objects.           |
| `adl_apply`     | `<jh/metax/adl_apply.h>`     | ✅ Stable | ADL-enabled universal `apply` for tuple-like types — drop-in replacement for `std::apply`. |

---

## ⚙️ Aggregation Headers

| Header           | Purpose                                                                                                 |
|------------------|---------------------------------------------------------------------------------------------------------|
| `<jh/meta>`      | Aggregates all metax utilities into the `jh::meta` namespace for user inclusion.                        |
| `<jh/jindallae>` | Poetic alias re-exporting `jh::meta` as `jh::jindallae` — a tribute to elegant meta-programming design. |

---

## 🧩 Module Summary

* `jh::meta` is the **metaprogramming core** of JH Toolkit.
* All utilities are purely compile-time constructs with deterministic behavior.
* The module connects conceptual traits, tuple reflection, and constexpr hashing into a unified system.

---

## 🧭 Navigation

|           Resource           |                                                                       Link                                                                        |
|:----------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------:|
|    🏠 **Back to README**     |                   [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                    |
|     📘 **Go to `char`**      |                [![Go to char Reference](https://img.shields.io/badge/Go%20to%20Char%20Reference-green?style=flat-square)](char.md)                |
|     📗 **Go to `hash`**      |                [![Go to hash Reference](https://img.shields.io/badge/Go%20to%20Hash%20Reference-green?style=flat-square)](hash.md)                |
| 📙 **Go to `flatten_proxy`** | [![Go to flatten\_proxy Reference](https://img.shields.io/badge/Go%20to%20Flatten%20Proxy%20Reference-green?style=flat-square)](flatten_proxy.md) |
|   📘 **Go to `adl_apply`**   |       [![Go to adl\_apply Reference](https://img.shields.io/badge/Go%20to%20ADL%20Apply%20Reference-green?style=flat-square)](adl_apply.md)       |
