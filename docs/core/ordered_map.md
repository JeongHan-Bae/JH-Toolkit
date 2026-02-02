# 🧱 **JH Toolkit — `jh::ordered_map` / `jh::ordered_set` API Reference**

📁 **Header:** `<jh/core/ordered_map.h>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## What It Is

`jh::ordered_map<K, V, Alloc>` and `jh::ordered_set<K, Alloc>` are **ordered associative containers** implemented as a
**contiguous AVL tree**.

Unlike `std::map` / `std::set` (node-based red–black trees):

* **all nodes live inside a single `std::vector`**
* tree links are **indices, not pointers**
* no per-node allocation
* erase compacts storage by moving the last node

This design prioritizes:

* allocator predictability
* minimal fragmentation
* cache-friendly traversal
* PMR-friendly `clear()` behavior

They are **not drop-in replacements** for STL containers; they trade iterator stability for memory and latency
determinism.

---

## Storage Model (Critical to Understand)

Each element is stored as an AVL node inside a contiguous vector:

```cpp
struct avl_node {
    store_t stored_;          // K or pair<K,V>
    size_t parent, left, right;
    uint16_t height;
};
```

Key properties:

* **no node is individually allocated**
* tree structure is maintained by indices
* erasing an element:

    * removes the node
    * moves the last node into the freed slot
    * updates indices
* therefore **iterators are invalidated on erase**

This is a **compact, relocation-based tree**, not a pointer-stable one.

---

## When You Should Use It

Use `ordered_map` / `ordered_set` when:

* memory fragmentation matters
* allocator churn must be avoided
* PMR `clear()` should be near `O(1)`
* iteration and lookup dominate over insertion
* long-running systems require stable latency
* bulk construction from sorted data is common

Do **not** use it when:

* you rely on stable iterators across erase
* you store external references to nodes
* you need heterogeneous lookup with custom comparators (not supported)

---

## Complexity Summary

| Operation     | Complexity                  | Notes                 |
|---------------|-----------------------------|-----------------------|
| `find`        | `O(log N)`                  | AVL lookup            |
| `insert`      | `O(log N)`                  | rotations via indices |
| `erase`       | `O(log N)` + `O(1)` compact | moves last node       |
| iteration     | `O(N)`                      | cache-friendly        |
| `clear()`     | `O(1)` (vector reset)       | especially under PMR  |
| `from_sorted` | `O(N)`                      | no rotations          |

---

## Iterator Semantics (Important)

### Validity rules

* Any **erase invalidates all iterators**
* except **the iterator returned by `erase()`**
* insertions may also relocate nodes
* iterators are **index-based, not pointer-based**

This is a deliberate design tradeoff.

---

## API Overview

### Type aliases

```cpp
jh::ordered_set<K, Alloc>
jh::ordered_map<K, V, Alloc>
```

Internally both are backed by:

```cpp
jh::avl::tree_map<K, V, Alloc>
```

with `V = jh::typed::monostate` for set semantics.

---

### Lookup

```cpp
iterator find(const K& key);
const_iterator find(const K& key) const;

iterator lower_bound(const K& key);
iterator upper_bound(const K& key);

std::pair<iterator, iterator> equal_range(const K& key);
```

All lookup operations are non-mutating and `O(log N)`.

---

### Insertion — set

```cpp
std::pair<iterator, bool> insert(const K& key);
std::pair<iterator, bool> emplace(Args&&... args);
```

* keys are unique
* no hint insertion
* duplicate insert returns `{existing, false}`

---

### Insertion — map

```cpp
template<typename P>
requires jh::concepts::pair_like_for<K, V, P>
std::pair<iterator, bool> insert(P&& p);

std::pair<iterator, bool>
insert_or_assign(K&& key, V&& value);

template<typename... Args>
std::pair<iterator, bool> emplace(Args&&... args);
```

Notes:

* `value_type` is **not** constructed directly
* key and mapped value are consumed separately
* any tuple-like `(K, V)` is accepted if types match

See [`jh::concepts::pair_like_for`](../conceptual/tuple_like.md) for details.

---

### Element access (map only)

```cpp
V& at(const K& key);
const V& at(const K& key) const;

