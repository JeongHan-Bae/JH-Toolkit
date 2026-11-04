# 🎍 **JH Toolkit — Concurrency Module Overview**

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
| `pointer_pool<T>` / `observe_pool<T>`                 | Weak-observed pools for pointer-stable, immutable data (the latter auto-deduces hash/equality). |

`<jh/concurrency>` aggregates every `<jh/concurrent/*.h>` helper into `jh::conc`. When you only need the alias-based
pools, include `<jh/pool>` so that `jh::observe_pool`/`jh::resource_pool` sit directly under `jh::`.

---

## 🔹 Core Components

| Component                                              | Header                            | Status   | Description                                                                                          |
|--------------------------------------------------------|-----------------------------------|----------|------------------------------------------------------------------------------------------------------|
| [`occ_box<T>`](occ_box.md)                             | `<jh/concurrent/occ_box.h>`       | ✅ Stable | OCC container with optimistic reads, copy-on-write writes, and optional multi-box transactions.      |
| [`flat_pool<Key, Value>`](flat_pool.md)                | `<jh/concurrent/flat_pool.h>`     | ✅ Stable | Key-based, contiguous pool exposing reference-counted handles, validator guards, and health metrics. A final template parameter selects the RW mutex. |
| [`pointer_pool<T, Hash, Eq>`](pointer_pool.md)         | `<jh/concurrent/pointer_pool.h>`  | ✅ Stable | Weak-pointer observer for immutable heap objects that must stay at fixed addresses. A final template parameter selects the RW mutex. |
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

The pool components use standard C++ mutexes by default and accept a custom
RW mutex type. A conforming mutex synchronizes a release with a later
successful acquisition on the same
mutex; see the [C++ mutex requirements](https://eel.is/c++draft/thread.mutex.requirements)
and [shared-mutex requirements](https://eel.is/c++draft/thread.sharedmutex.requirements.general).
Correctly protected data has the corresponding visibility guarantees on every
conforming platform.

`pointer_pool::acquire()` holds the pool's RW mutex across capacity checks,
cleanup, and insertion. Under a mutex that supplies the required cross-thread
synchronization, the pool's own accesses are data-race-free. The
`rw_mutex_like` concept checks the API shape; it cannot establish the runtime
synchronization quality of a particular library build. Callers must serialize
concurrent move assignments involving the same pool.

`pointer_pool<T, Hash, Eq, Mutex>`, `observe_pool<T, Mutex>`,
`flat_pool<Key, Value, Hash, Alloc, Mutex>`, and the `resource_pool` aliases
accept a custom RW mutex. A platform-specific implementation that has been
verified to provide the required synchronization can be supplied when the default mutex
shows a measured visibility problem. The project has measured high-contention
instability on Windows MinGW builds; the particular standard library and
runtime combination must be identified before
assigning its cause.

Windows provides [slim reader/writer (SRW) locks](https://learn.microsoft.com/en-us/windows/win32/sync/slim-reader-writer--srw--locks)
as lightweight, single-process synchronization objects. Microsoft describes
them as [typically operating in user mode, with a possible kernel transition
under contention](https://learn.microsoft.com/en-us/windows/win32/sync/about-synchronization).
Their implementation and scheduling behavior differ from POSIX reader/writer
locks. This platform distinction is relevant to the measured Windows gap; the
exact backend selected by a particular C++ standard library must be checked
for that build.

On Windows, several pool lock sites use `posix_smtx_*_lock` aliases, which
select RAII adapters that add `seq_cst` fences at those boundaries. Testing has
shown that these fences improve behavior on the affected configurations. They
cannot complete a missing library/runtime synchronization path for independent
atomic operations: a fence alone does not create a release/acquire relationship
for an unrelated atomic value. The mitigation does not guarantee every
high-contention workload. Other platforms use standard lock adapters. The
[`strong_lock` reference](../synchronous/strong_lock.md) describes the
adapter behavior. The [C++ fence rules](https://eel.is/c++draft/atomics.fences)
specify when fences participate in synchronization through atomic operations.

The [contribution guide](../../CONTRIBUTING.md) defines the minimum Windows
support requirement and separately reports occasional Windows CI runner freezes.

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
