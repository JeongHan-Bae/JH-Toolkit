# 🧱 **JH Toolkit — `jh::pool` API Reference**

📁 **Header:** `<jh/core/pool.h>`  
🔄 **Forwarding Header:** `<jh/pool>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/Back_to_README-blue?style=flat-square)](../../README.md)

</div>

---

## ⚙️ Overview

`jh::pool<T>` is a **duck-typed adapter** of [`jh::sim_pool`](sim_pool.md),
providing automatic hashing and equality deduction for immutable or structurally immutable types.

It infers both **hashing** and **equality** semantics for any type `T` that
implements a valid hashing method (see [`jh::hash`](../conceptual/hashable.md))
and supports logical equality comparison (`operator==()`).

This enables seamless, content-based deduplication of shared immutable objects
without manually specifying hash or equality functors.

---

## 🔹 Automatic Type Deduction (since 1.3.5)

From **version 1.3.5**, `jh::pool` supports full *duck-typed deduction*
through [`jh::hash<T>`](../conceptual/hashable.md).

For any type `T` that:

* is [`extended_hashable`](../conceptual/hashable.md) —
  meaning it defines `std::hash<T>`, an ADL-discoverable `hash(const T&)`, or a member `T::hash()`, and
* **supports equality comparison** via a logical `operator==()`,

the specialization `jh::pool<T>` automatically binds:

```cpp
jh::weak_ptr_hash<T>  // uses jh::hash<T> for content-based hashing
jh::weak_ptr_eq<T>    // delegates equality to T::operator==()
```

This allows the pool to recognize and merge logically equivalent objects
without requiring explicit hash or equality configuration.

> `jh::hash<T>` is a **deduction template**, not a registration point.  
> It automatically selects from `std::hash`, ADL `hash()`, or `T::hash()`
> — see [hashable.md](../conceptual/hashable.md) for complete rules.

---

## 🔹 Core Characteristics

| Property             | Description                                                       |
|----------------------|-------------------------------------------------------------------|
| **Base type**        | Inherits from `jh::sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T>>` |
| **Ownership model**  | Observes shared objects via `std::weak_ptr` (non-owning).         |
| **Deduplication**    | Content-based, using `jh::hash<T>` and `operator==()`.            |
| **Thread safety**    | Same as `sim_pool` — concurrent access via `std::shared_mutex`.   |
| **Cleanup model**    | Event-driven and opportunistic, identical to `sim_pool`.          |
| **Capacity policy**  | Adaptive resizing (doubling/halving, floor at 16).                |
| **Type requirement** | Type must be *extended-hashable* and support equality comparison. |

---

## 🧭 Design Rationale

`jh::pool` serves as a **semantic alias** for [`jh::sim_pool`](sim_pool.md),
removing the need to specify hash and equality traits manually.

It targets **immutable or structurally immutable** types —
where equality (`operator==`) and hashing (`hash()`) fully define identity semantics.  
Objects representing the same logical value are unified into one shared instance
while remaining independently managed through `std::shared_ptr`.

This design guarantees deterministic, thread-safe deduplication
without ownership coupling or intrusive lifetime management.

---

## 🔹 Hashing & Equality Behavior

| Mechanism      | Behavior                                                                         |
|----------------|----------------------------------------------------------------------------------|
| **Hashing**    | Automatically deduced through [`jh::hash<T>`](../conceptual/hashable.md).        |
| **Equality**   | Delegated to `T::operator==()` via `jh::weak_ptr_eq<T>`.                         |
| **Weak model** | Uses `std::weak_ptr` for observation; lifetimes remain independent of the pool.  |
| **Fallback**   | Non-hashable types must use `jh::sim_pool` with explicit hash/equality functors. |

---

## 🔬 API Breakdown

### 🏗️ **Construction**

```cpp
pool();
explicit pool(std::uint64_t reserve_size);
```

| Aspect               | Description                                                              |
|----------------------|--------------------------------------------------------------------------|
| **Purpose**          | Initializes the pool with or without specified capacity.                 |
| **Behavior**         | Delegates to `sim_pool` base constructor; pre-reserves internal storage. |
| **Default capacity** | `MIN_RESERVED_SIZE` (16).                                                |

---

### 🧩 **Object Acquisition**

```cpp
template<typename... Args>
std::shared_ptr<T> acquire(Args&&... args);
```

Retrieves an existing shared instance or creates a new one.

**Acquisition Flow**

1. Constructs a provisional object using forwarded arguments.
2. Acquires pool lock for lookup/insertion.
3. If an equivalent instance exists (`Eq`), it is reused.
4. Otherwise, inserts and returns the new shared instance.

| Aspect                    | Description                                                       |
|---------------------------|-------------------------------------------------------------------|
| **Hash source**           | `T::hash()`                                                       |
| **Equality rule**         | `T::operator==()` via `weak_ptr_eq<T>`                            |
| **Thread safety**         | Safe for concurrent access; atomic insertion.                     |
| **Construction strategy** | Construct-first, lock-late.                                       |
| **Discard rule**          | Provisional objects may be immediately released on deduplication. |

---

### 🧹 **Cleanup**

```cpp
void cleanup();
```

Removes expired weak references.

| Aspect       | Description                                         |
|--------------|-----------------------------------------------------|
| **Behavior** | Identical to `sim_pool`: event-driven, manual-safe. |
| **Effect**   | Does not affect live `shared_ptr` instances.        |

---

### 🧩 **Cleanup with Shrink**

```cpp
void cleanup_shrink();
```

Cleans up expired entries and conditionally halves capacity.

| Aspect            | Description                                             |
|-------------------|---------------------------------------------------------|
| **Shrink policy** | Conservative — capacity halved (or floored at 16).      |
| **Rationale**     | Prevents oscillation between expansion and contraction. |

---

### 📊 **Capacity Query**

```cpp
[[nodiscard]] std::uint64_t capacity() const;
[[nodiscard]] std::uint64_t size() const;
```

| Function          | Description                                                |
|-------------------|------------------------------------------------------------|
| `capacity()` | Returns current adaptive capacity.                         |
| `size()`          | Returns number of stored weak entries (including expired). |

---

### 🔄 **Clear**

```cpp
void clear();
```

Clears all observed entries and resets capacity to baseline.

| Aspect              | Description                                       |
|---------------------|---------------------------------------------------|
| **Effect**          | Removes only internal observation records.        |
| **Object lifetime** | Unaffected — external `shared_ptr`s remain valid. |

---

## ⚙️ Concurrency & Behavior

| Mechanism           | Description                                                      |
|---------------------|------------------------------------------------------------------|
| **Synchronization** | Shared/exclusive lock (read vs write).                           |
| **Atomicity**       | Insertions are atomic per element.                               |
| **Cleanup timing**  | On insertion or explicit call.                                   |
| **Destruction**     | Safe; external objects persist.                                  |
| **Thread model**    | Lock granularity minimal — lock held only for insertion/cleanup. |

---

## 🧩 Summary

`jh::pool` is the **specialized, type-inferred version** of [`jh::sim_pool`](sim_pool.md),
providing automatic hashing and equality for immutable types implementing `hash()` and `operator==()`.

| Aspect               | Description                                           |
|----------------------|-------------------------------------------------------|
| **Ownership**        | Observation only (`weak_ptr`-based).                  |
| **Deduplication**    | Content-defined via `hash()` and `operator==()`.      |
| **Cleanup**          | Event-driven, opportunistic.                          |
| **Resizing**         | Adaptive doubling/halving, floor at 16.               |
| **Thread Safety**    | Fully concurrent; shared/exclusive lock model.        |
| **Type Requirement** | Must expose `hash()` and `operator==()`.              |
| **Limitations**      | No implicit support for `std::hash<T>`.               |
| **Future Expansion** | `weak_ptr_hash` may support `std::hash<T>` detection. |

---

**Summary Statement:**

> `jh::pool` is the default interning facility for immutable identity-based objects
> within JH Toolkit. It automates `jh::sim_pool` for most C++ value types
> with built-in logical equivalence, offering the same thread-safe,
> adaptive, and deterministic behavior with zero ownership coupling.
