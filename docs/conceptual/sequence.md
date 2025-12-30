# 🧩 **JH Toolkit — `jh::concepts::sequence` API Reference**

📁 **Header:** `<jh/conceptual/sequence.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

### 🧭 Submodule Notice

* `jh::concepts::sequence` is a **behavioral submodule** under `jh::concepts`,
  providing **duck-typed sequence recognition** for any object that can be traversed via `begin()` / `end()`.
* It defines the canonical behavioral model leveraged by `jh::ranges` and `jh::views`
  for transparent and automatic range adaptation.

---

## 🧩 Introduction

This header defines the **semantic concept of a "sequence"** within the JH Toolkit —
a type that can be traversed through `begin()` and `end()` in a stable, repeatable manner,
without assuming it must behave like a full `std::ranges::range`.

This is a form of tolerance for third-party or user-defined containers. 
As long as you appear to be a container that permits non-exhaustive traversal, we can promote you to a `range`.

Unlike the standard `range` concept,  
`jh::concepts::sequence` models the *everyday programming notion* of "something iterable".
It focuses on the **behavior of traversal**, not the presence of formal range traits.

This concept treats any object that supports consistent iteration as a sequence —
STL containers, raw arrays, pointers, or custom types —
and normalizes them into a range-compatible form through
[`jh::ranges::range_adaptor`](../ranges/range_adaptor.md).

The const-qualified check ensures that `begin()` and `end()` are callable on a `const T&`,
meaning iteration itself does not consume or mutate the underlying object,
while still allowing writable access through iterators when appropriate.

---

## 🔹 Core Concepts

### `sequence<T>`

Defines what it means for a type to behave like a *sequence* —
a structure that can be traversed predictably and repeatedly using `begin()` and `end()`.

Requires:

* `std::begin(t)` and `std::end(t)` valid for `const T&`;
* returned iterators satisfy `jh::concepts::input_iterator`;
* `{ std::begin(t) == std::end(t) }` and `{ std::begin(t) != std::end(t) }`
  are valid and convertible to `bool`.

This concept ensures traversal stability without requiring the object
to implement full range semantics or rvalue-based iteration.
Sequences are typically *lvalue-bound* — persistent containers or views that
can be iterated multiple times without consuming state.

The const-qualified validation simply guarantees that
`begin()` and `end()` can be invoked safely on const objects,
signifying non-destructive iteration.
Writable iterators remain allowed as long as iteration itself
does not break traversal consistency.

`jh::concepts::sequence` thus captures the **programmer's intuitive definition**
of a sequence — "something you can iterate over and expect consistent results" —
serving as the foundation for toolkit-wide iteration behavior.

---

### `sequence_value_t<T>`

Deduces the element type of a sequence.  
Requires:

* `iterator_t<T>` must be deducible via the iterator resolution chain;
* the deduced iterator must provide a valid `iterator_value_t<I>`.

Removes `cv` and reference qualifiers, unifying value deduction
for both STL-style and duck-typed containers.

---

### `is_sequence<T>`

Compile-time boolean trait testing whether a type satisfies `sequence<T>`.  
Requires:

* `sequence<T>` must be satisfied.

Provides a `constexpr bool` shorthand for SFINAE or static assertions.

---

### `jh::to_range(seq)`

Transforms any valid sequence into a `std::ranges::range`-compatible object.
Requires:

* The input type satisfies `jh::concepts::sequence`.

Behavior:

* If the input already models `std::ranges::range`:

    * returns it directly if movable or copyable;
    * otherwise wraps it with `std::ranges::subrange` for const-lvalue access.
* If the input does *not* model `std::ranges::range`:

    * wraps lvalues using `jh::ranges::range_adaptor<Seq&>`;
    * wraps rvalues or values using `jh::ranges::range_adaptor<Seq>`.

The resulting object is always **movable and forwardable**, even when the original sequence is not.
Some STL range algorithms perform overly strict move/copy checks —
**even when the algorithm itself never actually moves or copies the range**.
`jh::to_range()` guarantees such checks will pass by producing a proxy object that
behaves as a movable `std::ranges::range` through `range_adaptor` or `subrange`.

This adaptation is **idempotent for all valid inputs**, following four deterministic paths:

1. **`std::ranges::range` + movable/copyable** → returned directly (transparent pass-through).
2. **`std::ranges::range` + not movable/copyable** → wrapped by `std::ranges::subrange`.
3. **`sequence` lvalue** → wrapped by `jh::ranges::range_adaptor<Seq&>`.
4. **`sequence` rvalue/value** → wrapped by `jh::ranges::range_adaptor<Seq>`.

The outputs of cases 2–4 all satisfy case 1,
so any subsequent call to `jh::to_range()` simply forwards the object unchanged.
This is why the operation is *natively idempotent*—
once an object has been adapted, further adaptation is a no-op.

In practice, this requires that the input must satisfy `jh::concepts::sequence`, 
meaning it must be iterable and iteration itself must not consume the container.
This guarantees that the output must be a `std::ranges::range`. 
If the input is a lvalue, the output must be compatible with `std::views::all`.

See [`range_adaptor`](../ranges/range_adaptor.md) for implementation details.

---

### `sequence_difference_t<T>`

Behavioral equivalent of `std::ranges::range_difference_t`.  
Requires:

* `T` satisfies `jh::concepts::sequence`.

Deduces the **difference type** of the adapted range returned by `jh::to_range(T)`.
Unlike `iterator_difference_t<I>`, which may yield `void` for non-arithmetic iterators,
`sequence_difference_t<T>` is **always valid and non-void** —
`to_range()` guarantees a proper `std::ranges::range`,
defaulting to `std::ptrdiff_t` when the underlying type does not define its own difference type.

---

## 🧩 Summary

- `jh::concepts::sequence` defines the **behavioral recognition layer**
for objects that can be traversed consistently, without imposing strict STL `range` semantics.

- It formalizes the programmer's intuitive sense of "iterable" types,
and acts as the semantic foundation for `jh::to_range()`,
`jh::ranges::range_adaptor`, and `jh::views` throughout the toolkit.

## 📦 **Forwarding Header — `jh/sequence` (1.3.x only)**

Backward-compatible entry maintained through **1.3.x**.  
It re-exports `jh::concepts::sequence` and related utilities to the root namespace,
but **does not affect** `jh::to_range`, which is a common infrastructure utility
used across multiple modules (e.g. `sequence`, `generator`).

```cpp
#pragma once
#include "jh/conceptual/sequence.h"

namespace jh {
    using namespace jh::concepts;
}
```

This header will also be **removed in 1.4.0**.  
Use `jh::concepts::sequence` directly, and keep `jh::to_range`
as a shared top-level function.