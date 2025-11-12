# 🧊 **JH Toolkit — `jh::pod::string_view` API Reference**

📁 **Header:** `<jh/pods/string_view.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.5+  
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

Computes a **constexpr-safe**, deterministic 64-bit hash of the string contents
using the algorithms defined in [`jh::meta::hash`](../metax/hash.md).

```cpp
constexpr std::uint64_t
hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64) const noexcept;
```

| Parameter     | Description                                   |
|---------------|-----------------------------------------------|
| `hash_method` | Hash algorithm selector (default: `fnv1a64`). |

**Example:**

```cpp
using namespace jh::pod;

constexpr auto sv = string_view::from_literal("example");
constexpr auto h1 = sv.hash(); // FNV-1a 64-bit
constexpr auto h2 = sv.hash(jh::meta::c_hash::xxhash64);
```

**Behavior and Notes:**

* The hash is based strictly on the `len` field —
  including or excluding the trailing `'\0'` produces different results.
* Fully `constexpr` and `consteval`-safe: may be evaluated at compile time.
* Uses the same deterministic algorithms provided by [`jh::meta::hash`](../metax/hash.md).
* No allocation, no RTTI, and no platform-specific variance.
* Intended for identifiers, string literals, and compile-time symbol mapping.
* For hashing arbitrary runtime memory, use [`jh::pod::bytes_view::hash()`](bytes_view.md#-hashhash_method--fnv1a64).

**Supported Algorithms:** see
👉 [`jh::meta::hash` — Core Components](../metax/hash.md#-core-components)

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

### 🔹 `to_std()` and Explicit Conversion

Provides **interoperability with `std::string_view`** while keeping POD semantics.

```cpp
explicit constexpr operator std::string_view() const noexcept;
constexpr std::string_view to_std() const noexcept;
```

| Function                               | Description                                                        |
|----------------------------------------|--------------------------------------------------------------------|
| `explicit operator std::string_view()` | Explicit conversion (requires `static_cast` or brace-init).        |
| `to_std()`                             | Named helper; returns `std::string_view` directly, no cast needed. |

**Semantics**

* Both conversions perform **no allocation or copy** — they simply wrap the existing pointer and length.
* Pointer and size are preserved **1:1**.
* `explicit` form is used to prevent implicit conversions in overload resolution.
* `to_std()` is a convenience wrapper for readability in mixed API contexts.

**Example:**

```cpp
jh::pod::string_view sv = jh::pod::string_view::from_literal("world");
std::string_view stdv = sv.to_std();        // direct named conversion
std::string_view stdv2 = static_cast<std::string_view>(sv); // explicit cast
```

---

### 🔹 Three-Way Comparison (`operator<=>`)

Performs a **lexicographical three-way comparison**,
returning a `std::strong_ordering` consistent with **`std::string` and `std::string_view`** semantics.

```cpp
constexpr std::strong_ordering
operator<=>(const jh::pod::string_view& rhs) const noexcept;
```

**Semantics**

* Returns:

    * `std::strong_ordering::less` if `*this < rhs`
    * `std::strong_ordering::equal` if `*this == rhs`
    * `std::strong_ordering::greater` if `*this > rhs`
* Implements **lexicographic comparison** identical to `compare()`.
* Uses `std::strong_ordering` specifically to **align with the standard C++ `std::string` and `std::string_view`
  three-way comparison** behavior,
  ensuring consistent results and interoperability with standard library algorithms.

**Example:**

```cpp
using namespace jh::pod;

constexpr auto a = string_view::from_literal("abc");
constexpr auto b = string_view::from_literal("abd");

static_assert((a <=> b) == std::strong_ordering::less);
```

**Properties**

* Fully `constexpr` and `noexcept`.
* Provides **strict total ordering** identical to the standard string types.
* Automatically enables all relational operators (`<`, `<=`, `>`, `>=`).
* Guarantees bitwise consistency with `compare()` and `operator==`.

---

### 🧩 Evaluation Model

Except for `copy_to()` — which exists purely as a **debugging and interop tool** —
**all functions in `jh::pod::string_view` are `constexpr`**,
and most internally use `std::is_constant_evaluated()` to **differentiate compile-time and runtime execution**.

This dual-path design ensures:

* **Full compile-time support** (usable in `consteval` expressions).
* **Optimized runtime behavior**, leveraging `memcmp`/`memcpy` for speed.

As a result, `jh::pod::string_view` can participate seamlessly in both
**compile-time symbolic metaprogramming** and **high-performance runtime pipelines** without branching overhead.

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
