# 📦 JH Toolkit: `data_sink` API Documentation

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

## ⚠️ Important Sections

- [✅ Valid `{}` Usage in emplace_back](#-three-valid-forms-of-construction)
- [❌ Why `{}` Can't Be Perfect-Forwarded](#-limitations)
- [⚠️ Mixed-Type `{}` = Compilation Error](#-note-on--brace-initialization)
- [✅ Best Practice Summary Table](#-best-practice-summary)
- [🧠 Constructor Behavior Note](#-void-emplace_backargs)

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

> In `jh::data_sink<T, BLOCK_SIZE>`,
**brace-initialization (`{...}`)** is supported under specific and limited conditions,
due to how C++ handles `std::initializer_list` and braced-init-lists.


1. **Explicit list-initialization via object construction:**
   ```c++
   sink.emplace_back(MyStruct{arg1, arg2, arg3}); // ✅ uses MyStruct's brace-constructor
   ```

2. **Standard constructor syntax:**
   ```c++
   sink.emplace_back(arg1, arg2, arg3); // ✅ uses MyStruct(arg1, arg2, arg3)
   ```

3. **Initializer-list constructor for homogenous types only:**
   ```c++
   sink.emplace_back({1, 2, 3}); // ✅ only if T has constructor from std::initializer_list<int>
   ```

   This works *only* because:
    - The types inside `{...}` are all the same (e.g. `int`)
    - `T` (or `U` if `T = std::unique_ptr<U>`) has a constructor accepting `std::initializer_list<T>`

---
#### ⚠️ Limitations

- **Brace-init-lists (`{...}`) cannot be perfect-forwarded.**  
  That means:
  ```c++
  template<typename... Args>
  void emplace_back(Args&&... args);
  ```
  will not match `sink.emplace_back({a, b, c})` — it results in compilation failure because `{...}` does **not** bind to a deduced `Args&&...` pack.

- **Mixed-type `{...}` (e.g., `{"John", 42, true}`) is not a valid `initializer_list`.**  
  A `std::initializer_list<T>` requires **all elements to be of the same type**. The compiler will not deduce it otherwise.

  ```c++
  sink.emplace_back(Person{"John", 42, true}); // ✅ OK
  // sink.emplace_back({"John", 42, true});    // ❌ INVALID — no matching initializer_list
  ```

- `std::initializer_list<T>` is mainly useful when initializing **container-like types** (e.g., `std::vector`, `std::set`, etc.).  
  For these types, `{...}` is not just syntactic sugar — it's a semantically meaningful way to describe the **internal contents** of the container:

  ```c++
  sink.emplace_back({1, 2, 3}); // ✅ For vector/set types — clearly declares contents
  ```

---

#### ✨ Best Practice Summary

| Syntax                              | Works? | Reason                                                  |
|-------------------------------------|--------|---------------------------------------------------------|
| `sink.emplace_back({1, 2, 3})`      | ✅      | All elements same type + matches initializer_list ctor  |
| `sink.emplace_back("a", 1, true)`   | ✅      | Forwarded to constructor (e.g., `Person("a", 1, true)`) |
| `sink.emplace_back(MyType{...})`    | ✅      | Explicit brace-init object construction                 |
| `sink.emplace_back({"a", 1, true})` | ❌      | Mixed-type list — not a valid initializer_list          |
| `sink.emplace_back(std::move(obj))` | ✅      | Valid if `obj` is the correct type                      |

---

#### ⚠️ Note on `{}` Brace Initialization

`data_sink.emplace_back({ ... })` is only supported when:

- All elements inside `{}` are of the **same type**
- The stored type `T` (or `U` if `T = std::unique_ptr<U>`) has a constructor accepting `std::initializer_list<T>`

> 💡 **Caution:** Even if `{}` appears valid, it may introduce **constructor ambiguity**  
> due to overload resolution between brace-init and variadic forwarding.  
> **Avoid using `{}`** unless you are intentionally targeting a container-like type.

```c++
sink.emplace_back({1, 2, 3}); // ✅ only works if T or U has std::initializer_list<int> ctor
sink.emplace_back({});        // 🚫 can be ambiguous if multiple constructors exist
```

> ⚠️ Do **not** rely on `{}` to invoke default construction.  
> This may accidentally match an `initializer_list` overload, resulting in **unexpected behavior** or a **compilation error**.  
> **Always prefer:**
```c++
sink.emplace_back(); // ✅ default-constructs object safely
```

Additionally:

- The stored type `T` (or `U` if `T = std::unique_ptr<U>`) must explicitly support construction from `std::initializer_list<X>`.  
  If not, brace-based syntax like `{1, 2}` will **not compile**.
- **Fundamental types** like `int`, `float`, or `bool` do support `{}` construction (e.g., `int{42}`),  
  but this syntax is **unnecessary** and offers **no readability or performance advantage**.  
  Just write:

```c++
sink.emplace_back(42);       // ✅ simpler and clearer
// sink.emplace_back({42});  // 🚫 avoid — redundant for scalar types
```

However:

> ⚠️ **Tip: Don’t use `{}` unless you're explicitly initializing a container-like type.**

Prefer:

```c++
sink.emplace_back({1, 2, 3}); // ✅ For vector/set types — clearly declares contents
```

> #### ⚠️ Initializing `std::unordered_map` or `std::map`
>
> Do **not** use brace-init lists directly for `std::unordered_map`:
> ```c++
> sink.emplace_back({{1, 2}, {3, 4}}); // ❌ ambiguous or rejected
> sink.emplace_back(std::unordered_map<int, int>{{1, 2}, {3, 4}}); // ⚠️ copies!
> ```
>
> **Instead**, construct externally and `std::move`:
> ```c++
> std::unordered_map<int, int> tmp_map{{1, 2}, {3, 4}};
> sink.emplace_back(std::move(tmp_map)); // ✅ performs a move, not a copy
> ```

Although the syntax
```c++
auto p1 = Person{"John", 42, true};
Person p2("John", 42, true);
```  
are **functionally equivalent**,
the brace form offers **no additional benefit** for non-container types
and may introduce confusion when overloading or template deduction is involved.

Avoid:
```c++
sink.emplace_back({"John", 42, true}); // ❌ Invalid — not a homogeneous initializer list
```

Prefer one of the following:
```c++
sink.emplace_back("John", 42, true);          // ✅ clean forwarding
sink.emplace_back(Person{"John", 42, true});  // ✅ clear explicit object (copies)
```

---

### 📌 `void bulk_append(std::ranges::viewable_range R)`
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

- 📏 **Outperforms `std::vector` (×2.3) and `std::deque` (×4.7)** during high-throughput `emplace_back` operations **without prior knowledge of total volume**.
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

## 📌 Storing STL Containers via `unique_ptr`

Although `jh::data_sink` is **primarily designed for primitive types and POD-style structs**, it **permits** storing standard containers like `std::vector`, `std::map`, or `std::unordered_map` — **as long as they are wrapped in `std::unique_ptr<T>`**:

```c++
data_sink<std::unique_ptr<std::vector<int>>> sink;

sink.emplace_back({1, 2, 3});                         // ✅ OK: initializer_list
sink.emplace_back(std::vector<int>{1, 2, 3});         // ✅ OK: explicit vector construction
// ❌ Not allowed: cannot pass unique_ptr directly
// sink.emplace_back(std::make_unique<std::vector<int>>(10, 42));
```

> 💡 **Note:** This pattern is supported but **not encouraged**.  
> `data_sink` is optimized for **flat, fixed-size, cache-local data**.  
> STL containers introduce extra heap allocations and indirections, potentially defeating that purpose.

However, for niche use cases like:
- Nested buckets
- Temporary or scoped buffers
- Object reuse across phases

this trade-off might be justifiable.

---

### 📦 Summary

| Use Case                                | Supported | Recommended | Notes                                    |
|-----------------------------------------|-----------|-------------|------------------------------------------|
| `std::unique_ptr<MyPODStruct>`          | ✅         | ✅           | Ideal scenario                           |
| `std::unique_ptr<std::vector<int>>`     | ✅         | ⚠️          | Works, but with runtime overhead         |
| `std::unique_ptr<std::unordered_map<>>` | ✅         | ⚠️          | Must pre-construct and pass via `move()` |
| `std::vector<T>` (direct as T)          | ❌         | ❌           | Not supported by type constraints        |

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
