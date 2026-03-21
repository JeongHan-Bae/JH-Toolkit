# ⚗️ JH Toolkit — `jh::meta::t_str` / `TStr` API Reference

📁 **Header:** `<jh/metax/t_str.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** **1.4.x** (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`


<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::t_str<N>` (name `t_str` and alias `TStr` are shortened forms of "Template String Literal") is a **C++20
compile-time string wrapper** designed to make
**string literals usable as non-type template parameters (NTTP)**.

It stores the literal in a fixed-size `jh::pod::array<char, N>`, **always including
the null terminator**, and exposes a rich set of constexpr utilities for:

* compile-time validation
* compile-time transformation
* compile-time concatenation
* compile-time hashing
* safe bridging between strings and byte buffers

`TStr` is a convenience alias that enables direct NTTP usage without explicitly
spelling the size parameter.

> `t_str` is **not** a runtime string abstraction and **not** a replacement for
> `std::string`. It exists exclusively to support compile-time metaprogramming.

---

## 🎯 Motivation

Before C++20, string literals could not be bound directly as template arguments.
Associating string-based metadata with types required indirection via:

* virtual or static member functions
* trait specializations
* external registries

`t_str` eliminates this indirection entirely:

```cpp
template<jh::meta::TStr Name>
struct field {};
```

```cpp
using id = field<"user_id">;
```

The literal becomes part of the **type identity itself**, validated and fixed at
template instantiation time.

---

## 🌍 Core Capabilities

| Capability                      | Description                                                  |
|---------------------------------|--------------------------------------------------------------|
| **Direct NTTP binding**         | String literals can be used directly as template arguments   |
| **Structural equality**         | Identical literals instantiate identical templates           |
| **Compile-time validation**     | Digit, number, ASCII, UTF-8 legality, hex, Base64, Base64URL |
| **Compile-time hashing**        | FNV-1a, DJB2, SDBM, Murmur64, xxHash64                       |
| **Compile-time transformation** | Uppercase, lowercase, case flipping                          |
| **Safe concatenation**          | constexpr concatenation with a global 16 KB limit            |
| **Byte bridging**               | Explicit, zero-overhead conversion to/from byte arrays       |
| **Zero runtime cost**           | No allocation, no hidden state, no runtime checks            |

---

## 📦 Basic Usage

### Using `TStr` as a template parameter

```cpp
template<jh::meta::TStr S>
struct tag {};

using a = tag<"Hello">;
using b = tag<"Hello">;
```

`a` and `b` are **the same type**.

This is a fundamental property:
**template identity is derived from the literal content, not from an address**.

---

## 🧠 Structural Template Equality

`t_str / TStr` provides **structural equality** for template parameters.

* Two identical string literals → **same template instantiation**
* No manual comparison logic required
* No dependency on pointer identity or linkage

This is in sharp contrast to templates based on `constexpr const char*`:

```cpp
template<const char* S>
struct bad_tag {};
```

Such templates:

* depend on object identity, not content
* often require extra traits or comparisons
* break easily across translation units or refactors

`t_str` avoids all of these issues by embedding the characters directly into the
template argument.

---

## 🔍 Accessors & Views

```cpp
constexpr jh::meta::t_str s{"Hello"};
```

| API          | Description                                       |
|--------------|---------------------------------------------------|
| `val()`      | Returns `const char*` (null-terminated)           |
| `size()`     | Length excluding the null terminator              |
| `view()`     | `std::string_view` over the stored characters     |
| `pod_view()` | `jh::pod::string_view` over the stored characters |
| `str()`      | Converts to `std::string`                         |


`pod_view()` provides a POD-compatible view type used across JH Toolkit POD
containers and lookup structures.
> Note: Introduced in version 1.4.1+

---

## 🔐 Compile-time Validation

All validation functions are `constexpr` and suitable for `static_assert` or
`requires` clauses.

### Character-class predicates

```cpp
s.is_digit();
s.is_alpha();
s.is_alnum();
s.is_ascii();
s.is_printable_ascii();
```

### Numeric semantics

```cpp
t_str{"-12.5e+3"}.is_number(); // true
```

### Encoding checks

```cpp
s.is_hex();
s.is_base64();
s.is_base64url();
```

### UTF-8 / printable legality

```cpp
s.is_legal();
```

> Note: Introduced in version 1.4.1+

This function validates that the string is composed of printable ASCII or valid
UTF-8 byte sequences.

### POSIX relative path validation

```cpp
s.is_valid_relative_path();
s.is_valid_relative_path<true>();
```

> Note: Introduced in version 1.4.1+

Validates a strict POSIX-style relative path.

Compiler requirement:

* **Clang**
* **GCC 14+**

GCC 13 contains a known `constexpr` evaluation defect affecting NTTP string
instances where the compiler may incorrectly report that `this` does not exist
during evaluation.
Because this behavior is nondeterministic (sometimes compiling, sometimes failing),
the API is intentionally disabled under GCC 13 to prevent unstable development
behavior.

Invalid literals can be rejected **at compile time**, before they propagate into
other templates.

See [`jh::meta::base64`](base64.md) for Base64 / Base64URL encoding and decoding utilities.

---

## 🔧 Compile-time Transformation

```cpp
constexpr auto a = t_str{"AbC"};
constexpr auto b = a.to_lower();   // "abc"
constexpr auto c = a.to_upper();   // "ABC"
constexpr auto d = a.flip_case();  // "aBc"
```

Each operation returns a **new `t_str`**, fully evaluated at compile time.

---

## ➕ Compile-time Concatenation

```cpp
constexpr auto x = t_str{"Hello"};
constexpr auto y = t_str{"World"};
constexpr auto z = x + y; // "HelloWorld"
```

Rules:

* The left-hand null terminator is ignored
* A new null terminator is appended
* Total size must not exceed **16 KB**
* Violations cause **compile-time failure**

---

## 🔎 Compile-time Substrings

```cpp
constexpr auto s = t_str{"HelloWorld"};

