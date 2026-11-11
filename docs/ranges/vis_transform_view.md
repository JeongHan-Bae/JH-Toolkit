# 🗂️ **JH-Toolkit — `jh::ranges::vis_transform_view` API Reference**

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
a transformation view intended for repeatable observations.

The standard transform view does not provide JH's reentrancy contract for
transformed observations. This view offers that contract when its source is
repeatable and its callable leaves the observed state unchanged.

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

A constexpr-compatible class template that wraps an existing range and applies
`F` lazily upon dereference.

---

## 🔹 Behavior

| Aspect                  | Description                                                                                                        |
|-------------------------|--------------------------------------------------------------------------------------------------------------------|
| **Purpose**             | Applies a callable lazily under an observational contract. |
| **Callable constraint** | The function `F` must satisfy [`jh::concepts::vis_function_for<F, R>`](../conceptual/range_traits.md#-concept--vis_function_forf-r).         |
| **Reentrancy**          | Requires a repeatable source and a callable that preserves observed state. |
| **Laziness**            | The callable runs on dereference; its return type determines result storage. |
| **Const behavior**      | All operations are const-qualified; the callable is stored immutably and invoked per iteration.                    |
| **Iterator model**      | Mirrors the traversal category of the underlying range (input → random access).                                    |
| **Composability**       | Implements `std::ranges::view_interface`, enabling seamless integration with other JH and STL range adaptors.      |

Iterator operations reflect the base iterator where the implementation
provides them. Iterator category does not establish semantic repeatability.
The iterator stores a pointer to the `vis_transform_view` so it can invoke the
stored callable. The view therefore does not model `borrowed_range`: keep the
view alive while using an iterator obtained from it.

---

## 🔹 Semantic Model

`vis_transform_view` exposes a const-accessible projection. Its contract is:

1. **Non-consuming traversal** —
   dereferencing elements applies the callable without advancing or altering source data.

2. **Stable observations** —
   callers supply a callable whose result remains consistent for unchanged source data.

3. **Transparent iteration** —
   `begin()` and `end()` are const-accessible and mirror the base range’s traversal behavior.

4. **Structural checks** —
   `vis_function_for<F, R>` requires const-accessible input iteration,
   const invocation, and a non-void result. It cannot prove side-effect freedom.

---

## 🔹 Integration Notes

* Internal implementation backing [`jh::ranges::views::vis_transform`](views/vis_transform.md).
* Available through `<jh/ranges/vis_transform_view.h>` and higher-level view headers.
* Requires `jh::concepts::vis_function_for<F, R>` for structural checks.
* Declared under `<jh/ranges/vis_transform_view.h>`; included indirectly via `<jh/views>` or
  `<jh/ranges/views/vis_transform.h>`.
* All members are constexpr-friendly and trivially inlineable — no dynamic dispatch or allocation.

---

### 🧩 **Summary**

`jh::ranges::vis_transform_view` is the projection core for callers that
require JH's observational contract and can uphold it for the source and
callable.
