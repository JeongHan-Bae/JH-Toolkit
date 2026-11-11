# 🧩 **JH-Toolkit — `jh::concepts::container_traits` API Reference**

📁 **Header:** `<jh/conceptual/container_traits.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧩 Introduction

`jh::concepts::container_traits` defines the **unified element-type deduction model**
used throughout the JH conceptual subsystem.  
It provides a canonical way to identify the *value type* of a container-like type,
ensuring consistency across all container concepts and range adaptors.

The core utility, `jh::concepts::container_value_t<C>`,
serves as the foundational type resolver used by
[`closable_container_for`](closable_container.md) and
[`collectable_container_for`](collectable_container.md)
to determine compatibility between ranges and containers at compile time.

---

## 🔹 Purpose

Containers expose element-type information through varying conventions:  
some declare `value_type`, others only reveal it through iterators,
and custom or adapter types may omit it entirely.

This trait provides a single, **deterministic**, and **conflict-resolving**
mechanism that extracts one stable `value_type` for any container-like type,
regardless of its internal structure or declaration form.

It ensures that all higher-level components —
especially those performing range-to-container transformations —
operate on a consistent and validated notion of "element type".

---

## 🔹 Deduction Logic

`container_value_t` unifies three deduction strategies under a fixed priority system:

| Priority                  | Strategy                                             | Description                                                                                                                                                                                                     |
|---------------------------|------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **1. User override**      | `jh::container_deduction<C>::value_type`             | Explicit user registration; <br> always takes precedence and short-circuits all subsequent checks.                                                                                                              |
| **2. Declared type**      | `C::value_type`                                      | Considered only when no user override exists. <br> If an iterator-based deduction also exists, the two must satisfy `std::common_reference_with<declared_t, deduced_t>` — in that case, `declared_t` is chosen. |
| **3. Iterator inference** | `iterator_value_t<iterator_t<C>>`                    | Used when no declared type is present. <br> Provides element semantics derived purely from iterator behavior.                                                                                                   |
| **Conflict resolution**   | `!std::common_reference_with<declared_t, deduced_t>` | When declared and deduced types exist but are **not** semantically compatible, deduction fails and yields `void`.                                                                                               |
| **Fallback**              | `void`                                               | Returned when no valid or consistent deduction path exists.                                                                                                                                                     |

All detection is constexpr and SFINAE-safe; incomplete or proxy-based types are handled gracefully.

---

## 🔹 User Registration

Developers can explicitly register value types for non-standard containers by specializing:

```cpp
template<>
struct jh::container_deduction<MyContainer> {
    using value_type = MyElement;
};
```

This override mechanism takes absolute priority and
is used to integrate third-party or structurally opaque containers
that do not expose iterator or value declarations in a conventional way.

---

## 🔹 Design Characteristics

* **Purely structural:**  
  No modification, instantiation, or runtime inspection occurs.

* **Deterministic resolution:**  
  Each container–type pair always yields one well-defined result.

* **Non-intrusive:**  
  Users may register custom containers without touching the framework internals.

* **Compatible with incomplete types:**  
  Fully constexpr and safely evaluable in meta-contexts.

* **Foundation-level:**  
  All range and container concepts in the JH framework depend on it.

---

## 🔹 Role in Concept Hierarchy

`container_value_t` underpins all *container–range compatibility concepts* in the JH system.

| Dependent Concept                                       | Dependency Role                                                                                    |
|---------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| [`closable_container_for`](closable_container.md)       | Uses `container_value_t` to check whether a range's element type can construct the container.      |
| [`collectable_container_for`](collectable_container.md) | Uses it to verify element compatibility between a materialized collector and its output container. |
| `iterator_t`, `sequence_t`                              | Provide iterator and sequence deduction supporting this trait.                                     |

Thus, `container_traits` acts as the **type identity layer**
linking iterator semantics and container construction concepts.

---

## 🔹 Semantic Guarantees

| Guarantee         | Description                                                                            |
|-------------------|----------------------------------------------------------------------------------------|
| **Consistency**   | All container-like types yield a single stable value-type result.                      |
| **Safety**        | Deductions are purely compile-time; no side effects or instantiation.                  |
| **Extensibility** | Supports external and third-party containers via `container_deduction<>` registration. |
| **Determinism**   | Resolution order and behavior are strictly defined.                                    |
| **Integration**   | Serves as a required dependency for all range–container adaptation concepts.           |

---

## 🧩 Summary

* `jh::concepts::container_value_t` provides a **unified compile-time definition**
  of a container's element type.
* It merges declared, iterator-based, and user-registered sources
  under a deterministic resolution hierarchy.
* This trait is the **foundation** on which
  `closable_container_for` and `collectable_container_for`
  determine container–range compatibility.
* It guarantees consistency and extensibility across
  all JH conceptual and adaptor modules.
