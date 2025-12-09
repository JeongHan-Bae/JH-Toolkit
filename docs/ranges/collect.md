# 🌗 **JH Toolkit — `jh::ranges::collect` API Reference**

📁 **Header:** `<jh/ranges/collect.h>`  
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

`jh::ranges::collect` is an **eager materialization adaptor** —
it terminates a lazy or proxy-based range pipeline and realizes it into a concrete container **`C`**.  
It represents the **materialization half** of C++23's proposed [
`std::ranges::to`](https://en.cppreference.com/w/cpp/ranges/to).

Unlike [`jh::ranges::to`](./to.md), which performs *structural construction* of a closable container,
`collect` performs *explicit evaluation* — converting any compatible range into a stable, value-semantic container.

This adaptor defines a clear **evaluation boundary** within a pipeline:  
it marks where deferred computation stops and data becomes concrete.

---

## 🔹 Definition

```cpp
namespace jh::ranges {

template <typename C>
inline constexpr collect_fn<C> collect{};

}
```

---

## 🔹 Interface

### 1. Direct form

```cpp
template <typename C, std::ranges::range R>
requires jh::concepts::collectable_container_for<C, R>
constexpr auto collect(R&& r);
```

### 2. Pipe form

```cpp
template <typename C>
constexpr auto collect();
```

### Equivalent usage

```cpp
auto v1 = jh::ranges::collect<std::vector<int>>(range);
auto v2 = range | jh::ranges::collect<std::vector<int>>();
```

---

## 🔹 Description

`collect()` performs **explicit eager evaluation** of a range into a concrete container `C`,
consuming the entire source range and producing a fully materialized result.

It is governed by the [`collectable_container_for<C, R>`](../conceptual/collectable_container.md) concept,
which generalizes the possible construction mechanisms for a container `C` from range `R`.  
The adaptor automatically chooses the most appropriate form according to the capabilities of `C` and `R`.

When `C` and `R` are *structurally closable*,
`collect` automatically delegates to [`to_adaptor`](./to.md) for direct construction.  
Otherwise, it iteratively performs insertion or emplacement according to container traits.

---

## 🔹 Behavior

The evaluation process follows a **priority chain** — each later step applies only when the former is not viable:

| Step | Condition                                                                                                                                | Action                                                                                     | Description                                                               |
|------|------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------|---------------------------------------------------------------------------|
| ①    | `C` and `R` satisfy [`closable_container_for`](../conceptual/closable_container.md)                                                      | Delegate to `to_adaptor<C>(r)`                                                             | Uses direct constructor or underlying adapter.                            |
| ②    | Otherwise, `C` and `R` satisfy [`collectable_container_for`](../conceptual/collectable_container.md) <br> (direct insertion/emplacement) | Iterate through the range and perform `emplace_back`, `push_back`, `emplace`, or `insert`. | Standard path for all common containers.                                  |
| ③    | When the element type is **tuple-like** and direct insertion is invalid                                                                  | Unpack tuple-like elements and forward them to `emplace_back` / `emplace`.                 | Enables reconstruction from proxy elements such as `zip_reference_proxy`. |

Additionally, if the container type supports `reserve()` and the range models `sized_range`,
capacity is **preallocated before any insertion**, regardless of which semantic path is taken.  
This optimization applies orthogonally to all steps above.

---

## 🔹 Example

```cpp
#include <jh/ranges/collect.h>
#include <jh/ranges/to.h>
#include <jh/ranges/views/enumerate.h>
#include <vector>
#include <unordered_map>
#include <memory_resource>
#include <string>

int main() {
    using namespace jh::ranges;

    std::vector<std::string> input_strings = {
        "apple", "banana", "cherry"
    };

    std::pmr::monotonic_buffer_resource pool;

    auto result = input_strings
        | views::enumerate()
        | collect<std::vector<std::pair<size_t, std::string>>>()
        | to<std::pmr::unordered_map<size_t, std::string>>(
              0,
              std::hash<size_t>{},
              std::equal_to<size_t>{},
              std::pmr::polymorphic_allocator<
                  std::pair<const size_t, std::string>
              >(&pool)
          );

    // result: unordered_map { {0,"apple"}, {1,"banana"}, {2,"cherry"} }
}
```

---

## 🔹 Design Semantics

`jh::ranges::collect` is the **materialization half** of the `collect + to` design pair.  
It focuses on *evaluation and normalization*, while `to` focuses on *construction and adaptation*.

| Adaptor   | Semantic Role                          | Concept                                                               | Accepts Extra Args | Stage      |
|-----------|----------------------------------------|-----------------------------------------------------------------------|--------------------|------------|
| `collect` | Eagerly realize and normalize data     | [`collectable_container_for`](../conceptual/collectable_container.md) | ❌ No               | Evaluation |
| `to`      | Structurally construct final container | [`closable_container_for`](../conceptual/closable_container.md)       | ✅ Yes              | Adaptation |

---

## ✅ Recommended usage

* **If your stream is consumptive or constructs by unpacking + emplace**,
  **and** your final container type has *high incremental insertion cost*
  (e.g. `std::set`, `std::map`, `std::unordered_map`, `std::flat_map`),  
  then even if your final type requires no extra constructor arguments,
  it is recommended to **materialize first**:

  ```cpp
  range
    | collect<std::vector<T>>()
    | to<std::set<T>>();
  ```

  This avoids redundant rebalancing, hashing, or allocator churn
  by constructing the final container in one structural step.

* **If your stream is non-consumptive** and your types are *directly closable*,
  you may safely use a single stage:

  ```cpp
  auto set1 = range | collect<std::set<T>>();
  auto set2 = range | to<std::set<T>>();
  ```

  In this case, `collect` and a parameter-less `to` are semantically equivalent —
  both perform direct construction through `to_adaptor` dispatch.

* **If your container type stores `std::pair`** (e.g. map-like containers),
  and your stream yields `tuple` or other tuple-like elements (such as from `enumerate`, `zip`, etc.),
  prefer using `collect`.  
  This ensures cross-library compatibility, since
  *libc++* allows implicit conversion from `tuple` to `pair`,
  while *libstdc++* does not.  
  Using `collect` guarantees portable code: both libraries will compile,
  differing only in the construction path used internally.

* **For `collect + to` pipelines**,
  `std::vector` is the *recommended intermediate container*:  
  it offers optimal cache locality, minimal insertion cost,
  and the broadest allocator and construction support.

---

## 🔹 Design Rationale

* **Explicit materialization boundary** — separates lazy and eager stages clearly.
* **Predictable semantics** — no hidden container construction or implicit evaluation.
* **Unified insertion model** — covers all STL-style insertion mechanisms.
* **Tuple unpack fallback** — reconstructs value objects from tuple-like proxies.
* **Composability with `to`** — integrates into a deterministic two-phase pipeline.

This separation makes pipeline intent clear:  
`collect` defines *when* data becomes concrete,
`to` defines *how* the final container is built.

---

## 🔹 Integration Notes

* Controlled by [`jh::concepts::collectable_container_for`](../conceptual/collectable_container.md).
* Delegates automatically to [`to_adaptor`](./to.md) when `closable_container_for` holds.
* Safe and idempotent — multiple `collect` applications yield the same result.
* Fully compatible with lazy JH and STL view adaptors (`zip`, `enumerate`, `transform`, etc.).
* Included automatically via `<jh/ranges_ext>` or directly with `<jh/ranges/collect.h>`.

---

## 🧩 Summary

`jh::ranges::collect` is the **deterministic eager materializer** of the JH Ranges framework.  
It explicitly ends lazy evaluation, normalizes data into a value-semantic container,
and forms the first half of the robust two-stage pipeline with [`jh::ranges::to`](./to.md).

> **In short:**  
> `collect` = *force evaluation and normalize data* →
> `to` = *construct and adapt final container*.
> Together, they provide explicitness, safety, and composability beyond `std::ranges::to`.
