# 🛰️ **JH Toolkit — `jh::sync::ipc::process_counter` API Reference**

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/process_counter.h>`  
📦 **Namespace:** `jh::sync::ipc`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Sync](https://img.shields.io/badge/%20Back%20to%20Sync-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Overview

`jh::sync::ipc::process_counter<"name", HighPriv>` is a **cross-process shared 64-bit integer counter**
implemented using **OS-level named shared memory**, with **explicit inter-process locking**.

It provides a **globally visible read–modify–write counter** suitable for coordination patterns such as:

* reference counting

* reader / writer tracking

* barrier-style protocols

* distributed state observation

* Each unique **compile-time string literal** identifies exactly one shared counter.

* All processes referencing the same name observe and modify the **same shared value**.

* Naming rules are validated **at compile time** via `jh::sync::ipc::limits`.

Instances are accessed exclusively through the `instance()` singleton.
Copy and move operations are disabled to preserve global identity.

---

## 🎯 Standard Library Correspondence

`jh::sync::ipc::process_counter` is the **InterProcess Coordination counterpart** of:

> **`std::atomic<uint64_t>`**

### Conceptual mapping

| Aspect          | `std::atomic<uint64_t>`  | `process_counter`       |
|-----------------|--------------------------|-------------------------|
| Scope           | Single process           | Multiple processes      |
| Storage         | In-process memory        | OS-named shared memory  |
| Atomicity       | Lock-free (if supported) | Lock-protected          |
| RMW operations  | Yes                      | Yes                     |
| Memory ordering | C++ memory model         | Explicit fences + mutex |
| Lifetime        | Object lifetime          | OS-managed              |
| Identity        | Per-instance             | Per-name (singleton)    |

In short:

> **`process_counter` is a named, process-wide analogue of `std::atomic<uint64_t>`.**

---

### Critical semantic clarification

Despite the API resemblance:

> **`process_counter` is *not* a C++ atomic type.**

Reasons:

* The C++ memory model does **not** define atomics over memory-mapped regions.
* On non-Linux platforms, there is no portable support for cross-process atomics.
* Therefore, atomicity is provided by a **dedicated inter-process mutex**.

This is an **intentional design choice**, favoring correctness and portability over lock-free behavior.

---

### Design intent

This alignment is intentional:

* Code written against `std::atomic<uint64_t>` can often be lifted mechanically.
* Coordination logic remains explicit and analyzable.
* The primitive remains **low-level**, suitable for building higher-order IPC protocols.

> As with other IPC primitives, `process_counter` explicitly states
> **which STL type it corresponds to**.

---

## 🧱 Naming and Compile-Time Constraints

The template parameter `S` represents the **bare logical name** of the counter:

```cpp
jh::sync::ipc::process_counter<"example_counter">
```

### Name resolution

| Platform | OS-visible backing                       |
|----------|------------------------------------------|
| POSIX    | Shared memory `"/example_counter"`       |
| Windows  | File mapping `"Global\\example_counter"` |

Prefixes are **automatically applied**.
User code must never include platform-specific prefixes.

---

### Reserved suffix

Internally, `process_counter` allocates an associated mutex:

```text
S + ".loc"
```

To guarantee correctness:

* The compile-time name limit is reduced by 4 characters.
* Users **must not** define:

    * `process_mutex<S>`
    * `process_mutex<S + ".loc">`

---

### Compile-time validation

Instantiation requires:

```cpp
limits::valid_object_name<S, limits::max_name_length - 4>()
```

This enforces:

* Allowed characters: `[A–Z a–z 0–9 _ . -]`
* No leading `/`
* Platform-aware maximum length (max length - 4 to reserve suffix)
* Failure results in a **hard compile-time error**

**See:** [`valid_object_name`](ipc_limits.md) — *IPC Object Name Constraints and Validation*

---

## 🔧 Key Operations

### Load operations

| Member                   | Description                                                  |
|--------------------------|--------------------------------------------------------------|
| `uint64_t load()`        | Lightweight relaxed read; may observe slightly stale values. |
| `uint64_t load_strong()` | Sequentially consistent read with full memory fence.         |
| `uint64_t load_force()`  | Fully synchronized read under inter-process lock.            |

---

### Store / modify operations

| Member                         | Description                              |
|--------------------------------|------------------------------------------|
| `void store(uint64_t)`         | Atomically replace the value under lock. |
| `uint64_t fetch_add(uint64_t)` | Add and return previous value.           |
| `uint64_t fetch_sub(uint64_t)` | Subtract and return previous value.      |
| `uint64_t fetch_apply(F)`      | Apply custom transformation atomically.  |

All modification operations are **globally serialized** within the named counter scope.

---

## 🧠 Semantic Model

`process_counter` obeys the following rules:

* **Single global value** per name across all processes.
* **Linearizable updates** — all writers observe a single global order.
* **Explicit synchronization** — atomicity provided by `process_mutex<S + ".loc">`.
* **Thread-safe and process-safe** — usable concurrently by many threads and processes.

Read freshness is **explicitly selectable** via the chosen load method.

---

## ⚙️ Platform Notes

### POSIX (Linux, BSD, Darwin)

* Backed by:

    * `shm_open` + `mmap`
* Permissions controlled by `JH_PROCESS_MUTEX_SHARED` (`0644` / `0666`)
* No special privileges required
* Behavior is deterministic and stable

---

### Windows / MSYS2

* Backed by:

    * `CreateFileMapping`
    * `MapViewOfFile`
* Uses the `Global\\` namespace
* Requires **Administrator privilege**
* Shared memory and mutex lifetime tied to open handles

> Windows support preserves **API compatibility**, not atomic equivalence.

---

## ⚠️ Windows Usage Policy

As with other shared-memory IPC primitives:

* **MinGW / MSYS2 + GCC**
  Suitable **only for prototype development**.
* **Production usage on Windows**
  Use **WSL2 + GCC** to obtain POSIX semantics and predictable behavior.

This recommendation applies to:

* [`process_counter`](process_counter.md)
* `process_cond_var`
* Other shared-memory–based IPC primitives

---

## 🔐 Privilege and Unlink Semantics

### `HighPriv` template parameter

```cpp
process_counter<"name", true>
```

* Enables `unlink()` on POSIX.
* Deleted at compile time when `HighPriv == false`.
* Intended for cleanup or supervisory roles.

---

### Unlink behavior

* **POSIX**:

    * Calls `shm_unlink()` on the counter.
    * Unlinks both internal mutexes:

        * `process_mutex<S>`
        * `process_mutex<S + ".loc">`
* **Windows**:

    * No explicit unlink.
    * Resources are reclaimed when last handle closes.
* Operations are **idempotent**.

---

## 🧬 Lifetime and Destruction Semantics

* Shared memory is created or opened on first `instance()` access.
* Initialization is guarded by `process_mutex<S>`.
* Destruction only releases local mappings or handles.
* Final reclamation is performed by the OS.

The destructor never mutates the namespace.

---

## Summary

* `process_counter<"name">` is a named, process-wide shared counter.
* Semantics correspond to `std::atomic<uint64_t>`, extended across processes.
* Atomicity is provided explicitly via inter-process locking.
* Naming is validated at compile time via `ipc_limits`.
* POSIX provides deterministic behavior.
* Windows provides **API-level compatibility only**.
* For production use on Windows, **WSL2 + GCC is recommended**.
