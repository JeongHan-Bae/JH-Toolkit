# 📦 **JH Toolkit — `jh::utils::base64` API Reference**

📁 **Header:** `<jh/utils/base64.h>`  
📦 **Namespace:** `jh::utils::base64`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Utils](https://img.shields.io/badge/%20Back%20to%20Utils-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::utils::base64` provides **standard Base64 serialization and deserialization utilities** —
the first officially supported data representation system within the JH Toolkit.  

It enables safe, reversible encoding of binary data to a Base64 string,
and decoding back to `POD`-safe memory buffers or textual views.  

This component is a **header-only runtime system**,
with compile-time verified lookup tables for decoding.  

---

## 🔹 Overview

| Aspect                   | Description                                                             |
|--------------------------|-------------------------------------------------------------------------|
| **Purpose**              | Convert between raw binary data and Base64 textual representation.      |
| **Implementation Model** | Header-only, runtime algorithms with compile-time validated tables.     |
| **Compatibility**        | Works seamlessly with `jh::pod::bytes_view` and `jh::pod::string_view`. |
| **Error Handling**       | Invalid Base64 input throws `std::runtime_error` — never UB.            |
| **Planned Extensions**   | `Base64URL` variant planned for 1.4.x.                                  |

---

## 🔹 Core Constants

| Symbol             | Description                                                                    |
|--------------------|--------------------------------------------------------------------------------|
| `kBase64Chars`     | Canonical Base64 character set (`A–Z`, `a–z`, `0–9`, `+`, `/`).                |
| `is_base64_char()` | Validates whether a character belongs to the Base64 alphabet or padding `'='`. |
| `kDecodeTable`     | A precomputed lookup table (generated at compile time) used for decoding.      |

---

## 🔹 API Reference

### `encode()`

```cpp
[[maybe_unused]] inline std::string encode(
    const uint8_t* data,
    std::size_t len
) noexcept;
```

Encodes raw binary data into a Base64 string.

| Parameter | Type             | Description                   |
|-----------|------------------|-------------------------------|
| `data`    | `const uint8_t*` | Pointer to the source buffer. |
| `len`     | `std::size_t`    | Number of bytes to encode.    |

**Returns:**
A `std::string` containing a valid Base64-encoded text with proper `'='` padding.  

**Notes:**

* Produces canonical Base64 output compatible with all decoders.  
* Operates entirely at runtime, using the static `kBase64Chars` lookup.  
* Zero-copy safe: output constructed directly from encoded buffer.  

---

### `decode(const std::string&)`

```cpp
[[maybe_unused]] inline std::vector<uint8_t> decode(
    const std::string& input
);
```

Decodes a Base64 string into a newly allocated binary buffer.  

**Returns:**  
A `std::vector<uint8_t>` containing the decoded bytes.  

**Behavior:**

* Throws `std::runtime_error` for invalid input (bad length, padding, or characters).  
* Buffer ownership is transferred to the caller.  
* Ensures strict validation of every symbol.  

---

### `decode(const std::string&, std::vector<uint8_t>&)`

```cpp
[[maybe_unused]] inline jh::pod::bytes_view decode(
    const std::string& input,
    std::vector<uint8_t>& output_buffer
);
```

Decodes Base64 into a **user-provided** byte buffer,  
returning a [`jh::pod::bytes_view`](../pods/bytes_view.md) referencing the result.  

| Parameter       | Type                    | Description                            |
|-----------------|-------------------------|----------------------------------------|
| `input`         | `std::string`           | Base64 input string.                   |
| `output_buffer` | `std::vector<uint8_t>&` | Target buffer; cleared before writing. |

**Returns:**  
A lightweight `bytes_view` referencing the decoded region.  

**Notes:**  

* Zero-copy: no extra allocation beyond `output_buffer`.  
* Lifetime of the returned view equals that of the buffer.  
* Ideal for use in low-level POD or network decoding routines.  

---

### `decode(const std::string&, std::string&)`

```cpp
[[maybe_unused]] inline jh::pod::string_view decode(
    const std::string& input,
    std::string& output_buffer
);
```

Decodes Base64 into a `std::string`,
returning a [`jh::pod::string_view`](../pods/string_view.md) of the decoded text.

| Parameter       | Type           | Description                     |
|-----------------|----------------|---------------------------------|
| `input`         | `std::string`  | Base64-encoded text.            |
| `output_buffer` | `std::string&` | Target string for decoded data. |

**Returns:**  
A `string_view` pointing into the decoded content.  

**Notes:**  

* Best suited for UTF-8 textual payloads.  
* Returned view invalidated if the buffer changes.  
* Compatible with POD-level string abstractions.  

---

## 🧩 Summary

`jh::utils::base64` implements the **canonical Base64 text serialization layer**
for all binary-safe and POD-compatible systems within the JH Toolkit.  

It is designed as a **safe, portable runtime utility**
with compile-time validation of the decoding map, ensuring reliability across toolchains.  

Future versions (≥1.4.0) will introduce **Base64URL** support
and optional constexpr-mode encoding for small buffers.  

---

> **Note:**
> `jh::utils::base64` is the **only officially supported serialization system**
> in the 1.3.x series of JH Toolkit.  
> Use it for binary encoding, textual interchange, and POD-safe data transfer.  
