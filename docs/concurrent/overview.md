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
| `occ_box<T>`                                          | OCC-wrapped mutable state with wait-free readers and atomic copy-and-replace commits.           |
| `flat_pool<Key, Value>` / `resource_pool<Key, Value>` | Contiguous key-driven pool for copyable/movable entries, exposing reference-counted handles.    |
| `pointer_pool<T>` / `observe_pool<T>`                 | Weak-observed pools for pointer-stable, immutable data (the latter auto-deduces hash/equality). |

`<jh/concurrency>` aggregates every `<jh/concurrent/*.h>` helper into `jh::conc`. When you only need the alias-based
pools, include `<jh/pool>` so that `jh::observe_pool`/`jh::resource_pool` sit directly under `jh::`.

---

## 🔹 Core Components

| Component                                              | Header                            | Status   | Description                                                                                          |
|--------------------------------------------------------|-----------------------------------|----------|------------------------------------------------------------------------------------------------------|
| [`occ_box<T>`](occ_box.md)                             | `<jh/concurrent/occ_box.h>`       | ✅ Stable | OCC container with optimistic reads, copy-on-write writes, and optional multi-box transactions.      |
| [`flat_pool<Key, Value>`](flat_pool.md)                | `<jh/concurrent/flat_pool.h>`     | ✅ Stable | Key-based, contiguous pool exposing reference-counted handles, validator guards, and health metrics. |
| [`pointer_pool<T, Hash, Eq>`](pointer_pool.md)         | `<jh/concurrent/pointer_pool.h>`  | ✅ Stable | Weak-pointer observer for immutable heap objects that must stay at fixed addresses.                  |
| [`observe_pool<T>`](observe_pool.md)                   | `<jh/concurrent/observe_pool.h>`  | ✅ Stable | Duck-typed alias of `pointer_pool` that derives hash/equality from `jh::hash<T>`/ADL.                |
| [`resource_pool<Key, Value, Alloc>`](resource_pool.md) | `<jh/concurrent/resource_pool.h>` | ✅ Stable | Simplified alias of `flat_pool` keyed by `jh::hash<Key>`, plus `resource_pool_set<Key>`.             |

---

## 🧩 Module Summary

* **Design goal:** expose deterministic, lock-conscious containers where the surface API mirrors the managed resource
  rather than lock choreography.
* **Entry points:** `<jh/concurrency>` delivers the raw `jh::conc` helpers; `<jh/pool>` re-exports the alias-based pools
  for `jh::` namespace convenience.
* **Caveat:** when `jh::hash<T>` cannot be deduced (e.g., types inside `std` such as `std::variant` that forbid
  registering `std::hash`, ADL `hash`, or member `hash()`), instantiate the raw `jh::conc::*_pool` template with
  explicit `Hash`/`Eq` functors, because the auto-deduction path has no hook into closed namespaces.
* **Multi-commit:** `occ_box` optionally enables multi-box transactions via `JH_OCC_ENABLE_MULTI_COMMIT` (default `1`),
  placing multi-write commits ahead of single writers and readers.

---

## 🎍 Pool Module (Windows Semantics Notice)

📁 **Module:** `<jh/pool>`  
📦 **Namespace:** `jh`  
📍 **Location:** `jh/concurrent/`

`<jh/pool>` re-exports alias-based pools (`observe_pool`, `resource_pool`, etc.) directly under `jh::`.  
These helpers are engineered around POSIX-style behavioral assumptions.

As of 1.4.x, **Windows is treated as a compatibility platform, not a fully supported concurrency baseline** for the
entire `jh/pool` module.

---

### ⚠️ Windows Memory-Model Limitation

We introduced:

```cpp
std::atomic_thread_fence(std::memory_order_seq_cst);
```

around `shared_mutex` boundaries (via <code>posix_smtx_&#42;_lock</code>).

This is the strongest ordering primitive available in ISO C++ at the language level.

However, it does **not** fully close the visibility gaps observed under:

* MinGW toolchains (MinGW-w64 / MinGW-clang)
* Windows runtimes (MSVCRT / UCRT)
* Interaction with OS-level primitives (e.g., SRWLock)

The behavior remains **data-race-free (DRF)** at the algorithmic level.  
The instability arises from environmental constraints beyond the abstract C++ model.

*Note:*  
On Windows, inserting
`std::atomic_thread_fence(std::memory_order_seq_cst);`
around `shared_mutex` boundaries measurably improves practical stability under contention.
It raises the threshold at which race-like symptoms manifest and delays instability in stress scenarios.
However, it cannot fully eliminate the underlying visibility gaps nor guarantee POSIX-level behavioral equivalence.

---

### 1️⃣ Fence Jurisdiction Limitation

`std::atomic_thread_fence` can only constrain memory operations:

* Emitted by the current compiler
* Within the C++ abstract machine
* Visible in user-space generated code

It **cannot**:

* Control hidden synchronization inside system DLLs
* Strengthen ordering inside kernel-implemented locks
* Inject barriers into opaque runtime components

On Windows, primitives such as `SRWLock` are implemented inside system libraries (e.g., `ntdll.dll`).  
If their internal memory ordering is optimized without full fences, user-inserted `seq_cst` cannot retroactively enforce
stronger guarantees.

The fence does not cross the ABI boundary.

---

### 2️⃣ Atomicity ≠ Global Visibility

Atomic operations guarantee:

* **Atomicity** — operations are indivisible.
* **Ordering (within abstract machine)** — relative sequencing.

They do **not** guarantee:

* Immediate global visibility across CPU cores.
* Hardware-level total ordering.

Modern CPUs use:

* Store buffers
* Cache-coherence protocols (e.g., MESI)
* Speculative execution

Even with `seq_cst`, there can be a physical delay between:

* Atomic write completion on CPU A
* Cache-line propagation to CPU B

If the Windows lock implementation does not enforce a bus-lock–grade synchronization boundary, extremely high-frequency
contention may expose transient visibility windows.

This is a hardware-level constraint, not a C++ bug.

---

### 3️⃣ Compiler / Runtime Coupling Effects

In the MinGW + UCRT/MSVCRT environment:

* The compiler may assume system calls imply sufficient ordering.
* The runtime may assume atomic operations are independently ordered.
* `std::shared_ptr` (atomic refcount) and `std::shared_mutex` (system lock) may not form a strongly coupled ordering
  chain at the machine-code level.

This weak cross-domain coupling can expose rare race-like symptoms under stress.

---

### Why We Do Not Escalate Further

Once `std::memory_order_seq_cst` is insufficient, only two paths remain:

1. Insert hardware-specific instructions (`MFENCE`, `LOCK` prefixes, inline assembly).
2. Accept that the environment cannot provide POSIX-level closure.

Option 1 destroys portability and maintainability.  
Option 2 is engineering reality.

If the strongest ISO C++ ordering primitive cannot bridge the gap, the issue is systemic, not algorithmic.

---

### Official Position

* `jh/pool` implementations remain **algorithmically DRF**.
* POSIX platforms (macOS, Linux GCC ≥13) define the reference behavior.
* On Windows:

    * API availability is guaranteed.
    * Single-threaded or low-pressure multi-threaded usage is recommended.
    * Full POSIX-equivalent global ordering is not guaranteed.

The inserted fences:

* Raise the instability threshold.
* Delay manifestation under contention.
* Improve convergence in practice.

They:

* Do **not** eliminate hardware-level reordering.
* Do **not** establish a true global total order.

This is a boundary where software cannot defeat hardware and platform policy.

Accordingly, the entire `<jh/pool>` module does **not** provide full Windows concurrency support.

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
