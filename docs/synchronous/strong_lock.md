## ⏱️ **JH Toolkit — `jh::sync::strong_lock` API Reference**

📁 **Header:** `<jh/synchronous/strong_lock.h>`  
📦 **Namespace:** `jh::sync`  
📅 **Version:** 1.4.1+ (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

---

## 🧭 Introduction

`jh::sync::strong_unique_lock` and `jh::sync::strong_shared_lock`
provide **ordering-strengthened RAII adapters** for mutex-like synchronization primitives.

They insert:

```cpp
std::atomic_thread_fence(std::memory_order_seq_cst);
```

* Immediately after lock acquisition
* Immediately before lock release

The purpose is to **reduce cross-domain ordering anomalies**
when locks and atomics interact under high concurrency.

---

## 🔸 Ordering Semantics

Standard C++ locking provides acquire-release semantics.  
However, ISO C++ does **not** guarantee that such semantics fully replicate
the observable global ordering often associated with POSIX `pthread_rwlock`.

The strong adapters strengthen ordering within the **C++ abstract machine**
by introducing sequentially-consistent fences around lock boundaries.

### What This Guarantees

* Stronger cross-thread visibility at the language level
* Reduced observable reordering when mixing:

    * `std::shared_mutex`
    * atomics (`load`, `fetch_*`)
    * reference-counted objects (`shared_ptr`, `weak_ptr`)

### What This Does NOT Guarantee

* No hardware-level global total order
* No strict equivalence to POSIX `pthread_rwlock`
* No elimination of all visibility anomalies under extreme stress

> This is a bounded, practical strengthening — not a formal cross-platform equivalence layer.

---

## 🔹 Overview

| Aspect                | Description                                       |
|-----------------------|---------------------------------------------------|
| **Purpose**           | Strengthen lock boundary ordering                 |
| **Mechanism**         | `seq_cst` fences after acquire and before release |
| **Exclusive Variant** | `strong_unique_lock`                              |
| **Shared Variant**    | `strong_shared_lock`                              |
| **Platform Alias**    | `posix_smtx_*_lock`                               |
| **Primary Target**    | POSIX                                             |
| **Windows Strategy**  | Language-level strengthening approximation        |

---

## 🔹 Core Components

| Symbol                        | Type           | Description                      |
|-------------------------------|----------------|----------------------------------|
| `strong_unique_lock<Mutex>`   | Class Template | Strong exclusive lock adapter    |
| `strong_shared_lock<Mutex>`   | Class Template | Strong shared lock adapter       |
| `posix_smtx_unique_lock<Mtx>` | Alias Template | Platform-adaptive exclusive lock |
| `posix_smtx_shared_lock<Mtx>` | Alias Template | Platform-adaptive shared lock    |

---

## 🔹 Class Reference — `strong_unique_lock`

### Declaration

```cpp
template <jh::concepts::basic_lockable Mutex>
class strong_unique_lock;
```

### Requirements

* `Mutex` satisfies `jh::concepts::basic_lockable`
* Must provide:

    * `lock()`
    * `unlock()`

### Behavior

| Phase        | Action                                    |
|--------------|-------------------------------------------|
| Construction | Calls `lock()` → issues `seq_cst` fence   |
| Destruction  | Issues `seq_cst` fence → calls `unlock()` |

---

## 🔹 Class Reference — `strong_shared_lock`

### Declaration

```cpp
template <jh::concepts::shared_lockable Mutex>
class strong_shared_lock;
```

### Requirements

* `Mutex` satisfies `jh::concepts::shared_lockable`
* Must provide:

    * `lock_shared()`
    * `unlock_shared()`

### Behavior

| Phase        | Action                                           |
|--------------|--------------------------------------------------|
| Construction | Calls `lock_shared()` → issues `seq_cst` fence   |
| Destruction  | Issues `seq_cst` fence → calls `unlock_shared()` |

---

## 🔹 Platform-Adaptive Aliases — `posix_smtx_*_lock`

### On POSIX Systems

| Alias                    | Expands To         |
|--------------------------|--------------------|
| `posix_smtx_unique_lock` | `std::unique_lock` |
| `posix_smtx_shared_lock` | `std::shared_lock` |

Relies on native pthread-backed semantics.

---

### On Windows

| Alias                    | Expands To           |
|--------------------------|----------------------|
| `posix_smtx_unique_lock` | `strong_unique_lock` |
| `posix_smtx_shared_lock` | `strong_shared_lock` |

Provides a language-level strengthening approximation.

---

## 🔹 Recommended Usage Pattern

Use `jh::sync::posix_smtx_*_lock` as a **drop-in replacement**
for `std::*_lock` when operating on:

* `std::shared_mutex`
* `std::shared_timed_mutex`

For simple scoped locking:

```cpp
std::shared_mutex sm;

{
    jh::sync::posix_smtx_unique_lock lock(sm);
    // exclusive section
}

{
    jh::sync::posix_smtx_shared_lock lock(sm);
    // shared (reader) section
}
```

---

## 🔸 ISO Semantics vs Windows Observations (Extended Analysis)

Under ISO C++ rules, replacing:

```
std::unique_lock / std::shared_lock
```

with:

```
jh::sync::posix_smtx_*_lock
```

remains fully conforming and **UB-risk-free at the language level**.

The adapters only introduce additional
`std::atomic_thread_fence(std::memory_order_seq_cst)`
calls and do not alter lock ownership semantics.

From the perspective of the C++ abstract machine:

* `seq_cst` operations are globally ordered
* Acquire–release relationships are preserved
* No additional undefined behavior is introduced

---

### 🔹 Windows (MinGW) Runtime Characteristics

Empirical stress testing on:

* MinGW-w64 / MinGW-clang
* Windows runtime libraries (msvcrt / ucrt)
* High-core-count systems
* Extreme high-concurrency workloads

reveals behavior that differs from typical POSIX convergence characteristics.

---

### 1️⃣ Likely `shared_mutex` Backend: SRWLock

On Windows, `std::shared_mutex` is very likely implemented on top of **SRWLock**.

SRWLock:

* Is optimized primarily for throughput
* Prioritizes efficiency over fairness
* May exhibit scheduling asymmetry under contention
* Does not aim to emulate strict POSIX-style ordering convergence

This design decision favors performance, not deterministic global ordering.

---

### 2️⃣ Cross-Layer Assumption Risk (Compiler vs Runtime)

In MinGW environments:

* The compiler may assume the Windows runtime provides sufficient atomic guarantees.
* The runtime may assume the compiler emits sufficient fencing for atomic types.

As a result:

* Explicit `seq_cst` fences inserted at lock boundaries may not always translate
  into sufficiently strong hardware barriers across all optimized paths.
* Some fence effects may be absorbed or weakened at the machine-code level,
  depending on architecture and optimization.

This creates the possibility of **ordering dilution across abstraction layers**.

---

## 🔸 Practical Stress Amplification (Hardware-Originated Effects)

On Windows (MinGW), increasing:

* Thread count
* Call frequency (pressure per unit time)
* Per-operation computational cost
* Lock + atomic interaction density

increases the probability of:

* **Visibility widening**
* **Transient ordering anomalies**

It is critical to clarify:

These effects originate from **physical CPU behavior**, including:

* Cache coherence propagation delay
* Store buffer effects
* Core-to-core scheduling asymmetry
* Microarchitectural reordering windows

At the **ISO C++ abstract machine level**,
`seq_cst` operations are globally ordered and not reordered.

The anomalies described here are therefore **not violations of the C++ memory model**.

They arise from:

* Real hardware propagation latency
* SRWLock scheduling characteristics
* Runtime-level atomic implementation details

Under extreme pressure, the time window between:

* "logically ordered"
* and "physically visible on all cores"

may widen enough to become observable.

---

## 🔸 Interaction Model: `shared_mutex` + Atomic Types

The visibility effects are most pronounced when:

* `std::shared_mutex` is used
* and **atomic types** interact with it

In this document, **atomic types** include:

* `std::atomic<T>`
* `std::shared_ptr`
* `std::weak_ptr`

The issue is **not** the mixture of `atomic<T>` and `shared_ptr` with each other.

The problematic pattern is:

> atomic types interacting with `shared_mutex` under high contention.

Typical scenarios include:

* Atomic counters updated inside or near shared/exclusive regions
* Reference counting occurring concurrently with lock transitions
* High-frequency ownership or lifetime transitions
* Expensive operations inside lock-protected sections

---

## 🔸 Harmful Consequence: Stale Atomic Reads

Reading stale values from atomic types is not benign.

A particularly sensitive case is:

```
std::weak_ptr::lock()
```

Risk scenario:

1. Logical reference count transitions from 1 → 0
2. Managed object is destroyed
3. Another core reads a stale count value of 1
4. `weak_ptr::lock()` succeeds
5. The returned `shared_ptr` dereferences freed memory

At the language level, this constitutes **undefined behavior**.

In practice, it most commonly manifests as:

* Use-after-free
* Invalid memory access
* Out-of-bounds memory reads
* Rare but severe crash conditions under stress

While ISO C++ specifies sequential consistency,
extreme real-world hardware scheduling can widen the propagation gap enough
to expose such edge cases.

---

## 🔸 Rehash and Memory Movement Amplifiers

Additional amplification factors include:

* `unordered_*` rehash operations
* Allocator pressure
* Page-level memory movement
* Cache-line bouncing under contention

These behaviors are permitted by ISO C++ and do not violate the model.

However, under extreme stress they may increase the width of observable
cross-core visibility gaps.

---

## 🔸 Structural Limitation

Once execution enters:

* Runtime-managed reference counting (`shared_ptr`)
* OS-level primitives (SRWLock)
* Compiler-emitted atomic sequences

this library cannot:

* Inject fences into runtime internals
* Modify `shared_ptr` reference counting logic
* Alter SRWLock scheduling policy
* Enforce hardware-level total order

Therefore, no further defensive layer exists within this implementation.

---

## 🔸 Platform Scope and Design Priority

JH-Toolkit is designed primarily for POSIX platforms.

Windows (MinGW) support is:

* A bounded engineering approximation
* A best-effort strengthening strategy
* Not a full semantic reconstruction

A complete Windows-level solution would likely require:

* Replacing `shared_mutex`
* Replacing `shared_ptr` reference counting mechanisms
* Designing a custom concurrency substrate

Such changes would:

* Dramatically increase implementation complexity
* Significantly increase cognitive and maintenance burden
* Reduce performance characteristics

This is outside the intended scope of the project.

---

## 🔸 Final Limitation Statement

This implementation:

* Strengthens ordering at the C++ abstract machine level
* Reduces many cross-domain anomalies
* Improves practical determinism under stress

But it **cannot guarantee**:

* Elimination of all hardware-originated visibility gaps
* Strict hardware-level global total order
* Full equivalence to POSIX `pthread_rwlock` semantics

Under extreme concurrency on Windows (MinGW),
developers must not assume perpetual global total-order convergence,
especially when atomic types interact with `shared_mutex`.

---

## 🧩 Summary

`strong_lock` provides:

* RAII lock adapters
* Sequentially-consistent fences at lock boundaries
* Platform-adaptive behavior
* Practical strengthening for mixed lock/atomic workloads

It does **not** claim:

* Hardware-level total order
* Strict POSIX equivalence
* Complete elimination of concurrency anomalies

> `strong_lock` is a pragmatic ordering reinforcement layer,
> not a formal global-ordering guarantee.
