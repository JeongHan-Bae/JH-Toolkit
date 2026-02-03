# 🍯 JH Toolkit — `jh::serio::huffman` API Reference

📁 **Header:** `<jh/serialize_io/huffman.h>`  
📦 **Namespace:** `jh::serio`  
📅 **Version:** 1.4.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::serio::huffman` is a **binary Huffman compression codec** providing
high-performance encoding and decoding with optional **canonical Huffman**
support.

It is designed for **actual data compression**, producing compact binary
streams rather than textual encodings.

The codec is implemented as a single class template parameterized by:

* a **compile-time stream signature** (`jh::meta::TStr`)
* a **codec variant** (`huff_algo`)

This enables strict format validation, deterministic decoding behavior,
and selectable symbol ranges (ASCII or full byte).

---

## 🔑 Stream Signature (`jh::meta::TStr`)

The first template parameter, `Signature`, is a compile-time string type
[`jh::meta::TStr`](../metax/t_str.md).

The signature is written **verbatim at the beginning of every compressed
stream** and validated during decompression.

### ✔ String-literal shortcut (recommended)

`TStr` supports **direct use of string literals** as non-type template
parameters:

```cpp
using HUF =
    jh::serio::huffman<"demo", huff_algo::huff256_canonical>;
```

No explicit `TStr` construction is required.

### ⚠ Consistency rule

The same `Signature` **must** be used for both `compress()` and
`decompress()`.

A mismatch causes:

```text
std::runtime_error("Bad signature")
```

---

## Huffman Variants (`huff_algo`)

```cpp
enum class huff_algo : std::uint8_t {
    huff128,
    huff256,
    huff128_canonical,
    huff256_canonical
};
```

### 🌍 Feature Matrix

| Variant             | Symbols | Encoding | Decoding                | Best Use                       |
|---------------------|---------|----------|-------------------------|--------------------------------|
| `huff128`           | 0–127   | Fast     | Tree traversal (slower) | Legacy ASCII formats           |
| `huff128_canonical` | 0–127   | Fast     | Table-based (very fast) | High-performance ASCII         |
| `huff256`           | 0–255   | Fast     | Tree traversal (slower) | General binary data            |
| `huff256_canonical` | 0–255   | Fastest  | Table-based (very fast) | High-throughput binary streams |

---

## 🧩 Canonical vs Tree-Based Huffman

### Canonical Huffman

* Stores **code-length tables**
* Generates deterministic prefix codes
* Enables table-driven **O(1)** decoding
* Recommended for performance-critical use

### Tree-Based Huffman

* Stores full frequency tables
* Preserves traditional Huffman structure
* Requires tree traversal during decoding
* Intended for compatibility or debugging

---

## 📦 Generated Stream Format

### Canonical variants

```
[ Signature ]
[ code-length table (N bytes) ]
[ total bit count (uint64) ]
[ compressed bitstream ]
```

### Tree-based variants

```
[ Signature ]
[ frequency table (N × uint32) ]
[ total bit count (uint64) ]
[ compressed bitstream ]
```

Where `N` is 128 or 256 depending on the algorithm.

---

## ⚠ Binary Stream Requirement (Important)

**Huffman compression produces arbitrary binary data.**
The output is **not text-safe**.

### ✔ Recommended stream types

Use one of the following:

```cpp
std::stringstream ss(
    std::ios::in | std::ios::out | std::ios::binary);
```

or:

```cpp
std::ifstream / std::ofstream
```

opened explicitly with:

```cpp
std::ios::binary
```

### ❌ Do NOT use text-mode streams

Using text-mode streams may corrupt:

* embedded `'\0'` bytes
* high-bit data
* platform-dependent newline translations

This applies equally to:

* in-memory streams
* file streams

---

## 🔹 Template API

```cpp
template<jh::meta::TStr Signature,
         huff_algo Algo = huff_algo::huff256_canonical>
class huffman;
```

### Static Member Functions

| Function                                    | Description                                                            |
|---------------------------------------------|------------------------------------------------------------------------|
| `compress(std::ostream&, std::string_view)` | Writes signature, metadata, bit count, and compressed binary bitstream |
| `decompress(std::istream&) -> std::string`  | Validates signature and reconstructs original data                     |

### Exceptions

* `std::runtime_error("Bad signature")`
* `std::runtime_error("ASCII only")`
* `std::runtime_error("huffman code length exceeds 32 bits")`

---

## 🚀 Usage Example

```cpp
using jh::serio::huff_algo;
using HUF =
    jh::serio::huffman<"demo", huff_algo::huff256_canonical>;

std::stringstream ss(
    std::ios::in | std::ios::out | std::ios::binary);

HUF::compress(ss, "hello world");

ss.seekg(0);

std::string recovered = HUF::decompress(ss);
std::cout << recovered << std::endl;
```

---

## 🔄 Huffman vs Base64

Although both belong to `jh::serio`, **Huffman and Base64 serve completely
different purposes**.

### ❗ Fundamental difference

| Aspect      | Huffman            | Base64                  |
|-------------|--------------------|-------------------------|
| Purpose     | Data compression   | Binary-to-text encoding |
| Output size | Smaller than input | ~33% larger             |
| Output type | Arbitrary binary   | ASCII text              |
| Text-safe   | ❌ No               | ✅ Yes                   |
| Reversible  | ✅ Yes              | ✅ Yes                   |

### When to use **Huffman**

Use `jh::serio::huffman` when:

* You want **actual compression**
* You control the storage or transport layer
* Binary data is acceptable (files, sockets, IPC)
* Performance and size matter

### When to use **Base64**

Use [`jh::serio::base64`](base64.md) when:

* Binary data must be embedded in **text-only formats**
  (JSON, XML, HTTP headers, logs)
* Human readability or portability is required
* Size expansion is acceptable

### ❌ Do not confuse the two

* Huffman **reduces size but produces binary**
* Base64 **increases size but guarantees text safety**

They are complementary tools, not alternatives.

---

## Summary

* `jh::serio::huffman` is a **true binary compression codec**
* Always use **binary-mode streams**
* Canonical variants provide deterministic, high-speed decoding
* `TStr` signatures ensure strict format validation
* Huffman and Base64 solve **fundamentally different problems**
