# 🧱 **JH Toolkit — `jh::sim_pool` API Reference**

📁 **Header:** `<jh/core/sim_pool.h>`  
🔄 **Forwarding Header:** `<jh/sim_pool>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/Back_to_README-blue?style=flat-square)](../../README.md)

</div>

---

## ⚙️ Overview

`jh::sim_pool` (**Smart Immutable-objects Managing Pool**)
is a **weak-pointer–observed**, **non-intrusive** object pool for **immutable** or **structurally immutable** objects.  

It deduplicates logically equivalent instances while ensuring that external `std::shared_ptr` holders remain valid even after the pool itself is destroyed.  
The pool never owns its elements — it only *observes* lifetimes.

---

### 🧭 Design Overview

`sim_pool` provides lightweight, race-safe interning of shared objects.  
It supports both **truly immutable** and **structurally immutable** objects —  
where *identity-defining fields* (those affecting hash and equality) remain constant for life.

Typical usage includes:

* Shared text primitives like [`jh::immutable_str`](immutable_str.md).  
* Handle- or resource-type wrappers whose unique identity is fixed.

---

### 🔹 Design Principles

| Aspect                       | Description                                                         |
|------------------------------|---------------------------------------------------------------------|
| **Ownership**                | Observation only — never claims or extends object lifetime.         |
| **Lifetime coupling**        | None — destruction order between pool and objects irrelevant.       |
| **Thread model**             | Shared/exclusive locking for concurrent reads and atomic insertion. |
| **Cleanup policy**           | Event-driven: opportunistic removal of expired entries.             |
| **Resizing policy**          | Adaptive; doubles or halves capacity on high/low watermark.         |
| **Immutability requirement** | Fields defining identity must remain constant.                      |

---

## 🔹 Core Behavior

1. Objects are constructed first (with forwarded arguments).  
2. The pool lock is acquired only during lookup/insertion.  
3. If an equivalent object already exists, it is reused and the temporary is discarded.  
4. If not found, the new object is inserted and returned.  

This **construct-first, lock-then-insert** model minimizes contention,
and supports even non-copyable or non-movable types such as `immutable_str`.  
Temporary objects must be *cheap to discard* or support *lazy initialization*.

---

## 🔹 Immutability & Structural Identity

`sim_pool` enforces the concept of **structural immutability**:

| Concept                           | Meaning                                                                                                                                   |
|-----------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
| **Immutable object**              | All state is constant after construction.                                                                                                 |
| **Structurally immutable object** | Only identity-defining fields (those used in hash / equality) are constant;<br> mutable internals are allowed if externally synchronized. |

Objects with mutable fields may still be stored,
but all fields influencing `Hash` or `Eq` **must remain invariant**
throughout the object's lifetime.

---

## 🔹 Cleanup Model

* **Attempt-based cleanup** — Expired entries are removed automatically during insertion or capacity expansion.  
* **Non-aggressive reclamation** — Cleanup is event-driven, not periodic.  
* **Adaptive resizing** — When capacity thresholds are reached, the pool first performs cleanup, then:
    * Doubles capacity if still near full (`>87.5%`).  
    * Halves capacity if underused (`<25%`).  
* Shrinkage is always **conservative** — capacity halves (or stops at 16)
  even if usage is much smaller, to prevent oscillation and allocation jitter.  

---

## 🔬 API Breakdown

### 🏗️ **Construction**

```cpp
explicit sim_pool(std::uint64_t reserve_size = MIN_RESERVED_SIZE);
```

Initializes the pool with reserved capacity.

| Aspect        | Description                                                 |
|---------------|-------------------------------------------------------------|
| **Parameter** | `reserve_size` — initial reserve (default `16`).            |
| **Behavior**  | Preallocates internal hash storage; no objects constructed. |
| **Guarantee** | Pool never shrinks below `MIN_RESERVED_SIZE`.               |

---

### 🚫 **Copy / Move Semantics**

| Operation | Availability | Semantics                                                    |
|-----------|--------------|--------------------------------------------------------------|
| Copy      | ❌ Deleted    | Prevents duplicate observers for same object space.          |
| Move      | ✅ Available  | Transfers weak references and reserved size; source cleared. |

Live `shared_ptr` instances are unaffected by move operations.

---

### 🧩 **Object Acquisition**

```cpp
template<typename... Args>
std::shared_ptr<T> acquire(Args&&... args);
```

Retrieves a shared instance of `T` from the pool,
or constructs and inserts a new one if none exists.  

**Acquisition Flow**

1. A temporary object is constructed using forwarded arguments.  
2. The pool lock is acquired for lookup and possible insertion.  
3. If a logically equivalent instance exists (as determined by `Eq`), it is reused.  
4. Otherwise, the new object is inserted and returned.  

| Aspect                  | Description                                                                                      |
|-------------------------|--------------------------------------------------------------------------------------------------|
| **Template parameters** | Forwarded constructor argument types.                                                            |
| **Return type**         | `std::shared_ptr<T>` — pooled or newly inserted instance.                                        |
| **Thread safety**       | Safe for concurrent calls; insertion atomic.                                                     |
| **Construction model**  | Construct-first, lock-late for minimal contention.                                               |
| **Design note**         | Supports non-copyable / non-movable `T`. Temporary objects may be discarded if duplicates exist. |

---

### 🧹 **Cleanup**

```cpp
void cleanup();
```

Removes expired `weak_ptr` entries from the pool.

| Aspect            | Description                                               |
|-------------------|-----------------------------------------------------------|
| **Purpose**       | Reclaim hash table space after released shared objects.   |
| **Trigger**       | Manual call or internal capacity check.                   |
| **Thread safety** | Exclusive lock.                                           |
| **Effect**        | Does not affect live instances — only expired references. |

---

### 🧩 **Cleanup with Shrink**

```cpp
void cleanup_shrink();
```

Performs cleanup and conditionally reduces capacity.

| Aspect            | Description                                                               |
|-------------------|---------------------------------------------------------------------------|
| **Behavior**      | After cleanup, if usage < 25%, reserved capacity halves (or stops at 16). |
| **Shrink model**  | Always halves — never fine-tunes below half; avoids jitter.               |
| **Policy**        | Conservative: memory reuse readiness favored over minimal footprint.      |
| **Thread safety** | Exclusive lock.                                                           |

---

### 📊 **Capacity Query**

```cpp
[[nodiscard]] std::uint64_t reserved_size() const;
```

Returns current reserved capacity (adaptive threshold).

| Aspect            | Description                                |
|-------------------|--------------------------------------------|
| **Thread safety** | Shared read.                               |
| **Behavior**      | Reflects internal limit, not active count. |

---

### 📏 **Element Count**

```cpp
[[nodiscard]] std::uint64_t size() const;
```

Returns current total stored weak entries (expired included).

| Aspect            | Description  |
|-------------------|--------------|
| **Thread safety** | Shared read. |

---

### 🔄 **Clear**

```cpp
void clear();
```

Removes all entries and resets baseline capacity to `16`.  

| Aspect            | Description                                                 |
|-------------------|-------------------------------------------------------------|
| **Effect**        | Discards observation records only; live objects unaffected. |
| **Thread safety** | Exclusive write lock.                                       |
| **Use caution**   | Not recommended for resource/handle-type pools.             |

---

## ⚙️ Concurrency & Safety

| Guarantee                 | Description                                          |
|---------------------------|------------------------------------------------------|
| **Concurrent acquire()**  | Safe across threads; insertion is atomic.            |
| **Locking granularity**   | Shared for reads, exclusive for modifications.       |
| **Lifetime independence** | External `shared_ptr`s valid after pool destruction. |
| **Cleanup timing**        | Only during insertion or explicit maintenance.       |

---

## ⚙️ Adaptive Resizing Logic

| Threshold      | Ratio | Action                           |
|----------------|-------|----------------------------------|
| High watermark | 0.875 | Double capacity.                 |
| Low watermark  | 0.25  | Halve capacity (never below 16). |
| Otherwise      | —     | Retain current capacity.         |

Even if current usage is far below 25%,
capacity only halves — no multi-step reduction —
to preserve performance stability.

---

## 🧱 Performance & Characteristics

| Metric               | Behavior                                            |
|----------------------|-----------------------------------------------------|
| **Insertion cost**   | O(1) amortized (hash lookup + possible cleanup).    |
| **Cleanup cost**     | Linear in current pool size.                        |
| **Lock contention**  | Minimal, lock held only for short insertion period. |
| **Memory stability** | Conservative adaptive model prevents oscillation.   |
| **Thread safety**    | Guaranteed via shared mutex.                        |

---

## 🧩 Summary

`jh::sim_pool` is a **non-owning, weak-pointer–based deduplication pool**
designed for immutable or identity-stable objects.  
It provides:

* Safe concurrent interning across threads  
* Adaptive cleanup and resizing  
* Lifetime independence and predictable memory behavior  
* Minimal coupling between pool and shared object ownership  

It forms the **foundation** for higher-level interning utilities within the JH Toolkit.
