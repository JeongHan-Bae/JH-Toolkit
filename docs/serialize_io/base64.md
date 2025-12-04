# 🍯 **JH Toolkit — `jh::serio::base64` API Reference**

📁 **Header:** `<jh/serialize_io/base64.h>`  
📦 **Namespace:** `jh::serio::base64` / `jh::serio::base64url`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::serio::base64` defines the **Base64 / Base64URL serialization layer**
of the JH Toolkit — a modern, constexpr-safe, and cross-platform
binary-to-text codec designed for predictable interoperability.

It implements both:

* **Base64** — standard form (RFC 4648 §4)
* **Base64URL** — URL-safe variant (RFC 4648 §5)

All functions are **header-only**, **constexpr-verified**, and
throw **clear, standard exceptions** on malformed input.

---

## 🌍 Overview

| Property        | Description                                                       |
|-----------------|-------------------------------------------------------------------|
| **Module**      | Serialization I/O (`jh::serio`)                                   |
| **Purpose**     | Convert between binary buffers and Base64/Base64URL text          |
| **Safety**      | Input-validated, exception-based error model                      |
| **Integration** | Natively interoperable with `jh::pod::bytes_view` / `string_view` |
| **Variants**    | `base64` (padded) and `base64url` (unpadded by default)           |

---

## 🔹 Core API

| Function                                                 | Description                                                                                            |
|----------------------------------------------------------|--------------------------------------------------------------------------------------------------------|
| `encode(const uint8_t* data, std::size_t len)`           | Encode raw bytes as padded Base64 text.                                                                |
| `encode(const uint8_t* data, std::size_t len, bool pad)` | Encode bytes as Base64URL text (padding optional).                                                     |
| `decode(const std::string&)`                             | Decode text to `std::vector<uint8_t>`.                                                                 |
| `decode(const std::string&, std::vector<uint8_t>&)`      | Decode text into a provided byte buffer, returning [`jh::pod::bytes_view`](../pods/bytes_view.md).     |
| `decode(const std::string&, std::string&)`               | Decode text into a provided string buffer, returning [`jh::pod::string_view`](../pods/string_view.md). |

> 🧩 **Type-based Return Policy**  
> The returned *view type* is automatically inferred from the buffer type:  
> `std::vector<uint8_t>` → [`bytes_view`](../pods/bytes_view.md),  
> `std::string` → [`string_view`](../pods/string_view.md).  
> This design yields intuitive, buffer-aware decoding behavior.  

---

## ✳️ Base64 Variant (`jh::serio::base64`)

### `encode()`

```cpp
[[nodiscard]] inline std::string encode(
    const uint8_t* data,
    std::size_t len
);
```

Encodes a binary buffer into canonical Base64 text (always `'='`-padded).

**Throws:** `std::invalid_argument` if `data` is `nullptr` while `len > 0`.

---

### 📘 Decode Overloads

#### 1️⃣ `decode(const std::string&)`

Decodes text into a new `std::vector<uint8_t>`.

```cpp
[[nodiscard]] inline std::vector<uint8_t> decode(
    const std::string& input
);
```

Performs full validation (length, character set, padding).

---

#### 2️⃣ `decode(const std::string&, std::vector<uint8_t>&)`

Decodes into a **provided byte buffer**, returning a `bytes_view` bound to it.

```cpp
inline jh::pod::bytes_view decode(
    const std::string& input,
    std::vector<uint8_t>& output_buffer
);
```

* Returned view is **non-owning**, referencing `output_buffer`.  
* Reallocation or modification of `output_buffer` invalidates the view.

---

#### 3️⃣ `decode(const std::string&, std::string&)`

Decodes directly into a text buffer, returning a `string_view`.

```cpp
inline jh::pod::string_view decode(
    const std::string& input,
    std::string& output_buffer
);
```

* The view provides immediate read-only access to decoded text.  
* It shares the lifetime of `output_buffer`.

---

## ✳️ Base64URL Variant (`jh::serio::base64url`)

### `encode()`

```cpp
[[nodiscard]] inline std::string encode(
    const uint8_t* data,
    std::size_t len,
    bool pad = false
);
```

Encodes binary data using the **URL-safe alphabet** (`'-'`, `'_'`).  
Padding (`'='`) is optional and disabled by default for JWT/Web use.

---

### 📘 Decode Overloads

All decoding overloads are identical in semantics to the Base64 variant:  
they differ only in character set and padding rules.

```cpp
inline std::vector<uint8_t> decode(const std::string& input);
inline jh::pod::bytes_view decode(const std::string& input, std::vector<uint8_t>& buffer);
inline jh::pod::string_view decode(const std::string& input, std::string& buffer);
```

---

## 🧠 Design Notes

* **Buffer-aware views:**  
  The API selects the correct lightweight view type automatically —
  `bytes_view` for binary buffers, `string_view` for text.  
  Each view provides high-utility, constexpr-safe methods such as:

    * `at<T>()`, `fetch<T>()`, `clone<T>()`, `hash()` for binary introspection.  
    * `compare()`, `starts_with()`, `ends_with()`, `find()`, `hash()` for textual data.

* **Exception model:**  
  Clear validation with `std::invalid_argument` (null input)
  and `std::runtime_error` (malformed text).

* **Performance:**  
  Constexpr lookup tables ensure branchless encode/decode
  with zero heap allocation inside the codec core.

* **Interoperability:**  
  Output is compliant with RFC 4648 and safely interchangeable
  across languages and toolchains.

---

## 🧩 Summary

`jh::serio::base64` forms the **binary-to-text foundation** of the JH Toolkit.  
Its dual namespaces (`base64`, `base64url`) offer full control over
padding, alphabet, and output safety while retaining trivial integration
with the POD and serialization layers.

| Variant     | Specification | Default Padding          | Common Use                         |
|-------------|---------------|--------------------------|------------------------------------|
| `base64`    | RFC 4648 §4   | Always padded            | General serialization, data export |
| `base64url` | RFC 4648 §5   | Optional (default = off) | JWT, HTTP, filename-safe transfer  |

---

> 💡 **Lifetime Rule**  
> Views returned by decoding overloads are **non-owning** and remain valid
> only while their backing buffer (`std::vector<uint8_t>` or `std::string`)
> stays intact. Modifying or moving the buffer invalidates the view immediately,
> ensuring predictable ownership and zero-overhead access semantics.
