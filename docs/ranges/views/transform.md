# 🔭 **JH Toolkit — `jh::ranges::views::transform` API Reference**

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
  meaning the range is non-consuming and the callable is a *pure observation*
  (returns a non-void, non-mutating value).
* `std::views::transform` — for all other combinations,
  including consumptive or stateful transformations.

This adaptor unifies both semantics under a single entry point,
preserving **reentrancy** when possible while maintaining full
compatibility with the standard C++23 pipeline model.

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

* If `vis_function_for<F, R>` is satisfied —
  meaning the transformation is observational and non-consuming —
  it delegates to [`jh::ranges::views::vis_transform`](vis_transform.md).
* Otherwise, it falls back to `std::views::transform`,
  preserving standard consumptive semantics.

This mechanism ensures that observational adaptors like
[`flatten`](flatten.md) and analytical pipelines automatically benefit
from non-consuming behavior, while normal data transformations
still follow standard range rules.

Dispatch occurs inside the call operator,
where both `R` and `F` are fully deduced,
ensuring correct compile-time routing.

---

## 🔹 Concept Dependency

The dispatch logic depends on
[`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r),
defined in [`jh/conceptual/range_traits.h`](../../conceptual/range_traits.md).

This concept models the **visual relation** between a callable and a range:

* The range supports stable, non-consuming traversal.
* The callable produces a readable, non-void value.
* The callable does not mutate elements or consume iteration state.

When these hold, the adaptor guarantees a *reentrant, observation-only* transformation.
Otherwise, `std::views::transform` is used, respecting consumption semantics.

---

## 🔹 Behavior

| Aspect                     | Description                                                                                               |
|----------------------------|-----------------------------------------------------------------------------------------------------------|
| **Dispatch rule**          | Compile-time selection between `vis_transform` and `std::views::transform`.                               |
| **Concept dependency**     | [`jh::concepts::vis_function_for<F, R>`](../../conceptual/range_traits.md#-concept--vis_function_forf-r). |
| **Consumption semantics**  | Preserved automatically — non-consuming if possible.                                                      |
| **Reentrancy**             | Inherited from the chosen underlying adaptor.                                                             |
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

    // Mutating transformation (consumptive)
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

In the first case, the transformation is non-consuming and can be
reiterated freely; in the second, the function mutates elements,
so the adaptor correctly falls back to `std::views::transform`.

---

## 🔹 Integration Notes

* Dispatch and concept checking occur per `(R, F)` pair.
* The adaptor never assumes non-consuming semantics unless proven by the concept.
* Internally, this mechanism is shared with [`vis_transform`](vis_transform.md)
  and used implicitly by observational adaptors such as [`flatten`](flatten.md).
* Depends on
  [`jh::concepts::vis_function_for`](../../conceptual/range_traits.md#-concept--vis_function_forf-r)
  defined in [`jh/conceptual/range_traits.h`](../../conceptual/range_traits.md).

---

## 🧩 Summary

`jh::ranges::views::transform` is a **semanticly adaptive transformation adaptor**
that unifies standard and observational behaviors.  
It preserves reentrancy automatically when applicable,
ensures consumption correctness otherwise,
and acts as the **central dispatch point** connecting
`std::views::transform`, `vis_transform`, and higher-level adaptors
within the JH Ranges Toolkit.
