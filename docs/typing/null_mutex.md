# 🧬 **JH Toolkit — `jh::typed::null_mutex` API Reference**

📁 **Header:** `<jh/typing/null_mutex.h>`  
📦 **Namespace:** `jh::typed`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::typed::null_mutex` defines a **zero-cost semantic placeholder**
for generic code that requires a `mutex_like` synchronization object
but where actual locking is **structurally unnecessary**.

It is **fully compatible** with all synchronization concepts under
[`jh::concepts::mutex_like`](../conceptual/mutex_like.md), including `timed_mutex_like`,
`rw_mutex_like`, and `reentrance_capable_mutex`.

All locking interfaces (`lock()`, `unlock()`, `try_lock()`, etc.)
are implemented as **no-ops**, and every `try_*()` function always returns `true`.

---

## 🔹 Overview

| Aspect                    | Description                                                                    |
|---------------------------|--------------------------------------------------------------------------------|
| **Purpose**               | Semantic substitute for real mutexes in "lock-required" generic contexts.      |
| **Implementation Model**  | Header-only, stateless, all members `constexpr` and `noexcept`.                |
| **Lock Behavior**         | All operations are pure no-ops; `try_*()` → `true`.                            |
| **Thread Safety**         | Declares single-thread ownership; not an actual synchronization primitive.     |
| **Concept Compatibility** | `mutex_like`, `timed_mutex_like`, `rw_mutex_like`, `reentrance_capable_mutex`. |
| **Reentrance Type**       | *Idempotent* reentrance (no counters, no nesting semantics).                   |
| **Singleton Constant**    | `inline null_mutex_t null_mutex;`                                              |

---

## 🔹 Components

| Symbol                | Description                                                      |
|-----------------------|------------------------------------------------------------------|
| `struct null_mutex_t` | Dummy mutex implementing all standard lock interfaces.           |
| `null_mutex`          | Global singleton instance representing the universal no-op lock. |

---

## 🔹 API Reference

### `struct null_mutex_t`

```cpp
struct null_mutex_t {
    using is_reentrant_tag = std::true_type;

    // Exclusive locks
    void lock() noexcept {}
    void unlock() noexcept {}
    static bool try_lock() noexcept { return true; }

    template <typename Rep, typename Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>&) noexcept { return true; }

    template <typename Clock, typename Duration>
    bool try_lock_until(const std::chrono::time_point<Clock, Duration>&) noexcept { return true; }

    // Shared locks
    void lock_shared() noexcept {}
    void unlock_shared() noexcept {}
    static bool try_lock_shared() noexcept { return true; }

    template <typename Rep, typename Period>
    bool try_lock_shared_for(const std::chrono::duration<Rep, Period>&) noexcept { return true; }

    template <typename Clock, typename Duration>
    bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration>&) noexcept { return true; }
};
```

A **structurally reentrant**, **concept-complete**, and **zero-state** mutex.  
It provides every locking method required by the full [`mutex_like`](../conceptual/mutex_like.md) concept stack,
with all implementations being semantically inert.

**Key Properties**

* Trivial, stateless, `constexpr`-safe, and fully optimized away.
* Compatible with all locking concepts (`mutex_like`, `rw_mutex_like`, etc.).
* **Idempotent reentrance:** repeated `lock()` calls on the same object
  are no-ops and always valid — unlike recursive locks, no counter is maintained.
* Suitable for single-thread, compile-time, or read-only environments.

---

### Concept Validation

```cpp
static_assert(jh::concepts::mutex_like<jh::typed::null_mutex_t>);
static_assert(jh::concepts::timed_mutex_like<jh::typed::null_mutex_t>);
static_assert(jh::concepts::rw_mutex_like<jh::typed::null_mutex_t>);
static_assert(jh::concepts::reentrance_capable_mutex<jh::typed::null_mutex_t>);
```

---

### `null_mutex`

```cpp
inline jh::typed::null_mutex_t null_mutex;
```

Global singleton instance representing the universal *no-op* mutex.  
Used whenever a lock object is syntactically required but logically redundant.

**Example**

```cpp
#include <jh/typed>
#include <jh/immutable_str>
#include <jh/synchronous/const_lock.h>

auto s = jh::safe_from("example", typed::null_mutex);
jh::sync::const_lock guard(typed::null_mutex); // no-op
```

**Semantic Note**

`null_mutex` is a **declarative artifact** —
it explicitly states that the guarded resource is:

* confined to a single thread, or
* immutable and read-safe across threads.

It does *not* implement or simulate mutual exclusion.

---

### 🧩 Why No `operator==`

`jh::typed::null_mutex_t` deliberately omits `operator==`, for the same reason
that neither `std::mutex` nor `std::shared_mutex` are comparable.

**Reasons**

1. **Behavioral, not value semantics**:  
   A mutex represents a locking *capability*, not a comparable value.
   Equality between mutexes is undefined and meaningless.

2. **Conceptual consistency**:  
   To remain interface-compatible with all `mutex_like` types,
   `null_mutex_t` avoids exposing any comparison operators.

3. **Type-level identity only**:  
   There is only one valid instance (`jh::typed::null_mutex`).
   Any comparison would always yield `true`, serving no purpose.

4. **Template reasoning**:  
   Generic code relies on *type matching*, not runtime equality,
   to infer "no-op" semantics.

> **In summary:**  
> `null_mutex_t` is a *semantic placeholder type*, not a *comparable object*.  
> Defining equality would incorrectly imply value semantics
> and violate parity with real synchronization primitives.

---

## 🧩 Summary

| Category            | Description                                                                   |
|---------------------|-------------------------------------------------------------------------------|
| **Purpose**         | Semantic, zero-overhead stand-in for `mutex_like`                             |
| **Runtime Cost**    | None                                                                          |
| **Reentrance Type** | Idempotent (not recursive)                                                    |
| **Comparison**      | Not defined                                                                   |
| **Lock Behavior**   | All methods are no-ops                                                        |
| **Concepts**        | `mutex_like`, `timed_mutex_like`, `rw_mutex_like`, `reentrance_capable_mutex` |
| **Analogy**         | `std::nullptr_t` for synchronization                                          |
| **Usage Rule**      | Use the global `jh::typed::null_mutex`; do not instantiate manually           |

---

> **Note:**  
> `jh::typed::null_mutex` exists purely to preserve **type-level uniformity**
> in generic synchronization code, enabling zero-branching lock wrappers
> to operate transparently even when locking is unnecessary.
