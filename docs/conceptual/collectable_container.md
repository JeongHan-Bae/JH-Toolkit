# 🧩 **JH Toolkit — `jh::concepts::collectable_container` API Reference**

📁 **Header:** `<jh/conceptual/collectable_container.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

### 🧭 Submodule Notice

* Defines the **incremental population model** used by [`jh::ranges::collect`](../ranges/collect.md).
* Serves as the **fallback concept** when a container–range pair is not directly constructible (`closable_container_for`).

---

## 🧩 Introduction

A container is **collectable** from a range
if it is **closable**, or can be safely populated element by element
using one of its insertion operations (`emplace_back`, `push_back`, `emplace`, or `insert`),
without requiring constructor arguments or ownership transfer.

`jh::concepts::collectable_container_for<C, R>`
is the **superset concept** of [`closable_container_for`](closable_container.md).
If `<C, R>` is closable, `collect` delegates internally to `to_adaptor`;
otherwise, it performs incremental insertion via `collect_adaptor`.

Thus, `collectable_container_for` ensures that `collect`
always has a valid, deterministic construction path —
either direct or incremental — depending on what the container supports.

---

## 🔹 Collection Modes

| Mode                    | Logical Pattern                                         | Description                                    |
|-------------------------|---------------------------------------------------------|------------------------------------------------|
| **closable**            | [`closable_container_for<C, R>`](closable_container.md) | Direct construction — handled by `to_adaptor`. |
| **emplace_back_direct** | `c.emplace_back(v)`                                     | Append elements in place.                      |
| **push_back_direct**    | `c.push_back(v)`                                        | Append elements by copy or move.               |
| **emplace_direct**      | `c.emplace(v)`                                          | Insert via emplacement at logical end.         |
| **insert_direct**       | `c.insert(v)`                                           | Insert into associative or ordered containers. |
| **emplace_back_unpack** | `c.emplace_back(get<0>(t), get<1>(t), …)`               | Tuple-like unpack into `emplace_back()`.       |
| **emplace_unpack**      | `c.emplace(get<0>(t), get<1>(t), …)`                    | Tuple-like unpack into `emplace()`.            |

Each mode represents a valid, deterministic strategy for populating `C` from `R`.
If none apply, the pair is not `collectable_container_for`.

---

## 🔹 Core Concept

### `collectable_container_for<C, R>`

Indicates that container `C` can be constructed or incrementally filled
from range `R` according to one of the above modes.

#### Requirements

* `R` models `std::ranges::input_range`.
* A valid collection mode exists (`collectable_status != none`).

#### Semantics

If the concept holds:

* `collect` can materialize `C` directly from `R`.
* If `C` is closable, construction is handled by `to_adaptor`;
  otherwise, elements are inserted through `collect_adaptor`.

---

## 🔹 Relationship with Other Modules

| Module                                     | Role                                   | Interaction                                                         |
|--------------------------------------------|----------------------------------------|---------------------------------------------------------------------|
| **`jh::ranges::collect`**                  | Materializes a container from a range. | Uses `collectable_container_for` as compile-time eligibility check. |
| **`jh::ranges::to`**                       | Performs direct construction.          | Invoked via `to_adaptor` when closable.                             |
| **`jh::concepts::closable_container_for`** | Defines direct constructibility.       | Subset of `collectable_container_for`.                              |
| **`jh::concepts::container_traits`**       | Deduces `container_value_t`.           | Used for type compatibility checks.                                 |

---

## 🔹 Summary

* `collectable_container_for` **extends** `closable_container_for`.
* Ensures `collect` can always form a valid container, either by direct construction or incremental insertion.
* Serves as the compile-time backbone for the `collect` pipeline — minimal, deterministic, and extensible.
