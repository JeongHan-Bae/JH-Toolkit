# 🧩 **JH Toolkit — `jh::concepts::range_traits` API Reference**

📁 **Header:** `<jh/conceptual/range_traits.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/Back_to_README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/Back_to_Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::concepts::range_traits` defines the **general conceptual aggregation layer**
for range-related constructs that are not specific to a single subdomain such as
*views*, *containers*, or *sequences*.

It provides **cross-cutting abstractions** that are reused throughout the
JH range subsystem, especially for:

* adaptor and visitor validation,
* unified forwarding of value categories,
* deduction of range storage policy inside wrappers and views.

This module acts as the **conceptual catch-all** for *range-adjacent* logic
that does not belong to lower-level categories like iterators or sequences.

---

## 🔹 Concept — `vis_function_for<F, R>`

### Behavioral Description

`vis_function_for` expresses the **visual relation** between
a callable type `F` and a range type `R`.

A type pair `(F, R)` satisfies this concept when the following behavioral
conditions hold:

1. **Stable traversal** —
   The range `R` supports non-consuming iteration; invoking `begin()` and `end()`
   repeatedly does not alter its observable state.

2. **Element accessibility** —
   Dereferencing the iterator returned by `begin(R)` produces a valid element
   reference that can be read without mutation.

3. **Callable compatibility** —
   Applying `F` to an element of `R` (as if by `std::invoke(f, *begin(r))`)
   is well-formed and produces a result.

4. **Non-void semantics** —
   The invocation result must not be `void`; the operation is *visual* or
   *transformational*, not purely procedural.

5. **Non-mutating relation** —
   The application of `F` must not modify either the range `R` itself or its
   individual elements; the relation is *observational only*.

### Conforming Behavior

* Ranges whose iterators are stable and readable without ownership transfer.
* Callables that act as *inspectors* or *mappers*, producing values based on
  elements rather than mutating them.
* View or adaptor layers that perform visual transformations (e.g. diagnostic
  pipelines) without consuming the source range.

### Non-Conforming Behavior

* Callables that mutate or consume the range elements.
* Ranges whose `begin()` or `end()` alter internal state or invalidate iteration.
* Operations that return `void` or rely on side effects rather than observable
  transformation.

---

## 🔹 Trait — `range_storage_traits<R, UseRefWrapper>`

### Behavioral Description

`range_storage_traits` defines how a range or range-like object is **held
internally** when wrapped inside a higher-level view or adaptor.

It provides a **compile-time policy** that determines whether a range is:

* stored **by value** (ownership semantics),
* stored **by reference** (borrowed semantics), or
* stored **via reference wrapper** for deferred safe access.

This abstraction allows all wrapper-based views in JH Toolkit to maintain
consistent lifetime and forwarding semantics regardless of the range’s
value category.

---

### Storage Behavior Rules

1. **Rvalue or temporary sources**  
   Are stored *by value*.  
   The wrapper assumes full ownership of the range object.

2. **Lvalue references (local or persistent ranges)**  
   Are stored as *references*, allowing lightweight borrowing.

3. **Reference-wrapper mode (`UseRefWrapper = true`)**  
   Enforces *safe reference semantics*, ensuring deferred or pipelined
   transformations do not outlive their source objects.

4. **Const propagation**  
   When `R` is a const reference, storage preserves const-qualification
   across all accessors.

5. **Retrieval consistency**  
   Regardless of how a range is stored, the wrapper must expose a consistent
   access model through `get()` — returning a valid reference to the underlying
   range for both const and mutable contexts.

---

## 🧩 Integration Notes

* `range_storage_traits` is used by view types such as
  `jh::ranges::range_wrapper` and `jh::ranges::vis_transform_view`.
* Ensures uniform forwarding and lifetime handling across all range adaptors.
* `vis_function_for` complements it by providing callable validation for
  non-mutating traversal semantics.
* The header intentionally remains domain-neutral and conceptual,
  serving as a unifying utility layer for all range-related modules.

---

## 🧾 Summary

| Aspect          | Description                                                     |
|-----------------|-----------------------------------------------------------------|
| Category        | Conceptual / range utility layer                                |
| Namespace       | `jh::concepts`                                                  |
| Core Components | `vis_function_for`, `range_storage_traits`                      |
| Purpose         | Behavioral contracts for visual traversal and storage semantics |
| Version         | 1.3.5+                                                          |
| Header          | `<jh/conceptual/range_traits.h>`                                |
| ABI             | Header-only, constexpr-evaluated                                |
| Dependencies    | `<iterator>`, `<ranges>`, `<functional>`, `<type_traits>`       |

---

> ⚙️ **Design Policy**
>
> * Acts as the **conceptual catch-all** for range-adjacent utilities.
> * Maintains strict separation from specific domains like `sequence` or `iterator`.
> * Defines reusable behavior contracts across range adaptors,
>   transformation views, and range storage logic.
> * Represents **foundational behavior**, not implementation detail —
>   its purpose is to describe *what qualifies as a valid concept or trait*,
>   not *how it is implemented*.
