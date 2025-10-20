# 📦 **JH Toolkit — `jh::typed` API Reference**

📁 **Header:** `<jh/utils/typed.h>`  
📦 **Namespace:** `jh::typed`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Utils](https://img.shields.io/badge/%20Back%20to%20Utils-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::typed` provides **lightweight type primitives** for POD-based environments.  
It defines a minimal placeholder type, `monostate`, that represents *"no value"*
without relying on heavy STL headers like `<variant>` or `<utility>`.  

Unlike `std::monostate`, which depends on `<variant>`,
this implementation is **fully header-only**, **constexpr-safe**,
and designed for use inside low-level, STL-free systems such as `jh::pod` components.  

This header also introduces detection traits and a `concept` for compile-time type queries.  

---

## 🔹 Overview

| Aspect                   | Description                                                                  |
|--------------------------|------------------------------------------------------------------------------|
| **Purpose**              | Provide a trivial "empty type" with zero state.                              |
| **Implementation Model** | Header-only, no STL dependencies beyond `<type_traits>`.                     |
| **Use Case**             | Placeholder in POD tuples, static variants, or unused type slots.            |
| **Comparison Semantics** | All `monostate` instances are equal (`==` always true).                      |
| **Migration Note**       | May move to `<jh/typed.h>` in future releases as part of the type subsystem. |

---

## 🔹 Components

| Symbol             | Description                               |
|--------------------|-------------------------------------------|
| `struct monostate` | Trivial POD type representing "no value". |
| `is_monostate<T>`  | Type trait for compile-time detection.    |
| `monostate_t<T>`   | Concept alias for constraint-based code.  |

---

## 🔹 API Reference

### `struct monostate`

```cpp
struct monostate final {
    constexpr bool operator==(monostate) const noexcept { return true; }
    constexpr bool operator!=(monostate) const noexcept { return false; }
};
```

A trivial empty type that models *"no state"*.
It has no data members and all instances are interchangeable.

**Properties:**

* Trivially constructible and copyable.
* Standard layout, `constexpr` safe.
* Equality always returns `true`; inequality always returns `false`.

**Use Case Examples:**

```cpp
jh::typed::monostate a, b;
static_assert(a == b);  // always true
```

**Design Guarantees:**

```cpp
static_assert(std::is_trivially_copyable_v<jh::typed::monostate>);
static_assert(std::is_standard_layout_v<jh::typed::monostate>);
```

---

### `is_monostate<T>`

```cpp
template<typename T>
struct is_monostate : std::false_type {};

template<>
struct is_monostate<monostate> : std::true_type {};
```

A compile-time trait that detects whether a type is `jh::typed::monostate`.

**Usage Example:**

```cpp
static_assert(jh::typed::is_monostate<jh::typed::monostate>::value);
```

---

### `monostate_t<T>`

```cpp
template<typename T>
concept monostate_t = is_monostate<T>::value;
```

C++20 concept equivalent of `is_monostate<T>`,
intended for use in template constraints or SFINAE.

**Usage Example:**

```cpp
template<jh::typed::monostate_t T>
void f(T) {
    // only matches jh::typed::monostate
}
```

---

## 🧩 Summary

`jh::typed` defines the **minimal type layer** for the JH Toolkit.  
It allows expressing "no value" semantics without importing heavyweight STL constructs,
ensuring full control over dependency boundaries and build size.  

This component will serve as the **foundation** for upcoming type utilities such as:  

* Lightweight compile-time traits,
* POD-safe type identifiers,
* Trivial meta-type wrappers.

Future versions (≥1.4.0) may promote this module to the **root namespace**
as `<jh/typed.h>` once the type subsystem expands.

---

> **Note:**
> `jh::typed::monostate` is not equivalent to `std::nullopt_t` or nullable placeholders.  
> It is purely structural — meant for compile-time or POD-level use only.
