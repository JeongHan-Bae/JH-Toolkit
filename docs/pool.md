# 🧱 **JH Toolkit — `jh::pool` API Reference**

📁 **Header:** `<jh/pool.h>`  
🔄 **Forwarding Header:** `<jh/pool>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/Back_to_README-blue?style=flat-square)](../README.md)

</div>

---

## ⚙️ Overview

`jh::pool<T>` is a **duck-typed specialization** of [`jh::sim_pool`](sim_pool.md),
providing automatic hashing and equality deduction for immutable or structurally immutable types.  

It offers the same non-intrusive, weak-pointer–observed deduplication system as `sim_pool`,
but without the need to explicitly specify hash and equality functors for compatible types.  

---

## 🔹 Core Characteristics

| Property             | Description                                                        |
|----------------------|--------------------------------------------------------------------|
| **Base type**        | Inherits from `jh::sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T>>`  |
| **Ownership model**  | Observes shared objects via `std::weak_ptr` (non-owning)           |
| **Deduplication**    | Automatic via `hash()` and `operator==()`                          |
| **Thread safety**    | Same as `sim_pool` — concurrent read/write via `std::shared_mutex` |
| **Cleanup model**    | Event-driven, adaptive, identical to `sim_pool`                    |
| **Capacity policy**  | Doubles or halves adaptively; minimum 16                           |
| **Type requirement** | Type `T` must provide both `hash()` and `operator==()`             |
| **Lifetime model**   | Pool and object lifetimes fully decoupled                          |

---

## 🧭 Design Rationale

`jh::pool` provides a simplified interface to `jh::sim_pool`,
automatically applying weak-pointer–aware hashing and equality semantics for
types that define `T::hash()` and `T::operator==()`.

It is designed for **immutable identity types**,
where `hash()` and `operator==()` define complete logical equivalence.

This class acts as a **semantic alias** for the most common `sim_pool` configurations,
eliminating repetitive boilerplate for hash/equality wrappers.

---

## 🔹 Hashing & Equality Inference

| Mechanism    | Behavior                                                                                               |
|--------------|--------------------------------------------------------------------------------------------------------|
| **Hashing**  | Automatically deduced from `T::hash() const noexcept`.                                                 |
| **Equality** | Delegated via `jh::weak_ptr_eq<T>`, which locks `weak_ptr` and forwards to `T::operator==()`.          |
| **Fallback** | No fallback to `std::hash<T>` specialization — user must explicitly use `jh::sim_pool` for such cases. |

---

### ⚠️ Important Notes

1. **Automatic hashing requires `T::hash()`.**
   If the type only defines `std::hash<T>`, it is **not automatically supported**.
   Use `jh::sim_pool<T, YourHash, weak_ptr_eq<T>>` instead for such types.

2. **Equality semantics** reuse `T::operator==()` transparently through `weak_ptr_eq<T>`.
   The pool itself performs no comparison logic beyond pointer locking.

3. **Future expansion:**
   `jh::weak_ptr_hash` may support detection of `std::hash<T>` in a later release.
   This extension is currently *planned but not implemented*,
   and will be documented when added.

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
[[nodiscard]] std::uint64_t reserved_size() const;
[[nodiscard]] std::uint64_t size() const;
```

| Function          | Description                                                |
|-------------------|------------------------------------------------------------|
| `reserved_size()` | Returns current adaptive capacity.                         |
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
