# 📦 **JH Toolkit — `jh::utils::hash_fn` API Reference**

📁 **Header:** `<jh/utils/hash_fn.h>`  
📦 **Namespace:** `jh::utils::hash_fn`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Utils](https://img.shields.io/badge/%20Back%20to%20Utils-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::utils::hash_fn` provides **constexpr-safe, non-cryptographic hashing utilities**
for lightweight identifiers, symbol indexing, and compile-time hashing.  

It implements several classic 64-bit hash algorithms
— all available as pure `constexpr` functions without any runtime heap or STL dependency.  

These functions are designed for **reproducibility, speed, and compile-time usability**,
not for security or cryptographic validation.  

---

## 🔹 Overview

| Aspect                   | Description                                                                      |
|--------------------------|----------------------------------------------------------------------------------|
| **Purpose**              | Deterministic, non-cryptographic hashing for POD or string data.                 |
| **Implementation Model** | Fully `constexpr`; no dynamic allocation or runtime state.                       |
| **Compatibility**        | Works on `const char*`, `uint8_t*`, or `int8_t*`; portable across architectures. |
| **Error Handling**       | No exceptions — all functions are pure and `noexcept`.                           |
| **Algorithm Selection**  | Controlled via `jh::utils::hash_fn::c_hash`.                                     |

---

## 🔹 Core Components

| Symbol              | Type        | Description                                                  |
|---------------------|-------------|--------------------------------------------------------------|
| `enum class c_hash` | Enumeration | Algorithm selector for `fnv1a64`, `fnv1_64`, `djb2`, `sdbm`. |
| `fnv1a64()`         | Function    | Default algorithm — FNV-1a 64-bit (xor before multiply).     |
| `fnv1_64()`         | Function    | FNV-1 64-bit (multiply before xor).                          |
| `djb2()`            | Function    | Classic string hash (`h * 33 + c`).                          |
| `sdbm()`            | Function    | Legacy hash used in DB engines and file systems.             |
| `hash()`            | Function    | Dispatcher that selects the algorithm by `c_hash`.           |

---

## 🔹 Algorithm Reference

### `enum class c_hash`

```cpp
enum class c_hash : std::uint8_t {
    fnv1a64 = 0,  // FNV-1a 64-bit (default)
    fnv1_64 = 1,  // FNV-1 64-bit
    djb2    = 2,  // DJB2
    sdbm    = 3   // SDBM
};
```

Used as the compile-time selector for all hash functions.

---

### `fnv1a64()`

```cpp
constexpr std::uint64_t fnv1a64(
    const char* data,
    std::uint64_t size
) noexcept;
```

**Description:**
FNV-1a 64-bit hash — performs `xor` before multiply.
It is the **default** algorithm for most identifiers in JH Toolkit.

**Notes:**

* Excellent distribution for short keys.
* Compact implementation suitable for `constexpr` evaluation.

---

### `fnv1_64()`

```cpp
constexpr std::uint64_t fnv1_64(
    const char* data,
    std::uint64_t size
) noexcept;
```

**Description:**
Classic FNV-1 variant — multiply before `xor`.
Provides similar performance to `fnv1a64`, with subtly different avalanche properties.

---

### `djb2()`

```cpp
constexpr std::uint64_t djb2(
    const char* data,
    std::uint64_t size
) noexcept;
```

**Description:**
DJB2 — the iconic hash used for strings (`hash * 33 + c`).
Compact, portable, and effective for ASCII keys or short literals.

---

### `sdbm()`

```cpp
constexpr std::uint64_t sdbm(
    const char* data,
    std::uint64_t size
) noexcept;
```

**Description:**
SDBM — the classic database hash algorithm used in `dbm` and filesystem tables.
Known for its simple recurrence and good dispersion properties.

---

### `hash()`

```cpp
constexpr std::uint64_t hash(
    c_hash algo,
    const char* data,
    std::uint64_t size
) noexcept;
```

**Description:**
Generic dispatcher that selects a hash algorithm at compile-time or runtime
based on the provided `c_hash` enumeration.

| Parameter | Type            | Description               |
|-----------|-----------------|---------------------------|
| `algo`    | `c_hash`        | Algorithm selector.       |
| `data`    | `const char*`   | Pointer to byte sequence. |
| `size`    | `std::uint64_t` | Input length in bytes.    |

**Returns:**
`std::uint64_t` — resulting hash value.

**Example:**

```cpp
using namespace jh::utils::hash_fn;

constexpr auto id = hash(c_hash::fnv1a64, "hello", 5);
// Evaluates at compile time
```

---

## 🔹 Input Type Policy

All functions accept **byte-compatible pointer types**,
converted internally via `static_cast<uint8_t>`.

| Accepted Types                            | Note                                                                 |
|-------------------------------------------|----------------------------------------------------------------------|
| `const char*`                             | Canonical form.                                                      |
| `const unsigned char*` / `const uint8_t*` | Interpreted as bytes.                                                |
| `const signed char*` / `const int8_t*`    | Safely promoted to `uint8_t`.                                        |
| `std::byte*`                              | ❌ Not directly supported — must use `reinterpret_cast<const char*>`. |

This policy guarantees **constexpr and consteval compatibility**
while remaining ABI-safe at runtime.

---

## 🧩 Summary

`jh::utils::hash_fn` provides a **constexpr-safe hashing foundation**
for ID generation, bucket indexing, and lightweight key mapping.

It balances simplicity and determinism,
serving as a reliable utility layer for POD-based systems or compile-time registries.

All algorithms are deterministic across platforms and require no external dependencies.

---

> **Note:**
> These functions are **non-cryptographic**.
> They should never be used for password hashing, checksums, or security-sensitive contexts.
