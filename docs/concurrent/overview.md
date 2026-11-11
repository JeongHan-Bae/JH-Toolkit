# 🎍 **JH-Toolkit — Concurrency Module Overview**

📁 **Module:** `<jh/concurrency>`  
📦 **Namespace:** `jh::conc`  
📍 **Location:** `jh/concurrent/`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::conc` is the collection of concurrency-aware containers that keep synchronization inside each helper, letting
callers think in terms of resources (objects, keys, handles) rather than mutexes.

| Helper                                                | Role                                                                                            |
|-------------------------------------------------------|-------------------------------------------------------------------------------------------------|
| `occ_box<T>`                                          | OCC-wrapped mutable state with atomic snapshot readers and copy-and-replace commits.           |
| `flat_pool<Key, Value>` / `resource_pool<Key, Value>` | Contiguous key-driven pool for copyable/movable entries, exposing reference-counted handles.    |
| `pointer_pool<T>` / `observe_pool<T>`                 | Pools with their own handles for pointer-stable, immutable data; the latter auto-selects hash/equality. |

`<jh/concurrency>` aggregates every `<jh/concurrent/*.h>` helper into `jh::conc`. When you only need the alias-based
pools, include `<jh/pool>` so that `jh::observe_pool`/`jh::resource_pool` sit directly under `jh::`.

---

## 🔹 Core Components

| Component                                              | Header                            | Status   | Description                                                                                          |
|--------------------------------------------------------|-----------------------------------|----------|------------------------------------------------------------------------------------------------------|
| [`occ_box<T>`](occ_box.md)                             | `<jh/concurrent/occ_box.h>`       | ✅ Stable | OCC container with optimistic reads, copy-on-write writes, and optional multi-box transactions.      |
| [`flat_pool<Key, Value>`](flat_pool.md)                | `<jh/concurrent/flat_pool.h>`     | ✅ Stable | Key-based, contiguous pool exposing reference-counted handles, validator guards, and health metrics. A final template parameter selects the RW mutex. |
| [`pointer_pool<T, Hash, Eq>`](pointer_pool.md)         | `<jh/concurrent/pointer_pool.h>`  | ✅ Stable | Sorted, reusable hash slots for immovable heap objects; its handle and index use the selected RW mutex. |
| [`observe_pool<T>`](observe_pool.md)                   | `<jh/concurrent/observe_pool.h>`  | ✅ Stable | Duck-typed alias of `pointer_pool` with an optional RW mutex parameter. |
| [`resource_pool<Key, Value, Alloc>`](resource_pool.md) | `<jh/concurrent/resource_pool.h>` | ✅ Stable | Simplified alias of `flat_pool` keyed by `jh::hash<Key>`, plus `resource_pool_set<Key>`; both accept an optional RW mutex parameter. |

---

## 🧩 Module Summary

* **Design goal:** expose deterministic, lock-conscious containers where the surface API mirrors the managed resource
  rather than lock choreography.
* **Entry points:** `<jh/concurrency>` delivers the raw `jh::conc` helpers; `<jh/pool>` re-exports the alias-based pools
  for `jh::` namespace convenience.
* **Caveat:** when `jh::hash<T>` cannot be deduced (e.g., types inside `std` such as `std::variant` that forbid
  registering `std::hash`, ADL `hash`, or member `hash()`), instantiate the raw `jh::conc::*_pool` template with
  explicit `Hash`/`Eq` functors, because the auto-deduction path has no hook into closed namespaces.
* **Multi-commit:** `occ_box` optionally enables multi-box transactions via `JH_OCC_ENABLE_MULTI_COMMIT` (default `1`);
  participating boxes reserve commit gates while the transaction validates and publishes its states.

---

## Pool platform support

Windows MinGW runs of `tdd.composed.immutable_str` have intermittently failed
and passed on rerun. GCC behaved as expected in separate local tests; UCRT and
MSVCRT have not been evaluated independently. The observed gap belongs to the
MinGW and Windows runtime combination, with no single component identified as
its cause.

`flat_pool` uses atomic reference counts and a selectable RW mutex. With a
custom RW mutex and latch-controlled ordering tests, its observed ordering
failure stopped occurring. The former `pointer_pool` had an additional path
through `weak_ptr::lock()` and the `shared_ptr` control block, which the
selected mutex could not govern. Its new `ptr` handle removes that path.

The remaining engineering requirement is an RW mutex that reliably
synchronizes the pool's state, including atomic counts where present, on the
target runtime. The default is `std::shared_mutex`; callers can select another
type when that default is unreliable. `rw_mutex_like` checks the interface,
while target-platform tests must check the synchronization behavior. Windows
CI validation of the new observed pool remains pending. See the
[`strong_lock` reference](../synchronous/strong_lock.md) for the lock adapters
used at pool boundaries.

---

## 🧭 Navigation

| Resource                                 |                                                                       Link                                                                       |
|------------------------------------------|:------------------------------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**                    |                   [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                   |
| 📘 **`occ_box<T>` Reference**            |          [![Go to occ_box Reference](https://img.shields.io/badge/Go%20to%20Occ%20Box%20Reference-green?style=flat-square)](occ_box.md)          |
| 📗 **`flat_pool<Key, Value>` Reference** |       [![Go to flat_pool Reference](https://img.shields.io/badge/Go%20to%20Flat%20Pool%20Reference-green?style=flat-square)](flat_pool.md)       |
| 📙 **`pointer_pool<T>` Reference**       |  [![Go to pointer_pool Reference](https://img.shields.io/badge/Go%20to%20Pointer%20Pool%20Reference-green?style=flat-square)](pointer_pool.md)   |
| 📘 **`observe_pool<T>` Reference**       |  [![Go to observe_pool Reference](https://img.shields.io/badge/Go%20to%20Observe%20Pool%20Reference-green?style=flat-square)](observe_pool.md)   |
| 📗 **`resource_pool` Reference**         | [![Go to resource_pool Reference](https://img.shields.io/badge/Go%20to%20Resource%20Pool%20Reference-green?style=flat-square)](resource_pool.md) |
