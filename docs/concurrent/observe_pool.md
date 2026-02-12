# 🎍 **JH Toolkit — `jh::observe_pool` API Reference**

📁 **Header:** `<jh/concurrent/observe_pool.h>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## Overview

`jh::observe_pool<T>` is a **top-level, user-facing pooling interface** for
**content-based interning of immutable or structurally immutable objects**.

It is built directly on top of
[`jh::conc::pointer_pool<T, Hash, Eq>`](pointer_pool.md)
and provides **duck-typed convenience** by automatically selecting:

* `jh::weak_ptr_hash<T>`
* `jh::weak_ptr_eq<T>`

All **concurrency, lifetime, cleanup, and resizing semantics** are inherited
*unchanged* from `pointer_pool`.

> `observe_pool` is **not a new abstraction**,
> it is the **intended user entry point** for pointer-based interning.

---

## Aggregate Header Availability

`jh::observe_pool` is available from the **same aggregate headers** as other pool facilities:

### `#include <jh/pool>`

* Pool-only facilities
* Intended for:

    * object interning
    * resource reuse
    * GC-style containers

### `#include <jh/concurrency>`

* Full concurrency toolkit
* Includes all `jh::conc::*` primitives

The availability mirrors `resource_pool` exactly;
only **behavioral semantics differ**.

---

## Relationship to `pointer_pool`

`observe_pool<T>` is defined as:

```cpp
using observe_pool =
    jh::conc::pointer_pool<
        T,
        jh::weak_ptr_hash<T>,
        jh::weak_ptr_eq<T>
    >;
```

However, this is **not** a trivial alias in spirit.

### What Is Fixed

* Hashing is performed on `std::weak_ptr<T>`
* Equality is performed on `std::weak_ptr<T>`
* Both delegate to **logical content of `T`**

### What Is *Not* Changed

* construct-first lookup model
* weak observation (no ownership)
* adaptive capacity management
* cleanup behavior
* concurrency guarantees
* platform limitations

For **all semantic questions**, refer to: [`jh::conc::pointer_pool`](pointer_pool.md)

---

## Hash and Equality Semantics

### `weak_ptr_hash<T>`

* If the weak pointer is expired → returns `0`
* Otherwise:

    * locks once
    * applies unified `jh::hash<T>` to `*T`

Hash deduction follows this precedence chain:

```text
std::hash<T>
→ hash(t) via ADL
→ t.hash()
```

### `weak_ptr_eq<T>`

* If either pointer is expired → `false`
* Otherwise:

    * locks both
    * compares via `T::operator==`

---

## Type Requirements (Strict)

`jh::observe_pool<T>` is **only available** if **all** of the following hold:

* `T` satisfies `jh::concepts::extended_hashable`
* `T` supports `operator==`
* hash and equality are:

    * **stable**
    * **identity-defining**
    * **not affected by mutation**

If **hash or equality cannot be deduced**,
`observe_pool<T>` is **ill-formed**.

This is **identical** to `pointer_pool`’s constraints.

---

## Identity and Immutability Model

Because lookup requires **full object construction**, identity must be:

* intrinsic to `T`
* stable for the object’s entire pooled lifetime

Two valid models exist:

### 1. Fully Immutable Objects

* all fields participate in identity
* no mutation after construction

### 2. Structurally Immutable Objects (Recommended)

* only a **subset of fields** defines identity
* identity-defining fields never change
* other fields may be mutable or lazily initialized

---

## Recommended Two-Phase Construction Pattern

Because **temporary objects may be discarded**, construction should be cheap.

Recommended approach:

1. **Eagerly construct identity-defining state**

    * fields used by `hash` / `operator==`
2. **Defer heavy initialization**

    * lazy allocation
    * `std::once_flag`
    * first-use initialization

This ensures:

* discarded temporaries are cheap
* only canonical instances pay full initialization cost

---

## Ownership and Lifetime

* The pool **never owns objects**
* All ownership is via returned `std::shared_ptr<T>`
* Internally, the pool stores only `std::weak_ptr<T>`

Consequences:

* destroying the pool does **not** destroy objects
* clearing the pool removes only observation records
* expired entries are cleaned opportunistically

---

## Concurrency and Safety

* Concurrent `acquire()` is safe
* Deduplication is atomic
* Returned `shared_ptr` objects remain valid regardless of pool lifetime

### Windows-Specific Warning (Critical)

On POSIX platforms, `pointer_pool` / `observe_pool` behavior is generally stable in practice,
as `std::shared_mutex` implementations tend to exhibit strong global ordering characteristics.

On Windows, however:

* ISO C++ guarantees only acquire–release semantics for `std::shared_mutex`.
* `std::weak_ptr` / `std::shared_ptr` rely on atomic reference counting.
* `unordered_map` introduces additional internal synchronization and rehash behavior.

The interaction between:

* atomic reference-count operations (`shared_ptr` control blocks),
* `std::shared_mutex`,
* and `unordered_map` bucket management,

creates a more complex memory-ordering surface than `flat_pool`.

Even though:

* `std::atomic_thread_fence(std::memory_order_seq_cst)` is inserted around lock boundaries,
* the implementation is algorithmically **DRF**,
* no undefined behavior exists at the ISO C++ level,

empirical testing shows that:

* pointer-based pools (`pointer_pool` / `observe_pool`)
  are **more sensitive to Windows runtime + MinGW coupling**
  than contiguous-storage `flat_pool` / `resource_pool`.
* Under extreme contention, instability manifests earlier than in `flat_pool`.

This is likely due to:

* additional indirection layers,
* `weak_ptr` lock operations,
* `unordered_map` rehash activity,
* and weaker cross-domain ordering between atomics and system locks.

Important:

* Neither `observe_pool` nor `flat_pool` guarantees high-pressure stability on Windows.
* Both remain standards-compliant and DRF.
* Behavior may degrade under extreme multi-core contention.

Official guidance:

* Windows is treated as a **compatibility platform**, not a strong-ordering baseline.
* Use low to moderate concurrency levels on Windows.
* For maximum robustness under heavy load, prefer POSIX platforms.
---

## Intended Use Cases

`observe_pool` is intended for:

* non-copyable objects
* non-movable objects
* objects without external keys
* objects requiring stable pointer identity
* content-based deduplication

Typical examples:

* immutable resource descriptors
* handle-like objects
* structural singletons
* identity-bound runtime objects

---

## When *Not* to Use `observe_pool`

Do **not** use `observe_pool` if:

* a stable external key exists → use `resource_pool`
* objects are copyable or movable → use `flat_pool`
* large-scale storage is required
* cache locality is critical
* high-concurrency workloads are expected

In many cases, a better alternative is:

```cpp
jh::resource_pool<Key, std::shared_ptr<T>>
```

This preserves pointer stability while moving identity to a key.

---

## Design Summary

* `observe_pool` is **not a simplification**
* it is a **semantic specialization**
* hash / eq are lifted to `weak_ptr` level
* all constraints and limits remain
* construct-first cost is fundamental

---

## One-Sentence Summary

> **`jh::observe_pool` is the user-facing, content-based interning pool for immutable objects, built on `pointer_pool`,
fixing weak-pointer hashing and equality while preserving all semantic constraints and limitations.**
