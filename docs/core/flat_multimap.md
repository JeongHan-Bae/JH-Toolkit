# 🧱 JH Toolkit — `jh::flat_multimap` API Reference

📁 **Header:** `<jh/core/flat_multimap.h>`  
🔄 **Forwarding Header:** `<jh/flat_multimap>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

---

## What It Is

`jh::flat_multimap<K, V, Alloc>` is an **ordered multimap implemented as a flat, contiguous container**.

It is **not**:

* a multimap extension of `ordered_map`,
* a tree-based associative container,
* or a node-oriented data structure.

Instead, it is a **containerized algorithm**:

> **A sorted contiguous sequence with explicit, first-class support for multimap range semantics.**

---

## Core Model: Stable Ordering over a Flat Sequence

Internally, `flat_multimap` stores elements as:

```cpp
std::vector<std::pair<K, V>, Alloc>
```

maintained under the following invariants:

* the sequence is **stably sorted by key**,
* elements with equivalent keys are stored **contiguously**,
* the **relative order of equivalent keys is preserved**.

All multimap semantics are expressed as **range operations** over this sequence:

* `equal_range` → two binary searches
* `erase(key)` → erase a contiguous subrange
* iteration → sequential memory traversal

This model is intentional and fundamental.

---

## Why `stable_sort`

### Stability Is Semantics, Not an Implementation Detail

`stable_sort` is required because:

* for equivalent keys, **earlier elements must remain earlier**,
* insertion order within a key group is meaningful and preserved,
* batch operations must not silently reorder values.

This is part of the observable behavior of the container.

### `stable_sort` Exploits Existing Order

All practical implementations of `std::stable_sort`:

* exploit **existing partial order** in the input,
* run near-linear time when the data is already mostly sorted,
* behave optimally on append-heavy workloads.

In `flat_multimap`, this property is fundamental.

---

## Bulk Insertion: Append + `stable_sort` Is Optimal

The intended bulk workflow is:

```
append new elements at the end
stable_sort the entire sequence
```

Why this is fast:

* existing elements are already sorted,
* newly inserted elements form a small unsorted suffix,
* `stable_sort` naturally minimizes movement,
* no per-node allocation or rebalancing occurs.

This is not a workaround — it is the **fast path by design**.

---

## API Overview

### Lookup

```cpp
iterator find(const K& key);
const_iterator find(const K& key) const;

std::pair<iterator, iterator> equal_range(const K& key);
std::pair<const_iterator, const_iterator> equal_range(const K& key) const;
```

Semantics:

* `find(key)`

    * returns an iterator to the **first** element with the given key
    * returns `end()` if the key does not exist
    * equivalent to `equal_range(key).first` when the range is non-empty

* `equal_range(key)`

    * returns `{lower_bound(key), upper_bound(key)}`
    * the returned range is **half-open**
    * all elements in the range have keys equivalent to `key`
    * if no such key exists, both iterators equal `end()`

Complexity:

* all lookup operations are **non-mutating**
* complexity is `O(log N)` via binary search
* no iterator stability guarantees are provided

---

### Insertion — multimap semantics

Unlike `ordered_map`, **duplicate keys are always allowed**.
Insertion never fails due to key duplication.

---

#### `emplace`

```cpp
template<typename... Args>
iterator emplace(Args&&... args);
```

Semantics:

* constructs a temporary `std::pair<K, V>` from `args...`
* the key and mapped value are then **moved independently**
  into the container
* the new element is inserted **after all existing equivalents**
  of the same key
* preserves relative order among equivalent keys

Notes:

* this follows standard `emplace` intent, but
  **no in-place node construction occurs**
* all iterators are invalidated except the returned one

---

#### `insert` — tuple-like generalized insertion

```cpp
template<typename P>
requires jh::concepts::pair_like_for<P, K, V>
iterator insert(P&& p);
```

Semantics:

* accepts **any tuple-like value** `p` such that:

    * `get<0>(p)` is exactly `K` (after `remove_cvref`)
    * `get<1>(p)` is exactly `V` (after `remove_cvref`)

* the container **consumes the key and value separately**

* the element is inserted **after all existing equivalents**
  of the same key

Accepted examples:

```cpp
std::pair<K, V>
std::tuple<K, V>
structured-binding-compatible proxy types
reference-returning tuple adaptors
```

Rejected examples:

* mismatched element types
* implicit conversions
* tuple-like values with more or fewer than two elements

Notes:

* `value_type` (`std::pair<K, V>`) is **not required**
* this reflects the actual semantic model: *key + mapped value*
* insertion order among equivalent keys is preserved

For precise requirements, see [
`jh::concepts::pair_like_for`](../conceptual/tuple_like.md#-strict-pair-semantics-for-associative-containers).

---

### Erase

```cpp
iterator erase(iterator pos);
iterator erase(const_iterator pos);

iterator erase(iterator first, iterator last);
iterator erase(const_iterator first, const_iterator last);

size_t erase(const K& key);
```

Erase behavior:

* `erase(pos)`

    * removes the element at `pos`
    * returns the logical successor iterator

* `erase(first, last)`

    * removes the entire half-open range `[first, last)`
    * throws `std::logic_error` if `last` precedes `first`

* `erase(key)`

    * removes **all elements with the given key**
    * implemented as a **single contiguous range erase**
    * returns the number of elements removed

Iterator rules:

* any erase invalidates **all iterators**
* except the iterator explicitly returned by the call

---

### Capacity & lifecycle

```cpp
bool empty() const;
size_t size() const;

