# 🧩 JH Toolkit: `sequence` Concept API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## 🧭 Overview

The `jh::sequence` module defines a **lightweight C++20 concept** that recognizes types supporting **immutable, repeatable forward iteration**.  
Unlike `std::ranges::range`, it does not require `sentinel_for`, inheritance from `view_interface`, or CRTP.  
Instead, it focuses on what matters most for *read-only iteration*.

---

## ✅ What `sequence` Checks

```c++
template<typename T>
concept sequence = requires(const T t) {
    { t.begin() } -> jh::input_iterator;
    { t.end() };
    { t.begin() == t.end() } -> std::convertible_to<bool>;
    { t.begin() != t.end() } -> std::convertible_to<bool>;
};
```

A type `T` satisfies `jh::sequence<T>` if:

- `const T` has `.begin()` returning an **input iterator**
- `const T` has `.end()` (of any type)
- `begin()` can be compared to `end()` via `==` / `!=`

> 🔍 `end()` does **not** need to be the same type as `begin()` — only **comparable** to it.  
> This enables support for **custom sentinels** (like `std::default_sentinel_t`).

---

## ⚙️ Design Rationale

| Feature                         | Purpose                                                 |
|---------------------------------|---------------------------------------------------------|
| ✅ Const-based iteration         | Guarantees immutability and repeatability               |
| ✅ Duck-typed end comparison     | More permissive than `std::sentinel_for`                |
| ✅ `input_iterator` enforcement  | Prevents write-only, move-only, or non-repeatable types |
| ❌ No output/mutable/consuming   | Excludes `generator<T>`, `istream_iterator<T>`, etc.    |
| ❌ No `range` inheritance needed | POD structs and raw types can qualify                   |

---

## 🔍 Example Usage

```c++
static_assert(jh::sequence<std::vector<int>>);
static_assert(jh::sequence<std::array<float, 4>>);

// Supports POD container
struct CustomArray {
    int data[3]{1, 2, 3};
    const int* begin() const { return data; }
    const int* end() const { return data + 3; }
};
static_assert(jh::sequence<CustomArray>);

// ❌ Fails: uses sentinel end
static_assert(!jh::sequence<jh::generator_range<int>>);
```

---

## 🧬 Extracting Value Type

```c++
template<typename T>
using sequence_value_type = /* deduced from iterator_t<T> */;
```

Determines the element type by inspecting the `begin()` iterator:

```c++
using A = jh::sequence_value_type<std::list<double>>;  // double
using B = jh::sequence_value_type<std::map<int, std::string>>;  // std::pair<const int, std::string>
```

> Internally resolved via `iterator_traits<iterator_t<T>>::value_type`.

---

## 🧠 Checking at Compile Time

```c++
template<typename T>
constexpr bool is_sequence = sequence<T>;
```

Allows use in static asserts, templates, or SFINAE:

```c++
static_assert(jh::is_sequence<std::vector<int>>);
static_assert(!jh::is_sequence<std::istream_iterator<int>>);
```

---

## 🔄 Converting to Standard Ranges

```c++
template<sequence Seq>
auto to_range(const Seq& s);
```

Wraps any sequence in a `std::ranges::subrange`:

```c++
jh::pod::array<int, 3> vec = {1, 2, 3};
auto range = jh::to_range(vec);

std::ranges::for_each(range, [](int x) {
    std::cout << x << " ";
});
```

> ✅ Works with types that don't inherit from `view_interface`.  
> ❌ Fails on one-shot or non-repeatable views.

---

## 📎 Integration with `jh::iterator_t<T>`

All `sequence` checks rely on `jh::iterator_t<T>`, which can:

- Use `jh::iterator<T>::type` if specialized
- Fall back to `.begin()` return type
- Support raw arrays (`T[]`) and pointer types

This allows `sequence` to support nearly anything usable in a `for (auto : x)` loop.

---

## 🚫 Not a Sequence If...

| Type                       | ❌ Why not                                      |
|----------------------------|------------------------------------------------|
| `jh::generator<T>`         | Not repeatable, requires mutation              |
| `std::istream_iterator<T>` | Non-repeatable, move-only input                |
| `mutable_vector_view<T>`   | No `const begin()` or end()                    |

---

## 🧰 Related Modules

| Module                   | Description                                         |
|--------------------------|-----------------------------------------------------|
| `jh::view`               | Range combinators built on top of `sequence`        |
| `jh::pod::array<T>`      | POD-based static array compatible with `sequence`   |
| `jh::generator_range<T>` | Repeatable lazy generator — not a sequence          |
| `jh::to_range(...)`      | Generalized overloads for stream/generator adapters |

---

## 📘 Compared to `std::ranges::range`

| Feature                        | `std::ranges::range` | `jh::sequence`                 |
|--------------------------------|----------------------|--------------------------------|
| Requires `sentinel_for`        | ✅ Yes                | ❌ No                           |
| `begin()` on `const T`         | Optional             | ✅ Required                     |
| Supports `default_sentinel`    | ✅ But strict         | ✅ If `==` works with `begin()` |
| Inherits from `view_interface` | Often necessary      | ❌ Never required               |
| POD-friendly                   | ❌ Not easily         | ✅ Fully supported              |

---

## ✅ Summary

| Feature                     | Description                                    |
|-----------------------------|------------------------------------------------|
| `sequence<T>`               | Minimal concept for const-safe input iteration |
| `sequence_value_type<T>`    | Deduces element type via `iterator_t`          |
| `is_sequence<T>`            | Boolean trait for SFINAE/meta-programming      |
| `to_range(const T&)`        | Converts sequence to standard `subrange`       |
| Supports raw & custom types | Works with PODs, raw arrays, custom iterables  |

---

## 🚀 Philosophy: Duck-Typed, Zero-Overhead

> “If it quacks like a const-safe iterable, it sequences like one.”

The `jh::sequence` concept avoids the verbosity of C++ concepts like `ranges::range`,
while embracing composable, predictable, low-overhead iteration — even in embedded or performance-critical codebases.

📄 See `sequence.h` for implementation  
📘 See [`views.md`](views.md) for how `sequence` powers the JH view system.
