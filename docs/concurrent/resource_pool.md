# 🎍 **JH Toolkit — `jh::resource_pool` API Reference**

📁 **Header:** `<jh/concurrent/resource_pool.h>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## Overview

`jh::resource_pool<Key, Value, Alloc>` is a **convenience interface** built on top of  
`jh::conc::flat_pool<Key, Value, jh::hash<Key>, Alloc>`.

It exists in the **`jh::` namespace**, not as a foundational concurrent primitive, but as a
**practical, user-facing entry point** that removes unnecessary template friction for common
resource-pooling use cases.

All concurrency, lifetime, and storage semantics are **identical** to `jh::conc::flat_pool`.
No behavior is altered or restricted.

---

## Aggregate Header Availability

you may include `jh::resource_pool` through **two different aggregate headers**, depending on
your intent:

### `#include <jh/pool>`

* Provides **only pool-related facilities**
* Intended for users who want:

    * resource pools
    * object interning
    * GC-style reuse containers
* Excludes unrelated concurrency utilities

### `#include <jh/concurrency>`

* Aggregates **all headers under `jh/concurrent/*`**
* Intended for users who want:

    * concurrent pools
    * other concurrency primitives
* The exact contents are version-dependent —
  refer to **[`overview.md`](overview.md)** for the current module composition

This split allows users to choose between a **focused pool-only dependency** and a **full
concurrency toolkit**.

---

## Why `resource_pool` Exists

At first glance, `resource_pool` may appear redundant:

* `flat_pool` already defaults to `jh::hash`
* All features are available in the base template

However, this overlooks a **very common usability problem** found throughout the STL and PMR
ecosystems.

---

## Template Friction in Real Code

In theory, template parameters are flexible.
In practice, they accumulate noise.

A familiar example from the standard library:

```cpp
std::pmr::unordered_map<
    Key,
    Value,
    std::hash<Key>,
    std::equal_to<Key>
>
```

Here, the user's **only intent** is often:

> "I want to specify an allocator."

Yet they are forced to repeatedly spell out:

* hash
* equality
* defaulted policy types

This problem becomes more pronounced in generic code and public APIs.

---

## Parameter Ordering as an API Design Tool

`resource_pool` deliberately reorders *what users are expected to customize first*.

Conceptually, for `flat_pool`:

| Number of template parameters | Meaning                         |
|-------------------------------|---------------------------------|
| 1                             | **Set-like pool** (Key only)    |
| 2                             | **Map-like pool** (Key → Value) |
| 3                             | Custom hash                     |
| 4                             | Custom allocator                |

Most real-world use cases fall into:

* **(1)** or **(2)**
* occasionally **(4)**
* rarely **(3)**

Custom hash policies are uncommon.
Custom allocators are common.

---

## What `resource_pool` Fixes

`jh::resource_pool` makes two deliberate choices:

1. **Fix the hash policy** to `jh::hash<Key>`
2. **Expose the allocator as the last (and only optional) policy**

This allows users to write exactly what they mean:

```cpp
jh::resource_pool<Key, Value, MyAllocator> pool;
```

instead of repeatedly restating defaults they do not care about.

The same applies to the set-like form.

---

## `resource_pool_set`

```cpp
jh::resource_pool_set<Key, Alloc>
```

is an alias for:

```cpp
jh::resource_pool<Key, jh::typed::monostate, Alloc>
```

It exists for the same reason:

* eliminate fixed, repetitive template arguments
* emphasize intent
* keep allocator customization trivial

Conceptually:

* 1 parameter → set
* 2 parameters → set + custom allocator

No additional semantics are introduced.

---

## When *Not* to Use `resource_pool`

There is exactly one class of failure cases:

### Hash Cannot Be Deduced

If `Key` resides in a **closed namespace** (notably `std`) and cannot provide:

* `std::hash<Key>`
* an ADL-visible `hash(key)`
* a `Key::hash()` member

then `jh::hash<Key>` cannot be resolved.

Typical example:

```cpp
using Key = std::variant<A, B, C>;
```

In such cases, use the base interface directly:

```cpp
jh::conc::flat_pool<Key, Value, CustomHash, Alloc>
```

This is an explicit escape hatch, not a limitation.

---

## Design Summary

* `resource_pool` is **not a new abstraction**
* it is **not a restricted wrapper**
* it is a **zero-cost, intent-focused alias**

Its sole purpose is to:

> **Move allocator customization forward and push fixed policies out of the user's way.**

---

## One-Sentence Summary

> **`jh::resource_pool` is the practical, allocator-friendly entry point to `flat_pool`, designed to eliminate template
noise while preserving all concurrency and lifetime semantics.**
