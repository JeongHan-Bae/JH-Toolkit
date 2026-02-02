# 🛰️ **JH Toolkit — `jh::sync::ipc::process_mutex` API Reference**

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/process_mutex.h>`  
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

`jh::sync::ipc::process_mutex<"name", HighPriv>` is a **cross-platform, named process-wide mutex** equivalent in
contract to `std::timed_mutex`, implemented using **OS-level named semaphores**.

* Each unique **compile-time string literal** corresponds to exactly one globally visible coordination object.
* The mutex participates in **InterProcess Coordination**, not message passing or data transport.
* Naming rules are validated **at compile time** via `jh::sync::ipc::limits`.

Instances are accessed exclusively through the `instance()` singleton.
Copy and move operations are disabled to preserve global identity.

---

## 🎯 Standard Library Correspondence

`jh::sync::ipc::process_mutex` is the **InterProcess Coordination counterpart** of the C++ Standard Library
synchronization primitive:

> **`std::timed_mutex`**

### Conceptual mapping

| Aspect        | `std::timed_mutex`          | `process_mutex`              |
|---------------|-----------------------------|------------------------------|
| Scope         | Single process              | Multiple processes           |
| Visibility    | In-process object           | Globally named OS object     |
| Ownership     | Exclusive                   | Exclusive                    |
| Recursion     | Non-recursive               | Non-recursive                |
| Timed locking | `try_lock_for / until`      | `try_lock_for / until`       |
| Lifetime      | Automatic (object lifetime) | OS-managed (named semaphore) |
| Identity      | Per-instance                | Per-name (singleton)         |

In other words:

> **`process_mutex` is a named, process-wide, singleton form of `std::timed_mutex`.**

---

### Key differences from `std::timed_mutex`

While the locking contract is intentionally identical, there are **fundamental semantic differences**:

* The mutex identity is defined by a **compile-time name**, not by object address.
* All processes using the same name coordinate on the **same underlying OS semaphore**.
* Lifetime is **decoupled from C++ object lifetime** and managed by the operating system.
* Namespace mutation (`unlink`) is explicitly gated via `HighPriv`.

---

### Design intent

This alignment is intentional:

* Developers familiar with `std::timed_mutex` can use `process_mutex` **without relearning semantics**.
* Porting single-process synchronization logic to **multi-process coordination** requires minimal conceptual change.
* The API forms a **one-to-one mental model extension** of STL primitives into the IPC domain.

> Future IPC synchronization components in `jh::sync::ipc` follow the same rule:
> **each primitive explicitly states which STL synchronization type it corresponds to.**

---

## 🧱 Naming and Compile-Time Constraints

The template parameter `S` represents the **bare logical name** of the mutex:

```cpp
jh::sync::ipc::process_mutex<"example_lock">
```

### Name resolution

| Platform | OS-visible name         |
|----------|-------------------------|
| POSIX    | `"/example_lock"`       |
| Windows  | `"Local\\example_lock"` |

The prefix is **automatically applied** by the implementation.
User code must never include platform prefixes.

---

### Compile-time validation

Before the class template can be instantiated, the name is validated by:

```cpp
jh::sync::ipc::limits::valid_object_name<S>()
```

This enforces:

* Allowed characters: `[A–Z a–z 0–9 _ . -]`
* No leading `/`
* Platform-aware maximum length
* Failure results in a **hard compile-time error**

**See:** [`valid_object_name`](ipc_limits.md) — *IPC Object Name Constraints and Validation*

---

## 🔧 Key Operations

| Member                            | Description                                                                                       |
|-----------------------------------|---------------------------------------------------------------------------------------------------|
| `void lock()`                     | Blocks until the mutex is acquired. Throws `std::runtime_error` on system failure. Non-recursive. |
| `bool try_lock()`                 | Attempts to acquire without blocking. Returns `false` if already held.                            |
| `bool try_lock_for(duration)`     | Attempts acquisition for a relative duration. Emulates timed waits where unavailable.             |
| `bool try_lock_until(time_point)` | Attempts acquisition until an absolute time point.                                                |
| `void unlock()`                   | Releases ownership. Calling without ownership is undefined behavior.                              |
| `static void unlink()`            | Removes the name from the OS namespace (POSIX only, `HighPriv == true`).                          |

The semantic contract mirrors `std::timed_mutex`.

---

## 🧠 Semantic Model

`process_mutex` obeys the following rules:

* **Exclusive ownership** — only one participant may hold the lock at any time.
* **Non-recursive** — re-locking without unlocking results in deadlock.
* **Process-wide scope** — synchronization is visible across all processes using the same name.
* **Thread-agnostic ownership** — ownership is not tracked per-thread; misuse is undefined.

Unlocking without prior acquisition or unlocking multiple times is **undefined behavior**.

---

## ⚙️ Platform Notes

### POSIX (Linux, UNIX, BSD)

* Implemented using `sem_open`, `sem_wait`, `sem_post`
* Timed waiting:

    * Uses `sem_timedwait` when POSIX.1b is available
    * Otherwise emulated via `sem_trywait` + exponential backoff
* `unlink()` maps to `sem_unlink`
* Permission bits control **namespace access**, not locking semantics

---

### Windows / MSYS2

* Implemented using `CreateSemaphoreA`, `WaitForSingleObject`, `ReleaseSemaphore`
* No `unlink()` concept
* Named semaphore lifetime is tied to open handles
* Access control is handled by the Win32 API

---

## 🔐 Privilege and Permission Model

### `HighPriv` template parameter

```cpp
process_mutex<"name", true>
```

* Enables `unlink()` on POSIX
* Deleted at compile time when `HighPriv == false`
* Intended for:

    * Initializers
    * Cleanup utilities
    * Supervisory processes

---

### `JH_PROCESS_MUTEX_SHARED` macro (POSIX only)

Controls namespace permissions at creation time:

| Value             | Mode   | Effect                  |
|-------------------|--------|-------------------------|
| `false` (default) | `0644` | Only creator may unlink |
| `true`            | `0666` | Any user may unlink     |

Windows ignores this macro.

---

## 🧬 Lifetime and Destruction Semantics

* The semaphore is created or opened on first access to `instance()`
* On POSIX:

    * `unlink()` removes the name immediately
    * Existing handles remain valid
    * Object is destroyed after last `sem_close`
* On Windows:

    * Object is destroyed automatically when last handle closes

The destructor only closes the local handle; it never removes the name.

---

## Summary

* `process_mutex<"name">` is a deterministic, named, process-wide mutex.
* Semantics match `std::timed_mutex`, extended across process boundaries.
* Names are validated at compile time via `ipc_limits`.
* `HighPriv` explicitly gates namespace mutation (`unlink()`).
* Zero dynamic allocation, zero runtime name checking.
