# 🧩 **JH Toolkit — `jh::concepts::iterator` API Reference**

📁 **Header:** `<jh/conceptual/iterator.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

> **Note:**  
> `jh::concepts::iterator` is the name of this submodule, not a type.  
> The only same-named symbol defined in this header is  
> `jh::iterator<Container>`, a **forward declaration** in namespace `jh`.

---

## 🧭 Introduction

This header defines the unified, duck-typed iterator model of the JH Toolkit.  
Unlike STL iterators, which depend on `std::iterator_traits` and strict inheritance,
these iterators are **recognized entirely by behavior** —  
a type qualifies if it supports readable or writable iteration expressions,
even if it lacks typedefs like `iterator_category`.

The system detects **semantic capability**, not declared conformance.  
All checks are static (compile-time), and any conflict in declared semantics
(e.g., invalid operator-, mismatch between `difference_type` and arithmetic)  
will result in a **hard failure** via `static_assert`.

---


## 🧩 `jh::iterator<Container>`

### 📦 Namespace: `jh`

`jh::iterator<Container>` is a **forward-declared customization point**  
used by `jh::concepts::iterator_t` to resolve the iterator type of a container.

It allows both intrusive and non-intrusive iterator definitions,  
and serves as the **highest-priority deduction source** in the entire iterator resolution chain.

```cpp
template<typename Container>
struct iterator;
```

### Behavior

If `jh::iterator<Container>::type` is defined,
it takes **absolute priority** and overrides all other deduction paths.
This is how you explicitly bind an iterator type to a container.

If it is **not defined**, deduction proceeds automatically in the following order:

1. `decltype(container.begin())` — uses the return type of `begin()`
2. if `Container` is an array — deduces to `T*`
3. if `Container` is already a pointer — keeps it as-is
4. otherwise — deduction fails

### Usage

* You can **implement and bind an iterator directly inside a specialization**,
  defined within the `jh` namespace — this pattern is analogous to
  how the C++ standard library exposes open template specializations:

  ```cpp
  namespace jh {
      template<>
      struct iterator<my_container> {
          struct iterator {
              /* iterator implementation */
          };
          using type = iterator;  // bind this specialization as the iterator type
      };
  }
  ```

* If the container already provides an iterator or belongs to a **non-standard or third-party library**,
  and automatic deduction fails because its `begin()` or `end()`
  do not follow standard semantics,
  you can also define a specialization in the same way to **manually bind**
  an existing iterator type:

  ```cpp
  namespace jh {
      template<>
      struct iterator<third_party_container> {
          using type = third_party_iterator;
      };
  }
  ```

### Deduction Priority

`jh::iterator<Container>::type` has the **highest priority** in all iterator resolution.
When present, it fully overrides automatic deduction via `begin()`/`end()`,
array, or pointer inference.

This mechanism allows both explicit iterator definitions
and non-intrusive bindings for third-party containers,
making them recognized by `jh::concepts::iterator_t`
and compatible with `jh::concepts::sequence`
and the toolkit's range/view infrastructure.

---

## 🔹 Behavioral Traits

| Trait                                       | Description                                                                                  |
|---------------------------------------------|----------------------------------------------------------------------------------------------|
| `iterator_value_t<I>`                       | Deduces element value type from `*it`, `I::value_type`, or pointer decay.                    |
| `iterator_reference_t<I>`                   | Deduces reference type; verifies consistency between declared and actual dereference result. |
| `iterator_rvalue_reference_t<I>`            | Deduce rvalue reference via ADL `iter_move`, `I::iter_move()`, or `std::move(*it)`.          |
| `iterator_difference_t<I>`                  | Deduces distance type from valid subtraction or declared `difference_type`.                  |
| `has_value_type<T>` / `has_value_type_v<T>` | Detects presence of `T::value_type`. Used internally for iterator and sequence traits.       |

All traits verify **behavioral coherence** —
for instance, if both `reference` and `operator*()` exist, they must be convertible to each other.

---

## 🔹 Core Concepts

### `is_iterator<I>`

Behavioral equivalent of `std::input_or_output_iterator`.  
Requires:
- Valid expressions: `*it`, `++it`, `it++`.
- If `difference_type` is defined, it must be a **signed integral type**.
- If both `difference_type` and `operator-` are defined,  
  the result of `a - b` must be **convertible to** `difference_type`.
- Declared `difference_type` must not conflict with arithmetic semantics.  
  Rejects inconsistent or ambiguous iterator arithmetic definitions.

---

### `indirectly_readable<I>`

Behavioral equivalent of `std::indirectly_readable`.  
Requires:
- `iterator_value_t<I>`, `iterator_reference_t<I>`, and `iterator_rvalue_reference_t<I>`  
  are all well-defined and non-void.
- Expression `*it` is valid and convertible to `iterator_reference_t<I>`.
- ADL `iter_move(it)` is valid.
- Both reference and rvalue reference types must be convertible  
  to `iterator_value_t<I>`.  
  Checks ensure dereference consistency and read stability.

---

### `indirectly_writable<Out, T>`

Behavioral equivalent of `std::indirectly_writable`.  
Requires:
- Assignable expressions through dereference:
    - `*o = value`,
    - `*std::forward<Out>(o) = value`.
- Const-qualified rvalue dereference assignment is also valid:
    - `const_cast<const iterator_reference_t<Out>&&>(*o) = value`.  
      Ensures the iterator supports writing values through `*it`.

---

### `sentinel_for<S, I>`

Behavioral equivalent of `std::sentinel_for` (relaxed).  
Requires:
- Mutual equality and inequality comparability:
    - `{ i == s }`, `{ s == i }`, `{ i != s }`, `{ s != i }`.
- All results convertible to `bool`.  
  Does **not** require special construction or range-category relationships.  
  Designed for lightweight duck-typed range boundaries.

---

### `input_iterator<I, S = I>`

Behavioral equivalent of `std::input_iterator`.  
Requires:
- Satisfies `is_iterator<I>` and `indirectly_readable<I>`.
- Compatible sentinel type (`sentinel_for<S, I>`).
- Valid expressions:
    - `++it` returns `I&`,
    - `it++` is convertible to `I`.  
      Represents **readable**, single-pass sequential traversal.

---

### `output_iterator<I, T = iterator_value_t<I>>`

Behavioral equivalent of `std::output_iterator`.  
Requires:
- Satisfies `is_iterator<I>` and `indirectly_writable<I, T>`.
- Valid expression:
    - `*it++ = value`.  
      Represents **writable**, single-pass sequential traversal.

---

### `forward_iterator<I, S = I>`

Behavioral equivalent of `std::forward_iterator`.  
Requires:
- Satisfies `input_iterator<I, S>`.
- `std::copyable<I>`.
- Self-sentinel compatibility (`sentinel_for<I, I>`).
- Valid expressions:
    - `++it` returns `I&`,
    - `it++` returns `I`,
    - `*it` is exactly `iterator_reference_t<I>`.  
      Represents **multi-pass**, stable, reentrant traversal.

---

### `bidirectional_iterator<I, S = I>`

Behavioral equivalent of `std::bidirectional_iterator`.  
Requires:
- Satisfies `forward_iterator<I, S>`.
- Valid reverse operations:
    - `--it` returns `I&`,
    - `it--` convertible to `I`.  
      Represents **reversible** traversal in both directions.

---

### `random_access_iterator<I, S = I>`

Behavioral equivalent of `std::random_access_iterator`.  
Requires:
- Satisfies `bidirectional_iterator<I, S>`.
- Full relational comparison support:
    - `<`, `>`, `<=`, `>=` (all convertible to `bool`).
- Valid `iterator_difference_t<I>` (non-void).
- Arithmetic and indexing operations:
    - `i += n`, `i -= n`,
    - `j + n`, `n + j`, `j - n`,
    - `j[n]` returns `iterator_reference_t<I>`.  
      Represents **constant-time positional access** and offset iteration.

## 🔹 Type Deduction Utility

### `iterator_t<Container>`

Meta-type alias deducing an iterator type from any iterable source.

Resolution order:

1. `jh::iterator<Container>::type` if exists and satisfies `is_iterator`;
2. `decltype(Container::begin())` if valid and satisfies `is_iterator`;
3. pointer type `T*` for raw pointer inputs;
4. decayed pointer for arrays `T[N]` or `T[]`.

```cpp
template<typename Container>
using iterator_t = typename detail::iterator_resolver<Container>::type;
```

This mechanism ensures **non-intrusive** interoperability for third-party types,
bridging them into `jh::concepts::sequence` and the range system.

---

## 🧩 Conflict Policy

Although the detection is behavior-based and permissive,
the model enforces **semantic exclusivity**:

* Inconsistent `difference_type` or invalid `operator-` → compile-time failure.
* Mismatched dereference/reference type → compile-time failure.
* Ambiguous `iter_move()` resolution → compile-time failure.

Every valid `jh::concepts::*iterator` guarantees **non-contradictory** semantics.

---

## 📦 **Forwarding Header — `jh/iterator` (1.3.x only)**

Provided for backward compatibility during the **1.3.x** series.  
This header includes the conceptual definition and temporarily
re-exports iterator-related symbols into the root namespace:

```cpp
#pragma once
#include "jh/conceptual/iterator.h"

namespace jh {
    using namespace jh::concepts;
}
```

This allows using `jh::iterator_t<T>` and related traits without
`jh::concepts::` qualification, at the cost of namespace bloat.  
The header will be **removed in 1.4.0** —  
include `<jh/conceptual/iterator.h>` or the aggregated `<jh/concepts>` instead.
