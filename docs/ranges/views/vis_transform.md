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
that guarantees non-consuming behavior for reentrant ranges.  
It is the foundation of observational adaptors such as
[`jh::ranges::views::transform`](transform.md) and diagnostics pipelines
where data should remain readable without ownership transfer.

Unlike `std::views::transform`, which always assumes consumptive traversal,
`vis_transform` preserves the original range’s reentrancy whenever possible,
making it safe for analytical and compositional pipelines.

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
that guarantees **non-consuming traversal** of the input.

This adaptor enforces
[`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r) —
a behavioral contract ensuring the callable and range form a **visual relation**:

* The range supports stable, non-consuming iteration.
* The callable produces a readable, non-void transformation.
* Neither the callable nor the range performs mutation.

Because the relation is *bidirectional*, `vis_transform` dispatches safely
only when the range and callable mutually satisfy these invariants.  
This design ensures stable reuse and compositional reentrancy across views.

---

## 🔹 Design Semantics

| Aspect                 | Description                                                                                                                                                               |
|------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Consumption model**  | Always non-consuming; iteration is stable and reentrant.                                                                                                                  |
| **Concept dependency** | Relies on `jh::concepts::vis_function_for<F, R>` for validation.                                                                                                          |
| **Dispatch model**     | Same dispatch mechanism used by [`jh::ranges::views::transform`](transform.md); the adaptor layer selects `vis_transform_view` when the callable is purely observational. |
| **Reentrancy**         | Guaranteed whenever the underlying range is reentrant.                                                                                                                    |
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

The iteration can be repeated safely because
`vis_transform` preserves reentrancy and does not consume its input.

---

## 🔹 Relationship to `transform`

[`jh::ranges::views::transform`](transform.md) uses
[`jh::concepts::vis_function_for`](../../conceptual/range_traits.md#-concept--vis_function_forf-r)
internally to determine **which adaptor path** to select:

* If the function–range pair satisfies `vis_function_for`,
  the adaptor delegates to `vis_transform_view` (non-consuming).
* Otherwise, it falls back to the standard, possibly consuming path.

Thus, `vis_transform` serves as the **explicit form** of that same logic —
a direct request for non-consuming behavior, independent of automatic dispatch.
It can also be used standalone when the developer intends
to guarantee non-destructive inspection.

---

## 🔹 Integration Notes

* `vis_transform` is an *explicit* adaptor:  
  it always preserves reentrancy, never consumes input.
* Implemented through [`jh::ranges::vis_transform_view`](../vis_transform_view.md).
* Depends on [`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r)
  for concept enforcement.
* Used internally by [`flatten`](flatten.md) and other structural adaptors.
* Fully compatible with [`jh::ranges::views::common()`](common.md) and [`jh::ranges::to`](../to.md) .

---

## 🧩 Summary

`jh::ranges::views::vis_transform` is the **explicit non-consuming variant**
of `transform`, designed for observation-only transformations.  
It ensures full reentrancy, integrates seamlessly with `common()` and `to`,
and forms the semantic base layer for flattening and structural composition
throughout the JH Ranges Toolkit.
