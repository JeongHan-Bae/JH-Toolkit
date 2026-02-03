# 🧩 **JH Toolkit — `jh::concepts::mutex_like` API Reference**

📁 **Header:** `<jh/conceptual/mutex_like.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

### 🧭 Submodule Notice

* `jh::concepts::mutex_like` defines the **foundational synchronization concept layer**
  that governs all *lock-like* types within the JH Toolkit.
* It provides **compile-time validation** for mutex semantics —
  exclusive, shared, timed, and reentrant —  
  allowing structural or custom synchronization primitives
  to integrate seamlessly into higher-level systems such as guards or thread pools.

---

## 🧩 Introduction

This header formalizes the **behavioral definition of mutex-like synchronization objects**.  
Rather than identifying specific types, it models *what operations must exist*
for a type to act as a lockable synchronization primitive.

Each concept is purely **behavior-based**:  
no inheritance or base class requirements —
only the presence of valid locking interfaces such as `lock()`, `try_lock()`, or `lock_shared()`.

The system enables **zero-overhead structural conformance**,
ensuring any compliant type can be safely used in toolkit synchronization
and reentrance trait detection at compile time.

---

## 🔹 Concept Hierarchy

| Layer                    | Concepts                                                         | Description                                                        |
|:-------------------------|:-----------------------------------------------------------------|:-------------------------------------------------------------------|
| **Exclusive Lock Layer** | `basic_lockable`, `excl_lockable`, `timed_excl_lockable`         | Define blocking, tryable, and timed exclusive locks.               |
| **Shared Lock Layer**    | `shared_lockable`, `timed_shared_lockable`                       | Define reader-style shared locks and their timed variants.         |
| **Unified Lock Layer**   | `mutex_like`, `timed_mutex_like`, `rw_mutex_like`                | Combine exclusive and shared behaviors under unified abstractions. |
| **Reentrance Layer**     | `recursive_mutex`, `reentrant_mutex`, `reentrance_capable_mutex` | Capture recursive and idempotent reentrance traits.                |

---

## 🔹 Core Concepts

### `basic_lockable<M>`

Represents the **minimal lockable interface** —
requires `lock()` and `unlock()` only.  
Equivalent to the essential semantics of `std::mutex`.

---

### `excl_lockable<M>`

Extends `basic_lockable` by adding **try-lock capability** via `try_lock()`.  
Models objects supporting non-blocking exclusive acquisition.

---

### `timed_excl_lockable<M>`

Extends `excl_lockable` with timed operations:  
`try_lock_for()` and `try_lock_until()`.  
Covers timed mutexes such as `std::timed_mutex` or `std::recursive_timed_mutex`.

---

### `shared_lockable<M>`

Models **reader (shared) locks** that allow multiple concurrent readers.  
Requires `lock_shared()`, `unlock_shared()`, and `try_lock_shared()`.

---

### `timed_shared_lockable<M>`

Timed variant of `shared_lockable`.  
Adds `try_lock_shared_for()` and `try_lock_shared_until()` for time-based acquisition.

---

### `mutex_like<M>`

Generic mutex abstraction —
satisfied if the type supports either exclusive or shared locking.  
Represents any object that behaves as a synchronization primitive.

---

### `timed_mutex_like<M>`

Timed version of `mutex_like`.  
Satisfied if a type supports either `timed_excl_lockable` or `timed_shared_lockable`.

---

### `rw_mutex_like<M>`

Represents **read–write locks** —
types that implement both exclusive and shared interfaces,
such as `std::shared_mutex` or structural substitutes.

---

## 🔹 Reentrance Traits

### `recursive_registry<T>`

A **trait registry** detecting *counting-style reentrance*
(i.e., recursive locks with internal ownership depth).  
Specialized for `std::recursive_mutex` and `std::recursive_timed_mutex`.  
Defaults to `false_type`; extendable for user-defined recursive types.

---

### `reentrant_registry<T>`

A **trait registry** for *idempotent reentrance* —
locks that are structurally safe to re-lock in the same context
without internal counters (e.g., `jh::typed::null_mutex_t`).  
Defaults to `false_type`; extendable for user-defined reentrant types.

---

## 🔹 Reentrance Concepts

### `recursive_mutex<M>`

Represents **counting reentrant locks** allowing multiple acquisitions
by the same thread while tracking depth.  
Detected via `recursive_registry` or `M::is_recursive_tag`.

---

### `reentrant_mutex<M>`

Represents **structural reentrant locks**,
where repeated locking within the same context is a no-op.  
Detected via `reentrant_registry` or `M::is_reentrant_tag`.

---

### `reentrance_capable_mutex<M>`

Unified abstraction for any lock supporting **some form of reentrance**.  
Satisfied if either `recursive_mutex` or `reentrant_mutex` holds.

---

## 🧩 Summary

* The **`mutex_like` model** defines the complete conceptual lattice
  for mutex-style synchronization in the JH Toolkit.
* It provides **strong static contracts** ensuring semantic validity
  of exclusive, shared, timed, and reentrant locks.
* Through trait registries, it enables **zero-cost structural detection**
  of recursive or idempotent reentrance.
* The design promotes **non-intrusive extensibility** —
  user-defined locks can integrate purely through behavior and optional tag traits.
