# 🗂️ **JH Toolkit — `jh::ranges::vis_transform_view` API Reference**

📁 **Header:** `<jh/ranges/vis_transform_view.h>`  
📦 **Namespace:** `jh::ranges`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::vis_transform_view` implements the underlying mechanism
for [`jh::ranges::views::vis_transform`](views/vis_transform.md) —
a **non-consuming**, **reentrant**, and **observation-oriented** transformation view.

Unlike `std::views::transform`, which may consume its source,
`vis_transform_view` guarantees that iteration over the underlying range
is purely observational: it never mutates, invalidates, or exhausts the base sequence.

This view type serves as the *semantic foundation* for
**visual (read-only)** transformations inside the JH Ranges framework,
allowing side-effect-free projections and inspection pipelines.

---

## 🔹 Definition

```cpp
template <std::ranges::range R, typename F>
requires jh::concepts::vis_function_for<F, R>
class vis_transform_view : public std::ranges::view_interface<vis_transform_view<R, F>>;
```

A constexpr-compatible, lightweight class template that wraps an existing range
and applies a callable `F` lazily upon dereference, without consuming elements.

---

## 🔹 Behavior

| Aspect                  | Description                                                                                                        |
|-------------------------|--------------------------------------------------------------------------------------------------------------------|
| **Purpose**             | Provides a view that applies a callable to elements for observation, without consuming or mutating the base range. |
| **Callable constraint** | The function `F` must satisfy [`jh::concepts::vis_function_for<F, R>`](../conceptual/vis_function_for.md).         |
| **Reentrancy**          | Fully reentrant — iteration can be repeated safely if the underlying range itself is reentrant.                    |
| **Laziness**            | Evaluation occurs only on dereference; no storage, allocation, or copying of results.                              |
| **Const behavior**      | All operations are const-qualified; the callable is stored immutably and invoked per iteration.                    |
| **Iterator model**      | Mirrors the traversal category of the underlying range (input → random access).                                    |
| **Composability**       | Implements `std::ranges::view_interface`, enabling seamless integration with other JH and STL range adaptors.      |

The view preserves the category and borrowability of its base range,
automatically specializing `std::ranges::enable_borrowed_range` where applicable.

---

## 🔹 Semantic Model

`vis_transform_view` is the **read-only projection** counterpart of `transform_view`.  
It enforces the following semantics:

1. **Non-consuming traversal** —
   dereferencing elements applies the callable without advancing or altering source data.

2. **Deterministic projection** —
   the same element dereferenced multiple times yields identical values.

3. **Transparent iteration** —
   `begin()` and `end()` are const-accessible and mirror the base range’s traversal behavior.

4. **Type safety** —
   construction is only permitted when the callable `F` is statically verified
   to be non-mutating and non-consuming over `R`.

---

## 🔹 Integration Notes

* Internal implementation backing [`jh::ranges::views::vis_transform`](views/vis_transform.md).
* Not intended for direct inclusion — automatically imported through higher-level view headers.
* Requires `jh::concepts::vis_function_for<F, R>` to ensure non-destructive callable semantics.
* Fully compatible with `std::ranges::enable_borrowed_range` to preserve range borrowability.
* Declared under `<jh/ranges/vis_transform_view.h>`; included indirectly via `<jh/views>` or
  `<jh/ranges/views/vis_transform.h>`.
* All members are constexpr-friendly and trivially inlineable — no dynamic dispatch or allocation.

---

### 🧩 **Summary**

`jh::ranges::vis_transform_view` defines the **non-consuming transformation core**
of the JH Ranges visual pipeline system.  
It enables callable-based projections that preserve data integrity,
ensure reentrancy, and compose naturally with other views and adaptors.

> **In short:**  
> A *pure observational transform view* — no mutation, no consumption,
> only precise, repeatable projection.
