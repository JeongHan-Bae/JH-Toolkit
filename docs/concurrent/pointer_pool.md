# 🎍 **JH Toolkit — `jh::conc::pointer_pool` API Reference**

📁 **Header:** `<jh/concurrent/pointer_pool.h>`  
📦 **Namespace:** `jh::conc`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## Overview

`jh::conc::pointer_pool<T, Hash, Eq>` is a **pointer-based interning container** designed for objects whose identity:

* is **intrinsic to the object itself**
* **cannot** be expressed via an external key
* requires **stable address identity**
* may be **non-copyable and non-movable**

The pool deduplicates objects using **full-object hashing and equality**, while ownership remains entirely with
`std::shared_ptr<T>` held by the user.

Internally, the pool stores **only `std::weak_ptr<T>`** for observation and reuse.
It never owns, destroys, or manages object lifetime.

---

## Design Philosophy

`pointer_pool` exists to support types that **cannot participate in key-based or contiguous storage models**.

Typical constraints include:

* non-copyable / non-movable types
* equality defined by full object state
* identity inseparable from the object itself
* mandatory pointer stability throughout lifetime

Because of these constraints:

* objects must be **constructed before lookup**
* deduplication is based on **object equality**, not keys
* fragmentation is **accepted and unavoidable**
* allocator customization is **intentionally avoided**

This container prioritizes **correctness and semantic identity** over storage efficiency.

---

### Lookup Model (Construct-First)

Unlike key-driven containers, `pointer_pool` does **not** provide `find()`.

The acquisition flow is always:

1. A **temporary object is constructed**
2. The pool performs a **hash-based lookup** using the object
3. If an equivalent object exists:

    * that instance is reused
    * the temporary object is discarded
4. Otherwise:

    * the new object becomes the canonical instance

This is required because hashing and equality depend on the object itself.

---

### Recommended Object Pattern

Because provisional construction may be frequent, objects should support **cheap identity construction**.

Recommended pattern:

* construct only **identity-defining fields** eagerly
* defer heavy or mutable initialization
* perform expensive setup lazily (e.g. via `std::once_flag`)

This ensures discarded temporary objects are inexpensive.

---

### Lifetime and Ownership Model

* The pool **never owns objects**
* All ownership resides in `std::shared_ptr<T>` returned to the user
* The pool stores only `std::weak_ptr<T>`

As a result:

* object lifetime is independent of the pool
* destroying the pool does **not** affect live objects
* expired entries are removed opportunistically

---

### Cleanup and Adaptive Capacity

Cleanup is **best-effort**:

* expired entries are removed during:

    * insertion
    * expansion
    * explicit cleanup calls

Capacity management is **adaptive**:

* cleanup occurs before expansion
* capacity grows if usage exceeds a high-watermark
* capacity shrinks if usage falls below a low-watermark
* shrinking is conservative (halving, never minimal-fit)

This avoids oscillation and rehash jitter under fluctuating workloads.

---

## Member Summary

### 🔒 Template Requirements

`pointer_pool<T, Hash, Eq>` requires:

* `Hash(weak_ptr<T>) -> size_t`
* `Eq(weak_ptr<T>, weak_ptr<T>) -> bool`

Hash and equality must reflect **object identity**.

---

### 🔢 Constants

| Member              | Type                 | Description                              |
|---------------------|----------------------|------------------------------------------|
| `MIN_RESERVED_SIZE` | `constexpr uint64_t` | Minimum reserved capacity (default: 16). |

---

### 🏗 Constructors

| Member                                                        | Description                                       |
|---------------------------------------------------------------|---------------------------------------------------|
| `explicit pointer_pool(uint64_t reserve = MIN_RESERVED_SIZE)` | Constructs a pool with initial reserved capacity. |

**Deleted / Restricted:**

| Member                             | Description                              |
|------------------------------------|------------------------------------------|
| Copy constructor / copy assignment | Disabled — pools must not be duplicated. |
| `acquire(...) const`               | Deleted — acquisition mutates the pool.  |

---

### 🔁 Move Semantics

| Member                   | Description                                          |
|--------------------------|------------------------------------------------------|
| Move constructor         | Transfers observation state; source pool is cleared. |
| Move assignment operator | Replaces observation scope atomically.               |

Move operations transfer **observation**, not ownership.

---

### 🔑 Acquisition

| Member                                  | Description                                                                      |
|-----------------------------------------|----------------------------------------------------------------------------------|
| `std::shared_ptr<T> acquire(Args&&...)` | Constructs a candidate object, deduplicates, and returns the canonical instance. |

Properties:

* construction happens **before locking**
* temporary objects may be discarded
* deduplication is atomic
* returned object is always owned by the caller

---

### 🧹 Maintenance

| Member                  | Description                                                 |
|-------------------------|-------------------------------------------------------------|
| `void cleanup()`        | Removes expired weak entries.                               |
| `void cleanup_shrink()` | Removes expired entries and conditionally shrinks capacity. |
| `void clear()`          | Clears all entries and resets capacity to minimum.          |

Notes:

* cleanup is safe at any time
* `clear()` removes observation only — objects remain alive if owned externally
* `clear()` is not recommended for structural resource pools

---

### 📊 Observers

| Member                      | Description                                        |
|-----------------------------|----------------------------------------------------|
| `uint64_t size() const`     | Number of stored weak entries (including expired). |
| `uint64_t capacity() const` | Current reserved capacity limit.                   |

---

## Concurrency and Safety

* Concurrent `acquire()` calls are safe
* Deduplication is atomic under exclusive locking
* Shared objects remain valid even if:

    * the pool is cleared
    * the pool is destroyed

The pool uses `std::shared_mutex` internally.

---

## Platform Notes (Windows)

On Windows environments based on **Universal CRT** (including MinGW):

* `shared_ptr` / `weak_ptr` may exhibit incorrect refcount synchronization
* `weak_ptr::lock()` may succeed on already-destroyed objects
* inserting `weak_ptr` into unordered containers may incur heavy jitter

As a result:

> High-pressure concurrent use of `pointer_pool` is **not recommended** on Windows UCRT-based toolchains.

---

## Intended Use Cases

`pointer_pool` is intended for objects that:

* cannot be copied or moved
* cannot be represented by an external key
* require full-object equality for deduplication
* require stable pointer identity

Typical examples:

* immutable resource handles
* structurally immutable objects
* objects with identity-bound invariants

---

## Comparison with `flat_pool`

| Aspect            | `pointer_pool`  | `flat_pool`             |
|-------------------|-----------------|-------------------------|
| Identity          | Object itself   | External key            |
| Storage           | Heap + pointers | Contiguous              |
| Lookup            | Construct-first | Lookup-before-construct |
| `find()`          | ❌ Not supported | ✅ Supported             |
| Fragmentation     | Inevitable      | Minimized               |
| Pointer stability | Guaranteed      | Not guaranteed          |
| Allocator control | ❌               | ✅                       |

`pointer_pool` exists **specifically** for cases where `flat_pool` is inapplicable.

---

## When *Not* to Use `pointer_pool`

Do **not** use `pointer_pool` if:

* object identity can be expressed by a key
* objects are copyable or movable
* contiguous storage is acceptable
* large-scale, cache-sensitive storage is required

In those cases, prefer `flat_pool` or `resource_pool`.

---

## Summary

> **`pointer_pool` is a weak-observed, pointer-stable interning container for objects whose identity is inseparable from
the object itself.**

* construct-first deduplication
* no ownership or destruction
* stable pointer identity
* best-effort cleanup
* adaptive capacity
* semantics-first design
