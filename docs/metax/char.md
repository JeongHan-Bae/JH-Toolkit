# ⚗️ **JH Toolkit — `jh::meta::char` API Reference**

📁 **Header:** `<jh/metax/char.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::char` defines **constexpr-safe character semantics** for one-byte fundamental character types.  
It provides the `any_char` concept and a suite of constexpr classification and transformation utilities,
forming the foundation for **type-safe compile-time hashing**, **string reflection**, and **meta utilities**.

---

## 🔹 Overview

| Aspect                   | Description                                                                        |
|--------------------------|------------------------------------------------------------------------------------|
| **Purpose**              | Character semantics and classification for `char`, `signed char`, `unsigned char`. |
| **Implementation Model** | Pure `constexpr` C++20 utilities with no STL dependency.                           |
| **Concept**              | `any_char` — constrains templates to 1-byte fundamental character types.           |
| **Error Handling**       | All functions are `noexcept`.                                                      |
| **Compatibility**        | Works with ASCII-compatible single-byte encodings.                                 |

---

## 🔹 Core Components

| Symbol                 |   Type   | Description                                                   |
|------------------------|:--------:|---------------------------------------------------------------|
| `any_char`             | Concept  | Constrains `char`, `signed char`, and `unsigned char`.        |
| `is_alpha()`           | Function | Check for alphabetic characters (`A–Z`, `a–z`).               |
| `is_digit()`           | Function | Check for decimal digits (`0–9`).                             |
| `is_alnum()`           | Function | Check for alphanumeric characters.                            |
| `is_hex_char()`        | Function | Check for valid hexadecimal characters (`0–9`, `A–F`, `a–f`). |
| `is_base64_core()`     | Function | Check for Base64 alphabet (`A–Z`, `a–z`, `0–9`, `+`, `/`).    |
| `is_base64url_core()`  | Function | Check for Base64URL alphabet (`A–Z`, `a–z`, `0–9`, `-`, `_`). |
| `is_ascii()`           | Function | Check if value is within 7-bit ASCII range.                   |
| `is_printable_ascii()` | Function | Check if character is printable ASCII (`32–126`).             |
| `is_valid_char()`      | Function | Reject control and DEL characters.                            |
| `to_upper()`           | Function | Convert letter to uppercase.                                  |
| `to_lower()`           | Function | Convert letter to lowercase.                                  |
| `flip_case()`          | Function | Invert alphabetic case.                                       |

---

## 🔹 Concept Reference

### `any_char`

```cpp
template<typename T>
concept any_char =
    std::same_as<T, char> ||
    std::same_as<T, signed char> ||
    std::same_as<T, unsigned char>;
```

**Description:**
Defines a **concept-level constraint** for 1-byte character-semantics types,
ensuring that template parameters represent the *prototype form* of a character type
— i.e., the unqualified base type (`char`, `signed char`, `unsigned char`).

---

#### ✅ Included Types

| Type            | Description                        |
|-----------------|------------------------------------|
| `char`          | Standard character type.           |
| `signed char`   | Explicitly signed one-byte type.   |
| `unsigned char` | Explicitly unsigned one-byte type. |

#### 🚫 Excluded Types

| Type        | Reason for Exclusion                              |
|-------------|---------------------------------------------------|
| `char8_t`   | Represents UTF-8 code units, not raw bytes.       |
| `std::byte` | Binary abstraction, not semantically a character. |
| `bool`      | Logical type, not part of textual domain.         |

---

**Rationale and Type Semantics:**

* `any_char` constrains the **template’s own parameter type**, not its call-site arguments.
  That means a template like `template<any_char Char>` defines the *prototype* of character semantics.
  Inside the template, parameters such as `const Char`, `const Char*`, or `Char&` can still appear normally.

* When passing arguments (e.g., `Char&`), the compiler performs a **copy into `Char`** rather than binding a reference.
  This design provides two major benefits:

    1. ✅ The copy is trivially cheap for one-byte values and often optimized out.
    2. ✅ It enables **compile-time determinism** — all data are concrete and immutable in constexpr evaluation.

* This prototype-based constraint avoids forwarding references (`T&&`)
  and ensures predictable evaluation in `constexpr` or `consteval` contexts.

