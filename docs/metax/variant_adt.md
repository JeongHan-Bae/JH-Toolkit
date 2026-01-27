# ⚗️ JH Toolkit — `jh::meta::variant_adt` Usage Guide

📁 **Header:** `<jh/metax/variant_adt.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`


<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## Overview

`jh::meta::variant_adt` is a compile-time utility layer that treats `std::variant` as a **closed Algebraic Data Type (
ADT)**.

It provides:

* **variant-wide static validation** (`check_all`)
* **compile-time type transformations** (`variant_transform_t`)
* **uniformity detection and collapse** (`variant_collapse_t`)

The design is **not trait-oriented**, and **not OOP-oriented**.
It enforces *closed-world semantics*, *deterministic layout*, and *compile-time verifiability*.

---

## Design Philosophy

### Why ADT, not inheritance?

* The set of alternatives is **closed at compile time**
* No vtables, no RTTI, no dynamic extension
* No writable dispatch pointers
* Predictable memory layout and CPU-friendly access
* Safe interaction with PMR containers
* ABI-stable static extension model

All APIs in this module assume:

> _"The variant defines the domain."_

---

## `check_all` — Variant-Wide Rule Validation

### What `check_all` actually accepts

`check_all` does **not** accept raw type traits.

Instead, it accepts a **rule object template**:

* a `struct` / `class` template
* exposing `static constexpr bool value`
* instantiated once per variant alternative

This is a **template-rule requirement**, not a stylistic choice.

---

### Valid rule shapes

#### Narrow form

```cpp
template<typename T>
struct Rule {
    static constexpr bool value = /* predicate on T */;
};
```

#### Wide form (recommended for libraries)

```cpp
template<typename T, typename Variant, typename... Args>
struct Rule {
    using _unused [[maybe_unused]] = Variant;
    static constexpr bool value = /* predicate on T and/or Variant */;
};
```

In the wide form:

* the **second template parameter is always the full variant type**
* users do **not** pass `Variant` explicitly
* this enables global semantic checks

---

### Invalid usage (by design)

```cpp
// WRONG — std::is_default_constructible is a trait, not a rule
static_assert(
jh::meta::check_all<std::is_default_constructible, MyVariant>
);
```

Reason:

* `std::is_default_constructible<T>` is a type trait
* `check_all` requires a *rule object*
* this distinction is enforced by template rules

---

### ✔ Correct usage

```cpp
template<typename T, typename Variant>
struct default_constructible_rule {
    using _unused [[maybe_unused]] = Variant;
    static constexpr bool value =
        std::is_default_constructible_v<T>;
};

static_assert(
    jh::meta::check_all<
        default_constructible_rule,
        MyVariant
    >
);
```

---

### What `check_all` guarantees

* Every alternative is validated
* Evaluation short-circuits on first failure
* Failure is **deterministic and compile-time**
* No runtime penalty
* No partial acceptance

This models **ADT invariants**, not per-type traits.

---

## `variant_transform_t` — Compile-Time Variant Mapping

### Purpose

`variant_transform_t` applies a unary type transformation to **every alternative** in a variant and produces a **new
variant**.

```cpp
template<typename Variant, template<typename> typename TpTrans>
using variant_transform_t = /* std::variant<...> */;
```

---

### Transformation requirements

For every alternative `T` in `Variant`:

```cpp
TpTrans<T>::type
```

must exist and must **not** be `void`.

Failure for **any** alternative causes substitution failure.

---

### Example

```cpp
using V = std::variant<int, double>;

template<typename T>
struct as_unique_ptr {
    using type = std::unique_ptr<T>;
};

using R = jh::meta::variant_transform_t<V, as_unique_ptr>;
// R == std::variant<std::unique_ptr<int>, std::unique_ptr<double>>
```

---

### No semantic filtering

`variant_transform_t` performs **no deduplication** and **no collapse**.

If the transformation produces duplicate types:

```cpp
std::variant<T, T>
```

the compiler will reject it — intentionally.

Semantic filtering is handled by `variant_collapse_t`.

---

## `variant_collapse_t` — Uniformity Detection

### Purpose

`variant_collapse_t` detects whether **all alternatives map to the same type**.

If so → that type
Otherwise → `void`

---

### Example (uniform)

```cpp
using V = std::variant<A, B, C>;

template<typename>
struct to_size {
    using type = std::size_t;
};

using R = jh::meta::variant_collapse_t<V, to_size>;
// R == std::size_t
```

---

### Example (non-uniform)

```cpp
template<typename T>
struct mixed {
    using type = std::conditional_t<
        std::is_integral_v<T>,
        int,
        double
    >;
};

using R = jh::meta::variant_collapse_t<V, mixed>;
// R == void
```

Mixed semantic outcomes are **explicitly rejected**.

---

## Family-Aware ADT Design

### Problem

Mappings like:

```
A  -> TA
BA -> TB
BB -> TB
BC -> TB
C  -> TC
```

are **not flat** semantics.

---

### Correct ADT structure

```cpp
using VB = std::variant<BA, BB, BC>;
using V  = std::variant<A, VB, C>;
```

Then:

* collapse `VB` independently
* transform outer variant consistently

This preserves semantic integrity and avoids accidental partial collapse.

---

## Static Extension Model

A library may expose:

```cpp
template<typename Variant>
struct BusinessLogic {
    static_assert(jh::meta::check_all<Rule, Variant>);
};
```

Users provide:

```cpp
using MyVariant = std::variant<A, B, C>;
BusinessLogic<MyVariant> logic;
```

Guarantees:

* closed type set
* no ABI breakage
* no inheritance hooks
* compile-time validation
* zero runtime overhead

This is **static extension**, not dynamic polymorphism.

---

## Summary

* `check_all` validates **ADT invariants**, not traits
* `variant_transform_t` maps alternatives mechanically
* `variant_collapse_t` enforces semantic uniformity
* Mixed semantics are rejected by design
* No vtables, no RTTI, no dynamic extension
* Predictable memory, predictable control flow

> **ADT is not a coding style.
> It is a machine-level design choice.**
