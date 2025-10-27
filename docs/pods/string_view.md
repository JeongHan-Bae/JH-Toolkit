# 🧊 **JH Toolkit — `jh::pod::string_view` API Reference**

📁 **Header:** `<jh/pods/string_view.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::string_view` is a **POD-safe, deep-comparison string view** —
a minimal, read-only representation of immutable character data.  

It preserves the semantics of `std::string_view` but constrains usage
to **POD-compatible** contexts, ensuring **ABI stability**, **constexpr hashing**,
and **binary transparency**.

---

## 🔹 Definition

```cpp
struct string_view final {
    const char* data;
    std::uint64_t len;
};
```

### Key Properties

| Aspect     | Description                                                 |
|------------|-------------------------------------------------------------|
| Layout     | Flat POD layout — `const char* + uint64_t`                  |
| Ownership  | Non-owning                                                  |
| Comparison | Deep bytewise (`memcmp`) equality                           |
| Hashing    | `constexpr` and `consteval`-safe                            |
| Lifetime   | Must not outlive underlying memory (no dangling references) |
| ABI        | Stable and deterministic                                    |

---

## 🔬 API Breakdown

### 🔹 `from_literal(const char(&lit)[N])`

Creates a `string_view` from a string literal, excluding the trailing null terminator.

```cpp
auto sv = jh::pod::string_view::from_literal("hello");
```

| Aspect   | Description                                                 |
|----------|-------------------------------------------------------------|
| Behavior | Returns `{lit, N - 1}` excluding the final `'\0'`.          |
| Safety   | Always valid at compile time (`constexpr` and `consteval`). |

---

### 🔹 `sub(offset, length = 0)`

Returns a substring view.

| Aspect   | Description                                           |
|----------|-------------------------------------------------------|
| Bounds   | If `offset > len`, returns empty.                     |
| Sentinel | If `length == 0`, extends to end.                     |
| Behavior | Truncates rather than overflows — never out of range. |

```cpp
auto hello = sv.sub(0, 5);
auto tail  = sv.sub(6);
```

---

### 🔹 `operator==(const string_view&)`

Performs **deep bytewise comparison**.

| Aspect         | Description                                          |
|----------------|------------------------------------------------------|
| Equality rule  | Returns `true` if lengths are equal and bytes match. |
| Implementation | Uses `memcmp(data, rhs.data, len)`.                  |
| Meaning        | Independent of pointer identity.                     |

---

### 🔹 `compare(const string_view&)`

Lexical comparison, similar to `strcmp()`.

| Return | Meaning      |
|--------|--------------|
| `< 0`  | `this < rhs` |
| `0`    | Equal        |
| `> 0`  | `this > rhs` |

---

### 🔹 `starts_with()` / `ends_with()`

Prefix and suffix checks based on raw bytes.

| Function              | Description                                          |
|-----------------------|------------------------------------------------------|
| `starts_with(prefix)` | Returns `true` if first `prefix.size()` bytes match. |
| `ends_with(suffix)`   | Returns `true` if last `suffix.size()` bytes match.  |

#### Details

* Matching is **strictly bytewise** — no special handling for `'\0'`.  
* Whether a terminator is compared depends on how the view was constructed:

  ```cpp
  jh::pod::string_view{"hello", strlen("hello")}   // excludes '\0'
  jh::pod::string_view{"hello", sizeof("hello")}   // includes '\0'
  ```
* To avoid ambiguity, use `from_literal()`, which **excludes the terminator automatically**.  
* `jh::immutable_str::pod_view()` follows the same rule — it never includes the final `'\0'`.

---

### 🔹 `find(char ch)`

Returns index of first occurrence of `ch`, or `-1` if not found.

```cpp
auto i = sv.find('o'); // e.g. 4
```

---

### 🔹 `hash(hash_method = fnv1a64)`

Computes a **constexpr-safe** 64-bit hash.

| Parameter     | Description                         |
|---------------|-------------------------------------|
| `hash_method` | Hash algorithm (default: `fnv1a64`) |

#### Supported algorithms

| Enum value | Algorithm     | Notes                                |
|------------|---------------|--------------------------------------|
| `fnv1a64`  | FNV-1a 64-bit | Default; robust general-purpose hash |
| `fnv1_64`  | FNV-1 64-bit  | Historical variant                   |
| `djb2`     | DJB2 classic  | Fast string hash                     |
| `sdbm`     | SDBM hash     | Used in DBM and `readdir`            |

#### Notes

* The hash depends strictly on the `len` field.  
  Including `'\0'` changes the hash result.
* `jh::pod::string_view{"hello", strlen("hello")}`
  and `jh::pod::string_view{"hello", sizeof("hello")}` produce **different** hashes.  
* Unlike `jh::pod::bytes_view`, this function is `consteval`-valid —
  not because of implementation details, but because the **semantics make sense**
  for compile-time constant strings.  
* For arbitrary runtime memory (as in `bytes_view`), compile-time hashing is nonsensical.

---

### 🔹 `copy_to(char* buffer, uint64_t max_len)`

Copies content into a C-style buffer with a null terminator.

| Aspect   | Description                                          |
|----------|------------------------------------------------------|
| Purpose  | For debug or interoperability only.                  |
| POD-safe | ❌ — Not strictly POD-safe; writes a null terminator. |
| Behavior | Truncates to `max_len - 1`, then writes `'\0'`.      |

> ⚠️ Recommended only for interop with legacy APIs.  
> Avoid in normal POD pipelines.

---

## 🧾 Debug Stringification

When streamed to an `std::ostream`,
a `jh::pod::string_view` renders its contents directly as a quoted literal:

```
string_view"Hello world"
```

Example:

```cpp
jh::pod::string_view sv = jh::pod::string_view::from_literal("Hello");
std::cout << sv; // → string_view"Hello"
```

> ⚠️ Output is **unescaped** — control or non-printable characters
> will appear as-is.  
> This is intentional: `string_view` is a **raw observation** type,
> not an owned string with formatting semantics.

---

## 🧩 Integration Notes

* `jh::pod::string_view` is commonly used as a **POD-safe view** returned from
  `jh::immutable_str::pod_view()`.  
* Like all `jh::pod` non-owning types, it must **not dangle** —
  the underlying character storage must remain valid.  
* Unlike `bytes_view`, it is intended for **semantic string views**,
  not arbitrary memory interpretation.  
* Supports compile-time usage in string-based metaprogramming
  (e.g., constexpr identifiers or symbol tables).

---

## 🧠 Summary

| Aspect     | Description                                 |
|------------|---------------------------------------------|
| Category   | POD string view                             |
| Ownership  | Non-owning                                  |
| Layout     | `const char* + uint64_t`                    |
| Comparison | Deep bytewise equality                      |
| Hashing    | Constexpr / Consteval safe                  |
| Printing   | `string_view"..."` unescaped literal output |
| Lifetime   | Must not dangle                             |
| ABI        | Stable POD structure                        |
| Consteval  | Supported (semantically meaningful)         |

---

> 📌 **Design Philosophy**
>
> `jh::pod::string_view` provides the semantic equivalent of `std::string_view`,
> constrained to a POD-safe ABI.  
> Its comparison and hashing are always **bytewise**,
> ensuring predictable behavior across toolchains and platforms.
>
> Unlike `bytes_view`, which represents *arbitrary reinterpretation of memory*,
> `string_view` embodies the semantics of **immutable text** —  
> making compile-time hashing, literal binding, and deep equality
> both valid and well-defined.
