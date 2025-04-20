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
- Bit-packed specialization for `bool` (like a custom `bitset`, *only if Alloc = jh::typed::monostate*)

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
template<typename T, typename Alloc = jh::typed::monostate>
struct runtime_arr;
```

- `T`: Type of element stored
- `Alloc`: Optional allocator (default = raw `new[]`)

---

## 🔹 Core API

### Construction

- `runtime_arr(std::uint64_t size)`  
  → Zero-initialized, safe for all types

- `runtime_arr(size, Alloc)`  
  → Uses custom allocator

- `runtime_arr(size, runtime_arr<T>::uninitialized)`  
  → POD-only: skips construction, zeroes bytes (⚠ see below)

- `runtime_arr(std::vector<T>&&)`  
  → Move from vector

- `runtime_arr(first, last)`  
  → Iterator-based construction

### Access & Mutation

- `operator[](i)` – unchecked
- `at(i)` – bounds-checked
- `set(i, args...)` – constructs value at index

### Meta & Utility

- `data()` – raw pointer (if applicable)
- `size()`, `empty()`
- `reset_all()` – zero or default-init all

### Conversion

```c++
std::vector<T> vec = std::move(arr);
```

---

## 👁‍🗨️ `runtime_arr<bool>` Specialization

Bit-packed boolean array using `uint64_t[]` and proxy references.

> ⚠ Only applies when `Alloc = jh::typed::monostate`

| Feature                      | Supported? |
|------------------------------|------------|
| Packed storage               | ✅          |
| `operator[]`, `set`, `unset` | ✅          |
| STL-style iteration          | ❌          |
| C++20 range support          | ❌          |
| `bit_ref` reference proxy    | ✅          |

---

## ❌ Limitations

- ❌ No resize / insert
- ❌ Copy disabled (move-only)
- ❌ Not thread-safe
- ❌ No RAII container types inside (PODs preferred)

---

## 🔎 Internals Summary

- Backed by `std::unique_ptr<T[], Deleter>`
- `reset_all()` is `memset()` for PODs
- `runtime_arr<bool>` uses bitmask backend
- `uninitialized` is a construction tag that zero-fills the bytes but skips `T()` constructor

---

## 🔧 Technical Highlights

- Minimal allocation overhead
- Optional allocator customization
- POD-aware memory model
- Specialized packed `bool` storage

---

## ✅ Recommended Usage

| Scenario                           | Use                             |
|------------------------------------|---------------------------------|
| Fixed-size POD / buffer            | `runtime_arr<T>`                |
| Zero-initialized POD block         | `runtime_arr<T>::reset_all()`   |
| High-performance flag container    | `runtime_arr<bool>`             |
| Large overwrite-only memory region | `runtime_arr<T>::uninitialized` |
| Incremental append/growth          | `std::vector<T>` + `reserve()`  |

---

## 🧪 Performance Benchmarks

### 🔍 Summary (construction + assignment @ N = 1024)

| Compiler  | Optimization | Operation     | Type               | Mean (ns) | Notes                                           |
|-----------|--------------|---------------|--------------------|-----------|-------------------------------------------------|
| **Clang** | `-O2`        | Construction  | `std::vector`      | 126.93    | Full heap + default-init (1024x)                |
|           |              | Construction  | `runtime_arr`      | 19.17     | POD optimized, no capacity                      |
|           |              | Set by Value  | `std::vector`      | 137.76    | `buffer[i] = value`                             |
|           |              | Set by Value  | `runtime_arr`      | 19.40     | `.set(i, T)`                                    |
|           |              | Set Primitive | `std::vector<int>` | 62.42     | reserve + clear overhead                        |
|           |              | Set Primitive | `runtime_arr<int>` | 19.41     | tight loop                                      |
|           |              | Zero Init     | `std::vector`      | 0.265     | `std::fill`                                     |
|           |              | Zero Init     | `runtime_arr`      | 0.265     | `reset_all()`                                   |
| **Clang** | `-O0`        | Construction  | `std::vector`      | 10,305    | Debug-mode STL overhead                         |
|           |              | Construction  | `runtime_arr`      | 307.25    | Lean in debug mode                              |
|           |              | Set by Value  | `std::vector`      | 10,866    |                                                 |
|           |              | Set by Value  | `runtime_arr`      | 320.17    |                                                 |
|           |              | Set Primitive | `std::vector<int>` | 9,867     |                                                 |
|           |              | Set Primitive | `runtime_arr<int>` | 307.72    |                                                 |
|           |              | Zero Init     | `std::vector`      | 3.52      | `std::fill`                                     |
|           |              | Zero Init     | `runtime_arr`      | 3.77      |                                                 |
| **GCC**   | `-O2`        | Construction  | `std::vector`      | 220.27    | Much slower than Clang                          |
|           |              | Construction  | `runtime_arr`      | 24.74     | Consistently fast                               |
|           |              | Set by Value  | `std::vector`      | 218.05    |                                                 |
|           |              | Set by Value  | `runtime_arr`      | 24.73     |                                                 |
|           |              | Set Primitive | `std::vector<int>` | 94.60     |                                                 |
|           |              | Set Primitive | `runtime_arr<int>` | 25.54     |                                                 |
|           |              | Zero Init     | *(skipped)*        | *(--)*    | **GCC optimized away**; results not reliable ⚠️ |
| **GCC**   | `-O0`        | Construction  | `std::vector`      | 709.81    |                                                 |
|           |              | Construction  | `runtime_arr`      | 163.15    |                                                 |
|           |              | Set by Value  | `std::vector`      | 693.56    |                                                 |
|           |              | Set by Value  | `runtime_arr`      | 164.73    |                                                 |
|           |              | Set Primitive | `std::vector<int>` | 792.39    |                                                 |
|           |              | Set Primitive | `runtime_arr<int>` | 163.93    |                                                 |
|           |              | Zero Init     | `std::vector`      | 3.46      |                                                 |
|           |              | Zero Init     | `runtime_arr`      | 3.22      |                                                 |

---

### 💡 Insights

- `runtime_arr<T>` is significantly faster than `std::vector<T>` for **fixed-size fill patterns**.
- POD + `reset_all()` is nearly free (just `memset()`).
- `uninitialized` **zeroes bytes** but skips constructor — only use when full overwrite is guaranteed.
- `std::vector::reserve()` is still best if:
  - You don't want initial construction
  - You're doing incremental `push_back` style usage

---

### ⚠️ Notes on Initialization Semantics

- `runtime_arr<T>` always ensures ownership + defined memory state.
- Even when using `runtime_arr<T>(N, runtime_arr<T>::uninitialized)`:
  - It does **not** leave garbage memory (it's zero-initialized).
  - But **T is not constructed** — useful only for POD / trivially copyable types.
- This is **not equivalent** to `std::vector<T>::reserve(N)` — which skips all initialization.

> 📌 **Attention :** Using `runtime_arr<T>`, the construction cost is **ALWAYS** paid. If you want to avoid construction, use `std::vector<T>` with `reserve(N)`.

---

## 🧠 Design Philosophy

> “If you don’t need to grow, you don’t need to pay the price of growth.”

- `runtime_arr<T>` targets **fixed-size buffer use cases**
- It ensures all memory is **safe, stable, and move-only**
- Use `vector` when you need flexibility
- Use `runtime_arr` when you need safety and predictability

---

## ✅ Summary

- `runtime_arr<T>` is ideal for:
  - POD buffers
  - Per-frame memory
  - Performance-critical batch jobs
  - Bit-packed flag arrays
- **Safer than raw arrays**, **faster than `std::vector`** (in fixed-size use)
- Just like `std::array`, but dynamic and heap-backed

📌 For any structure that:
- Requires a known size
- Is written once, read many times
- Should never be resized  
  ... `runtime_arr<T>` is likely the right choice.

---

📌 **For detailed module information, refer to `runtime_arr.h`.**  
📌 **For additional pod types, see [`pod.md`](pod.md).**

🚀 **Enjoy fast, fixed-capacity memory with `runtime_arr` in JH Toolkit!**