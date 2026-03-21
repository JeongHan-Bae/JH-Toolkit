# 🍯 **JH Toolkit — `jh::serio::uri` API Reference**

📁 **Header:** `<jh/serialize_io/uri.h>`  
📦 **Namespace:** `jh::serio::uri`  
📅 **Version:** **1.4.1+** (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::serio::uri` provides the **URI percent-encoding and decoding layer**
of the JH Toolkit — a safe and portable implementation of **RFC 3986
percent-encoding** designed for reliable transport of textual data
through URI/URL components.

The module offers two API tiers:

* **Basic APIs** — high-performance encoding/decoding without input validation.
* **Safe APIs** — strict UTF-8 validation and control-character rejection.

This design allows developers to choose between **maximum flexibility**
and **strong safety guarantees** depending on the context of the data.

All functions are **header-only**, **cross-platform**, and throw
**standard exceptions** when malformed data is encountered.

---

## 🌍 Overview

| Property            | Description                                             |
|---------------------|---------------------------------------------------------|
| **Module**          | Serialization I/O (`jh::serio`)                         |
| **Purpose**         | Encode and decode URI components using percent-encoding |
| **Specification**   | RFC 3986                                                |
| **Encoding format** | `%XX` hexadecimal byte representation                   |
| **Text-safe**       | ✅ Yes                                                   |
| **Binary support**  | Technically possible, but not recommended               |
| **Safety layer**    | Optional UTF-8 validation                               |
| **Integration**     | Uses `jh::pod::string_view` validation utilities        |

---

## 🔹 Core API

| Function                        | Description                                         |
|---------------------------------|-----------------------------------------------------|
| `encode(std::string_view)`      | Percent-encode a string without validation.         |
| `decode(std::string_view)`      | Decode percent-encoded text with syntax validation. |
| `encode_safe(std::string_view)` | Encode after verifying UTF-8 legality.              |
| `decode_safe(std::string_view)` | Decode and validate the resulting UTF-8 text.       |

---

## ✳️ Percent Encoding (`encode`)

### `encode()`

```cpp
[[nodiscard]] inline std::string encode(
    const std::string_view& input
);
```

Encodes a string into **URI percent-encoded form**.

Characters outside the **unreserved set defined by RFC 3986** are converted
into the `%XX` hexadecimal representation.

Example:

```cpp
auto encoded = jh::serio::uri::encode("hello world");
// "hello%20world"
```

### Behavior

* Unreserved characters remain unchanged.
* Reserved or unsafe characters are encoded as `%XX`.
* No UTF-8 or legality checks are performed.

### Notes

* Suitable when the caller already controls the input data.
* Allows encoding of arbitrary byte sequences.

---

## ✳️ Percent Decoding (`decode`)

### `decode()`

```cpp
[[nodiscard]] inline std::string decode(
    const std::string_view& input
);
```

Decodes percent-encoded URI text back into its original byte sequence.

Example:

```cpp
auto decoded = jh::serio::uri::decode("hello%20world");
// "hello world"
```

### Validation

The function verifies:

* Proper `%XX` structure
* Valid hexadecimal digits
* Correct sequence boundaries

### Throws

| Exception            | Condition                                 |
|----------------------|-------------------------------------------|
| `std::runtime_error` | Input contains malformed percent-encoding |

---

## 🔐 Safe API Variants

The safe variants enforce **textual legality guarantees**.

They validate that the processed data:

* Is **valid UTF-8**
* Contains **no control characters**

These checks use the utility methods from `jh::pod::string_view`.

---

### `encode_safe()`

```cpp
[[nodiscard]] inline std::string encode_safe(
    const std::string_view& input
);
```

Encodes a string into percent-encoded form **after validating its legality**.

### Validation Rules

The input must satisfy:

* Valid UTF-8 sequence
* No control characters

### Throws

| Exception            | Condition                                               |
|----------------------|---------------------------------------------------------|
| `std::runtime_error` | Input is not valid UTF-8 or contains control characters |

### Recommended Use

Use this variant when handling:

* HTTP query parameters
* URL path segments
* Web form input
* Any **external or untrusted text**

---

### `decode_safe()`

```cpp
[[nodiscard]] inline std::string decode_safe(
    const std::string_view& input
);
```

Decodes percent-encoded text and validates the resulting string.

### Validation Rules

After decoding, the output must satisfy:

* Valid UTF-8 sequence
* No control characters

### Throws

| Exception            | Condition                                                        |
|----------------------|------------------------------------------------------------------|
| `std::runtime_error` | Malformed percent-encoding                                       |
| `std::runtime_error` | Decoded output is not valid UTF-8 or contains control characters |

### Recommended Use

Use this function when decoding:

* URLs from browsers
* HTTP query strings
* External API inputs
* Any untrusted source

---

## 🧠 Design Notes

### Two-tier Safety Model

The API separates **encoding mechanics** from **semantic validation**:

| Layer     | Purpose                               |
|-----------|---------------------------------------|
| Basic API | Maximum flexibility and compatibility |
| Safe API  | Strict textual safety                 |

This avoids unnecessary overhead for trusted internal data while
still providing robust safety guarantees for external input.

---

### UTF-8 Legality Checking

The safety layer relies on:

```cpp
jh::pod::string_view::is_legal()
```

This check ensures:

* Proper UTF-8 byte sequences
* Absence of ASCII control characters

This validation cost is approximately equivalent to **a single linear
scan of the string**.

---

### Internal Implementation

The high-level API is built on optimized primitives located in:

```cpp
jh::detail::uri_common
```

These provide:

* fast percent-encoding loops
* precomputed character classification
* minimal branching during encoding/decoding

---

## 🔄 URI Percent Encoding vs Base64

Although both modules exist under `jh::serio`, they solve different problems.

| Aspect        | URI Encoding        | Base64                  |
|---------------|---------------------|-------------------------|
| Category      | Text escaping       | Binary-to-text encoding |
| Output format | `%XX` escapes       | Base64 alphabet         |
| Data model    | Primarily textual   | Binary data             |
| Output size   | Slightly larger     | ~33% larger             |
| Typical usage | URLs, query strings | binary transport        |

### Recommendation

For arbitrary binary payloads:

```cpp
jh::serio::base64
jh::serio::base64url
```

should be used instead of percent-encoding.

---

## 🧩 Summary

`jh::serio::uri` provides a **robust and standards-compliant
percent-encoding implementation** for URI and URL components.

Its dual API design allows developers to choose between:

* **Flexible encoding for internal pipelines**
* **Strict validation for external data**

| API             | Validation                   | Typical Usage     |
|-----------------|------------------------------|-------------------|
| `encode()`      | ✘ None                       | Internal encoding |
| `decode()`      | ✔ Syntax only                | General decoding  |
| `encode_safe()` | ✔ UTF-8 + control char check | User input        |
| `decode_safe()` | ✔ Full validation            | Web/HTTP data     |

The module integrates seamlessly with the JH Toolkit serialization
ecosystem and complements binary codecs such as
`jh::serio::base64` and `jh::serio::base64url`.
