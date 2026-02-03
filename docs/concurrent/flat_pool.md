# 🎍 **JH Toolkit — `jh::conc::flat_pool` API Reference**

📁 **Header:** `<jh/concurrent/flat_pool.h>`  
📦 **Namespace:** `jh::conc`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## Overview

`jh::conc::flat_pool<Key, Value, Hash, Alloc>` is a **concurrent, key-based interning container** that stores objects in
**contiguous memory** and provides **GC-like lifetime semantics** through explicit reference counting.

Each unique `Key` corresponds to **at most one logical entry** in the pool.
Acquisition returns a lightweight, reference-counted handle (`flat_pool::ptr`) that refers to a **stable index**, not a
raw pointer.

Unlike pointer-based pools, `flat_pool`:

* **owns its objects**
* **does not rely on `std::shared_ptr` for synchronization**
* **controls all concurrency internally**
* **reuses slots instead of eagerly destroying objects**

This design minimizes allocation churn, avoids platform-specific `shared_ptr` behavior, and provides predictable
performance under high concurrency.

---

> ### Complexity and Storage Characteristics
>
> `flat_pool` stores **values, control blocks, and indices** using `std::vector` or
> vector-like contiguous data structures.
> All internal memory regions grow monotonically and are reused in place, resulting
> in **minimal fragmentation** and highly predictable access patterns.
>
> Insertions write directly into contiguous storage. When a previously released slot
> exists, the pool locates the next available position using a lightweight scan over
> an occupation bitmap. Although this scan is *theoretically* amortized **O(log N)**,
> in practice it degenerates into a sequence of simple `0/1` checks with excellent
> branch predictability and cache locality.
>
> Once the pool reaches a stable working size, it is expected **not to expand further**.
> The intended usage pattern is:
>
> * grow to a peak size,
> * then continuously acquire and invalidate entries,
> * reusing existing slots rather than allocating new ones.
>
> Conceptually, this places `flat_pool` closer to an **arena + GC-style reuse model**
> than to a classical associative container.
>
> From a purely algorithmic standpoint, lookup and insertion in `flat_pool` are
> theoretically inferior to hash-table–based designs such as `pointer_pool`
> (which relies on `unordered_map`).
> However, in practice:
>
> * pointer-based containers suffer from **pointer-induced fragmentation**,
> * cache-line scattering and **L3 cache regression**,
> * and the non-trivial overhead of `std::shared_ptr` control blocks.
>
> As a result, `pointer_pool` is ill-suited for **high-frequency, large-volume,
> data-oriented workloads**.
>
> `flat_pool`, by contrast, is optimized for:
>
> * pure or near-pure data objects,
> * relatively large populations,
> * frequent reuse rather than destruction,
> * and cache-friendly, contiguous memory access.
>
> In summary:
>
> * **`flat_pool`** is designed for *data-centric storage* with high churn and reuse.
> * **`pointer_pool`** is better suited for *semantic objects*—objects whose identity,
    > lifetime, and destruction semantics are more important than raw storage efficiency.

---

### Identity Model (Key-Driven)

Object identity is defined **exclusively** by `Key`.

* The stored object is **never compared**
* Hashing and equality are performed only on `Key`
* Keys are accepted in cv/ref-qualified form and are **not retained** unless insertion succeeds

Requirements for `Key`:

* cheap to hash
* cheap to compare
* capable of representing identity independently of storage

This enables **lookup-before-construction** and avoids provisional object creation.

---


---

## 🔧 Member Summary

### 📦 Type Aliases

| Member           | Type                             | Description                                                                   |
|------------------|----------------------------------|-------------------------------------------------------------------------------|
| `value_type`     | `Key` or `std::pair<Key, Value>` | Stored element type. `Key` in set-like mode, `(Key, Value)` in map-like mode. |
| `allocator_type` | Rebound allocator                | Allocator used for contiguous value storage only.                             |

---

### 🏗 Constructors

| Member                                      | Type        | Description                                                                       |
|---------------------------------------------|-------------|-----------------------------------------------------------------------------------|
| `flat_pool()`                               | Constructor | Constructs an empty pool with minimum reserved capacity.                          |
| `explicit flat_pool(size_t reserve)`        | Constructor | Constructs an empty pool with explicit reserved capacity (≥ `MIN_RESERVED_SIZE`). |
| `explicit flat_pool(const allocator_type&)` | Constructor | Constructs a pool using a custom allocator.                                       |
| `flat_pool(size_t, const allocator_type&)`  | Constructor | Constructs a pool with custom allocator and reserved capacity.                    |

**Deleted:**

