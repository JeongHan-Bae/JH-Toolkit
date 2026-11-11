# ⚗️ **JH-Toolkit — Meta Module Overview**

📁 **Module:** `<jh/meta>`  
📦 **Namespace:** `jh::meta`  
📍 **Location:** `jh/metax/`  
📅 **Version:** **1.4.x** (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

<div align="center" style="margin-top: -32px;">
  <img src="https://raw.githubusercontent.com/JeongHan-Bae/JH-Toolkit/main/docs/img/Jindallae.svg"
       alt="Jindallae mascot"
       width=96px;>
</div>

---

## 🧭 Introduction

`jh::meta` defines the **compile-time metaprogramming core** of JH-Toolkit.

It provides a cohesive set of **constexpr / consteval–safe utilities** for:

* structural type reflection
* NTTP-based string identity
* deterministic hashing
* tuple-like structural manipulation
* compile-time data tables and ADT validation

All components are **header-only**, **heap-free**, **RTTI-free**, and designed to be
**fully evaluable during compilation**.

Starting from **1.4.x**, `jh::meta` evolves from a *pure helper layer* into a
**foundational compile-time infrastructure**, supporting string-driven NTTP design,
static registries, and closed-world algebraic modeling.

---

## 🌍 High-Level Capabilities

The Meta module enables **deterministic static computation** without relying on
runtime STL facilities.

| Capability                   | Purpose                                                                  |
|------------------------------|--------------------------------------------------------------------------|
| **Character semantics**      | Compile-time ASCII classification and constraints (`any_char`).          |
| **Compile-time hashing**     | Deterministic, platform-stable hash algorithms.                          |
| **Template string literals** | `TStr` / `t_str` — string literals as NTTPs with validation and hashing. |
| **Tuple manipulation**       | Recursive flattening and structural tuple proxies.                       |
| **ADL-based invocation**     | Universal `apply` for user-defined tuple-like types.                     |
| **Compile-time codecs**      | Base64 / Base64URL encode–decode at compile time.                        |
| **Static lookup tables**     | Fixed-capacity hash-dispatch maps (`lookup_map`).                        |
| **Closed ADT validation**    | Variant-wide invariant checks and transformations.                       |

---

## 🪼 Design Philosophy (1.4.x)

* 🔍 **Compile-time determinism**
  All observable behavior is defined at template instantiation time.

* 🧱 **Structural, not hierarchical**
  Relies on concepts, NTTPs, and ADL — not inheritance or virtual dispatch.

* 🧬 **Closed-world semantics**
  Variants, tables, and registries define their full domain statically.

* ⚙️ **Zero runtime cost**
  No allocation, no RTTI, no hidden initialization; templates inline completely.

* 🧭 **Explicit abstraction boundaries**
  Meta utilities describe *what exists* at compile time, not *how to extend it at runtime*.

---

## 🔹 Core Components (1.4.x)

| Component       | Header                       |  Status  | Description                                                           |
|-----------------|------------------------------|:--------:|-----------------------------------------------------------------------|
| `char`          | `<jh/metax/char.h>`          | ✅ Stable | Character classification and `any_char` concept.                      |
| `hash`          | `<jh/metax/hash.h>`          | ✅ Stable | Deterministic constexpr hash algorithms (FNV, Murmur, xxHash, …).     |
| `t_str / TStr`  | `<jh/metax/t_str.h>`         | ✅ Stable | Compile-time string literals as NTTPs with validation and hashing.    |
| `base64`        | `<jh/metax/base64.h>`        | ✅ Stable | Compile-time Base64 / Base64URL encode & decode.                      |
| `flatten_proxy` | `<jh/metax/flatten_proxy.h>` | ✅ Stable | Recursive flattening of nested `tuple_like` structures.               |
| `adl_apply`     | `<jh/metax/adl_apply.h>`     | ✅ Stable | ADL-enabled universal tuple invocation (`std::apply` generalization). |
| `lookup_map`    | `<jh/metax/lookup_map.h>`    | ✅ Stable | Fixed-size, hash-sorted static dispatch table (constexpr-capable).    |
| `variant_adt`   | `<jh/metax/variant_adt.h>`   | ✅ Stable | Closed ADT validation and transformation for `std::variant`.          |

---

## 🔹 Aggregation Headers

| Header           | Purpose                                                                                |
|------------------|----------------------------------------------------------------------------------------|
| `<jh/meta>`      | Aggregates all `metax` utilities into the `jh::meta` namespace.                        |
| `<jh/jindallae>` | Poetic alias re-exporting `jh::meta` as `jh::jindallae` (compile-time design tribute). |

---

## 🧭 Navigation

| Resource                     |                                                                       Link                                                                        |
|------------------------------|:-------------------------------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**        |                   [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                    |
| 📘 **Go to `char`**          |                [![Go to char Reference](https://img.shields.io/badge/Go%20to%20Char%20Reference-green?style=flat-square)](char.md)                |
| 📗 **Go to `hash`**          |                [![Go to hash Reference](https://img.shields.io/badge/Go%20to%20Hash%20Reference-green?style=flat-square)](hash.md)                |
| 📙 **Go to `t_str`**         |              [![Go to t\_str Reference](https://img.shields.io/badge/Go%20to%20TStr%20Reference-green?style=flat-square)](t_str.md)               |
| 📘 **Go to `base64`**        |             [![Go to Base64 Reference](https://img.shields.io/badge/Go%20to%20Base64%20Reference-green?style=flat-square)](base64.md)             |
| 📗 **Go to `flatten_proxy`** | [![Go to flatten\_proxy Reference](https://img.shields.io/badge/Go%20to%20Flatten%20Proxy%20Reference-green?style=flat-square)](flatten_proxy.md) |
| 📙 **Go to `adl_apply`**     |       [![Go to adl\_apply Reference](https://img.shields.io/badge/Go%20to%20ADL%20Apply%20Reference-green?style=flat-square)](adl_apply.md)       |
| 📘 **Go to `lookup_map`**    |     [![Go to lookup\_map Reference](https://img.shields.io/badge/Go%20to%20Lookup%20Map%20Reference-green?style=flat-square)](lookup_map.md)      |
| 📗 **Go to `variant_adt`**   |    [![Go to variant\_adt Reference](https://img.shields.io/badge/Go%20to%20Variant%20ADT%20Reference-green?style=flat-square)](variant_adt.md)    |
