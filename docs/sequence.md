# 🧩 JH Toolkit: `sequence` Concept API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## 🧭 Overview

The `jh::sequence` module defines a **minimal C++20 concept** for validating immutable iteration.  
It identifies types that support **read-only forward iteration** and enables them to seamlessly integrate into **generic pipelines**, including `jh::view`.

---

### ✅ Highlights

- 🧩 `sequence<T>`: Concept for immutable, input-iterator-based iteration
- 🔍 `sequence_value_type<T>`: Deduces value type from a sequence
- 🧠 `is_sequence<T>`: Boolean check for the `sequence` concept
- 🔄 `to_range(const T&)`: Converts any sequence to a standard-compliant range
- ⚙️ Looser constraints than `std::ranges::range`, optimized for lightweight structures
- 🚀 Powers the foundation of `jh::view` — supports more than just STL containers

---

## 🔍 Concept: `jh::sequence<T>`

```c++
template<typename T>
concept sequence = requires(const T t) {
    { t.begin() } -> jh::input_iterator;
    { t.end() }   -> jh::input_iterator;
};
```

### 📌 Description

A type satisfies `jh::sequence<T>` if it has a `const begin()` and `const end()` method, both returning **input iterators**.

---

### 🧠 Design Intent

- ✅ Simpler than `std::ranges::range`
- ✅ Works with raw pointers, POD containers, or types without `view_interface`
- ❌ Excludes consumable or non-const iterables (e.g., `generator<T>`)
- ❌ Excludes structures with non-iterator end types (e.g., `default_sentinel_t`)

> `sequence` is the **core concept powering `jh::view`**, enabling maximum flexibility with minimal requirements.

---

### ✅ Example

```c++
static_assert(jh::sequence<std::vector<int>>);
static_assert(jh::sequence<std::array<float, 4>>);
static_assert(!jh::sequence<jh::generator<int>>);           // ❌ not const-safe
static_assert(!jh::sequence<jh::generator_range<int>>);     // ❌ uses sentinel end
```

---

## 🧬 Type Extraction: `sequence_value_type<T>`

```c++
template<typename T>
using sequence_value_type = typename detail::sequence_value_type_impl<T>::type;
```

Extracts the **value type** of a valid sequence based on its iterator's `value_type`.

> Implemented internally as `detail::sequence_value_type_impl` to avoid unnecessary IDE suggestions.

### 🔎 Example

```c++
using T = jh::sequence_value_type<std::list<double>>;
static_assert(std::is_same_v<T, double>);
```

---

## 🧠 Boolean Trait: `is_sequence<T>`

```c++
template<typename T>
constexpr bool is_sequence = sequence<T>;
```

Provides a constexpr trait for use in SFINAE and meta-checks.

---

## 🔄 Adapter: `to_range(const Seq&)`

```c++
template<sequence Seq>
auto to_range(const Seq &s);
```

### 📌 Description

Creates a `std::ranges::subrange` from any valid `jh::sequence`.  
This allows non-view, non-`ranges::range` containers to be used in standard or JH-style view pipelines.

---

### 🧩 Why It Matters

- ✅ **Zero inheritance** — your type doesn't need to model `view_interface`
- ✅ ✅ Accepts **all types satisfying `jh::sequence`**
- ✅ Seamlessly works with standard algorithms and `jh::view`
- ❌ Not meant for `generator<T>` or types that don’t support `const begin()/end()`

---

### ✅ Example

```c++
jh::pod::array<int, 3> vec = {1, 2, 3}; // sequence, not a range
auto range_ = jh::to_range(vec);

std::ranges::for_each(range_, [&](const int a) {
                std::cout << a << " ";
            });
```

Output:
```
1 2 3
```

---

### 🧠 Notes on Overloads

> 🔀 Other modules may define **their own `to_range(...)`** overloads with different parameters  
> (e.g., coroutine factories, adapters, sendable streams).  
> These are **semantically distinct** but share the naming goal: produce a usable range.

| Function Signature            | Intent                             |
|-------------------------------|------------------------------------|
| `to_range(const Seq&)`        | For any valid `jh::sequence`       |
| `to_range(generator_factory)` | For coroutine-based `generator<T>` |
| `to_range(stream_adapter)`    | For custom stream or event sources |

---

## 🚫 What Doesn’t Qualify as a `sequence`

| Type                       | `sequence`? | Reason                                            |
|----------------------------|-------------|---------------------------------------------------|
| `jh::generator<T>`         | ❌           | Requires mutation; not const-safe                 |
| `jh::generator_range<T>`   | ❌           | Uses `default_sentinel_t` as end; not an iterator |
| `std::istream_iterator<T>` | ❌           | Not const-repeatable                              |

---

## 📚 Use with `jh::view`

The `sequence` concept was **intentionally decoupled** from `std::ranges::range`, to support:

- Lightweight containers (even C-style structs defining a raw array with `begin()`/`end()`)
- Non-CRTP types that don’t or can’t model `view_interface`
- Broader compatibility in range combinators like `zip`, `enumerate`, `take`, etc.

> `jh::view` is built on top of `sequence`, **not `std::ranges::range`**, for this reason.

---

## ✅ Summary

| Feature                  | Description                                                      |
|--------------------------|------------------------------------------------------------------|
| `sequence<T>`            | Checks if a type can be read-only iterated via `begin()`/`end()` |
| `sequence_value_type<T>` | Gets the `value_type` of a sequence                              |
| `is_sequence<T>`         | Boolean version of the concept                                   |
| `to_range(const Seq&)`   | Turns a sequence into a reusable range                           |
| 🤝 Works with `jh::view` | Enables full compatibility without needing `ranges::range`       |

---

## 🚀 Final Thoughts

`jh::sequence` provides the **simplest possible abstraction** for safely modeling readable iteration.  
It removes the overhead of modeling complex range traits, while still enabling **type-safe**, **zero-cost**, and **composable iteration**.

📘 For more on how `sequence` powers views and pipelines, see [`views.md`](views.md).  
📄 Implementation details are located in `sequence.h`.