void clear() noexcept;
void reserve(size_t n);
void shrink_to_fit();
```

Semantics:

* `clear()`

    * resets the underlying vector
    * preserves capacity
    * effectively constant-time
    * particularly efficient under PMR allocators

* `reserve(n)`

    * requests capacity for at least `n` elements
    * does not change ordering or size

* `shrink_to_fit()`

    * non-binding request to reduce capacity
    * may or may not reallocate
    * iterator validity is preserved if no reallocation occurs

---

### Bulk insertion

```cpp
template<typename It>
void bulk_insert(It first, It last);
```

Semantics:

* appends all elements in `[first, last)` to the container
* then performs a **stable sort of the entire sequence**
* preserves relative order among equivalent keys
* invalidates all iterators

Design intent:

* this is the **preferred construction path**
  for large datasets
* optimized for append-heavy and rebuild-oriented workflows
* explicitly trades incremental cost for predictable bulk behavior

---

### Summary of insertion semantics

| Operation     | Allows duplicates | Preserves order | Notes                     |
|---------------|-------------------|-----------------|---------------------------|
| `insert`      | yes               | yes             | tuple-like, generalized   |
| `emplace`     | yes               | yes             | constructs temporary pair |
| `bulk_insert` | yes               | yes             | append + stable rebuild   |

`flat_multimap` never rejects insertion based on key existence.
All multimap semantics are expressed through **range structure**, not key uniqueness.

---

## Why Not Extend [`ordered_map`](ordered_map.md)?

Multimap workloads are **structurally range-oriented**:

* process all values for a key,
* erase all values for a key at once,
* scan contiguous groups.

In a balanced tree (including contiguous AVL trees):

* range deletion becomes repeated node erasure,
* each erase may trigger rotations,
* cost and latency become unpredictable.

For this reason, the `ordered_*` family deliberately avoids:

* multimap semantics,
* bulk key erasure,
* range-oriented deletion APIs.

Attempting to support these would compromise their core invariants.

---

## Flat Multimap: Algorithm Containerization

`flat_multimap` takes the opposite approach:

> **Turn the "sorted vector + stable binary search" algorithm into a container with explicit semantics.**

It provides:

* guaranteed contiguity of equivalent keys,
* transparent range-based operations,
* predictable cost models based on bulk movement,
* no hidden structural side effects.

The container does not pretend to be something else.

---

## Iterator Semantics

* All insertions and erasures may invalidate iterators.
* No pointer or node identity is preserved.
* Elements may be freely relocated.

This is a deliberate tradeoff in favor of:

* contiguous storage,
* maximal cache locality,
* predictable long-term behavior.

---

## Complexity Summary

| Operation     | Complexity                                  | Notes                    |
|---------------|---------------------------------------------|--------------------------|
| `find`        | `O(log N)`                                  | binary search            |
| `equal_range` | `O(log N)`                                  | two binary searches      |
| `insert`      | `O(N)`                                      | contiguous shift         |
| `erase(key)`  | `O(N)`                                      | single range compaction  |
| iteration     | `O(N)`                                      | sequential memory access |
| `clear()`     | ~`O(1)`                                     | vector reset             |
| `bulk_insert` | theoretically `O(N log N)` but actually low | append + `stable_sort`   |

All costs are explicit and deterministic.

> **Notes:**
>
> Although some operations are expressed as `O(N)` in Big-O terms, the underlying cost model is dominated by
> contiguous memory movement rather than pointer traversal or dynamic allocation.
>
> In practice, these operations compile down to linear `memmove` / `memcpy`-style transfers that are highly
> optimized by modern CPUs, benefiting from cache-line locality, hardware prefetching, and SIMD acceleration.
>
> As a result, the constant factors are extremely small and the observed runtime is often lower than that of
> theoretically cheaper `O(log N)` tree-based containers whose operations involve cache misses, branch
> mispredictions, and allocator interaction.
>
> Big-O here describes asymptotic growth, not the real-world cost of the executed instructions.

---

## When You Should Use It

### Recommended

* large in-memory tables
* indexing and routing layers
* subscription registries
* batch-oriented workflows
* scan-heavy, read-dominated workloads
* systems sensitive to cache locality and allocator stability

### Not Recommended

* frequent random single-element insert/erase
* workloads requiring stable iterators
* very small datasets
* cases where multimap behavior is incidental rather than central

---

## Why There Is No `flat_multiset`

A multiset adds no meaningful structure here:

* a sorted sequence of duplicate keys is already fully expressible,
* `std::vector<K> + stable_sort` suffices,
* no key–value association or range deletion semantics exist to formalize.

`flat_multimap` exists because **multimap semantics are inherently range-based**.
A multiset is not.

---

## Performance Reality

Empirical benchmarks consistently show:

* random insertion favors tree-based multimaps,
* ordered or bulk construction strongly favors `flat_multimap`,
* sequential iteration shows **order-of-magnitude improvements**,
* lookup converges toward tree performance as datasets grow and cache effects dominate.

These results are **memory-layout driven**, not compiler-dependent.

---

## Design Summary

`jh::flat_multimap` follows the same engineering philosophy as the `ordered_*` family:

* prioritize memory layout over pointer stability,
* favor cache locality over per-operation asymptotics,
* embrace bulk rebuilds as first-class workflows,
* ensure behavior does not degrade over time.

But it applies this philosophy to a **different problem domain**:

> **Multimap semantics are range semantics.**

---

## Final Notes

`jh::flat_multimap` is:

* not a drop-in replacement for `std::multimap`,
* not a general-purpose associative container,
* a purpose-built, locality-optimized structure.

If your workload is **batch-oriented, range-centric, and cache-sensitive**,
this container expresses that reality directly — and efficiently.
