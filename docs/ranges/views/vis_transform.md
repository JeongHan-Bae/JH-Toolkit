# 🔭 **JH Toolkit — `jh::ranges::views::vis_transform` API Reference**

📁 **Header:** `<jh/ranges/views/vis_transform.h>`  
📦 **Namespace:** `jh::ranges::views`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::ranges::views::vis_transform` provides an **explicit, observation-only transform adaptor**
for reentrant ranges and observational callables.
It is the foundation of observational adaptors such as
[`jh::ranges::views::transform`](transform.md) and diagnostics pipelines
where data should remain readable without ownership transfer.

The standard transform view does not promise reentrant observations after
transformation. `vis_transform` provides that JH semantic contract when the
source remains repeatable and the callable preserves later observed results.

---

## 🔹 Definition

```cpp
namespace jh::ranges::views {

inline constexpr detail::vis_transform_fn vis_transform{};

}
```

---

## 🔹 Interface

1. **Direct form**

   ```cpp
   template <std::ranges::range R, typename F>
   requires jh::concepts::vis_function_for<F, R>
   constexpr auto vis_transform(R&& r, F&& f);
   ```

2. **Pipe form**

   ```cpp
   template <typename F>
   constexpr auto vis_transform(F&& f);
   ```

Equivalent usage:

```cpp
auto v1 = jh::ranges::views::vis_transform(r, f);
auto v2 = r | jh::ranges::views::vis_transform(f);
```

---

## 🔹 Description

`vis_transform` applies a callable `f` lazily to each element of range `r`,
producing a [`jh::ranges::vis_transform_view`](../vis_transform_view.md)
intended for **non-consuming traversal** of the input.

This adaptor enforces
[`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r),
which checks these structural requirements:

* The range has const-accessible input iteration.
* The callable is invocable through const access and returns a non-void result.

The caller must also ensure that repeated traversal of the source is valid and
that invoking the callable does not change the source or later results. The concept
cannot establish those runtime properties. Bidirectional iterator operations
are independent of this reentrancy contract.

---

## 🔹 Design Semantics

| Aspect                 | Description                                                                                                                                                               |
|------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Consumption model**  | Non-consuming when the source and callable uphold the observational contract. |
| **Concept dependency** | Requires `jh::concepts::vis_function_for<F, R>` for structural validation. |
| **Dispatch model**     | Explicitly constructs `vis_transform_view`; `transform` selects it when the concept holds. |
| **Reentrancy**         | Requires repeatable source traversal and stable results for unchanged source data. |
| **Integration**        | Compatible with `common()`, `to`, `flatten()`, and any adaptor requiring non-consuming semantics.                                                                         |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/vis_transform.h>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> xs = {1, 2, 3};

    auto squares = xs | jh::ranges::views::vis_transform([](int x) { return x * x; });

    for (auto v : squares)
        std::cout << v << " ";
}
```

**Output:**

```
1 4 9
```

With this vector and pure lambda, iterating `squares` again produces the same
values while `xs` remains unchanged.

---

## 🔹 Relationship to `transform`

[`jh::ranges::views::transform`](transform.md) uses
[`jh::concepts::vis_function_for`](../../conceptual/range_traits.md#-concept--vis_function_forf-r)
internally to determine **which adaptor path** to select:

* If the function–range pair satisfies `vis_function_for`,
  the adaptor delegates to `vis_transform_view` under its observational contract.
* Otherwise, it uses the standard transform view.

`vis_transform` is the explicit entry point when the developer can uphold the
observational contract for the source and callable.

---

## 🔹 Integration Notes

* `vis_transform` is an explicit request for the observational contract.
* Implemented through [`jh::ranges::vis_transform_view`](../vis_transform_view.md).
* Depends on [`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r)
  for concept enforcement.
* Used internally by [`flatten`](flatten.md) and other structural adaptors.
* Fully compatible with [`jh::ranges::views::common()`](common.md) and [`jh::ranges::to`](../to.md) .

---

## 🧩 Summary

`jh::ranges::views::vis_transform` builds the observational transform view
directly. Repeatability depends on the source and callable honoring the
documented contract.
