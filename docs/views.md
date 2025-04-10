# 🧩 JH Toolkit: `views` Module API Documentation

📌 **Module:** `jh::views`  
🕝 **Version:** 1.3  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  
🔧 **Depends on:** `jh::sequence`, `jh::make_pair`, C++20 `std::views`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## 🔭 Overview

The `jh::views` module provides **lazy, composable iteration adapters** over any type that satisfies the `jh::sequence` concept.  
These views emulate Python-like constructs such as `enumerate` and `zip`, without relying on STL `std::ranges`.

> 🧠 Designed for **read-only, allocation-free iteration**, with **zero inheritance**, **POD-safe dispatch**, and **drop-in structured bindings**.

---

### 📦 Header Inclusion

```c++
#include <jh/view>     // ✅ Recommended aggregated include
#include <jh/view.h>   // ✅ Legacy-compatible form
```

This expands to:

```c++
#include "jh/views/zip.h"
#include "jh/views/enumerate.h"
```

> You may also include components directly when needed.

---

## 🧠 Design Philosophy

- ✅ Duck-typed iteration via `jh::sequence`
- ✅ POD-friendly pairing and structured binding
- ❌ No dependency on `std::ranges::range`, CRTP, or STL view inheritance
- ✅ Lazy evaluation with zero allocations
- ✅ Seamless integration with STL containers and POD arrays

---

## 🔎 Key Features

| View                        | Output                       | Description                                      |
|-----------------------------|------------------------------|--------------------------------------------------|
| `jh::views::zip(a, b)`      | `(lhs, rhs)`                 | Zips two sequences; stops at shorter             |
| `jh::views::enumerate(seq)` | `(index, value)`             | Pairs element index with value, like Python      |

All pair-like values are returned via `jh::make_pair(...)`:

- ✅ `pod::pair<T1, T2>` for POD types — trivially copyable
- ✅ `ref_pair<T1&, T2&>` otherwise — safe for non-trivial types
- ⚠️ Return type is opaque — always use `const auto& [a, b] = ...`

---

## ⚙️ Input Model: `jh::sequence`

```c++
template<typename T>
concept sequence = requires(const T t) {
    { t.begin() } -> input_iterator;
    { t.end() }   -> input_iterator;
};
```

> No inheritance, tag traits, or STL view modeling required.

---

## 🔍 `enumerate(seq)`

### 📋 Signature

```c++
template<sequence R>
auto jh::views::enumerate(const R& r);
```

### 🎯 Behavior

Returns a lazily evaluated view of `(index, value)` pairs from the sequence.  
Internally maintains `std::uint64_t` index and input iterator state.

```c++
iterator {
    uint64_t index;
    It current;
    operator*() => make_pair(index, *current);
};
```

- ✅ POD values copied
- ✅ Non-POD values referenced
- ✅ Safe with STL containers, raw arrays, and fixed sequences

### Example

```c++
for (const auto& [i, v] : jh::views::enumerate(vec)) {
    std::cout << i << ": " << v << '\n';
}
```

---

## 🔍 `zip(a, b)`

### 📋 Signature

```c++
template<sequence R1, sequence R2>
auto jh::views::zip(const R1& a, const R2& b);
```

### 🎯 Behavior

Zips two sequences into a single lazy view of `(lhs, rhs)` pairs.  
The output length is truncated to the shorter sequence.

```c++
iterator {
    It1 it1;
    It2 it2;
    operator*() => make_pair(*it1, *it2);
};
```

- ✅ POD types returned by value
- ✅ Others returned by reference
- ❌ No copying of containers or buffers

### Example

```c++
for (const auto& [a, b] : jh::views::zip(nums, words)) {
    std::cout << a << " = " << b << '\n';
}
```

---

## 🪜 Guarantees for All Views

| Property                         | Guarantee |
|----------------------------------|-----------|
| Accepts `jh::sequence`           | ✅         |
| Supports STL + POD arrays        | ✅         |
| No heap allocation               | ✅         |
| Safe structured binding          | ✅         |
| POD-awareness (val/ref dispatch) | ✅         |
| Compatible with `auto&& [a, b]`  | ✅         |
| Pure C++20                       | ✅         |

---

## 🔧 Example

```c++
jh::pod::array<int, 3> data = {1, 2, 3};

for (const auto& [i, val] : jh::views::enumerate(data)) {
    std::cout << "[" << i << "] = " << val << '\n';
}

std::vector<std::string> labels = {"one", "two", "three"};
for (const auto& [n, word] : jh::views::zip(data, labels)) {
    std::cout << word << ": " << n << '\n';
}
```

---

## 📄 See Also

- [`sequence.md`](sequence.md) — Defines `jh::sequence` concept
- [`pod.md`](pod.md) — POD-compatible containers and helpers

---

> 💡 Write views as if `std::ranges` never existed.  
> 🧩 Just give me `begin()` and `end()`, and I’ll do the rest.