constexpr auto a = s.sub<0,5>();        // "Hello"
constexpr auto b = s.sub<5>();          // "World"

auto v1 = s.sub_view<0,5>();
auto v2 = s.sub_pod_view<5>();
```

> Note: Introduced in version 1.4.1+

The substring APIs allow extracting compile-time substrings either as a new
`t_str` object or as lightweight views.

**Important lifetime rule**

Never obtain a view directly from a temporary result such as:

```cpp
s.sub<0,5>().view(); // illegal: dangling view
s.sub<0,5>().pod_view(); // illegal: dangling view
(x + y).view(); // illegal: dangling view
```

Doing so creates a temporary `t_str` whose storage immediately expires,
resulting in a **dangling view**.

Recommended pattern:

```cpp
constexpr auto result_str = s.sub<0,5>();
auto view = result_str.pod_view();
```

---

## 🔑 Compile-time Hashing

```cpp
constexpr std::uint64_t id =
    t_str{"user_name"}.hash(); // default: jh::meta::c_hash::fnv1a64
```

See: [`jh::meta::hash`](hash.md) for details.

Supported algorithms (selectable via `jh::meta::c_hash` enum):

* `fnv1a64` (default)
* `fnv1_64`
* `djb2`
* `sdbm`
* `murmur64`
* `xxhash64`

(`murmur64` and `xxhash64` are designed for compile-time use instead of full-complexity runtime variants.)
Optional inclusion of the null terminator is supported.

If you need compile-time switch-like lookups based on string keys, consider using
[`jh::meta::lookup_map`](lookup_map.md). `jh::meta::lookup_map` with [`jh::pod::string_view`](../pods/string_view.md)
keys can support transparent lookups from `t_str` (constructed canonical `string_view`s from `t_str` instances).
`jh::meta::lookup_map` can avoid hash collisions as it checks `hash` and key equality both at compile time and runtime.

---

## 🔄 Bridging Between Strings and Bytes

### String → Bytes

```cpp
constexpr auto bytes =
    t_str{"ABC"}.to_bytes();
// jh::pod::array<std::uint8_t, 3>
```

* Excludes the null terminator
* Suitable for hashing, Base64 encoding, and binary protocols

---

### Bytes → String (⚠️ Correct Usage)

To reconstruct a `t_str` from a byte buffer, **the size must be expressed explicitly**:

```cpp
constexpr auto restored =
    jh::meta::t_str<bytes.size() + 1>::from_bytes(bytes);
```

#### Why this matters

* `bytes` does **not** contain a null terminator
* `t_str<N>` **always does**
* Declaring `N` as `bytes.size() + 1` guarantees correctness

This pattern remains correct when:

* `bytes` is refactored
* `bytes` is produced by macros
* `bytes` is passed through intermediate constexpr functions

It prevents silent size mismatches and keeps the type definition structurally sound.

---

## ⚖️ Equality Semantics

```cpp
static_assert(t_str{"A"} == t_str{"A"});
static_assert(t_str{"A"} != t_str{"B"});
```

* If sizes differ, comparison short-circuits to `false`
* If sizes match, all characters (including `\0`) are compared
* The operation is constexpr-friendly and trivially optimizable

---

## 🧩 Design Constraints

* Maximum supported size: **16 KB**
* Intended for **string literals only**
* Always null-terminated internally
* No dynamic allocation
* UTF-8 is validated for legality, not semantic meaning

---

## 🧠 Position Within JH Toolkit

`t_str / TStr` is **one of the most important components introduced in JH Toolkit 1.4.0+**.

It enables and underpins a large portion of the compile-time infrastructure:

* compile-time Base64 / Base64URL codecs
* string-keyed lookup tables
* NTTP-driven variant and ADT systems
* protocol constants and identifiers
* static metadata propagation across templates

Rather than being confined to `jh::meta`,
`t_str` serves as a **foundational abstraction across the entire toolkit** for
any feature that requires **string identity at compile time**.

---

## 🧠 Summary

* `t_str / TStr` enables string literals as first-class NTTPs
* Template identity is derived from **content**, not pointer identity
* Compile-time validation, transformation, concatenation, substring extraction,
  and hashing are built in
* Explicit `bytes.size() + 1` sizing ensures correctness when reconstructing strings
* Introduced in **JH Toolkit 1.4.0+** as a cornerstone for compile-time string-based design
