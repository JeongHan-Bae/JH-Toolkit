# 🧱 **JH Toolkit — `jh::immutable_str` API Reference**

📁 **Header:** `<jh/immutable_str.h>`  
🔄 **Forwarding Header:** `<jh/immutable_str>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../README.md)

</div>

---

## ⚙️ Global Configuration Macro

```cpp
#ifndef JH_IMMUTABLE_STR_AUTO_TRIM
#define JH_IMMUTABLE_STR_AUTO_TRIM true
#endif
```

### `JH_IMMUTABLE_STR_AUTO_TRIM`

Controls whether **all immutable strings automatically trim** leading and trailing ASCII whitespace
during construction.

| Macro value        | Behavior                                                   |
|--------------------|------------------------------------------------------------|
| `true` *(default)* | Removes whitespace automatically for semantic consistency. |
| `false`            | Preserves exact input bytes.                               |

**Notes**

* This macro is evaluated **at compile time** — changing it later has no effect.  
* All translation units must use the same value for deterministic behavior.  
* When `true`, trimming incurs **no measurable performance cost** — benchmarks show parity with `std::string` (within ±2%).  
* Trimmed behavior guarantees that two strings differing only by boundary whitespace are semantically equivalent.

---

## 🧭 Introduction

`jh::immutable_str` is a **truly immutable and thread-safe string type** for modern C++.  
Unlike `std::string`, whose internal buffer remains writable even when `const`,
`immutable_str` enforces immutability both **in type** and **in memory**.

It guarantees:

* No post-construction modification.
* Thread-safe concurrent reads.
* Deterministic hash and equality semantics.

Ideal for:

* Configuration or metadata caches
* Shared symbol tables
* Static registry and reflection systems

---

## 🔹 Overview

| Aspect                 | Description                                                   |
|------------------------|---------------------------------------------------------------|
| **Purpose**            | Represent string data that cannot be modified after creation. |
| **Thread Safety**      | Safe for concurrent reads.                                    |
| **Immutability Level** | Memory-enforced, no writable API.                             |
| **Hashing**            | Cached, lazily computed once.                                 |
| **Build Mode**         | Header-only or static library via Dual-Mode system.           |
| **Integration**        | Works directly with `jh::pool` for deduplication.             |

---

## 🔹 Core Characteristics

| Feature                   | `jh::immutable_str`    | `const std::string`    |
|---------------------------|------------------------|------------------------|
| Memory-level immutability | ✅ True                 | ❌ Mutable              |
| Thread safety             | ✅ Safe                 | ⚠️ Not guaranteed      |
| Reallocation risk         | ❌ None                 | ✅ Possible             |
| Hash caching              | ✅ Cached               | ❌ Recomputed each call |
| Pool compatibility        | ✅ Automatic            | ⚠️ Manual              |
| Storage model             | Compact (`unique_ptr`) | Dynamic capacity model |

---

## 🔹 Construction and Ownership

### From C-String

```cpp
jh::immutable_str hello("Hello, world!");
```

Creates an immutable copy of a null-terminated C-string.  
If `JH_IMMUTABLE_STR_AUTO_TRIM` is `true`, leading/trailing whitespace is automatically removed.  
A `nullptr` input becomes an empty string.

---

### From String View (Safe Mode)

```cpp
std::string buffer = "Config Value";
std::mutex buffer_lock;

