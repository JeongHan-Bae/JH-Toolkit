# 🔭 **JH-Toolkit — `jh::ranges::views::transform` API Reference**

📁 **Header:** `<jh/ranges/views/transform.h>`  
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

`jh::ranges::views::transform` is a **unified transformation adaptor** that
performs **semantic dispatch** between two transformation modes at compile time:

* [`jh::ranges::views::vis_transform`](vis_transform.md) — when the pair
  <b><code>&lt;Range, Function&gt;</code></b> satisfies
  [`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r),
  selecting the JH observational path under its non-consuming contract.
* `std::views::transform` — for other combinations, using the standard
  transform view's range and callable semantics.

The JH path exists because the standard transform view does not promise that
transformed observations remain reentrant, even when the supplied callable
is intended to be observational. Traversal category and reentrancy are
separate properties: the ability to move an iterator forward or backward
does not by itself establish repeatable observations.

---

## 🔹 Definition

```cpp
namespace jh::ranges::views {

inline constexpr detail::transform_fn transform{};

}
```

---

## 🔹 Interface

1. **Direct form**

   ```cpp
   template <std::ranges::range R, typename F>
   constexpr auto transform(R&& range, F&& func);
   ```

2. **Pipe form**

   ```cpp
   template <typename F>
   constexpr auto transform(F&& func);
   ```

Equivalent usage:

```cpp
auto v1 = jh::ranges::views::transform(r, f);
auto v2 = r | jh::ranges::views::transform(f);
```

---

## 🔹 Description

`transform()` applies a callable `f` to each element of range `r`
and returns a view determined by semantic dispatch:

* If `vis_function_for<F, R>` is satisfied, it delegates to
  [`jh::ranges::views::vis_transform`](vis_transform.md).
* Otherwise, it uses `std::views::transform`.

The caller must uphold the observational contract for a pair routed through
`vis_transform`: repeated traversal of the source must be valid, and the
callable must leave the source and later observed results unchanged.

Dispatch occurs inside the call operator,
where both `R` and `F` are fully deduced,
ensuring correct compile-time routing.

---

## 🔹 Concept Dependency

The dispatch logic depends on
[`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r),
defined in [`jh/conceptual/range_traits.h`](../../conceptual/range_traits.md).

The concept checks const-accessible input iteration, const invocation of the
callable on an element, and a non-void result. These checks do not establish
that the callable is free of side effects or that a source's `begin()` is
semantically repeatable. Those are requirements of the observational contract.

---

## 🔹 Behavior

| Aspect                     | Description                                                                                               |
|----------------------------|-----------------------------------------------------------------------------------------------------------|
| **Dispatch rule**          | Compile-time selection between `vis_transform` and `std::views::transform`.                               |
| **Concept dependency**     | [`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r). |
| **Consumption semantics**  | The JH branch has an observational contract; the standard branch follows its base and callable semantics. |
| **Reentrancy**             | Requires a repeatable source and observational callable on the JH branch.                                  |
| **Pipeline compatibility** | Fully composable with all JH and standard range adaptors.                                                 |
| **Laziness**               | Identical to the semantics of the underlying transformation view.                                         |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/transform.h>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> xs = {1, 2, 3};

    // Pure observation (non-consuming)
    auto squares = xs | jh::ranges::views::transform([](int x) { return x * x; });

    // Mutating transformation (standard transform path)
    auto negated = xs | jh::ranges::views::transform([](int& x) -> int& { x = -x; return x; });

    for (auto v : squares) std::cout << v << " ";
    std::cout << "\n";
    for (auto v : negated) std::cout << v << " ";
}
```

**Output:**

```
1 4 9
-1 -2 -3
```

The first case is repeatable while `xs` and the callable uphold the
observational contract. In the second case, the mutable reference parameter
selects `std::views::transform`; dereferencing that view changes `xs`.

---

## 🔹 Integration Notes

* Dispatch and concept checking occur per `(R, F)` pair.
* The concept selects the JH path; callers remain responsible for stable observations.
* Internally, this mechanism is shared with [`vis_transform`](vis_transform.md)
  and used implicitly by observational adaptors such as [`flatten`](flatten.md).
* Depends on
  [`jh::concepts::vis_function_for`](../../conceptual/range_traits.md#-concept--vis_function_forf-r)
  defined in [`jh/conceptual/range_traits.h`](../../conceptual/range_traits.md).

---

## 🧩 Summary

`jh::ranges::views::transform` selects between JH's observational view and
`std::views::transform` for each range-callable pair. The JH branch makes its
reentrancy contract explicit for callers and higher-level adaptors
within the JH Ranges Toolkit.
