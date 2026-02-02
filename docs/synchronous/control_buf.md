# ⏱️ **JH Toolkit — `jh::sync::control_buf` API Reference**

📁 **Header:** `<jh/synchronous/control_buf.h>`  
📦 **Namespace:** `jh::sync`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::sync::control_buf<T, Alloc>` is a **block-allocated control container** for
**non-relocatable, non-copyable, non-movable control objects** whose memory address
must remain stable for their entire lifetime.

Typical use cases include:

* `std::mutex`, `std::shared_mutex`
* `std::atomic<T>`
* OS-backed synchronization handles
* control-only state that must never be relocated

This container is **not a data structure**.
It exists solely to host **control blocks** safely.

---

## 🧠 Conceptual Model

`control_buf` is best understood as:

> a growable pool of permanently-addressed control objects.

Key properties:

* Elements are **never moved**
* Growth occurs by **allocating new blocks**
* Existing blocks are immutable in location
* Access is **index-based only**

This model intentionally rejects STL-style relocation semantics.

---

## 🧱 Block Allocation Model

Memory is allocated in **fixed-size blocks**.

```cpp
static constexpr std::size_t BLOCK_SIZE = JH_FIXED_VEC_BLOCK_SIZE;
```

### Block size configuration

`JH_FIXED_VEC_BLOCK_SIZE` can be configured in two ways:

1. **Preprocessor definition**

   ```cpp
   #define JH_FIXED_VEC_BLOCK_SIZE 128
   #include <jh/synchronous/control_buf.h>
   ```

2. **Build system (recommended)**

   ```cmake
   add_compile_definitions(JH_FIXED_VEC_BLOCK_SIZE=128)
   ```

If not defined, the default value is **64**.

---

### Allocation behavior

* Each block allocates `BLOCK_SIZE` elements at once
* All elements in a block are **default-constructed immediately**
* Blocks are never reallocated or moved
* Deallocation occurs only on `clear()` or destruction

This design ensures pointer stability and predictable lifetime.

---

## 🔹 `emplace_back()` — Actual Semantics

```cpp
T& emplace_back();
```

Although the API mirrors `std::vector::emplace_back()`,
the **actual behavior is fundamentally different**.

### What really happens

* When a new block is allocated:

    * **`BLOCK_SIZE` elements are constructed at once**
* For most calls:

    * `emplace_back()` simply:

        * advances an index
        * returns a reference to an already-constructed object

In other words:

> most `emplace_back()` calls do not construct anything.

They only **expose the next pre-constructed control object**.

This is intentional and required for control types that cannot be
constructed lazily or relocated.

---

## 🔹 Type Restrictions (Strict)

`control_buf` enforces the following at compile time:

* `T` **must be default-constructible**
* `T` **must not satisfy** `jh::concepts::is_contiguous_reallocable`
* `T` **must not rely on copy or move semantics**

These constraints ensure that:

* no relocation is ever required
* no semantic state is duplicated
* address identity is preserved

Attempting to store ordinary data types is a design error.

---

## 🔁 Copy Semantics — Topology Only

```cpp
control_buf(const control_buf&);
control_buf& operator=(const control_buf&);
```

Copying a `control_buf` performs a **topological copy**:

* The **shape** (number of elements) is preserved
* All elements are **default-constructed**
* **No element state is copied**
* No element is moved or relocated

This is mandatory for control objects whose internal state
must not be duplicated (e.g. mutex ownership).

---

## 🔁 Move Semantics — Block Ownership

```cpp
control_buf(control_buf&&);
control_buf& operator=(control_buf&&);
```

Move operations:

* transfer ownership of blocks
* preserve element addresses
* do not touch element state
* behave similarly to `std::vector` at the container level

The moved-from object remains valid but unspecified.

---

## 🧰 Allocator Behavior

`control_buf` is fully allocator-aware:

* The allocator is rebound to `T`
* Each block captures its allocator in a deleter
* Destruction and deallocation use `allocator_traits`
* Allocator state moves with the container

This allows safe use with:

* custom allocators
* PMR environments
* controlled memory domains

---

## ⚠️ Usage Rules

* **Only store control objects**
* **Never rely on iteration**
* **Never expect value semantics**
* **Treat copy as shape-only**
* **Treat move as ownership transfer**

If you need:

* data storage → use `vector`, `deque`, `ordered_*`
* pointer stability + traversal → use `ordered_*`
* control stability → use `control_buf`

---

## 🧩 Summary

`jh::sync::control_buf` is a **purpose-built control container**:

* block-allocated
* relocation-free
* allocator-aware
* topology-copyable
* index-addressed only

It exists to solve a problem that STL containers explicitly do not:
**hosting growable collections of non-relocatable control objects**.

It is intentionally narrow, intentionally strict, and intentionally safe.
