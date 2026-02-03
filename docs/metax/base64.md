# ⚗️ **JH Toolkit — `jh::meta::base64` API Reference**

📁 **Header:** `<jh/metax/base64.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** **1.4.x** (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::base64` provides **compile-time Base64 and Base64URL codecs**
for the `jh::meta` subsystem.

All operations are evaluated entirely at **compile time** using
non-type template parameters (`NTTP`) based on `TStr`.

* **Encoding** always produces a `TStr` (compile-time string literal).
* **Decoding** always produces a `jh::pod::array<std::uint8_t, N>` (byte data).

Invalid Base64 input is rejected at **compile time** via `TStr` constraints,
resulting in compilation failure rather than runtime errors.

---

## 🌍 Capabilities

| Feature               | Description                                                                   |
|-----------------------|-------------------------------------------------------------------------------|
| Base64 / Base64URL    | Implements RFC 4648 §4 (Base64, padded) and §5 (Base64URL, optional padding). |
| Compile-time decode   | Decode Base64 literals into byte arrays at compile time.                      |
| Compile-time encode   | Encode constexpr byte arrays into Base64/Base64URL literals.                  |
| Structural validation | `TStr` enforces Base64 / Base64URL rules during compilation.                  |
| Deterministic sizing  | Exact decoded sizes are computed at compile time.                             |
| Full round-trips      | Supports complete string ↔ bytes ↔ string transformations at compile time.    |

---

## 🔹 Decoding (TStr → Bytes)

### Base64 decoding

```cpp
template<TStr S>
requires S.is_base64()
constexpr auto decode_base64();
```

Decodes a Base64-encoded `TStr` literal into:

```cpp
jh::pod::array<std::uint8_t, N>
```

The output size `N` is computed exactly at compile time.

#### Example

```cpp
constexpr auto bytes =
    jh::meta::decode_base64<"SGVsbG8=">(); // "Hello"
```

---

### Base64URL decoding

```cpp
template<TStr S>
requires S.is_base64url()
constexpr auto decode_base64url();
```

Supports both padded and unpadded Base64URL literals.

---

## 🔹 Encoding (Bytes → TStr)

### Base64 encoding (padded)

```cpp
template<std::uint16_t N>
constexpr auto encode_base64(
    const jh::pod::array<std::uint8_t, N>& bytes
);
```

Returns a **padded Base64** `TStr<M>` with a built-in null terminator.

---

### Base64URL encoding (padding via type tag)

```cpp
template<std::uint16_t N, class PadT = std::false_type>
constexpr auto encode_base64url(
    const jh::pod::array<std::uint8_t, N>& bytes,
    PadT = {}
);
```

Padding is controlled at the **type level**:

| Form                                         | Result             |
|----------------------------------------------|--------------------|
| `encode_base64url(bytes)`                    | Unpadded Base64URL |
| `encode_base64url(bytes, std::false_type{})` | Unpadded Base64URL |
| `encode_base64url(bytes, std::true_type{})`  | Padded Base64URL   |

This design ensures that encoded length and layout are known during
template instantiation.

---

## 🔹 Bridging `TStr` and Byte Arrays

[`jh::meta::t_str`](t_str.md) provides explicit utilities to convert between
compile-time strings and byte buffers.

### String → Bytes

```cpp
constexpr jh::meta::t_str str{"Hello"};
constexpr auto bytes = str.to_bytes();
```

Produces:

```cpp
jh::pod::array<std::uint8_t, N>
```

---

### Bytes → String

```cpp
constexpr auto restored =
    jh::meta::t_str<bytes.size() + 1>::from_bytes(bytes);
```

This enables round-trips between textual and binary representations
entirely at compile time.

---

## 🔁 Complete Compile-time Base64 Round-trip

The following demonstrates the **full compile-time Base64 pipeline**:

```cpp
// Decode Base64 literal → bytes
constexpr auto bytes =
jh::meta::decode_base64<"SGVsbG8=">();

// Encode bytes → Base64 literal
constexpr auto encoded =
jh::meta::encode_base64(bytes);

// Convert Base64 literal → bytes again
constexpr auto decoded =
jh::meta::decode_base64<encoded>();
```

All steps are evaluated during compilation, with no runtime cost.

---

## 🔄 Relationship to Runtime Base64

This module uses the **same underlying algorithms** as the runtime codec,
implemented in `base64_common`.

However, the **usage model differs**:

| Aspect          | `jh::meta::base64`                 | Runtime Base64   |
|-----------------|------------------------------------|------------------|
| Evaluation      | Compile time                       | Runtime          |
| Input           | `TStr`, constexpr arrays           | Buffers, strings |
| Padding control | `std::true_type / std::false_type` | `bool`           |
| Error handling  | Compilation failure                | Exceptions       |

For runtime encoding/decoding, see [`jh::serio::base64`](../serialize_io/base64.md) and
[`jh::serio::base64url`](../serialize_io/base64.md).

---

## 🧠 Summary

* `jh::meta::base64` implements **fully compile-time Base64 / Base64URL**
* Encoding always yields `TStr`
* Decoding always yields `jh::pod::array<std::uint8_t, N>`
* `TStr` ↔ byte array conversions are explicit and supported
* Uses the same core logic as the runtime codec
* Intended for static assets, protocol constants, and NTTP-based metaprogramming
