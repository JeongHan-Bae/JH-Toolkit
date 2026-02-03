# 🎍 **JH Toolkit — `jh::conc::occ_box` API Reference**

📁 **Header:** `<jh/concurrent/occ_box.h>`  
📦 **Namespace:** `jh::conc`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::conc::occ_box<T>` is a **single-value container implementing Optimistic Concurrency Control (OCC)**.

It provides:

* **wait-free, snapshot-based reads**
* **atomic commit-replace writes**
* **optional atomic multi-box transactions**

All updates replace the entire state with a single CAS.
No reader ever observes partial or torn writes.

> We highly recommend using `occ_box<T>` during the prototype development/MVP phase, as it significantly reduces
> cognitive load.  
> The primary overhead of `occ_box` stems from its versatility (supporting lambda calls and generic operations).
> Once your business logic is fully implemented, if performance becomes a bottleneck, we suggest designing your own
> `occ_T` business model based on `occ_box<T>`. `occ_box` already provides various implementation approaches and models.

---

## Concurrency Model

`occ_box` uses **optimistic concurrency** instead of locks.

* Readers:

    * take a snapshot
    * run user code
    * validate that no commit intervened
* Writers:

    * build a fresh state
    * attempt atomic replacement
    * retry or fail depending on API

This eliminates deadlocks and lock ordering concerns.

---

## ⚠️ Fundamental Cost Model (Critical)

`occ_box` **heavily relies on atomic replacement of `std::shared_ptr`**.

This means:

* every write allocates new heap objects
* every CAS failure may discard allocations
* heap fragmentation is **inevitable**

> This is a structural limitation, not an implementation flaw.

### Performance Implication

`occ_box` is **not suitable** for:

* high-frequency mutation
* performance-critical hot paths
* cache-line–sensitive data structures

This limitation is inherent to all pointer-heavy OCC designs
(just like `std::map`, `std::list`, and other node-based containers).

---

## ⚙️ Control and Access

All access to an `occ_box<T>` is mediated through **user-provided callables**.
Each API imposes **strict semantic and type requirements** on the callable `f`.
These constraints are intentional and fundamental to the OCC model.

---

### 📖 Read Access

#### `read(f, args...)`

```cpp
template<typename F, typename... Args>
requires
    std::invocable<F, const T&, Args...> &&
    (!std::same_as<std::invoke_result_t<F, const T&, Args...>, void>)
auto read(F&& f, Args&&... args) const;
```

**Callable signature**

```cpp
R f(const T&, Args...)
```

where:

* `R` **must not be `void`**
* `Args...` are optional extra parameters
* `f` is invoked on a **snapshot** of `T`

**Semantics**

* Performs a **load → invoke → validate** sequence
* Retries internally until:

    * the snapshot is consistent
    * and no concurrent commit intervened
* Readers are **wait-free** and never block writers

**Why `void` is forbidden**

A `read()` operation is conceptually a **pure observation of a snapshot**.

Allowing `void` would imply:

* reads used only for side effects
* duplicated side effects on retry
* meaningless behavior under optimistic validation

This is incompatible with OCC.

**Permitted side effects**

Only *auxiliary* side effects are tolerated:

* sleep / backoff bookkeeping
* logging

Even these are acceptable **only because the callable still returns a value**.

---

#### `try_read(f, retries, args...)`

```cpp
template<typename F, typename... Args>
requires
    std::invocable<F, const T&, Args...> &&
    (!std::same_as<std::invoke_result_t<F, const T&, Args...>, void>)
std::optional<R> try_read(F&& f, std::uint16_t retries = 1, Args&&... args) const;
```

**Callable signature**

```cpp
R f(const T&, Args...)
```

(same as `read()`)

**Semantics**

* Identical snapshot semantics to `read()`
* Performs at most `retries` attempts
* `retries == 0` is normalized to **one attempt**
* Returns:

    * `std::optional<R>` on success
    * `std::nullopt` if validation fails every time

**Purity rule**

As with `read()`, pure side-effect-only callables are forbidden.

---

### ✍️ Write Access (Copy-Based)

#### `write(f, args...)`

```cpp
template<typename F, typename... Args>
requires
    std::invocable<F, T&, Args...> &&
    std::same_as<std::invoke_result_t<F, T&, Args...>, void>
void write(F&& f, Args&&... args);
```

**Callable signature**

```cpp
void f(T&, Args...)
```

**Semantics**

* Loads the current state
* **Deep-copies** the underlying `T`
* Applies `f` to the copy
* Attempts to commit via a single CAS
* Retries indefinitely until success

**Key properties**

* The original object is never mutated
* Readers never observe intermediate state
* Always completes before returning

---

#### `try_write(f, retries, args...)`

```cpp
template<typename F, typename... Args>
requires
    std::invocable<F, T&, Args...> &&
    std::same_as<std::invoke_result_t<F, T&, Args...>, void>
bool try_write(F&& f, std::uint16_t retries = 1, Args&&... args);
```

**Callable signature**

```cpp
void f(T&, Args...)
```

**Semantics**

* Same copy-on-write flow as `write()`
* Bounded by `retries`
* Returns:

    * `true` if committed
    * `false` if all attempts fail

**Guarantee**

* Cannot livelock
* Caller decides fallback or retry strategy

---

### 🔁 Write Access (Pointer-Based)

#### `write_ptr(f, args...)`

```cpp
template<typename F, typename... Args>
requires
    std::invocable<F, const std::shared_ptr<T>&, Args...> &&
    std::same_as<
        std::invoke_result_t<F, const std::shared_ptr<T>&, Args...>,
        std::shared_ptr<T>
    >