| Member                        | Description                                     |
|-------------------------------|-------------------------------------------------|
| `flat_pool(const flat_pool&)` | Copy construction disabled.                     |
| `flat_pool(flat_pool&&)`      | Move construction disabled.                     |
| Copy / move assignment        | Disabled to preserve index and handle validity. |

---

### 🔑 Acquisition & Lookup

| Member                                         | Type     | Description                                                                             |
|------------------------------------------------|----------|-----------------------------------------------------------------------------------------|
| `ptr acquire(KArg&& key)`                      | Set-like | Interns and returns a handle to a key (`Value == monostate`).                           |
| `ptr acquire(KArg&& key, std::tuple<Args...>)` | Map-like | Interns a key–value entry, constructing value once if absent.                           |
| `ptr find(const Key&)`                         | Lookup   | Returns a handle to an existing entry; **returns `nullptr` if the key is not present**. |

> Note: use `if (auto p = pool.find(...); p != nullptr)` as operator `bool` is currently not defined for `flat_pool::ptr`.

**Deleted (by design):**

| Member                             | Description                                         |
|------------------------------------|-----------------------------------------------------|
| `acquire(key)` (map-like)          | Deleted to prevent accidental value omission.       |
| `acquire(key, args...)` (set-like) | Ill-formed: no value construction in set-like mode. |

---

### 📎 `flat_pool::ptr` — Handle Type

| Member            | Type        | Description                                                |
|-------------------|-------------|------------------------------------------------------------|
| `ptr()`           | Constructor | Constructs a null handle.                                  |
| `ptr(nullptr_t)`  | Constructor | Explicit null handle.                                      |
| `ptr(const ptr&)` | Copy        | Shares the reference and increments refcount.              |
| `ptr(ptr&&)`      | Move        | Transfers ownership without refcount change.               |
| `~ptr()`          | Destructor  | Releases reference (GC-like).                              |
| `reset()`         | Modifier    | Releases the reference and becomes null.                   |
| `operator*()`     | Access      | Returns reference to stored object (guard required in MT). |
| `operator->()`    | Access      | Returns pointer to stored object (guard required in MT).   |
| `operator==`      | Comparison  | Compares handle identity or against `nullptr`.             |
| `guard()`         | Guard       | Prevents pool reallocation during dereference.             |

---

### 📊 Observers & Metrics

| Member                                  | Type     | Description                                                 |
|-----------------------------------------|----------|-------------------------------------------------------------|
| `bool empty()`                          | Observer | Returns `true` if no active entries exist.                  |
| `size_t size()`                         | Observer | Number of active (live) entries.                            |
| `size_t capacity()`                     | Observer | Current storage capacity (historical peak).                 |
| `pair<size_t, size_t> occupancy_rate()` | Observer | Snapshot of `(capacity, active_entries)` under shared lock. |

---

### 🧹 Maintenance

| Member               | Type        | Description                                                       |
|----------------------|-------------|-------------------------------------------------------------------|
| `void resize_pool()` | Maintenance | Shrinks storage to fit active entries (blocking, cold-path only). |

⚠️ **Must not be called while holding any pool-related guard or lock.**

---

### 🔒 Constants

| Member              | Type                 | Description                                     |
|---------------------|----------------------|-------------------------------------------------|
| `MIN_RESERVED_SIZE` | `constexpr uint64_t` | Minimum reserved capacity for internal storage. |

---

### 📝 Notes

* `flat_pool` internally relies on a **hash-ordered, contiguous index structure** based on `ordered_set`.
* `ordered_set` is itself a **public, reusable component** and is documented separately.

👉 See: [`ordered_set`](../core/ordered_set.md)

---

## Operating Modes

### Set-like Mode (`Value == jh::typed::monostate`)

The pool behaves as a **concurrent deduplicating set**.

```cpp
jh::conc::flat_pool<std::string> pool;

std::string key = "alpha";

auto p1 = pool.acquire(key);               // const std::string&
auto p2 = pool.acquire(std::string(key));  // std::string&&
```

Only keys are stored. No value construction occurs.

---

### Map-like Mode (`Value != jh::typed::monostate`)

The pool behaves as a **concurrent, deduplicating map**.

```cpp
jh::conc::flat_pool<int, std::string> pool;

auto p = pool.acquire(
    42,
    std::forward_as_tuple("meaning of life")
);
```

Internally, the stored type is:

```cpp
std::pair<Key, Value>
```

#### Construction Semantics

* Value construction occurs **exactly once per key**
* If the key already exists:

    * the provided argument tuple is **discarded**
    * the existing value is reused