---

## 🔹 Function Reference

### `is_alpha()`

```cpp
template<any_char Char>
constexpr bool is_alpha(Char c) noexcept;
```

Checks if `c` is an alphabetic letter (`A–Z`, `a–z`).

---

### `is_digit()`

```cpp
template<any_char Char>
constexpr bool is_digit(Char c) noexcept;
```

Checks if `c` is a decimal digit (`0–9`).

---

### `is_alnum()`

```cpp
template<any_char Char>
constexpr bool is_alnum(Char c) noexcept;
```

Checks if `c` is either a letter or digit.

---

### `is_hex_char()`

```cpp
template<any_char Char>
constexpr bool is_hex_char(Char c) noexcept;
```

Checks if `c` is a valid hexadecimal digit (`0–9`, `A–F`, `a–f`).

---

### `is_base64_core()`

```cpp
template<any_char Char>
constexpr bool is_base64_core(Char c) noexcept;
```

Checks if `c` belongs to the **Base64 alphabet** (`A–Z`, `a–z`, `0–9`, `'+'`, `'/'`).

---

### `is_base64url_core()`

```cpp
template<any_char Char>
constexpr bool is_base64url_core(Char c) noexcept;
```

Checks if `c` belongs to the **Base64URL alphabet** (`A–Z`, `a–z`, `0–9`, `'-'`, `'_'`).

---

### `is_ascii()`

```cpp
template<any_char Char>
constexpr bool is_ascii(Char c) noexcept;
```

Checks whether `c` is within 7-bit ASCII (`0–127`).

---

### `is_printable_ascii()`

```cpp
template<any_char Char>
constexpr bool is_printable_ascii(Char c) noexcept;
```

Returns `true` if `c` is printable ASCII (`32–126` inclusive).

---

### `is_valid_char()`

```cpp
template<any_char Char>
constexpr bool is_valid_char(Char c) noexcept;
```

Returns `true` for non-control characters (excludes control codes and `DEL`).

---

### `to_upper()`

```cpp
template<any_char Char>
constexpr Char to_upper(Char c) noexcept;
```

Converts a lowercase alphabetic character to uppercase;
non-alphabetic characters remain unchanged.

---

### `to_lower()`

```cpp
template<any_char Char>
constexpr Char to_lower(Char c) noexcept;
```

Converts an uppercase alphabetic character to lowercase;
others remain unchanged.

---

### `flip_case()`

```cpp
template<any_char Char>
constexpr Char flip_case(Char c) noexcept;
```

Inverts the case of alphabetic characters; leaves others unchanged.

---

## 🔹 Design Notes

| Design Aspect               | Description                                                                 |
|-----------------------------|-----------------------------------------------------------------------------|
| **Prototype-based Concept** | Models the base type (`Char`) form, not call-site qualifiers.               |
| **Predictable semantics**   | Values are copied into `Char`, ensuring deterministic constexpr evaluation. |
| **Type safety**             | Prevents misuse of `std::byte`, `char8_t`, or `bool` in meta contexts.      |
| **Constexpr compliance**    | All functions are `constexpr` and usable in `consteval` scopes.             |
| **1-byte guarantee**        | Ensures consistent ABI and predictable binary layout across platforms.      |

---

## 🧩 Summary

`jh::meta::char` provides **constexpr-safe character classification utilities**
and the **`any_char` concept** for compile-time character-type safety.

It serves as the foundational layer for higher-level meta components such as
`jh::meta::hash`, ensuring all compile-time algorithms operate on
**byte-compatible and semantically consistent character types**.

| Category       | Utilities                                                     | Description             |
|----------------|---------------------------------------------------------------|-------------------------|
| Classification | `is_alpha`, `is_digit`, `is_alnum`, `is_hex_char`, `is_ascii` | Character checks        |
| Encoding-aware | `is_base64_core`, `is_base64url_core`                         | Base64 symbol detection |
| Validation     | `is_printable_ascii`, `is_valid_char`                         | ASCII range validation  |
| Transformation | `to_upper`, `to_lower`, `flip_case`                           | Case manipulation       |