V& operator[](const K& key);  // requires default-initializable V
```

* `at()` throws on missing key
* `operator[]` inserts on miss

---

### Erase

```cpp
iterator erase(iterator pos);
iterator erase(const_iterator pos);
size_t erase(const K& key);
```

Erase behavior:

* compacts storage
* updates AVL structure
* invalidates all previous iterators
* returns successor iterator

---

### Capacity & lifecycle

```cpp
bool empty() const noexcept;
size_t size() const noexcept;

void clear() noexcept;
void reserve(size_t n);
void shrink_to_fit();
```

`clear()`:

* resets vector
* does **not** traverse nodes
* effectively `O(1)`
* ideal for PMR monotonic resources

---

## Bulk Construction (`from_sorted`)

```cpp
template<std::ranges::sized_range R>
static ordered_map from_sorted(R&& r);
```

Requirements:

* input is **strictly sorted**
* input is **strictly unique**
* size is known
* **no validation is performed**

Guarantees:

* perfect AVL shape
* no rotations
* near `O(N)` construction
* best possible iteration locality

Recommended pipeline:

```cpp
std::stable_sort(v.begin(), v.end());
v.erase(std::unique(v.begin(), v.end()), v.end());
auto m = jh::ordered_map<K,V>::from_sorted(v);
```

This is often **faster than random insertion**, even including sort cost.

---

## Range / Iterator Construction

In addition to incremental insertion and `from_sorted()`,
`jh::ordered_map` / `jh::ordered_set` also support **direct construction from iterator ranges and ranges**.

This form is intended for **one-shot population** of a container when elements already exist in another container or
range.

---

### Iterator-based construction

```cpp
template<std::input_iterator It>
ordered_map(It first, It last);
```

Semantics:

* the container is first reset to an empty state
* all elements in `[first, last)` are inserted using `insert()`
* ordering and uniqueness rules are identical to normal insertion

Optimization behavior:

* if `It` models `std::random_access_iterator`

    * the container will pre-reserve `last - first` nodes
    * this avoids intermediate reallocations
* otherwise

    * insertion proceeds without reservation

#### ⚠️ Iterator category correctness matters

Iterators **must accurately model their category**.

In particular:

* a fundamentally forward / bidirectional iterator
  **must not pretend to be random-access**
* iterators that emulate `operator+` / `operator[]` via repeated `++`
  will cause severe performance degradation

The implementation assumes:

> random-access distance and indexing are **O(1)** if the iterator claims to be random-access

Violating this assumption results in unintended `O(n²)` behavior.

---

### Range-based construction

```cpp
template<std::ranges::input_range R>
explicit ordered_map(R&& r);
```

Semantics:

* the container is cleared
* each element of `r` is inserted via `insert()`
* duplicate handling follows normal insertion rules

Optimization behavior:

* if `R` models `std::ranges::sized_range`

    * the container may reserve `std::size(r)` nodes in advance
* for random-access ranges, this is typically `O(1)`

This constructor is especially convenient when working with:

* `std::vector`
* `std::array`
* `std::span`
* range pipelines that preserve size information

---

### Relationship to `from_sorted()`

Range / iterator construction and `from_sorted()` serve **different purposes**:

| Construction method   | Purpose                    | Complexity   | Rotations |
|-----------------------|----------------------------|--------------|-----------|
| iterator / range ctor | convenience, general input | `O(N log N)` | yes       |
| `from_sorted()`       | optimal bulk build         | `O(N)`       | none      |

Guideline:

* use **range construction** when:

    * input order is unknown
    * simplicity is preferred
* use **`from_sorted()`** when:

    * data can be sorted and uniqued
    * build-time performance matters
    * maximum locality is desired

---

### Design note

These constructors exist to support **engineering workflows**, not as a replacement for `from_sorted()`.

They intentionally reuse `insert()` semantics to ensure:

* consistent balancing behavior
* consistent duplicate handling
* identical correctness guarantees

For performance-critical bulk builds, `from_sorted()` remains the preferred path.

---

## Comparison with `std::map` / `std::set`

| Aspect              | STL           | `jh::ordered_*`  |
|---------------------|---------------|------------------|
| Node storage        | heap-per-node | single vector    |
| Fragmentation       | high          | minimal          |
| Iterator stability  | strong        | weak (by design) |
| Cache locality      | poor          | excellent        |
| `clear()` under PMR | `O(N)`        | ~`O(1)`          |
| Bulk build          | none          | `from_sorted()`  |

---

## Engineering Philosophy: Why Vector Beats Pointers in Practice

This container is designed from an **engineering-first** perspective rather than a purely abstract data-structure view.

### Vector-based trees are more predictable than pointer-based trees

From a systems standpoint, a container built on top of `std::vector` has several structural advantages over
pointer-linked trees:

* a **single allocation domain**
* deterministic growth and reclamation behavior
* no per-node allocator interaction
* no pointer aliasing across unrelated memory regions

In contrast, pointer-based trees (`std::map` / `std::set`) fundamentally rely on:

* one allocation per node
* long-lived free lists
* allocator metadata scattered across memory
* unpredictable address reuse over long uptimes

These properties are acceptable for general-purpose containers, but become liabilities in real systems.

---

### Performance reality (measured, not theoretical)

Empirical testing shows a consistent pattern:

* **Traversal and lookup**

    * `ordered_*` is **faster than STL** due to contiguous memory and prefetch-friendly access
* **Random insertion**

    * `ordered_*` is **slower than STL**, due to AVL maintenance and compact storage
* **Erase**

    * `ordered_*` is also **slightly slower**, due to compactification
* **Difference magnitude**

    * the gap is usually small and stable, not pathological

This tradeoff is intentional and explicit.

---

### Worst-case behavior is bounded and non-pathological

The worst realistic case for `ordered_*` is:

* long-running system
* many inserts and erases
* parent / left / right indices eventually refer to nodes far apart in the vector

In this situation:

* hardware prefetching becomes less effective
* traversal locality advantage is reduced

However:

* there is **no L3 pointer-chasing regression**
* no TLB explosion
* no allocator free-list degeneration
* the worst case is simply “no locality benefit”, not *worse than STL*

By contrast, pointer-based trees **never had prefetchability to begin with**.

---

### Real workloads are rarely fully random

In actual systems, workloads are rarely adversarial:

* time-series keys
* monotonic IDs
* partially ordered logs
* batched updates
* rebuild-from-scratch phases

For these cases:

* partially ordered insertion has **comparable cost** to STL
* bulk rebuild is often preferable anyway

The recommended pattern is explicit:

```cpp
stable_sort(data.begin(), data.end());
data.erase(unique(data.begin(), data.end()), data.end());
auto m = jh::ordered_map<K,V>::from_sorted(data);
```

This is not a workaround — it is the **intended engineering path**.

---

### Erase is intentionally not optimized for bulk deletion

Unlike `std::map` / `std::multimap`, this container:

* does **not** provide range erase
* does **not** optimize for large-scale deletion
* assumes **erase is infrequent**

This reflects real usage:

* ordered maps are rarely used as “delete-heavy” structures
* bulk deletion is usually followed by rebuild
* PMR + `clear()` is the intended reset mechanism

The STL provides maximal generality.
`ordered_*` provides **engineering specialization**.

---

### Summary of the philosophy

* Pointer stability is traded for **memory stability**
* The design favors **predictable behavior over peak microbenchmarks**
* Worst cases are bounded and non-degenerate
* Partial order is the norm, not the exception
* Bulk rebuild is a first-class workflow
* This container is meant for **systems**, not toy benchmarks

In short:

> **Vector-based structures behave better over time.
> Pointer-based structures behave better in isolation.**

This library is written for the former.

---

## Design Intent

This container exists to solve problems that **pointer-based trees cannot**:

* allocator noise
* fragmentation over long uptimes
* unpredictable latency spikes
* poor iteration locality

It intentionally sacrifices iterator stability to gain:

* deterministic memory behavior
* compact representation
* predictable traversal cost

---

## Final Notes

* This is a **systems-oriented container**, not a generic STL clone.
* If you understand its invalidation rules, it is extremely robust.
* If you need stable iterators → use `std::map`.
* If you need predictable memory and fast iteration → use this.
