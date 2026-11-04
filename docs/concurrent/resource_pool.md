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

`jh::resource_pool<Key, Value, Alloc, Mutex>` is a **convenience interface** built on top of
`jh::conc::flat_pool<Key, Value, jh::hash<Key>, Alloc, Mutex>`.

It is available in the **`jh::` namespace**. Concurrency, lifetime, and storage
behavior follow [`jh::conc::flat_pool`](flat_pool.md).

---

## Aggregate Header Availability

You may include `jh::resource_pool` through **two different aggregate headers**, depending on
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

## Allocator customization

`resource_pool` selects `jh::hash<Key>` and exposes the allocator as its third
template argument. For example:

```cpp
jh::resource_pool<Key, Value, MyAllocator> pool;
```

---

## `resource_pool_set`

```cpp
jh::resource_pool_set<Key, Alloc, Mutex>
```

is an alias for:

```cpp
jh::resource_pool<Key, jh::typed::monostate, Alloc, Mutex>
```

The second template argument supplies the allocator for the set-like form.
Both aliases default `Mutex` to `std::shared_mutex`; the final template
argument can select a verified RW mutex for both internal lock sites.

---

## Custom hash policy

If `Key` resides in a **closed namespace** (notably `std`) and cannot provide:

* `std::hash<Key>`
* an ADL-visible `hash(key)`
* a `Key::hash()` member

then `jh::hash<Key>` cannot be resolved.

Typical example:

```cpp
using Key = std::variant<A, B, C>;
```

In such cases, supply a custom hash through the base interface:

```cpp
jh::conc::flat_pool<Key, Value, CustomHash, Alloc>
```
