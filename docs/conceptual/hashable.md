# 🧩 **JH Toolkit — `jh::concepts::hashable` API Reference**

📁 **Header:** `<jh/conceptual/hashable.h>`  
📦 **Namespace:** `jh::concepts`, `jh`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Submodule Notice

* `jh::concepts::hashable` defines the **semantic hashing model** for the JH framework.
* It generalizes `std::hash` by supporting **ADL-discovered** and **member-defined** hash functions.
* The `jh::hash` functor performs **compile-time deduction** of the correct hashing strategy —
  with zero runtime overhead and full STL interoperability.

---

## 🧩 Introduction

The standard C++ hashing system (`std::hash<T>`) provides a uniform entry point for associative containers,
but it cannot represent **semantic**, **lazy**, or **context-sensitive** hashing.

The JH hashing model extends this by unifying three non-invasive resolution paths:  
`std::hash<T>`, `hash(t)` (ADL), and `t.hash()`.  
This preserves full compatibility with the STL, while enabling flexible, domain-specific hash semantics.

Typical motivations include:

* **Lazy or cached** hash computation for immutable types.
* **Custom algorithm selection** (e.g., FNV-1a, CityHash, domain hashes).
* **Semantic hashing**, where a hash encodes logical identity rather than raw bytes.

---

## 🔹 Resolution Model

| Priority | Strategy            | Description                                                          |
|----------|---------------------|----------------------------------------------------------------------|
| **1.**   | `std::hash<T>{}(v)` | Standard hashing; used if specialization is available.               |
| **2.**   | `hash(v)` (ADL)     | Non-intrusive free function discovered by argument-dependent lookup. |
| **3.**   | `v.hash()`          | Member function returning a `size_t` hash value.                     |

This three-tier chain guarantees:

* Full STL compatibility.
* Extensibility for non-intrusive, user-defined types.
* No risk of hijacking or redefining system-provided hashes.

---

## 🔹 Core Concepts

### `has_std_hash<T>`

Satisfied when:

```cpp
std::hash<T>{}(v) -> std::convertible_to<size_t>;
```

### `has_adl_hash<T>`

Satisfied when an ADL-discoverable function exists:

```cpp
size_t hash(const T&);
```

### `has_mbr_hash<T>`

Satisfied when:

```cpp
v.hash() -> std::convertible_to<size_t>;
```

### `extended_hashable<T>`

A type `T` is `extended_hashable` if **any** of the above hold:

```cpp
has_std_hash<T> || has_adl_hash<T> || has_mbr_hash<T>
```

---

## 🔹 Unified Hash Functor

### `jh::hash<T>`

A **deduction template**, not a registration point.  
It automatically selects the appropriate hashing strategy
based on concept satisfaction at compile time.

Unlike `std::hash`, it **does not require or allow specialization** —
the behavior is inferred, not registered.

#### Resolution Order

1. `std::hash<T>`
2. ADL `hash(t)`
3. Member `t.hash()`

#### Example

```cpp
#include <jh/conceptual/hashable.h>

struct MyType {
    int value;
    size_t hash() const noexcept { return std::hash<int>{}(value); }
};

size_t h1 = jh::hash<int>{}(42);       // uses std::hash<int>
size_t h2 = jh::hash<MyType>{}({7});   // uses MyType::hash()
```

#### Notes

* `jh::hash` **deduces** the correct strategy; it never alters global behavior.
* It is **non-intrusive** — you do not specialize `jh::hash` or modify system types.
* All resolution happens statically via SFINAE and concept checks.

---

## 🔹 Non-intrusive Design and Restrictions

`jh::hash` is **a deduction template, not a registration template.**  
It discovers existing hash implementations — it never defines or registers new ones.

* `std::hash` remains the **only sanctioned registration interface**.
* ADL-based `hash()` is the **recommended mechanism** for custom types when
  you wish to avoid specializing `std::hash`.
* You **must not** redefine hashing for any STL or system-provided type.

If you need a non-standard hash function (e.g. for use with `std::unordered_map`),
**supply it manually** as a callable object or lambda,
rather than attempting to override global behavior.

#### ✅ Recommended for third-party types

* **Option 1:** Specialize `std::hash<T>` (inside `namespace std`) if allowed.
* **Option 2:** Define a free `hash(const T&)` function inside your own namespace.

Both approaches are **non-invasive** and fully compatible with `jh::hash`.

#### 🚫 Not Allowed

* Specializing `jh::hash<T>` directly.
* Redefining `std::hash` for STL types or built-ins.
* Providing an ADL `hash()` whose result is **non-deterministic within a single runtime**
  (e.g. seeded randomness that changes between calls).  
  Hash values may vary **across program runs**, but must remain stable
  for identical inputs **within one execution**.

---

## 🔹 Design Guarantees

| Guarantee          | Description                                                           |
|--------------------|-----------------------------------------------------------------------|
| **Non-intrusive**  | Uses deduction, never registration.                                   |
| **STL-compatible** | Fully interoperable with `std::hash` and standard containers.         |
| **Safe by design** | Prevents overriding of system-defined hashes.                         |
| **Extensible**     | Third-party types can add ADL `hash()` or `std::hash` specialization. |
| **Deterministic**  | Resolution is compile-time and well-defined.                          |

---

## 🧩 Summary

* `jh::hash<T>` is a **deduction-based**, non-intrusive hashing template.
* It unifies `std::hash`, ADL `hash()`, and `T::hash()` under a single interface.
* It **never replaces or registers** hashing behavior — it only detects and delegates.
* To extend hashing:

    * Use `std::hash<T>` if you control the type.
    * Use ADL `hash()` in your own namespace otherwise.
* For special hash algorithms (e.g. `unordered_map` customization),
  **pass the functor explicitly** — do not attempt to hack the template.