* Repeated `acquire()` calls **never update** the value

This is **intentional**: `flat_pool` is not a mutable map.

## Set-like Semantics (`Value == jh::typed::monostate`)

When `Value` is `jh::typed::monostate`, `flat_pool` operates in **set-like mode**.

In this mode, the pool represents a **concurrent, deduplicating set of keys**, rather than a key–value map.

---

### Conceptual Model

In set-like mode:

* **Only keys are stored**
* There is **no associated value**
* Each unique `Key` corresponds to **at most one logical entry**
* The stored element type is exactly:

```cpp
Key
```

rather than `std::pair<Key, Value>`.

The pool therefore acts as a **key-interning table** with GC-like lifetime semantics and contiguous storage.

---

### Acquisition Semantics

The only valid acquisition form in set-like mode is:

```cpp
ptr acquire(KArg&& key);
```

where `KArg` must be a **cv/ref-qualified variant of `Key`**:

* `Key&`
* `const Key&`
* `Key&&`

There is **no heterogeneous or transparent lookup**.

If an argument of another type is provided, it is **implicitly converted to `Key` first**, and the lookup is performed
using the resulting `Key` object.

Example with `Key = std::string`:

```cpp
jh::conc::flat_pool<std::string> pool;

std::string key = "alpha";

auto p1 = pool.acquire(key);               // const std::string&
auto p2 = pool.acquire(std::string(key));  // std::string&&
```

Properties:

* Both calls perform lookup using a `std::string`
* Deduplication is based solely on `std::string` hash and equality
* The two handles refer to the **same pooled slot**

---

### No Construction Arguments

In set-like mode:

* There is **no value**
* There is **no construction argument tuple**

As a result:

```cpp
acquire(key, args...)
```

is **ill-formed and deleted at compile time**.

This enforces a strict separation between:

* set-like identity interning
* map-like key–value construction

---

### Stored Object Semantics

Dereferencing a handle yields a reference to the stored key:

```cpp
Key&
```

Example:

```cpp
auto p = pool.acquire(key);

[[maybe_unused]] auto g = p.guard();
use_key(*p);   // *p is a Key&
```

The key is stored **by value** inside the pool and participates fully in:

* contiguous storage
* reference counting
* slot reuse
* deferred destruction

---

### Lifetime and Slot Reuse

Reference counting behaves identically to map-like mode:

* Copying `ptr` increments the reference count
* Destroying or resetting `ptr` decrements it
* When the reference count reaches zero:

    * the slot becomes **unoccupied**
    * the key is **not immediately destroyed**
    * the slot may later be reused for a different key

This GC-like behavior avoids frequent destruction and reallocation of keys.

---

### Concurrency and `guard()`

Even though only keys are stored, the same dereferencing rules apply:

* keys reside in **contiguous storage**
* insertions and `resize_pool()` may reallocate

Therefore:

* **single-threaded code**: `guard()` is unnecessary
* **multi-threaded code**: `guard()` is required when dereferencing

Recommended, migration-safe style:

```cpp
auto p = pool.acquire(key);

[[maybe_unused]] auto g = p.guard();
use_key(*p);
```

---

### Typical Use Cases

Set-like `flat_pool` is well-suited for:

* string interning
* symbol or token tables
* canonical identifiers
* shared configuration keys
* high-frequency deduplication of lightweight objects

In these scenarios:

* identity is fully represented by the key
* delayed destruction and slot reuse are desirable
* contiguous storage improves cache locality

---

### Relationship to Map-like Mode

Set-like mode is not a secondary or restricted feature.

Conceptually:

* set-like mode is map-like mode with the value omitted
* all concurrency, lifetime, and reuse semantics are identical
* only value construction and storage differ

This allows users to:

* begin with pure key interning
* later migrate to key–value storage
* without changing the fundamental usage or mental model

---

## `flat_pool::ptr` — Handle Semantics

`flat_pool::ptr` behaves *logically* like `std::shared_ptr`, but:

* it refers to a **pool-managed slot**
* it does **not** own memory directly
* destruction is **deferred**

### Reference Counting

* Copy → increments refcount
* Destruction / `reset()` → decrements refcount
* When refcount reaches zero:

    * the slot becomes **reusable**
    * the object is **not immediately destroyed**

This GC-like model avoids destructor / constructor churn.

---

## Dereferencing and `guard()`

### Why `guard()` Exists

`flat_pool` stores objects in **contiguous storage** (`std::vector`).

* Insertions or `resize_pool()` may trigger **reallocation**
* Reallocation invalidates references (`T&`, `T*`)
* The **handle itself remains logically valid**