void write_ptr(F&& f, Args&&... args);
```

**Callable signature**

```cpp
std::shared_ptr<T> f(const std::shared_ptr<T>&, Args...)
```

**Semantics**

* No deep copy is performed
* The callable constructs a **brand-new object**
* The returned `shared_ptr` is committed atomically
* Retries indefinitely until success

**Intended use**

* Large or expensive-to-copy objects
* Objects with discardable internal state
* Scenarios where rebuilding is cheaper than copying

---

#### `try_write_ptr(f, retries, args...)`

```cpp
template<typename F, typename... Args>
requires
    std::invocable<F, const std::shared_ptr<T>&, Args...> &&
    std::same_as<
        std::invoke_result_t<F, const std::shared_ptr<T>&, Args...>,
        std::shared_ptr<T>
    >
bool try_write_ptr(F&& f, std::uint16_t retries = 1, Args&&... args);
```

**Callable signature**

```cpp
std::shared_ptr<T> f(const std::shared_ptr<T>&, Args...)
```

**Semantics**

* Pointer-based replacement
* Bounded retry count
* Returns `false` on failure instead of spinning

---

### 🔁 Retry, Backoff, and High Contention

Under contention, **user code must explicitly back off**.

Recommended pattern inside `f`:

1. **sleep first**
2. then perform logic
3. update backoff duration externally

Example (conceptual):

```cpp
sleep(duration);
do_work();
```

* Initial sleep may be `0`
* Duration may grow exponentially
* Capped by a maximum

Sleeping **after** the operation is ineffective —
failed CAS attempts would otherwise spin uselessly.

---

### 🔎 Summary of Callable Signatures

| API                          | Required callable signature                              |
|------------------------------|----------------------------------------------------------|
| `read`, `try_read`           | `R(const T&, Args...)`, `R ≠ void`                       |
| `write`, `try_write`         | `void(T&, Args...)`                                      |
| `write_ptr`, `try_write_ptr` | `std::shared_ptr<T>(const std::shared_ptr<T>&, Args...)` |

These constraints are **not cosmetic** — they encode the OCC model directly into the type system.

---

## 🔁 Multi-box Transactions — `apply_to`

When `JH_OCC_ENABLE_MULTI_COMMIT == 1` (default),
`occ_box` supports **atomic multi-box updates** via:

```cpp
bool apply_to(
    std::tuple<Boxes&...> boxes,
    std::tuple<Funcs...>&& funcs
);
```

> **Note**
>
> `JH_OCC_ENABLE_MULTI_COMMIT` is a **compile-time configuration macro**.
>
> It can be overridden by:
>
> * defining it **before including** `<jh/concurrent/occ_box.h>`, or
> * passing `-DJH_OCC_ENABLE_MULTI_COMMIT=0` to the compiler
>
> When set to `0`, **multi-box transactions are completely disabled** and
> `apply_to` is not available.

### Signature Constraints (Strict)

* `sizeof...(Boxes) == sizeof...(Funcs)`
* **exactly one function per box**
* all functions must be **one of two styles**
* **mixing styles is a compile-time error**

---

### Two Mutually Exclusive Modes

#### 1. Copy-based Transaction

* Each box value is deep-copied
* Functions must be:

```cpp
void(T&)
```

* Best for:

    * small types
    * trivially copyable objects
* Changes are isolated until commit

---

#### 2. `shared_ptr`-based Transaction

* Functions construct **new objects directly**
* Functions must be:

```cpp
std::shared_ptr<T>(const std::shared_ptr<T>&)
```

* Best for:

    * large or complex objects
    * avoiding deep copy
* Recommended when mixing object sizes

---

### Transaction Semantics

* All boxes are committed **atomically**
* If **any conflict occurs**:

    * the entire transaction fails
    * **no retry is performed**
* Retry policy is **entirely user-controlled**

### Priority Rules

When enabled:

```
multi-write  >  single-write  >  read
```

Transactions cannot be broken by concurrent writes or reads.

---

## Retry & Backoff Guidance (Important)

For **high contention** scenarios, user code **must explicitly back off**.

### Recommended Pattern

Inside your lambda:

1. **sleep first**
2. then perform logic
3. update backoff duration externally

Example conceptually:

```cpp
sleep(duration);
do_work();
```

* Initial sleep may be `0`
* Duration increases exponentially
* Capped by a maximum

> Sleeping **after** work is useless —
> failed CAS results would spin without effect.

---

## Usage Guidance

### When `occ_box` Is a Good Fit

* read-mostly workloads
* low-frequency writes
* write priority matters
* transactional coordination
* prototypes and application-level state

### When It Is Not

Do **not** use `occ_box` for:

* tight loops
* frequent updates
* data-structure–level concurrency
* performance-critical subsystems

---

## Mental Model Advantage

Despite its cost, `occ_box` dramatically simplifies reasoning:

* no lock ordering
* no partial states
* no memory-order juggling
* retry logic is explicit and local

This makes it especially valuable for:

* early-stage systems
* correctness-first code
* coordination logic

---

## Summary

* `occ_box` implements OCC via atomic `shared_ptr` replacement
* reads are wait-free and consistent
* writes are atomic but allocation-heavy
* multi-box transactions are supported with strict priority
* fragmentation is unavoidable
* retry and backoff are explicit user responsibilities

---

## One-Sentence Summary

**`jh::conc::occ_box` is an optimistic, snapshot-based concurrency primitive that trades allocation cost for
deadlock-free semantics, strong atomicity, and dramatically simpler reasoning—ideal for read-heavy, low-frequency-write
coordination and transactional glue code.**
