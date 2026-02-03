# 🛰️ **JH Toolkit — `jh::sync::ipc::process_cond_var` API Reference**

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/process_cond_var.h>`  
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

`jh::sync::ipc::process_cond_var<"name", HighPriv>` is a **cross-process condition variable primitive** implemented
using **OS-level named IPC mechanisms**.

It provides a **minimal, globally visible signaling point** usable across processes and is intended to be composed with
other IPC primitives such as `process_mutex` and `process_counter`.

* Each unique **compile-time string literal** identifies one globally shared wait-set.
* The primitive participates in **InterProcess Coordination**, not data exchange.
* Naming rules are validated **at compile time** via `jh::sync::ipc::limits`.

Instances are accessed exclusively through the `instance()` singleton.
Copy and move operations are disabled to preserve global identity.

---

## 🎯 Standard Library Correspondence

`jh::sync::ipc::process_cond_var` is the **InterProcess Coordination counterpart** of the C++ Standard Library condition
variable family:

> **`std::condition_variable` / `std::condition_variable_any`**

### Conceptual mapping

| Aspect           | `std::condition_variable`   | `process_cond_var`                 |
|------------------|-----------------------------|------------------------------------|
| Scope            | Single process              | Multiple processes                 |
| Visibility       | In-process object           | Globally named OS object           |
| Wait-set         | Thread-local                | Process-global                     |
| Spurious wakeups | Allowed                     | Allowed                            |
| Timed wait       | `wait_until`                | `wait_until`                       |
| Notification     | `notify_one / all`          | `notify_one / all`                 |
| Lifetime         | Automatic (object lifetime) | OS-managed (shared memory / event) |
| Identity         | Per-instance                | Per-name (singleton)               |

In short:

> **`process_cond_var` is a named, process-wide condition variable.**

---

### Key semantic differences

Despite API similarity, there are **important semantic differences**:

* Identity is defined by a **compile-time name**, not object address.
* All processes using the same name participate in the **same wait-set**.
* Lifetime is **decoupled from C++ object lifetime**.
* Broadcast semantics (`notify_all`) are **platform-dependent**.
* No fairness or strict wake guarantees are provided.

These differences are inherent to cross-process coordination.

---

### Design intent

This alignment is intentional:

* Familiarity with STL condition variables transfers directly.
* Multi-process coordination can be built by **lifting single-process designs**.
* The primitive remains **low-level and explicit**, suitable for composition.

> As with `process_mutex`, all IPC synchronization primitives explicitly state
> **which STL type they correspond to**.

---

## 🧱 Naming and Compile-Time Constraints

The template parameter `S` represents the **bare logical name** of the condition variable:

```cpp
jh::sync::ipc::process_cond_var<"example_cond">
```

### Name resolution

| Platform | OS-visible backing                   |
|----------|--------------------------------------|
| POSIX    | Shared memory `"/example_cond"`      |
| Windows  | Named event `"Global\\example_cond"` |

Prefixes are **automatically applied**.
User code must never include platform-specific prefixes.

---

### Compile-time validation

Before instantiation, the name is validated by:

```cpp
jh::sync::ipc::limits::valid_object_name<S, limits::max_name_length - 4>()
```

This enforces:

* Allowed characters: `[A–Z a–z 0–9 _ . -]`
* No leading `/`
* Platform-aware maximum length (max length - 4 to reserve suffixes)
* Failure results in a **hard compile-time error**

**See:** [`valid_object_name`](ipc_limits.md) — *IPC Object Name Constraints and Validation*

---

## 🔧 Key Operations

| Member                        | Description                                                 |
|-------------------------------|-------------------------------------------------------------|
| `void wait_signal()`          | Blocks until signaled. Spurious wakeups may occur.          |
| `bool wait_until(time_point)` | Waits until signaled or timeout expires.                    |
| `void notify_one()`           | Wakes one waiting participant.                              |
| `void notify_all(int n = 32)` | Wakes up to `n` waiting participants (POSIX).               |
| `void notify_all()`           | Broadcast approximation (Windows).                          |
| `static void unlink()`        | Removes backing resources (POSIX only, `HighPriv == true`). |

The semantic contract mirrors `std::condition_variable`, adjusted for IPC constraints.

---

## 🧠 Semantic Model

`process_cond_var` obeys the following rules:

* **Process-wide wait-set** — all participants with the same name observe the same condition.
* **Spurious wakeups permitted** — callers must re-check predicates.
* **No fairness guarantees** — wake order is unspecified.
* **Thread-agnostic** — ownership is not tracked per thread.

The primitive does **not** embed a user mutex; coordination logic must be layered explicitly.

---

## ⚙️ Platform Notes

### POSIX (Linux, BSD, Darwin)

* Backed by:

    * `shm_open` + `mmap`
    * `pthread_cond_t` and `pthread_mutex_t`
* Both condition and mutex are configured as `PTHREAD_PROCESS_SHARED`.
* `notify_all(n)` signals up to `n` waiters via repeated `pthread_cond_signal`.
* No special privileges are required.

Behavior is **deterministic and close to pthread semantics**.

---

### Windows / MSYS2

* Backed by a named `Event` object in the `Global\\` namespace.
* Requires **Administrator privilege** to create or open.
* No true broadcast primitive exists:

    * `notify_all()` is simulated by setting the event for ~1 ms.
    * This is an **engineering approximation**, not a semantic equivalent.
* Wake counts and fairness are **not guaranteed**.

> Windows support preserves **API compatibility**, not behavioral parity.

---

## ⚠️ Windows Usage Policy

In the **jh-toolkit IPC model**, Windows is treated as a **second-class platform**:

* The API is available.
* Basic signaling works.
* Exact semantics (especially broadcast behavior) are **not guaranteed**.

### Recommendation

* **MinGW / MSYS2 + GCC**:
  Suitable **only for prototype development** and validation.
* **Production target on Windows**:
  Use **WSL2 + GCC** to obtain POSIX semantics and deterministic behavior.

This recommendation applies equally to:

* `process_cond_var`
* [`process_counter`](process_counter.md)
* Other shared-memory–based IPC primitives

---

## 🔐 Privilege and Unlink Semantics

### `HighPriv` template parameter

```cpp
process_cond_var<"name", true>
```

* Enables `unlink()` on POSIX.
* Deleted at compile time when `HighPriv == false`.
* Intended for cleanup or supervisory roles.

---

### Unlink behavior

* **POSIX**:

    * Calls `shm_unlink()` on the shared memory segment.
    * Also unlinks the internal `process_mutex<S>` used for initialization.
* **Windows**:

    * No explicit unlink.
    * Resources are released when the last handle closes.
* Operations are **idempotent**.

---

## 🧬 Lifetime and Destruction Semantics

* Shared resources are created or opened on first `instance()` access.
* Destruction only closes local mappings or handles.
* Actual resource reclamation is performed by the OS.

The destructor never mutates the namespace.

---

## Summary

* `process_cond_var<"name">` is a named, process-wide condition variable.
* Semantics correspond to `std::condition_variable`, extended across processes.
* Naming is validated at compile time via `ipc_limits`.
* POSIX provides strong, deterministic behavior.
* Windows provides **API-level compatibility only**.
* For production-grade behavior on Windows, **WSL2 + GCC is recommended**.
