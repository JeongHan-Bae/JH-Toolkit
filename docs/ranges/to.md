# 🌗 **JH Toolkit — `jh::ranges::to` API Reference**

📁 **Header:** `<jh/ranges/to.h>`  
📦 **Namespace:** `jh::ranges`  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](range_ext.md)

</div>

---

## 🧭 Introduction

`jh::ranges::to` is the **construction adaptor** complementing [`jh::ranges::collect`](./collect.md).  
While `collect` materializes data eagerly, `to` **finalizes** the pipeline by constructing
a target container **`C`** directly from a compatible range **`R`**.

`jh::ranges::to` requires the pair `(C, R)` to satisfy
[`closable_container_for<C, R>`](../conceptual/closable_container.md).
Its construction path is selected at compile time from the documented order
below. [`collect`](./collect.md) covers supported insertion and tuple-unpacking
paths when a separate materialization stage is useful.

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

### Example calls

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
This division makes the materialization stage explicit.

---

## 🔹 Behavior

`to()` checks these construction paths in priority order at compile time:

| Priority | Condition                                             | Strategy                      | Description                                                       |
|----------|-------------------------------------------------------|-------------------------------|-------------------------------------------------------------------|
| ①        | `C` constructible from `(begin(r), end(r))`           | **Direct range construction** | Invokes the container's iterator constructor.                    |
| ②        | `C` constructible from a whole vector of element type | **`via_vector_whole`**        | Materializes a vector, passes it by move.                         |
| ③        | `C` constructible via move iterators                  | **`via_vector_move`**         | Builds a vector, then passes move iterators.                       |
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

`jh::ranges::to` requires closability and checks the documented construction
paths in order. Insertion and tuple reconstruction belong to `collect`.

---

## 🔹 Return construction and materialization costs

The return expression determines which copy-elision rule applies:

| Code path | Return expression | C++17 and later guarantee |
|-----------|-------------------|---------------------------|
| `to_adaptor<C>` and the closable branch of `collect_adaptor<C>` | `return C(...)` or another `C` prvalue | The returned `C` result is initialized directly; no copy or move of that result object is required. |
| Insertion branch of `collect_adaptor<C>` | `C c{}; ...; return c;` | NRVO is permitted but optional. If omitted, the result is moved or copied according to the available operations. |

These rules concern the returned **container object**. They do not select how
its elements are transferred. The direct iterator path can copy elements;
`via_vector_move` explicitly uses move iterators; `via_vector_whole` moves the
intermediate vector into a compatible constructor. The selected container
constructors determine further element work and allocations. A whole-container
move can transfer management of the intermediate buffer when that constructor
and its allocator conditions support it; this avoids element-wise copying in
that case. A move-iterator path instead visits elements and requests a move
for each one.

`collect + to` can save time when repeated insertion into the final container
costs more than materializing an intermediate container and then constructing
the final container from it. Compare those two total costs for the actual
container and selected dispatch path. The intermediate container can also add
an allocation and a traversal, so this is a workload-dependent choice.

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

  `std::vector` is a practical intermediate type when contiguous storage and
  append operations suit the input and the final constructor accepts it.

---

## 🔹 Design Rationale

* **Strict closability enforcement** — prevents ambiguous runtime fallbacks.
* **Ordered performance dispatch** — prefers the most efficient constructor path first.
* **Explicit materialization** — `collect` makes the intermediate stage visible.
* **Explicit adaptability** — supports allocator / comparator / hash customization directly.
* **Auditable construction** — the selected path shows where moves and copies may occur.

The C++23 [`std::ranges::to`](https://eel.is/c++draft/range.utility.conv.to)
also specifies an ordered set of construction and insertion cases. JH Toolkit
places insertion and tuple reconstruction in `collect`, and documents the
priority of its `to` paths above. Neither interface guarantees a particular
runtime cost for the selected container's constructor.

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
It enforces compile-time structural validity and follows the documented
construction order. The JH model separates two stages:
`collect` handles unpacking and realization,
while `to` performs structural construction and parameter binding.

This strict separation enables:

* precise compile-time diagnostics (clear failure location between materialization and construction);
* an explicit place to compare the costs of insertion and intermediate construction;
* explicit semantic boundaries that reflect actual cost and intent;
* tuple-like proxy unpacking through `collect` when its constraints select that path.

> **In short:**  
> `collect` = *evaluate & unpack*  
> → `to` = *construct & adapt structurally*  
> → together = *an explicit two-stage container conversion when the workload benefits from it*.

