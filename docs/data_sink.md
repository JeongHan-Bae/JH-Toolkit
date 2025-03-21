### **JH Toolkit: data_sink API Documentation**

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **Overview**

`jh::data_sink<T, BLOCK_SIZE>` is a **high-performance, append-only container** optimized for **sequential data accumulation** and **read-only iteration**. Designed for **numerical buffers**, **event logs**, and **algorithmic pipelines** (e.g., radix sort buckets), it minimizes overhead and fragmentation using **cache-aligned block allocation**.

---

## **Key Features**

- ✅ **Append-only** semantics (no pop/erase).
- 🚀 **Bulk append** (`bulk_append`) from any range.
- 📦 **Contiguous block allocation**, CPU cache-friendly.
- 🔁 **Read-only FIFO iteration**.
- 🔧 **Transform in-place** with `inplace_map`.
- ♻️ **Memory reuse** with `clear_reserve`.
- 🧠 **Optimized for fundamental and pointer types**.

> ❗ Not thread-safe. Use external synchronization or implement your own `data_buffer` wrapper (see below for a minimal example).

---

## **Design Philosophy**

- ⚠️ **No random access**: strictly sequential iteration.
- 📚 **No removals or modifications**, except via bulk transformation.
- 🧱 **Fixed-size block chaining**, customizable via `BLOCK_SIZE`.
- 🔒 **Const-only iterators** to prevent accidental mutation.
- 🐉 **Pixiu-inspired**: only accepts input, never releases it.

---

## **Template Parameters**

```c++
template<typename T, std::uint32_t BLOCK_SIZE = 8192>
struct data_sink;
```

- `T`: Type of stored elements.
    - Supported types:
        - Fundamental types (`int`, `float`, `bool`, etc.)
        - Raw pointers (`T*`, only 64-bit)
        - `std::unique_ptr<U>`
- `BLOCK_SIZE`: Number of elements per block (must be power-of-two ≥ 1024).

---

## **Constructor**

#### 📌 `data_sink()`

Creates an empty `data_sink`.

---

## **Core Methods**

### 📌 `void emplace_back(Args&&...)`
Appends a single element.
- Constructs the element in-place.
- Supports both `T` and `std::unique_ptr<U>` construction.

#### ✅ Three Valid Forms of Construction
1. `sink.emplace_back(MyStruct{args...});`
    - Constructs `MyStruct` via **list-initialization** (brace-initializer).
    - Safest and most explicit form, especially for containers like `std::vector`.
    - **Recommended when using `std::unique_ptr<MyStruct>`** — this is how the internal object is actually created.

2. `sink.emplace_back(std::move(obj));`
    - Moves an **existing object of type `T`** (e.g., `MyStruct`) into the container.
    - Useful when you want to transfer ownership or avoid reconstruction.

3. `sink.emplace_back(args...);`
    - Uses **perfect forwarding**.
    - Equivalent to `T x(args...)`, i.e., **constructor-style initialization**.
    - ⚠️ **Note:** For some STL types, this may yield unexpected behavior.
      For example, `std::vector<int>(5)` creates a vector of five zeros, while `std::vector<int>{5}` creates a single element {5}.

> ⚠️ Avoid ambiguous syntax like `sink.emplace_back({args...})` unless the constructor explicitly supports initializer lists.

---

📌 **Special Behavior for `data_sink<std::unique_ptr<T>>`**
- You are **not** supposed to pass a `std::unique_ptr<T>` directly.
- Instead, `emplace_back(...)` internally constructs a `T` and stores it in a `std::unique_ptr<T>`.
- Examples:
  ```c++
  data_sink<std::unique_ptr<MyStruct>> sink;
  sink.emplace_back(MyStruct{1, 2, 3});       // ✅ constructs MyStruct and stores in unique_ptr
  MyStruct obj;
  sink.emplace_back(std::move(obj));          // ✅ moves MyStruct into internal unique_ptr
  sink.emplace_back(1, 2, 3);                 // ✅ forwards to MyStruct(1, 2, 3)
  // sink.emplace_back(std::make_unique<MyStruct>()); // ❌ Not allowed — cannot move unique_ptr itself
  ```

---
Appends a full range of elements.
- Equivalent to repeated `emplace_back`.
- Highly optimized for range-based input.

### 📌 `void clear()`
Clears all data and releases memory.

### 📌 `void clear_reserve(std::optional<std::uint64_t> blocks = std::nullopt)`
Clears all data but retains memory for reuse.
- Specify `blocks = N` to keep up to `N` blocks.
- Default retains **all blocks**.

