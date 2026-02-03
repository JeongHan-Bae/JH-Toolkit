# ⚗️ **JH Toolkit — `jh::meta::hash` API Reference**

📁 **Header:** `<jh/metax/hash.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::hash` provides **constexpr-safe, compile-time hash algorithms**
for meta-programming utilities such as **type reflection**, **compile-time identifiers**, and **static lookup maps**.

All implementations are:

* **Heap-free**, **STL-independent**, and **fully `constexpr`-evaluatable**
* **Deterministic across platforms**
* Designed for **speed and reproducibility**, *not for cryptographic use*

---

## 🔹 Overview

| Aspect                   | Description                                                              |
|--------------------------|--------------------------------------------------------------------------|
| **Purpose**              | Deterministic, non-cryptographic hashing for identifiers and reflection. |
| **Implementation Model** | Fully `constexpr`; no heap, no STL.                                      |
| **Compatibility**        | Works with `char`, `signed char`, and `unsigned char` via `any_char`.    |
| **Error Handling**       | No exceptions — all functions are pure and `noexcept`.                   |
| **Algorithm Selection**  | Controlled by `jh::meta::c_hash` enum.                                   |

---

## 🔹 Core Components

| Symbol              |    Type     | Description                                                    |
|---------------------|:-----------:|----------------------------------------------------------------|
| `enum class c_hash` | Enumeration | Algorithm selector for all available constexpr hash functions. |
| `fnv1a64()`         |  Function   | Default algorithm — FNV-1a 64-bit (xor before multiply).       |
| `fnv1_64()`         |  Function   | FNV-1 64-bit (multiply before xor).                            |
| `djb2()`            |  Function   | Classic string hash (`hash * 33 + c`).                         |
| `sdbm()`            |  Function   | Legacy DB-style hash (`c + (h << 6) + (h << 16) - h`).         |
| `murmur64()`        |  Function   | Seedless constexpr variant of MurmurHash3 (avalanche-mixed).   |
| `xxhash64()`        |  Function   | Deterministic constexpr xxHash64-like implementation.          |
| `hash()`            |  Function   | Dispatcher that selects algorithm by `c_hash`.                 |

---

## 🔹 Algorithm Reference

### `enum class c_hash`

```cpp
enum class c_hash : std::uint8_t {
    fnv1a64 = 0,  // FNV-1a 64-bit (default)
    fnv1_64 = 1,  // FNV-1 64-bit
    djb2    = 2,  // DJB2
    sdbm    = 3,  // SDBM
    murmur64 = 4, // constexpr-safe Murmur variant
    xxhash64 = 5  // constexpr xxHash64 variant
};
```

Selects which algorithm `hash()` will dispatch at compile time or runtime.

---

### `fnv1a64()`

```cpp
template<any_char Char>
constexpr std::uint64_t fnv1a64(const Char* data, std::uint64_t size) noexcept;
```

**Description:**
FNV-1a 64-bit hash — `xor` before multiply.
The **default** algorithm for identifiers and small keys.

**Notes:**

* Excellent balance of speed and distribution.
* Well-suited for `constexpr` evaluation.

---

### `fnv1_64()`

```cpp
template<any_char Char>
constexpr std::uint64_t fnv1_64(const Char* data, std::uint64_t size) noexcept;
```

**Description:**
Classic FNV-1 variant — multiply before `xor`.
Nearly identical to FNV-1a, but with slightly different avalanche behavior.

---

### `djb2()`

```cpp
template<any_char Char>
constexpr std::uint64_t djb2(const Char* data, std::uint64_t size) noexcept;
```

**Description:**
DJB2 — the classic hash function used in early UNIX string tables.
Uses `hash = hash * 33 + c`.

**Use Case:**
Lightweight compile-time hashing of short strings or identifiers.

---

### `sdbm()`

```cpp
template<any_char Char>
constexpr std::uint64_t sdbm(const Char* data, std::uint64_t size) noexcept;
```

**Description:**
Legacy database hash from the original `ndbm` and `readdir` implementations.
Formula: `hash = c + (hash << 6) + (hash << 16) - hash`.

---

### `murmur64()`

```cpp
template<any_char Char>
constexpr std::uint64_t murmur64(const Char* data, std::uint64_t size) noexcept;
```

**Description:**
A constexpr-friendly, **seedless** variant of **MurmurHash3 (64-bit)**.
Designed for reproducible compile-time use with small data blocks.

**Notes:**

* Includes full **avalanche finalization**.
* No runtime seed — ensures deterministic compile-time hash.

---

### `xxhash64()`

```cpp
template<any_char Char>
constexpr std::uint64_t xxhash64(const Char* data, std::uint64_t size) noexcept;
```

**Description:**
A constexpr-safe variant of **xxHash64**, implemented without seeds or heap usage.

**Features:**

* Uses prime constants from official xxHash.
* Excellent bit dispersion for larger strings.

---

### `hash()`

```cpp
template<any_char Char>
constexpr std::uint64_t hash(c_hash algo, const Char* data, std::uint64_t size) noexcept;
```

**Description:**
Generic dispatcher for all supported algorithms.

| Parameter | Type            | Description             |
|-----------|-----------------|-------------------------|
| `algo`    | `c_hash`        | Algorithm selector.     |
| `data`    | `const Char*`   | Pointer to input bytes. |
| `size`    | `std::uint64_t` | Input length in bytes.  |

**Returns:**
`std::uint64_t` — resulting hash value.

**Example:**

```cpp
using namespace jh::meta;

constexpr auto id = hash(c_hash::fnv1a64, "hello", 5);
// Evaluates entirely at compile time
```

---

## 🔹 Input Type Policy

All functions are templated on `any_char`, defined in `<jh/metax/char.h>`,
which accepts only *character-compatible types*.

| Accepted Types              | Note                                                      |
|-----------------------------|-----------------------------------------------------------|
| `char`                      | Canonical form.                                           |
| `signed char` / `int8_t`    | Safe to hash; auto-promoted to `uint8_t`.                 |
| `unsigned char` / `uint8_t` | Standard byte-safe input.                                 |
| ❌ `std::byte`, `bool`       | Not supported — must use `reinterpret_cast<const char*>`. |

This policy enforces **type safety and clarity** when hashing textual data at compile time.

For POD object hashing, see `jh::pod::bytes_view`.

---

## 🧩 Summary

`jh::meta::hash` is a **constexpr hashing toolkit**
optimized for **type reflection**, **meta-registry keys**, and **compile-time mapping**.

It provides a consistent, dependency-free base layer for meta utilities,
balancing simplicity, determinism, and constexpr capability.

| Algorithm | Style       | Best For                       |
|-----------|-------------|--------------------------------|
| FNV-1a64  | Balanced    | Identifiers, short strings     |
| DJB2      | Lightweight | ASCII literals, compact tables |
| SDBM      | Legacy      | Simple text indexing           |
| Murmur64  | Avalanche   | POD and general-purpose data   |
| xxHash64  | Fast-mixed  | Large compile-time string sets |

---

> ⚠️ **Note:**
> All functions are **non-cryptographic**.
> Do **not** use them for password hashing, checksums, or security-sensitive operations.
