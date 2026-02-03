# ⏱️ **JH Toolkit — `jh::sync::const_lock` API Reference**

📁 **Header:** `<jh/synchronous/const_lock.h>`  
📦 **Namespace:** `jh::sync`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::sync::const_lock` provides a **scope-based const-locking utility**
for mutex-like synchronization primitives.  
It enforces **immutability barriers** in read-only critical sections,
automatically acquiring and releasing locks through RAII semantics.

---

## 🔸 Const-Safety Semantics

A `const_lock` represents a **read-only synchronization contract**.  

Its protected scope must be **semantically const** —  
all accesses guarded by `const_lock` are required to be *purely observational*
and free of any mutation or side effect.

This definition forms the **core semantics** of `const_lock`:

* Performing mutation, state modification, or non-const operation
  inside a `const_lock`-protected region results in **undefined behavior (UB)**.
* The rule holds **even if** the underlying mutex is exclusive-only
  (such as `std::mutex`), because the const barrier is logical, not mechanical.
* The abstraction models "**shared const access**," not "non-exclusive lock."

> In essence, `jh::sync::const_lock` enforces **immutability as a synchronization guarantee** —
> it protects visibility of state, not modification of it.

---

## 🔹 Overview

| Aspect               | Description                                                                                           |
|----------------------|-------------------------------------------------------------------------------------------------------|
| **Purpose**          | Provides RAII-style synchronization for const or read-only access.                                    |
| **Lock Strategy**    | Uses `lock_shared()` / `unlock_shared()` if available; otherwise falls back to `lock()` / `unlock()`. |
| **Integration**      | Works with any type satisfying [`jh::concepts::mutex_like`](../conceptual/mutex_like.md).             |
| **Optimization**     | Automatically collapses to a no-op when using [`jh::typed::null_mutex_t`](../typing/null_mutex.md).   |
| **Thread Semantics** | Ensures deterministic, scoped locking for read-only critical regions.                                 |

---

## 🔹 Core Component

| Symbol              | Type           | Description                                                              |
|---------------------|----------------|--------------------------------------------------------------------------|
| `const_lock<Mutex>` | Class Template | Scope-based immutability guard for `mutex_like` synchronization objects. |

---

## 🔹 Class Reference — `const_lock`

### Declaration

```cpp
template <jh::concepts::mutex_like Mutex>
class const_lock;
```

### Description

A lightweight RAII guard that automatically acquires a shared or exclusive lock
on construction and releases it upon destruction.  
It enforces **const-correct synchronization semantics**, ensuring safe observation
of shared state across threads.

---

### Constructors & Destructor

| Function                                 | Description                                         |
|------------------------------------------|-----------------------------------------------------|
| `explicit const_lock(Mutex& m) noexcept` | Acquires the appropriate lock when constructed.     |
| `~const_lock() noexcept`                 | Releases the held lock automatically on scope exit. |

---

### Behavior

| Condition                                         | Action                                        |
|---------------------------------------------------|-----------------------------------------------|
| `Mutex` satisfies `jh::concepts::shared_lockable` | Invokes `lock_shared()` / `unlock_shared()`.  |
| Otherwise                                         | Invokes `lock()` / `unlock()`.                |
| `Mutex` is `jh::typed::null_mutex_t`              | Performs no operation at all (zero overhead). |

---

### Example

```cpp
std::shared_mutex sm;
jh::sync::const_lock guard(sm); // shared (read) lock

std::mutex m;
jh::sync::const_lock exclusive_guard(m); // exclusive lock

jh::sync::const_lock dummy(jh::typed::null_mutex); // no-op
```

---

## 🧩 Summary

`jh::sync::const_lock` provides a **const-oriented, RAII-managed synchronization primitive**
ensuring thread-safe observation of shared state.

| Feature                      | Description                                     |
|------------------------------|-------------------------------------------------|
| **Const-correct semantics**  | Guards read-only access only; mutation is UB.   |
| **Adaptive locking**         | Automatically selects shared or exclusive mode. |
| **RAII lifetime management** | Acquires and releases automatically.            |
| **No-op optimization**       | Eliminated entirely with `null_mutex_t`.        |

---

> 📌 `jh::sync::const_lock` is a semantic lock — it enforces immutability,
> not exclusion — defining const safety as a first-class concurrency contract.
