# 📦 JH Toolkit: `runtime_arr` API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## ✨ Overview

`jh::runtime_arr<T, Alloc>` is a **fixed-size**, **move-only**, **allocation-aware** container designed for use cases where **bounded memory**, **fast initialization**, and **high data locality** are paramount.

Unlike `std::vector`, it offers no resizing or insertion. Instead, it delivers:

- Optional uninitialized construction for **POD types**
- **STL-compatible iteration**, via pointers or C++20 ranges
- Efficient `reset_all()` with POD optimizations
- Bit-packed specialization for `bool` (like a custom `bitset`, *only if Alloc = std::monostate*)

---

## ⚡ Key Features

- ✅ Fixed-size allocation; no `resize()`/`insert()` semantics
- 🔄 Move-only design, no accidental copies
- 🖇️ Supports allocators or manual memory
- 📁 `reset_all()` for fast zero/init
- 🔀 STL-style iteration (`begin/end`)
- 🧱 Specialized packed-bits backend for `runtime_arr<bool>` *(only with default allocator)*

---

## 🔍 Use Cases

- Simulation pipelines / per-frame buffers
- Intermediate results for sorting, DP, or filters
- Flat storage for vectorized or SIMD data
- Lightweight allocator-bounded memory regions
- Bit flags, masks, and compact logical vectors
- Custom interface return buffers / scratchpads

---

## 💡 Design Intent

| Behavior                | `std::vector<T>` | `runtime_arr<T>`         |
|-------------------------|------------------|--------------------------|
| Dynamic growth          | ✅                | ❌ (fixed at alloc)       |
| Allocator override      | ✅                | ✅                        |
| POD memset optimization | ❌                | ✅ via `reset_all()`      |
| STL iterators           | ✅                | ✅                        |
| `bool` packed support   | ✅                | ✅ *(only default alloc)* |

---

## 🔹 Template Parameters

```c++
template<typename T, typename Alloc = std::monostate>
struct runtime_arr;
```

- `T`: Type of element stored
- `Alloc`: Optional allocator (default = no-op, raw new/delete)

---

## 🔹 Core API

### Construction

- `runtime_arr(std::uint64_t size)`
    - Zero-initialized

- `runtime_arr(size, Alloc)`
    - Uses custom allocator

- `runtime_arr(size, uninitialized)`
    - For PODs: skip init for performance

- `runtime_arr(std::vector<T>&&)`
    - Move from vector

- `runtime_arr(first, last)`
    - Iterator-based construction

---

### Element Access

- `operator[](i)` - unchecked
- `at(i)` - checked (throws on overflow)
- `set(i, args...)` - constructs value at index

### Iteration

- `begin() / end()` - pointer-style
- `cbegin() / cend()` - const iterators

### Meta & Utility

- `size()` - current element count
- `empty()` - true if size == 0
- `data()` - raw pointer
- `reset_all()` - zero or default reset

### Vector Conversion

```c++
std::vector<T> vec = std::move(arr);
```

- PODs use memcpy
- Others use `std::move`

---

## 👁‍🗨️ `runtime_arr<bool>` Specialization

Bit-packed boolean array with custom `bit_ref` proxy and `uint64_t` storage.

> ⚠️ Only applies when `Alloc = std::monostate`. If any custom allocator is used, `runtime_arr<bool>` will fall back to a regular array of `bool` with full STL compatibility.

### Features (bit-packed version)
- Efficient bit-level access via `operator[]`, `set(i)`, `unset(i)`, `test(i)`
- Compact memory layout: each bit is packed into 64-bit words
- Supports `reset_all()` for fast zeroing
- Provides `raw_data()` and `raw_word_count()` for direct access and SIMD or hashing
- Includes `bit_iterator` and `bit_ref` for proxy-based logic and iteration

### Limitations (bit-packed version)
- ❌ No `data()` pointer — bit-packed layout cannot expose a `bool*`
- ❌ No C++20 `range` support — use only range-based `for` loops or explicit iterators
- ❗ `bit_ref` is a lightweight proxy type, **not** a `bool&`. It supports assignment and conversion to `bool`, but cannot be used where a true reference is required (e.g., `bool*`, references to references, `std::swap`) — similar to `std::vector<bool>::reference`

### Compatibility Note
- When using **custom allocators**, the specialization disables bit-packing. In that mode, `runtime_arr<bool>` behaves like a normal `runtime_arr<T>` with full `bool*` access, iterator and range support, and higher compatibility at the cost of memory usage.

---

## ❌ Limitations

- ❌ No resize / insert
- ❌ Copy disabled (move-only)
- ❌ Not thread-safe
- ❌ No RAII containers inside (POD preferred — but not required)

> ✅ You may use any **move-constructible** type in `runtime_arr<T>`
>
> ✅ `reset_all()` works for any **default-constructible** type
> 
> 🔥 PODs are just more optimized — not required

---

## 🔎 Internals Summary

- Manual memory via RAII wrapper (`unique_ptr<T[], Deleter>`)
- Deleter type is allocator-bound for flexibility
- For POD types, avoids construction overhead
- For `bool`, raw `uint64_t[]` used as backend **only if allocator is monostate**

---

## 🔹 Recommended Practices

| Task                       | Method                  |
|----------------------------|-------------------------|
| POD zero-initialization    | `reset_all()`           |
| Bitmask-style operations   | `runtime_arr<bool>`     |
| High-freq allocator resets | reuse via move          |
| Raw pointer handoff        | `data()`                |
| SIMD or hash optimizations | `raw_data()` (bit mode) |

---

## 🎉 Conclusion

`runtime_arr<T>` is a performance-focused flat container best used in **bounded, predictable-size, high-throughput** contexts.

Use `runtime_arr<bool>` when you need bit-packed logic or compact flag buffers.

If your data doesn't need to grow, but must **go fast**, this is the structure to reach for.

Internally, it is a specialized RAII wrapper around `unique_ptr<T[], Deleter>`.

If you're defining interfaces or composing temporary buffers, it can save time.

However, **if you're writing complex algorithms**, prefer operating directly on `unique_ptr<T[], Deleter>` plus a manual `size_t` to avoid abstraction overhead.

For **RAII-capable types** like `std::tuple<...>`, `runtime_arr` still works well:
- Recommended to use zero-init or `reset_all()` only if type supports trivial/default construction
- In **small-scale or default-allocator usage**, performance exceeds `std::vector`
- With **allocator + RAII**, performance matches `vector` — with the benefit of fixed-capacity constraint

---

📌 **For detailed module information, refer to `runtime_arr.h`.**  
📌 **For additional pod definitions, see [`pod.md`](pod.md).**

🚀 **Enjoy coding with JH Toolkit!**

