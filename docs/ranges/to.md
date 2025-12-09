# 🌗 **JH Toolkit — `jh::ranges::to` API Reference**

📁 **Header:** `<jh/ranges/to.h>`  
📦 **Namespace:** `jh::ranges`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](ranges_ext.md)

</div>

---

## 🧭 Introduction

`jh::ranges::to` is the **construction adaptor** complementing [`jh::ranges::collect`](./collect.md).  
While `collect` materializes data eagerly, `to` **finalizes** the pipeline by constructing
a target container **`C`** directly from a compatible range **`R`**.

It corresponds to the *closable half* of the C++23 `std::ranges::to` proposal —
but with a **stricter and clearer semantic model**.  
Unlike the STL version, which performs multiple implicit fallbacks,
`jh::ranges::to` requires the pair `(C, R)` to satisfy
[`closable_container_for<C, R>`](../conceptual/closable_container.md),
ensuring **deterministic**, **optimized**, and **compile-time guaranteed** behavior.

> `jh::ranges::collect` always succeeds by iteration;
> `jh::ranges::to` succeeds only when structural construction is possible.
> This strict separation eliminates ambiguity and runtime fallbacks.

---

## 🔹 Definition

```cpp
namespace jh::ranges {

template <typename C>
inline constexpr to_fn<C> to{};

}
```

---

## 🔹 Interface

### 1. Direct form

```cpp
template <typename C, std::ranges::range R, typename... Args>
requires jh::concepts::closable_container_for<C, R, std::tuple<std::remove_cvref_t<Args>...>>
constexpr auto to(R&& r, Args&&... args);
```

### 2. Pipe form

```cpp
template <typename C, typename... Args>
constexpr auto to(Args&&... args);
```

### Equivalent usage

```cpp
auto v1 = jh::ranges::to<std::vector<int>>(range);
auto v2 = range | jh::ranges::to<std::pmr::vector<int>>(alloc);
```

---

## 🔹 Description

`jh::ranges::to()` constructs a target container `C` directly from a compatible range `R`,
optionally forwarding additional constructor arguments such as allocators or policy objects.

It is valid only when the `(C, R)` pair satisfies
[`closable_container_for`](../conceptual/closable_container.md).  
This means the container can be built *structurally* —
either through iterator constructors or dedicated range-adapting constructors —
without element-by-element insertion.

If the pair is not closable,
users should materialize the intermediate result via [`collect`](./collect.md) first,
then call `to`.  
This division enforces **explicitness**, **clarity**, and **compile-time optimization**.

---

## 🔹 Behavior

`to()` follows a deterministic cost-ordered dispatch model.
Each construction path is checked at compile time and chosen based on efficiency:

| Priority | Condition                                             | Strategy                      | Description                                                       |
|----------|-------------------------------------------------------|-------------------------------|-------------------------------------------------------------------|
| ①        | `C` constructible from `(begin(r), end(r))`           | **Direct range construction** | Invokes container's native range constructor — the optimal path.  |
| ②        | `C` constructible from a whole vector of element type | **`via_vector_whole`**        | Materializes a vector, passes it by move.                         |
| ③        | `C` constructible via move iterators                  | **`via_vector_move`**         | Uses `std::make_move_iterator` to transfer ownership efficiently. |
| ④        | `C` constructible via copy iterators                  | **`via_vector_copy`**         | Fallback to element copy if no move path exists.                  |
| ⑤        | `C` is an adaptor (e.g. `std::stack`, `std::queue`)   | **adapter_via_underlying**    | Builds the underlying container, then wraps it.                   |

This model is evaluated entirely at compile time —
no runtime branching or dynamic resolution occurs.

---

## 🔹 Example

```cpp
#include <jh/ranges/collect.h>
#include <jh/ranges/to.h>
#include <jh/ranges/views/enumerate.h>
#include <deque>
#include <memory_resource>
#include <vector>

int main() {
    using namespace jh::ranges;

    std::pmr::monotonic_buffer_resource pool;

    // Enumerate + materialize + construct a PMR deque
    auto pmr_deque = std::views::iota(0, 8)
        | std::views::transform([](int x) { return x * x; })
        | collect<std::vector<int>>()
        | to<std::pmr::deque<int>>(std::pmr::polymorphic_allocator<int>(&pool));

    // Result: pmr::deque<int> {0, 1, 4, 9, 16, 25, 36, 49}
}
```

Here:

* `collect` forces evaluation and type normalization into `std::vector<int>`,
  suitable for subsequent direct construction.
* `to` finalizes the pipeline by constructing a `std::pmr::deque<int>`
  using the provided polymorphic allocator.