### 📌 `void inplace_map(std::function<void(T&)> transform)`
Applies a transformation to **each element** in-place.
- E.g., normalization, negation, scaling.
- 🚀 Optimized for sequential, cache-local access.

### 📌 `std::uint64_t size() const`
Returns the total number of stored elements.

### 📌 `static constexpr std::uint32_t block_capacity()`
Returns the block size (`BLOCK_SIZE`).

---

## **Iteration Support**

### 📌 `const_iterator begin() const`
Returns a const iterator to the beginning.

### 📌 `static const_iterator end()`
Returns a sentinel iterator.

### **Iterator Behavior**
- Input iterator (forward-only).
- `*it` gives read-only access to element.
- Iteration skips empty blocks.

---

## **Conversion**

### 📌 `explicit operator std::ranges::subrange<iterator>() const`
Converts `data_sink` into a `std::ranges::subrange`.
- Enables compatibility with modern STL algorithms and ranges.

---

## **Concept Constraints**

- `T` must satisfy one of:
    - Fundamental types
    - `T*` where `sizeof(T*) == 8`
    - `std::unique_ptr<U>`
- `BLOCK_SIZE` must be power-of-two and ≥ 1024

---

## **Performance Notes**

- 📏 **Outperforms `std::vector` (×2.3) and `std::deque` (×4.0)** in high-volume radix sort.
- 🧠 Avoids reallocation and copying via block reuse.
- 🧱 Designed for **log-style writes**, **data staging**, and **stream processing**.

---

## **Best Use Cases**

- ✔️ High-frequency logging / tracing
- ✔️ Bucket buffer for radix sort
- ✔️ Batch data preloading
- ✔️ Append-only event streaming
- ✔️ Single-threaded numeric simulation buffers

---

## **Limitations**

- ❌ No erase / pop / insert
- ❌ No random access (e.g. `sink[i]` not allowed)
- ❌ Not thread-safe
- ❌ Cannot store complex user-defined structs (unless via `unique_ptr`)

---

## **Optional Wrapper: Minimal `data_buffer` (Thread-Safe)**

If basic thread-safety is needed, consider wrapping `data_sink` with a `std::shared_mutex`:

```c++
#include <shared_mutex>
#include <jh/data_sink>

namespace jh {
    template<typename T, std::uint32_t BLOCK_SIZE = 8192>
    struct data_buffer {
        data_sink<T, BLOCK_SIZE> sink_;
        mutable std::shared_mutex rw_mtx;

        template<typename... Args>
        void emplace_back(Args&&... args) {
            std::lock_guard lock(rw_mtx);
            sink_.emplace_back(std::forward<Args>(args)...);
        }

        template<std::ranges::viewable_range R>
        void bulk_append(R&& r) {
            std::unique_lock lock(rw_mtx);
            sink_.bulk_append(std::forward<R>(r));
        }

        std::uint64_t size() const {
            std::shared_lock lock(rw_mtx);
            return sink_.size();
        }

        bool empty() const {
            std::shared_lock lock(rw_mtx);
            return sink_.empty();
        }

        std::vector<T> get_snapshot() const {
            std::shared_lock lock(rw_mtx);
            std::vector<T> result;
            result.reserve(sink_.size());
            for (const auto& x : sink_) result.push_back(x);
            return result;
        }
    };
}
```

> 🧩 This wrapper provides shared read and exclusive write access using `std::shared_mutex`.
> It is **not as fast** as the raw `data_sink`, but **suitable for moderate multi-threaded workloads**.

---

## **Trivia: Why "Pixiu"? 🐉**
Like the legendary beast **貔貅**, `data_sink` only takes in data, never gives it back. It is the **perfect structure for greedy, high-speed append operations**.

> For mutable and flexible containers, use `std::vector`.
> For thread-safe variants, consider a `data_buffer` wrapper.

---

## **Conclusion**

The `jh::data_sink<T, BLOCK_SIZE>` offers a lightweight, high-throughput, append-only container ideal for sequential memory workloads. Its contiguous block structure ensures excellent cache locality, and its restriction to fundamental and pointer-based types allows for highly optimized memory access and storage behavior.

When paired with tools like `jh::generator`, it becomes a powerful component in composable, high-performance data pipelines, especially in domains like radix sorting, simulation buffering, or log/event ingestion.


📌 **For detailed module information, refer to `data_sink.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**