jh::immutable_str imm(buffer, buffer_lock);
```

Constructs an immutable copy from a `std::string_view` protected by a **mutex**.  

**Safety requirements**

* The provided `std::mutex` **must** protect the same memory region referenced by the view.  
  The constructor locks the mutex to prevent concurrent modification —
  if another thread can still write to that buffer, behavior is **undefined**.
* If the string view contains any embedded null characters (`'\0'`),
  the constructor throws `std::logic_error`.  
  `immutable_str` strictly accepts **semantically valid text strings**, not raw buffers.

---

### Shared Ownership

```cpp
auto shared = jh::make_atomic("JH Toolkit");
auto copy   = shared; // reference-counted, safe to share
```

Uses `atomic_str_ptr` (`std::shared_ptr<immutable_str>`) for thread-safe sharing.  
Recommended for registry or cache storage.  

---

## 🔹 Thread-Safe Hashing

```cpp
std::uint64_t h = shared->hash();
```

* The hash is computed once on first access and cached.
* Uses `std::once_flag` for lock-free thread safety.
* Identical content → identical hash, guaranteed.

---

## 🔹 Pool Integration

`immutable_str` is natively compatible with [`jh::pool<T>`](pool.md) for **automatic deduplication**:

```cpp
jh::pool<jh::immutable_str> pool;
auto x = pool.acquire("JH Toolkit");
auto y = pool.acquire("JH Toolkit");

if (x.get() == y.get()) {
    std::puts("deduplicated successfully");
}
```

`jh::pool` compares objects by `hash()` and `operator==`,
so semantically identical strings share a single immutable instance.  

---

## 🔹 Interoperability

### Atomic and Weak Aliases

```cpp
using jh::atomic_str_ptr; // std::shared_ptr<immutable_str>
using jh::weak_str_ptr;   // std::weak_ptr<immutable_str>
```

Safe for multi-threaded ownership and observation.

---

### Transparent Hashing and Equality

| Functor               | Purpose                                                      |
|-----------------------|--------------------------------------------------------------|
| `jh::atomic_str_hash` | Content-based hash for `atomic_str_ptr` and `const char*`.   |
| `jh::atomic_str_eq`   | Content equality, whitespace-insensitive if auto-trim is on. |

Example:

```cpp
std::unordered_set<
    jh::atomic_str_ptr,
    jh::atomic_str_hash,
    jh::atomic_str_eq
> registry;

registry.insert(jh::make_atomic("admin"));
if (registry.contains("admin")) {
    std::puts("found!");
}
```

---

## 🔹 Dual-Mode Header Integration

| Mode                                     | Description                                    |
|------------------------------------------|------------------------------------------------|
| **Header-only (`jh-toolkit`)**           | Inline definitions; simple inclusion.          |
| **Static library (`jh-toolkit-static`)** | Compiled separately for deterministic linking. |

You can detect the current linkage mode at runtime:

```cpp
if (jh::immutable_str::is_static_built()) {
    std::puts("running static build");
}
```

The static function
`bool jh::immutable_str::is_static_built();`
returns `true` when you are using the **static library** (`jh::jh-toolkit-static`),
and `false` when using the **header-only** (`jh::jh-toolkit`) mode.

---

## 🔹 Performance Notes

* Immutable buffer → zero reallocation.
* Constant-time hash and comparison (after first call).
* Read-dominant concurrency optimized.
* Memory footprint: pointer + size + cached hash (≈ 24 bytes).
* **Under default `auto_trim = true`, performance matches `std::string` within ±2%**,
  confirming that trimming introduces no measurable cost
  while ensuring that semantically equivalent strings are normalized.

---

## 🧩 Summary

`jh::immutable_str` is the **core immutable text primitive** of the JH Toolkit.  
It enforces strict immutability and thread safety without runtime overhead,
and integrates seamlessly with the toolkit's pool and atomic systems.

---

> **Tip:**
> You can include either
>
> ```cpp
> #include <jh/immutable_str>
> // or
> #include <jh/immutable_str.h>
> ```
>
> Both forms are functionally identical.
> In **CMake**, link against one of the official targets:
>
> ```cmake
> target_link_libraries(your_target PRIVATE jh::jh-toolkit)        # header-only mode  
> target_link_libraries(your_target PRIVATE jh::jh-toolkit-static) # static library mode
> ```
>
> The correct variant will be linked automatically,
> and you can verify it at runtime via `jh::immutable_str::is_static_built()`.