This demonstrates the exact design intention:
clear evaluation → structural construction → explicit resource binding.

---

## 🔹 Design Semantics

`jh::ranges::to` represents the **structural half** of the `collect + to` design pair.

| Adaptor   | Role                                | Concept                                                               | Accepts Extra Args | Stage      |
|-----------|-------------------------------------|-----------------------------------------------------------------------|--------------------|------------|
| `collect` | Eager normalization and realization | [`collectable_container_for`](../conceptual/collectable_container.md) | ❌ No               | Evaluation |
| `to`      | Structural container construction   | [`closable_container_for`](../conceptual/closable_container.md)       | ✅ Yes              | Adaptation |

Unlike `std::ranges::to`,
this version enforces a **strict closability requirement** and
**explicit construction path ordering** —
no ambiguous or hidden fallback conversions.

---

## 🔹 Optimization Semantics — `collect + to`

Although `collect` and `to` are semantically distinct,
their composition achieves **RVO-based object-level move** in practice.

Under C++17 guaranteed RVO and later standards (C++20+ here),
the intermediate container produced by `collect` is constructed
directly in the frame expected by `to`.  
Thus, even though no explicit `std::move` appears,
the compiler performs the entire operation as one contiguous construction sequence.

| Stage          | Behavior                                        | Effect                                |
|----------------|-------------------------------------------------|---------------------------------------|
| `collect<V>()` | Materializes into a temporary container `V`     | Constructed in caller frame under RVO |
| `to<C>(...)`   | Constructs `C` from `V` (via iterators or move) | Consumes `V` with move semantics      |
| Combined       | Two semantic stages → One physical construction | Zero-copy, zero-overhead execution    |

This means `collect + to` provides full clarity and composability,
yet remains *as efficient as direct constructor invocation*.

---

## 🔹 Recommended Usage

* **Use `to` directly**  
  when the range and container form a valid closable pair
  and you need to provide additional arguments (allocators, comparators, etc.):

  ```cpp
  auto pmr_deque = rng
      | to<std::pmr::deque<int>>(std::pmr::polymorphic_allocator<int>(&pool));
  ```

* **Use `collect + to`**  
  when the input range is non-closable, one-pass, or proxy-based
  (e.g. `enumerate`, `zip`, `transform`),
  to ensure complete materialization before structural construction:

  ```cpp
  auto result = range
      | collect<std::vector<int>>()
      | to<std::pmr::deque<int>>(std::pmr::polymorphic_allocator<int>(&pool));
  ```

  `std::vector` is recommended as the intermediate type
  due to its optimal locality, stable layout, and best compiler support for RVO.

---

## 🔹 Design Rationale

* **Strict closability enforcement** — prevents ambiguous runtime fallbacks.
* **Ordered performance dispatch** — prefers the most efficient constructor path first.
* **Zero-overhead composition** — merges seamlessly with `collect` under RVO.
* **Explicit adaptability** — supports allocator / comparator / hash customization directly.
* **Predictable lifetime semantics** — no hidden moves, no unintended copies.

Unlike STL's mixed-mode `std::ranges::to`,
`jh::ranges::to` maintains **semantic clarity**, **compilation determinism**,
and **consistent optimization behavior** across compilers.

---

## 🔹 Integration Notes

* Controlled by [`jh::concepts::closable_container_for`](../conceptual/closable_container.md).
* Fully compatible with both standard and JH views.
* Delegates through `to_adaptor` for construction dispatch.
* Included automatically via `<jh/ranges_ext>` or `<jh/ranges/to.h>`.

---

## 🧩 Summary

`jh::ranges::to` is the **deterministic structural constructor**
of the JH Ranges system — the closing stage of the `collect + to` pipeline.  
It enforces compile-time structural validity, follows cost-prioritized dispatch,
and, under RVO, achieves *object-level move equivalence*.

Unlike the STL `std::ranges::to`, which mixes materialization and construction in one template,
the JH model **explicitly separates** these stages:  
`collect` handles unpacking and realization,
while `to` performs structural construction and parameter binding.

This strict separation enables:

* precise compile-time diagnostics (clear failure location between materialization and construction);
* better compiler comprehension and optimization;
* explicit semantic boundaries that reflect actual cost and intent;
* unpacking and proxy reconstruction support — capabilities *not available in STL's version*.

> **In short:**  
> `collect` = *evaluate & unpack*  
> → `to` = *construct & adapt structurally*  
> → together = *semantically clearer, diagnostically stronger, and more expressive* than `std::ranges::to`.

