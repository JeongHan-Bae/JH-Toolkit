# 🧬 **JH Toolkit — `jh::typed::monostate` API Reference**

📁 **Header:** `<jh/typing/monostate.h>`  
📦 **Namespace:** `jh::typed`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::typed::monostate` defines a **trivial placeholder type** used to represent
*"no value"* in strictly POD-safe environments.

It serves as a structural equivalent to `std::monostate`,
but is implemented **header-only**, with **no dependency** on `<variant>`
or any other heavy STL headers.

This type was **moved** in **1.3.5+** from `jh/utils/typed.h` → `jh/typing/monostate.h`
as part of the new **Typing subsystem** refactor.

---

## 🔹 Overview

| Aspect                    | Description                                                   |
|---------------------------|---------------------------------------------------------------|
| **Purpose**               | Provide a minimal, trivial "empty type".                      |
| **Implementation Model**  | Header-only, STL-free (uses only `<type_traits>`).            |
| **Typical Use**           | Placeholder in POD tuples, static variants, or unused slots.  |
| **Comparison Semantics**  | All instances are always equal (`==` → true).                 |
| **Change (since 1.3.5+)** | Moved from `<jh/utils/typed.h>` to `<jh/typing/monostate.h>`. |

---

## 🔹 Components

| Symbol             | Description                                   |
|--------------------|-----------------------------------------------|
| `struct monostate` | Trivial POD representing "no value".          |
| `is_monostate<T>`  | Type trait that detects the placeholder type. |
| `monostate_t<T>`   | Concept alias for compile-time constraints.   |

---

## 🔹 API Reference

### `struct monostate`

```cpp
struct monostate final {
    constexpr bool operator==(monostate) const noexcept { return true; }
    constexpr bool operator!=(monostate) const noexcept { return false; }
};
```

A zero-state type that expresses *"no value"* purely at the type level.  
All instances are identical and interchangeable.

**Key Properties**

* Trivially copyable, constructible, and destructible.
* Standard layout, `constexpr`-safe, and `noexcept` operations.
* Suitable for static, embedded, or STL-free environments.

**Validation**

```cpp
static_assert(std::is_trivially_copyable_v<jh::typed::monostate>);
static_assert(std::is_trivially_constructible_v<jh::typed::monostate>);
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

Trait that evaluates to `true` if `T` is `jh::typed::monostate`.

**Example**

```cpp
static_assert(jh::typed::is_monostate<jh::typed::monostate>::value);
```

---

### `monostate_t<T>`

```cpp
template<typename T>
concept monostate_t = is_monostate<T>::value;
```

C++20 concept equivalent to `is_monostate<T>`.
Used for restricting templates to accept only `monostate`.

**Example**

```cpp
template<jh::typed::monostate_t T>
constexpr void f(T) {
    // Matches only jh::typed::monostate
}
```

---

## 🧩 Summary

`jh::typed::monostate` forms the **base layer** of the JH Typing subsystem.  
It enables clear "no value" semantics without importing `<variant>`, `<optional>`, or other heavy utilities.

This component provides the foundation for upcoming type utilities, including:

* POD-safe type traits and identifiers,
* Compile-time meta-type wrappers,
* Lightweight reflection primitives.

---

> **Note:**  
> `jh::typed::monostate` is purely *structural*, not a runtime null marker like `std::nullopt_t`.  
> It exists only to represent the *absence of data*, not the *absence of meaning*.
