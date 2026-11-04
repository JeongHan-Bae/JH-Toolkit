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

`jh::observe_pool<T, Mutex = std::shared_mutex>` is a **top-level, user-facing pooling interface** for
**content-based interning of immutable or structurally immutable objects**.

It is built directly on top of
[`jh::conc::pointer_pool<T, Hash, Eq, Mutex>`](pointer_pool.md)
and provides **duck-typed convenience** by automatically selecting:

* `jh::weak_ptr_hash<T>`
* `jh::weak_ptr_eq<T>`

All **concurrency, lifetime, cleanup, and resizing semantics** are inherited
*unchanged* from `pointer_pool`.

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

`observe_pool<T, Mutex>` is defined as:

```cpp
template<typename T, typename Mutex = std::shared_mutex>
using observe_pool =
    jh::conc::pointer_pool<
        T,
        jh::weak_ptr_hash<T>,
        jh::weak_ptr_eq<T>,
        Mutex
    >;
```

### What Is Fixed

* Hashing is performed on `std::weak_ptr<T>`
* Equality is performed on `std::weak_ptr<T>`
* Both delegate to **logical content of `T`**
* `Mutex` defaults to `std::shared_mutex` and may be replaced by a verified RW mutex

Construction, ownership, cleanup, capacity, and concurrency behavior follow
[`jh::conc::pointer_pool`](pointer_pool.md).

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

* Concurrent `acquire()` uses the pointer pool's RW mutex for capacity checks
  and insertion
* Deduplication is atomic
* Returned `shared_ptr` objects remain valid regardless of pool lifetime

### Platform notes

`observe_pool` uses the same locking and pointer-lifetime behavior as
`pointer_pool`. See its [concurrency warning](pointer_pool.md#concurrency-and-safety)
and [platform notes](pointer_pool.md#platform-notes-windows)
and the [pool platform support note](overview.md#pool-platform-support).

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
* high-contention workloads require a platform review; see the
  [pool platform support note](overview.md#pool-platform-support)

In many cases, a better alternative is:

```cpp
jh::resource_pool<Key, std::shared_ptr<T>>
```

This preserves pointer stability while moving identity to a key.
