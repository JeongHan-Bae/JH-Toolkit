# 🧱 **JH Toolkit — `jh::immutable_str` API Reference**

📁 **Header:** `<jh/core/immutable_str.h>`  
🔄 **Forwarding Header:** `<jh/immutable_str>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

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
| **Integration**        | Works directly with `jh::observe_pool` for deduplication.     |

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

### From String View (Safe Mode, since 1.3.5)

```cpp
std::string buffer = "Config Value";
std::mutex buffer_lock;

jh::immutable_str imm(std::string_view(buffer), buffer_lock);
```

Starting from **v1.3.5**, `jh::immutable_str` provides a general-form safe constructor:

```cpp
template <jh::concepts::mutex_like M>
immutable_str(std::string_view sv, M &mtx);
```

This overload accepts **any synchronization type satisfying**
[`jh::concepts::mutex_like`](../conceptual/mutex_like.md) —
including `std::mutex`, `std::shared_mutex`, or the semantic placeholder `jh::typed::null_mutex`
of type [`jh::typed::null_mutex_t`](../typing/null_mutex.md).

It allows safe, deterministic construction of an immutable copy from a **bounded string view**.
During initialization, the source region is locked by a **scope-based guard**:

```cpp
jh::sync::const_lock<M> guard(mtx);  // Scope-based lock, auto-detects shared/exclusive
```

[`jh::sync::const_lock`](../synchronous/const_lock.md) automatically detects
whether the mutex supports shared locking and applies the appropriate lock mode.
This ensures correct behavior across both read/write mutex families without code changes.

---

#### Example — Partial View Construction

```cpp
std::string text = "Hello world";
std::mutex text_lock;

auto space_pos = text.find(' ');
std::string_view partial(text.data(), space_pos);

jh::immutable_str imm(partial, text_lock);  // Copies only "Hello"
```

The constructor copies only the bytes within the provided `string_view`.
This allows *substring-based immutability* — a controlled copy of just the desired range
from a larger buffer, without additional allocation or slicing overhead.

---

#### Thread Safety and Lock Semantics

| Condition                | Behavior                                                                                                  |
|--------------------------|-----------------------------------------------------------------------------------------------------------|
| **Concurrent access**    | The mutex is locked for the full duration of the copy.                                                    |
| **Embedded null check**  | Uses `::strnlen()` to verify that the view contains no `'\0'` bytes. Throws `std::logic_error` otherwise. |
| **Lifetime requirement** | The provided mutex **must** guard the same memory region referenced by `sv.data()`.                       |
| **Compatibility**        | Works with `std::mutex`, `std::shared_mutex`, and any `mutex_like` object.                                |

---

#### No-Lock Mode (Single-Thread / Static Data)

If your source buffer is **thread-local** or **permanently immutable across threads**,
locking introduces no benefit. In such cases, instead of declaring a fake mutex,
use the global [`jh::typed::null_mutex`](../typing/null_mutex.md):

```cpp
#include <jh/typed>

jh::immutable_str imm(std::string_view("Hello, world"), jh::typed::null_mutex);
```

`null_mutex` is a **duck-typed, zero-cost mutex substitute**:
it satisfies all `mutex_like` concepts but performs **no actual locking**.
Every lock operation is a compile-time no-op, fully optimized away.

This makes your intent explicit —
you are stating that the underlying data is either single-thread confined
or globally immutable and therefore safe for concurrent reads.

> ⚠️ **Important:**
> Always ensure that the mutex (real or null) semantically protects the memory region you pass.
> Passing an unrelated or unlocked mutex violates the safety contract.

---

### Shared Ownership Factories

#### `jh::make_atomic`

```cpp
auto shared = jh::make_atomic("JH Toolkit");
auto copy   = shared; // reference-counted, safe to share
```

Creates a `std::shared_ptr<immutable_str>` (`atomic_str_ptr`) from a null-terminated C-string.
The preferred method for constructing immutable strings intended for shared caches or registries.

---

#### `jh::safe_from` *(since 1.3.5)*

```cpp
std::string config = "Host=localhost;Port=3306";
std::mutex config_lock;

auto key_view = std::string_view(config.data(), config.find('='));
auto key = jh::safe_from(key_view, config_lock);
```

`safe_from` mirrors the constructor above but returns a managed shared pointer:

```cpp
template <jh::concepts::mutex_like M>
atomic_str_ptr safe_from(std::string_view sv, M &mtx);
```

It performs the same thread-safe initialization via
[`jh::sync::const_lock`](../synchronous/const_lock.md),
ensuring atomic, exception-safe construction of an immutable string from a protected memory region.

**Key features**

* Accepts any `mutex_like` object — including [`jh::typed::null_mutex`](../typing/null_mutex.md) for no-lock semantics.
* Supports **substring construction** through arbitrary `std::string_view` bounds.
* Validates that no embedded null characters exist using `::strnlen()`.
* Ensures consistent locking semantics with the primary constructor.

---

#### Example — Safe Substring Creation

```cpp
std::string data = "Path:/usr/local/bin";
std::mutex m;

auto view = std::string_view(data.data() + 5, 13); // "/usr/local/bin"
auto path = jh::safe_from(view, m);
```

---

#### Comparison

| Feature                    | `make_atomic`       | `safe_from`                     |
|----------------------------|---------------------|---------------------------------|
| Input type                 | `const char*`       | `std::string_view + M`          |
| Locking                    | None                | Scoped (`jh::sync::const_lock`) |
| Supports substrings        | ❌ No                | ✅ Yes                           |
| Embedded null check        | Implicit (`strlen`) | Explicit (`::strnlen`)          |
| Thread-safe initialization | ❌ User-guaranteed   | ✅ Built-in                      |
| No-lock mode               | N/A                 | ✅ via `jh::typed::null_mutex`   |

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

`immutable_str` is natively compatible with [`jh::observe_pool<T>`](../concurrent/observe_pool.md) for **automatic deduplication**:

```cpp
jh::observe_pool<jh::immutable_str> pool;
auto x = pool.acquire("JH Toolkit");
auto y = pool.acquire("JH Toolkit");

if (x.get() == y.get()) {
    std::puts("deduplicated successfully");
}
```

`jh::observe_pool` compares objects by `hash()` and `operator==`,
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
> #include <jh/core/immutable_str.h>
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
