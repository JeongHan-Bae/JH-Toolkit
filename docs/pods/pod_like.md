# 🧊 **JH Toolkit — `jh::pod::pod_like` API Reference**

📁 **Header:** `<jh/pods/pod_like.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Overview

This header defines **fundamental POD concepts** used across all types in `jh::pod`.  
They establish what it means for a type to be safely represented as *plain old data*
and to maintain **trivial ABI and layout stability**.  

The concepts are used throughout the toolkit to restrict and validate
types in low-level containers like `jh::pod::array`, `jh::pod::pair`, and `jh::pod::optional`.

---

## 🔹 Concepts

### `pod_like<T>`

A type that behaves as *POD-compatible* under modern C++ object rules.  
It ensures that the type can be **bitwise copied, serialized, or reinterpreted**
without invoking any constructors or destructors.

| Property                               | Required | Description                          |
|----------------------------------------|----------|--------------------------------------|
| `std::is_trivially_copyable_v<T>`      | ✅        | Allows raw bitwise copies.           |
| `std::is_trivially_constructible_v<T>` | ✅        | Safe to create by zero or memcpy.    |
| `std::is_trivially_destructible_v<T>`  | ✅        | No custom destructor.                |
| `std::is_standard_layout_v<T>`         | ✅        | Predictable field order & alignment. |

```cpp
template<typename T>
concept pod_like =
    std::is_trivially_copyable_v<T> &&
    std::is_trivially_constructible_v<T> &&
    std::is_trivially_destructible_v<T> &&
    std::is_standard_layout_v<T>;
```

### `cv_free_pod_like<T>`

Equivalent to `pod_like<T>` but explicitly disallows
`const` or `volatile` qualified base types.

```cpp
template<typename T>
concept cv_free_pod_like =
    pod_like<T> &&
    !std::is_const_v<T> &&
    !std::is_volatile_v<T>;
```

**Rationale:**
A `const` or `volatile` inner type would make the outer container (e.g. `array<T>`)
non-trivial to assign or move, thus violating POD semantics.  

This ensures that POD wrappers (like `jh::pod::optional<T>`) remain trivially assignable,
even when nested.

---

## 🧩 Examples

```cpp
static_assert(jh::pod::pod_like<int>);
static_assert(jh::pod::pod_like<std::pair<int, int>>);
static_assert(!jh::pod::pod_like<std::string>);

static_assert(jh::pod::cv_free_pod_like<int>);
static_assert(!jh::pod::cv_free_pod_like<const int>);
```

---

## 🧩 Integration Notes

* Serves as the **conceptual base** for all `jh::pod` containers.  
* Enforced by templates such as `array`, `pair`, `optional`, and `bitflags`.  
* Guarantees deterministic ABI, no hidden ownership, and safe serialization.  
* In **1.4+**, conceptual unification under
  `jh/conceptual/pod.h` is considered.  
  However, the canonical definition remains in
  `<jh/pods/pod_like.h>` and the namespace `jh::pod`.  
  The conceptual mirror (under `jh::concepts`) will alias, not replace.
