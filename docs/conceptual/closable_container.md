# 🧩 **JH Toolkit — `jh::concepts::closable_container` API Reference**

📁 **Header:** `<jh/conceptual/closable_container.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

### 🧭 Submodule Notice

* `jh::concepts::closable_container` defines the **structural rule set**
  describing when a container `C` can be *directly constructed ("closed")*
  from a range `R` without mutation or insertion.
* This concept is used internally by [`jh::ranges::to`](../ranges/to.md)
  and [`jh::ranges::collect`](../ranges/collect.md)
  to ensure type-safe, deterministic range–container compatibility.

---

## 🧩 Introduction

A container is said to be **closable** from a range
if it can be constructed directly from that range's iterator interface
or through a deterministic bridge sequence,
without requiring element-wise insertion or mutation.

`jh::concepts::closable_container_for<C, R>` captures this compile-time relationship.  
It verifies that the container `C` exposes constructors capable of accepting
data from range `R` in a standard, non-destructive way.

Closability is **bidirectional** —
it depends not only on whether the container can accept data,
but also on whether the range can be safely and repeatedly consumed.  
A range may fail closability if it represents a **consuming view**,
one that invalidates or mutates its underlying source when traversed.

For example, `std::ranges::transform_view` is a consuming view:  
its projection function may depend on transient state or modify elements,
so it cannot be directly closed into a container.

By contrast, [`jh::ranges::views::transform`](../ranges/views/transform.md)
performs **semantic dispatch** between two behaviors:  
if the pair `<R, F>` satisfies `jh::concepts::vis_function_for`,
it routes to `jh::ranges::views::vis_transform` — a non-consuming, reentrant view
that remains closable;  
otherwise, it falls back to the standard `std::views::transform`, preserving consumption semantics.

Lazy but non-consuming views, such as `std::ranges::zip_view`,
remain fully closable because they do not alter their source state
and support reentrant traversal.

If a pair `<C, R>` is **not closable**,
then it cannot be used with [`jh::ranges::to`](../ranges/to.md).  
In such cases, you should instead use
[`jh::ranges::collect`](../ranges/collect.md),
which first stabilizes or materializes the range into a concrete container
and then constructs the final target.

This model enables `jh::ranges::to` to perform direct, declarative construction
only when the range–container pair is provably safe and non-consuming.

---

## 🔹 Design Intent

* **Declarative deduction:**
  Construction paths are discovered through `requires` expressions — no traits or SFINAE required.
* **Non-intrusive extensibility:**
  User containers that already expose standard constructors participate automatically.
* **Deterministic priority:**
  Evaluation follows a fixed, compile-time order to guarantee predictable results.
* **Non-destructive semantics:**
  Source ranges are only observed via iterators; no ownership transfer occurs.
* **Static validation only:**
  The concept never performs runtime checks or dispatch.

---

## 🔹 Closability Modes

The JH framework recognizes five canonical construction modes.
These are tested in deterministic order until a match is found:

| Mode                          | Logical Pattern                                                 | Description                                                                                                                |
|-------------------------------|-----------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------|
| **1. Direct construction**    | `C(begin(r), end(r))`                                           | The container natively supports construction from iterator pairs.                                                          |
| **2. Vector bridge — whole**  | `C(std::vector<T>(begin(r), end(r)))`                           | The container can be initialized from a fully materialized sequence (bridge object).                                       |
| **3. Vector bridge — move**   | `C(make_move_iterator(v.begin()), make_move_iterator(v.end()))` | The container supports move-based initialization from an intermediate vector.                                              |
| **4. Vector bridge — copy**   | `C(v.begin(), v.end())`                                         | The container accepts copy-based initialization from an intermediate vector.                                               |
| **5. Adapter via underlying** | `C(Underlying(...))`, where `Underlying = C::container_type`    | Adapter containers (e.g. `std::stack`, `std::queue`, `std::priority_queue`) that build from their underlying storage type. |

Each represents a progressively weaker but still valid construction strategy.
Once a match is found, deduction stops — guaranteeing deterministic compile-time resolution.

---

## 🔹 Core Concept

### `closable_container_for<C, R>`

Indicates that a container `C` can be safely and deterministically constructed
from a range `R` through one of the recognized closability modes.

#### Requirements

* `R` must satisfy `std::ranges::input_range`.
* At least one valid construction mode exists for the pair `(C, R)`.

#### Semantics

If this concept is satisfied, the pair `(C, R)` can be passed to
[`jh::ranges::to`](../ranges/to.md) for direct construction.
The source range remains valid and unmodified — the concept only confirms structural viability.

---

## 🔹 Relationship with Other Modules

| Module                               | Purpose                                                         | Interaction                                                          |
|--------------------------------------|-----------------------------------------------------------------|----------------------------------------------------------------------|
| **`jh::ranges::to`**                 | Performs final container construction from a range.             | Requires `closable_container_for<C, R>` to hold.                     |
| **`jh::ranges::collect`**            | Materializes non-closable ranges into stable, value containers. | Uses the concept to identify when a direct construction path exists. |
| **`jh::concepts::container_traits`** | Supplies `container_value_t` and related value-type deduction.  | Supports closability evaluation.                                     |

`closable_container_for` itself performs **no construction** —
it only defines the compile-time conditions that make such construction legal.

---

## 🔹 Semantic Guarantees

| Guarantee         | Description                                                                           |
|-------------------|---------------------------------------------------------------------------------------|
| **Safety**        | Construction through recognized modes never invalidates or consumes the source range. |
| **Determinism**   | A given container–range pair always resolves to the same closability outcome.         |
| **Extensibility** | Any user-defined container can participate if its constructors satisfy the model.     |
| **Isolation**     | The concept is pure — no allocation, mutation, or side effects.                       |
| **Integration**   | Serves as the conceptual dependency for `jh::ranges::to` and `jh::ranges::collect`.   |

---

## 🧩 Summary

* `closable_container_for` defines the **structural compatibility**
  between a range and a container.
* It is a **compile-time predicate**, verifying that construction
  can occur safely and deterministically through standard constructors.
* The concept enables the adaptor layer (`to` / `collect`)
  to perform direct construction without user intervention.
* It does not describe or imply runtime optimization — only static capability.