To prevent this, dereferencing must be protected in concurrent scenarios.

---

### Single-Threaded Usage

In **strictly single-threaded** code:

```cpp
auto p = pool.acquire(key);
use(*p);   // safe
```

`guard()` is **not required** and has no effect.

---

### Multi-Threaded Usage (Mandatory)

In **multi-threaded** contexts, if *any* thread may:

* insert
* release
* or call `resize_pool()`

then **dereferencing requires a guard**:

```cpp
auto p = pool.acquire(key);

[[maybe_unused]] auto g = p.guard();
use(*p);
```

#### Important properties of `guard()`:

* returns an **internal, private RAII type**
* **non-copyable**
* **non-movable**
* **non-operable**
* exists purely for **scope-based lifetime protection**

It must be used with **RAII / scope semantics**.

The `[[maybe_unused]]` attribute is recommended to silence static analysis warnings.

---

### Migration-Friendly Design

If code **may later migrate from single-threaded to multi-threaded**, you may:

```cpp
[[maybe_unused]] auto g = p.guard();
use(*p);
```

even in single-threaded builds.

* The performance cost is negligible
* The code becomes future-proof
* No behavioral change is introduced

---

## `value_factory` — Value Construction Policy

`value_factory<Value>` defines **how values are constructed** in map-like pools.

### Purpose

Its role is **not** to manage pooling itself, but to:

* customize how a `Value` instance is created
* optionally redirect allocation strategy
* reduce fragmentation for pointer-heavy types

This is especially useful when `Value`:

* is a handle type (`shared_ptr`, `unique_ptr`)
* internally owns dynamic buffers (`vector`, `string`)
* requires custom construction logic

---

### Default Behavior

```cpp
Value(args...)
```

Specializations are provided for:

* `std::shared_ptr<T>` → `std::make_shared<T>`
* `std::unique_ptr<T>` → `std::make_unique<T>`

---

### Custom Specialization Example

```cpp
namespace jh::conc::extension {
    template<>
    struct value_factory<Foo> {
        static Foo make(int a, int b) {
            return Foo(a * 2, b * 2);
        }
    };
}
```

This is an **intentional public extension point**.
No subclassing or pool modification is required.

---

## `resize_pool()` — Maintenance Operation

### Semantics

`resize_pool()`:

* acquires **exclusive locks**
* scans for the highest active slot
* shrinks storage to the next power-of-two ≥ required size
* respects `MIN_RESERVED_SIZE`

This is a **blocking operation**.

---

### Deadlock Warning (Critical)

⚠️ **Never call `resize_pool()` while holding pool-related locks.**

Examples of incorrect usage:

* calling `resize_pool()` while holding:

    * a `no_reallocate_guard`
    * an internal pool lock (directly or indirectly)

This will **immediately deadlock**.

#### Safe Rule

* `resize_pool()` must be called from a **cold path**
* no active dereferencing scopes
* no guards held

Holding a `flat_pool::ptr` **is fine**
(pointer ownership alone does not hold locks).

---

## Non-Copyable / Non-Movable Objects

If your object:

* is **not copyable or movable**
* does **not require immediate destruction**
* can be deduplicated via a key

then this is a **valid and recommended pattern**:

```cpp
jh::conc::flat_pool<Key, std::shared_ptr<V>>
jh::conc::flat_pool<Key, std::unique_ptr<V>>
```

### Why this works well

* The pool manages identity and reuse
* The pointer manages object lifetime
* You avoid `shared_ptr`-based interning containers

---

### Windows-Specific Note

`pointer_pool` / `observe_pool` rely on `shared_ptr` + `weak_ptr`.

On **Windows (including UCRT)**:

* `shared_ptr` implementations are relatively slow
* no Windows runtime guarantees high-concurrency performance for
  `shared_ptr + weak_ptr`

`flat_pool` avoids this entirely by controlling synchronization internally.

---

## When *Not* to Use `flat_pool`

Do **not** use `flat_pool` if:

* object destruction must occur **immediately**
* object identity depends on **address stability**
* object holds thread-affine or RAII-critical resources
* mutation-after-acquire semantics are required

Use `pointer_pool` / `observe_pool` instead.

---

## Summary

> **`flat_pool` is a concurrent, key-addressed, GC-like object table with contiguous storage.**

* Key defines identity
* Value is constructed once
* Handles behave like `shared_ptr`
* Destruction is deferred
* Slot reuse is preferred over churn
* Concurrency is pool-controlled and deterministic

This makes `flat_pool` ideal for **high-frequency deduplication**, **cache-friendly layouts**, and **platform-stable
concurrency behavior**.
