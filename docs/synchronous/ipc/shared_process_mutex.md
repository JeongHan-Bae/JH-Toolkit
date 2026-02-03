# 🛰️ **JH Toolkit — `jh::sync::ipc::shared_process_mutex` API Reference**

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/shared_process_mutex.h>`  
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

`jh::sync::ipc::shared_process_mutex<"name", HighPriv>` is a **process-wide shared/exclusive timed mutex**
providing semantics equivalent to **`std::shared_timed_mutex`**, but implemented entirely from
**named OS-level IPC primitives**.

It enables **read–write coordination across processes** without requiring the protected data itself
to reside in shared memory.

* Identity is defined by a **compile-time string literal**
* Visibility is **system-wide**
* Semantics are **deterministic and explicit**
* Fairness is **not guaranteed**

> This primitive participates in **InterProcess Coordination**, not data sharing.

Instances are accessed exclusively through the `instance()` singleton.
Copy and move are disabled to preserve global identity.

---

## 🎯 Standard Library Correspondence

`shared_process_mutex` is the **InterProcess Coordination counterpart** of:

> **`std::shared_timed_mutex`**

### Conceptual mapping

| Aspect        | `std::shared_timed_mutex` | `shared_process_mutex`            |
|---------------|---------------------------|-----------------------------------|
| Scope         | Single process            | Multiple processes                |
| Identity      | Object instance           | Compile-time name                 |
| Locking modes | Shared / Exclusive        | Shared / Exclusive                |
| Timed locking | Yes                       | Yes                               |
| Upgrade       | ❌                         | ✅ (HighPriv only)                 |
| Reentrancy    | ❌                         | ✅ (idempotent, participant-local) |
| Fairness      | Unspecified               | Unspecified                       |

In short:

> **`shared_process_mutex` is a named, process-wide shared timed mutex,
> extended with explicit upgrade semantics.**

---

## 🧱 Internal Composition

The mutex is a **composite IPC primitive**, built from the following named components:

| Component                       | Role                                        |
|---------------------------------|---------------------------------------------|
| `process_mutex<S + ".exc">`     | Blocks new readers; gates exclusive access  |
| `process_counter<S + ".cnt">`   | Tracks number of active readers system-wide |
| `process_cond_var<S + ".cond">` | Wakes writers / upgraders when readers exit |
| `process_mutex<S + ".pri">`     | Priority lock ensuring upgrade continuity   |

All components share the same **compile-time namespace** derived from `S`.

Declaring any of these names manually elsewhere is a **protocol violation**.

---

## 🧱 Naming and Compile-Time Constraints

The template parameter `S` represents the **bare logical name** of the shared process mutex:

```cpp
jh::sync::ipc::shared_process_mutex<"example_shared_mutex">
```

### Name resolution

| Platform | OS-visible backing                           |
|----------|----------------------------------------------|
| POSIX    | Shared memory `"/example_shared_mutex"`      |
| Windows  | Named event `"Global\\example_shared_mutex"` |

Prefixes are **automatically applied**.
User code must never include platform-specific prefixes.

---

### Compile-time validation

Before instantiation, the name is validated by:

```cpp
jh::sync::ipc::limits::valid_object_name<S, limits::max_name_length - 8>()
```

This enforces:

* Allowed characters: `[A–Z a–z 0–9 _ . -]`
* No leading `/`
* Platform-aware maximum length (max length - 8 to reserve suffixes)
* Failure results in a **hard compile-time error**

**See:** [`valid_object_name`](ipc_limits.md) — *IPC Object Name Constraints and Validation*

---

## 🔁 Reentrancy Model (Participant-Based)

### Participant definition

Reentrancy is defined in terms of a **participant**, not merely a thread or a process.

A participant is:

> A single execution domain identified by `thread_local` state.

This typically corresponds to:

* a native thread, or
* a coroutine together with the thread that first resumes it
  (the coroutine and that thread are treated as one participant)

---

### Idempotent reentrance (not recursive)

This mutex is **not recursive**.

Instead, it supports **idempotent reentrance** within the same participant:

* Re-acquiring a lock already held → **no-op**
* Releasing a lock already released → **no-op**
* No recursion depth is tracked
* No ownership stack exists

This applies to:

* `lock()`, `unlock()`
* `lock_shared()`, `unlock_shared()`

Idempotence exists **to support complex control flow**, especially upgrade paths,
without introducing recursive semantics.

---

### Reentrancy does not propagate

Reentrancy is **strictly participant-local**:

* Holding a lock in one thread does **not** imply ownership in another
* Holding a lock in one process does **not** imply ownership in another
* A coroutine resumed on a different thread is a **different participant**

Relying on cross-participant reentrancy is a **semantic error**.

---

## 🚫 Invalid Lock Transitions

The following patterns are **semantically invalid** and must never be attempted:

* Holding a **shared lock** and acquiring an **exclusive lock**
* Holding an **exclusive lock** and acquiring a **shared lock**
* Any direct lock-mode conversion

These transitions are **not supported** and result in **protocol-level undefined behavior**.

> This mutex is **not** a lock-conversion primitive.

---

### ❗ Unlock + relock is unsafe

The pattern:

```cpp
lock_shared();
unlock_shared();
lock();
```

is **race-prone**.

Between unlock and relock, another writer may acquire exclusive access.
This breaks continuity and is **not prevented by the mutex**.

If uninterrupted transition is required, use **upgrade**.

---

## 🔐 Upgrade Semantics (`HighPriv` only)

Upgrade provides a **continuous, exclusive transition** from shared to exclusive mode.

Properties:

* Only one upgrader may exist system-wide
* Writers are preempted
* Continuity is preserved
* Upgrade cannot yield

If multiple participants attempt upgrade concurrently:

* The protocol is violated
* Consistency cannot be preserved
* The implementation treats this as **fatal**

Current behavior:

* Forced `unlink()`
* Process termination

This is intentional.

> **Upgrade is privileged, singular, and explicit — not a fairness mechanism.**

---

## 🧭 Recommended Usage Practices

### Prefer RAII

Although manual locking is supported, **RAII-based locking is strongly recommended**:

* `std::shared_lock`
* `std::unique_lock`
* `std::lock_guard`

This ensures:

* Exception safety
* Deterministic release
* Reduced cognitive load

Idempotence exists as a **safety net**, not as a usage model.

---

### Long-running multi-process systems

In long-lived environments (daemons, services):

* A process may crash or exit while holding a lock
* OS cleanup may not restore higher-level protocol state immediately
* Indefinite blocking is possible

### Recommendation

Use **timed locking** with deferred RAII:

```cpp
std::unique_lock lock(m, std::defer_lock);
if (!lock.try_lock_for(timeout)) {
    // recovery or escalation
}
```

or

```cpp
std::shared_lock lock(m, std::defer_lock);
if (!lock.try_lock_until(deadline)) {
    // handle explicitly
}
```

This is **strongly recommended** for production IPC systems.

---

## ⚙️ Platform Notes

### POSIX

* Built from POSIX named semaphores and shared memory
* No special privileges required
* Deterministic behavior

### Windows

* Semaphores: `Local\\`
* Shared memory and events: `Global\\`
* `Global\\` requires **Administrator privilege**

Consequently:

* `shared_process_mutex` on Windows **requires elevation**
* POSIX semantics are preserved
* Privilege requirement is unavoidable

> For production-grade behavior on Windows,
> **WSL2 + GCC is recommended**.

---

## 🔐 Privilege and Cleanup

### `HighPriv == false`

* Shared / exclusive locking only
* No upgrade
* No unlink

### `HighPriv == true`

* Enables `upgrade_lock()`
* Enables `unlink()`
* Intended for supervisory or infrastructure roles

`unlink()` removes **all associated IPC objects**:
`.exc`, `.cnt`, `.cond`, `.pri`

Operation is idempotent.

---

## 🧠 Design Philosophy

* No implicit promotion
* No fairness guarantees
* No hidden coordination
* Explicit intent over convenience

> **Correctness is explicit.
> Reentrance is local.
> Coordination is global.**

---

## Summary

* `shared_process_mutex` is a named, process-wide shared timed mutex
* Corresponds to `std::shared_timed_mutex`
* Adds explicit, privileged upgrade semantics
* Reentrance is **idempotent**, **participant-local**, and **non-recursive**
* RAII + timed locking is the recommended production pattern
* Windows requires elevation; POSIX is first-class
