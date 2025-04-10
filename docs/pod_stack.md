# 📦 JH Toolkit: `pod_stack` API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **Overview**

`jh::pod_stack<T, BLOCK_SIZE>` is a **high-performance**, **stack-only container** optimized for **Plain Old Data (POD)** types. Tailored for LIFO (Last-In-First-Out) workloads, this container provides fast, cache-friendly `push`, `pop`, and `top` operations with **zero destructor overhead**.

---

## **Key Features**

- ✅ **POD-only**: Accepts only trivial, standard-layout types.
- ⚡ **Fast push/pop/top**, with stable top reference.
- 📦 **Contiguous block allocation** (cache-local).
- 🔧 **Block detachment and reuse** with `clear`, `clear_reserve`.
- ♻️ **Zero-cost clear-reserve mode** for high-frequency stack resets.

> 🐉 **Analogically inspired by the mythical beast 椒图 (Jiaotu)** — the strict gatekeeper who controls only the **entry and exit**.  
> `pod_stack` only gives access to the **top**, nothing else.

---

## **Template Parameters**

```c++
template<typename T, std::uint32_t BLOCK_SIZE = 2048>
struct pod_stack;
```

- `T`: Must satisfy `jh::pod::pod_like<T>`.
- `BLOCK_SIZE`: Must be a power-of-two ≥ 256

---

## **Constructor**

### 📌 `pod_stack()`

Creates an empty stack.
- No memory is allocated until the first `push`.

---

## **Core Methods**

### 📌 `void push(Args&&...)`
Pushes a new value to the top using placement-new.

```c++
stack.push(arg1, arg2); // Constructs T(arg1, arg2)
```

---

### 📌 `void emplace(T&& obj)`
Moves an existing `T` into the stack.  
Avoids copy; uses in-place move construction.

---

### 📌 `T& top()`
Returns a reference to the current top element.
- Must not be called when empty.

---

### 📌 `void pop()`
Removes the top element.
- Does not deallocate memory.
- Does not shrink the block chain.

---

### 📌 `void clean_pop()`
Pops the top, and if the current block becomes empty, **detaches** it unless it is the root.

---

### 📌 `bool empty() const`
Checks whether the stack is empty.

---

### 📌 `std::uint64_t size() const`
Returns the number of elements in the stack.

---

### 📌 `void clear()`

**Resets to an empty state, and detaches all blocks except the root**.
- Keeps root block alive and reusable.
- Frees all chained memory.

```c++
stack.clear(); // Lightweight reset, keeps root
```

---

### 📌 `void clear_reserve(std::optional<uint64_t> blocks = std::nullopt)`

Clears stack and optionally **preserves up to `N` blocks**.

- `clear_reserve()` with **no argument**:
    - Resets sizes but **retains all existing blocks**
    - Fastest path (no memory freed)

- `clear_reserve(0)`:
    - Behaves like `clear()`, freeing all except the root block

- `clear_reserve(N)`:
    - Retains the first `N` blocks and detaches any beyond

```c++
stack.clear_reserve();    // ⚡ fastest reset, no memory freed
stack.clear_reserve(0);   // same as clear()
stack.clear_reserve(2);   // keep only 2 blocks
```

---

## 🧠 **Design Philosophy**

- 📦 Stack memory is chunked into linked blocks.
- 🚫 No iterators or internal access — **LIFO only**
- 🧱 Cache-aligned storage for dense performance
- 🔒 Designed for **tight-loop**, **low-entropy** memory flows

---

## **Best Use Cases**

- ✔️ DFS / BFS simulation
- ✔️ Runtime context stacks
- ✔️ Recursion flattening
- ✔️ Interpreter scopes
- ✔️ Per-frame scratch storage

---

## **Summary Table**

| Method             | Memory Impact           | Performance | Notes                             |
|--------------------|-------------------------|-------------|-----------------------------------|
| `push()`           | Allocates block if full | O(1)        | Placement-new                     |
| `pop()`            | Retains memory          | O(1)        | Top-only removal                  |
| `clean_pop()`      | Detaches block if empty | O(1)        | Trims if current block is empty   |
| `clear()`          | Keeps only root block   | O(1)        | Frees all extra memory            |
| `clear_reserve()`  | Retains **all** blocks  | O(1)        | Fastest for reuse-heavy scenarios |
| `clear_reserve(0)` | Same as `clear()`       | O(1)        | Minimal retained memory           |
| `clear_reserve(N)` | Keeps first `N` blocks  | O(N)        | Partial release                   |

---

## ⚠️ Limitations

- ❌ No iterators
- ❌ No random access
- ❌ No non-POD types
- ❌ No built-in thread safety

---

## 🐲 The Jiaotu Principle

**Inspired by Jiaotu**, the mythical gate beast:

- You only interact with **the gate** — the top of the stack.
- What came before is hidden, sealed away.
- You can't look back, only control **push**, **pop**, or **reset**.

---

## **Conclusion**

The `jh::pod_stack<T, BLOCK_SIZE>` is a **precision-engineered LIFO container** for high-performance, POD-only workloads.  
Whether you're simulating frames, walking trees, or building interpreters, this structure **gets out of your way and stays fast**.

---

📌 **For detailed module information, refer to `pod_stack.h`.**  
📌 **For additional pod definitions, see [`pod.md`](pod.md).**

🚀 **Enjoy coding with JH Toolkit!**
