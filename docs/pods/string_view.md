# 🧊 **JH Toolkit — `jh::pod::string_view` API Reference**

📁 **Header:** `<jh/pods/string_view.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** **1.4.1+**  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

# 🏷️ Overview

`jh::pod::string_view` is a **POD-safe, deep-comparison string view** —  
a minimal, read-only representation of immutable character data.

It preserves the semantics of `std::string_view` but constrains usage
to **POD-compatible** contexts, ensuring **ABI stability**, **constexpr hashing**,
and **binary transparency**.

---

# 🔹 Definition

```cpp
struct string_view final {
    const char* data;
    std::uint64_t len;
};
```

### Key Properties

| Aspect     | Description                                |
|------------|--------------------------------------------|
| Layout     | Flat POD layout — `const char* + uint64_t` |
| Ownership  | Non-owning                                 |
| Comparison | Deep bytewise (`memcmp`) equality          |
| Hashing    | `constexpr` and `consteval`-safe           |
| Lifetime   | Must not outlive underlying memory         |
| ABI        | Stable and deterministic                   |

---

# 🔬 API Breakdown

---

# 🔹 Construction

### `from_literal(const char(&lit)[N])`

Creates a `string_view` from a string literal.

```cpp
auto sv = jh::pod::string_view::from_literal("hello");
```

| Aspect   | Description                                            |
|----------|--------------------------------------------------------|
| Behavior | Returns `{lit, N - 1}` excluding the final `'\0'`.     |
| Safety   | Always compile-time valid (`constexpr` / `consteval`). |

---

# 🔹 Basic Access

### `operator[](uint64_t index)`

Returns the character at `index`.

```cpp
char c = sv[0];
```

| Property   | Description        |
|------------|--------------------|
| Bounds     | No bounds checking |
| Complexity | O(1)               |

---

### `begin()` / `end()`

Returns raw iterators to the view.

```cpp
for (char c : sv) { ... }
```

| Function  | Description             |
|-----------|-------------------------|
| `begin()` | Pointer to first byte   |
| `end()`   | Pointer to `data + len` |

---

### `size()`

Returns the number of bytes in the view.

```cpp
std::uint64_t n = sv.size();
```

---

### `empty()`

Returns whether the view is empty.

```cpp
if (sv.empty()) { ... }
```

---

# 🔹 Substring

### `sub(offset, length = 0)`

Returns a substring view.

| Aspect   | Description                        |
|----------|------------------------------------|
| Bounds   | If `offset > len`, returns empty   |
| Sentinel | `length == 0` extends to end       |
| Safety   | Never produces out-of-range memory |

```cpp
auto hello = sv.sub(0,5);
auto tail  = sv.sub(6);
```

---

# 🔹 Comparison

### `operator==(const string_view&)`

Performs **deep bytewise comparison**.

| Rule           | Description                    |
|----------------|--------------------------------|
| Equality       | `len` equal AND contents equal |
| Implementation | `memcmp(data, rhs.data, len)`  |

---

### `compare(const string_view&)`

Lexical comparison similar to `strcmp()`.

| Return | Meaning |
|--------|---------|
| `<0`   | less    |
| `0`    | equal   |
| `>0`   | greater |

---

### Three-Way Comparison (`operator<=>`)

```cpp
constexpr std::strong_ordering
operator<=>(const string_view& rhs) const noexcept;
```

| Result    | Meaning      |
|-----------|--------------|
| `less`    | `this < rhs` |
| `equal`   | equal        |
| `greater` | `this > rhs` |

Properties:

* strict total ordering
* consistent with `std::string_view`
* automatically enables `< <= > >=`

---

# 🔹 Prefix / Suffix

### `starts_with(prefix)`

Returns `true` if the view begins with `prefix`.

### `ends_with(suffix)`

Returns `true` if the view ends with `suffix`.

Both operations compare **raw bytes**.

---

# 🔹 Search

### `find(char ch)`

Returns the index of the first occurrence.

```cpp
auto i = sv.find('o');
```

| Return | Meaning   |
|--------|-----------|
| index  | found     |
| `-1`   | not found |

---

# 🔹 Hash

### `hash(hash_method = fnv1a64)`

Computes a **constexpr-safe deterministic 64-bit hash**.

```cpp
constexpr auto h = sv.hash();
```

| Parameter     | Description        |
|---------------|--------------------|
| `hash_method` | algorithm selector |

Characteristics:

* constexpr / consteval safe
* deterministic
* byte-based

Supported algorithms are defined in:

```
jh::meta::hash
```

---

# 🔹 Character Validation

These APIs validate textual properties of the view.

---

### `is_digit()`

Checks if all characters are decimal digits.

```
0-9
```

---

### `is_number()`

Validates full decimal number syntax.

Grammar:

```
[+-]? DIGIT+ ('.' DIGIT+)? ([eE][+-]?DIGIT+)?
```

---

### `is_alpha()`

Checks if all characters are alphabetic.

```
A-Z a-z
```

---

### `is_alnum()`

Checks if characters are alphanumeric.

```
[A-Za-z0-9]
```

---

### `is_ascii()`

Returns true if every byte is within:

```
0 – 127
```

---

### `is_printable_ascii()`

Checks for printable ASCII.

```
32 – 126
```

---

### `is_legal()`

Validates that the string is composed of

* printable ASCII
* valid UTF-8 sequences

Rejects:

* invalid UTF-8
* illegal ASCII control characters

---

### `is_hex()`

Checks if the string is a valid hexadecimal sequence.

Requirements:

* length must be even
* all characters must be hex digits

---

### `is_base64()`

Checks if the string is valid Base64.

Constraints:

* length multiple of 4
* valid alphabet
* `=` padding allowed

---

### `is_base64url()`

Checks Base64URL format.

Characteristics:

* URL-safe alphabet
* optional padding

---

# 🔹 UTF-8 Utilities

### `semantic_len()`

Returns the number of **Unicode code points**.

```cpp
auto n = sv.semantic_len();
```

Notes:

* counts UTF-8 code points
* not grapheme clusters
* assumes valid UTF-8

---

# 🔹 Utilities

### `copy_to(char* buffer, uint64_t max_len)`

Copies the content into a C-style buffer.

Behavior:

* truncates to `max_len-1`
* always null-terminates

⚠️ Intended only for debug or legacy interop.

---

# 🔹 Interoperability

### Explicit conversion

```cpp
explicit operator std::string_view() const noexcept;
```

---

### `to_std()`

Named helper for conversion.

```cpp
std::string_view s = sv.to_std();
```

Characteristics:

* zero-copy
* zero-allocation

---

# 🔹 Literals

Namespace:

```cpp
jh::pod::literals
```

### `_psv`

User-defined literal for `string_view`.

```cpp
using namespace jh::pod::literals;

auto sv = "hello"_psv;
```

Properties:

* always safe
* literal storage is static
* never dangles

---

# 🧩 Evaluation Model

All functions except `copy_to()` are `constexpr`.

The implementation uses:

```cpp
std::is_constant_evaluated()
```

to distinguish between

* **compile-time execution**
* **optimized runtime paths**

This allows:

* compile-time hashing
* constexpr validation
* optimized runtime via `memcmp`

---

# 🧾 Debug Stringification

When streamed to `std::ostream`, a `jh::pod::string_view` renders its contents directly as a quoted literal:

```
string_view"Hello"
```

Example:

```cpp
std::cout << sv;
```

⚠️ Output is **not escaped** — control or non-printable characters will appear exactly as stored.

### Debug Support

The debug stringification mechanism is **not automatically included** by all headers.

It becomes available **only when the POD module is explicitly imported**:

```cpp
#include <jh/pod>
```

This header pulls in the internal debugging utilities:

```
jh/pods/stringify.h
```

Without including `<jh/pod>`, streaming operators for POD types are **not defined**.

---

### Interaction with Derived Views

`jh::pod::string_view` often appears as the return type of:

```cpp
jh::immutable_str::pod_view()
jh::meta::t_str::pod_view()
```

However:

* Including

```
<jh/immutable_str>
```

or

```
<jh/meta>
```

**does NOT automatically enable POD debug printing.**

The `std::ostream` streaming operator becomes available **only when `<jh/pod>` is included**.

---

### Stability Notice

The file

```
jh/pods/stringify.h
```

exists **purely for debugging and inspection of POD types**.

Its APIs are **not considered stable** and **may change without notice** between toolkit versions.

---

### Recommended Printing Method

For production or stable formatting, convert to `std::string_view`:

```cpp
std::cout << sv.to_std();
```

or

```cpp
std::string_view s = sv.to_std();
```

This guarantees stable behavior independent of the POD debug facilities.

---

# 🧩 Integration Notes

`jh::pod::string_view` commonly appears as the return type of:

```
jh::immutable_str::pod_view()
jh::meta::t_str::pod_view()
```

Key rules:

* non-owning
* must not outlive the underlying storage
* suitable for compile-time identifiers and symbol tables
* debug printing requires explicit inclusion of `<jh/pod>`

---

# 🧠 Summary

| Aspect     | Description                   |
|------------|-------------------------------|
| Category   | POD string view               |
| Ownership  | Non-owning                    |
| Layout     | `const char* + uint64_t`      |
| Comparison | Deep bytewise                 |
| Hashing    | constexpr safe                |
| Encoding   | ASCII / UTF-8 aware utilities |
| Printing   | raw literal form              |
| ABI        | stable POD                    |

---

> 📌 **Design Philosophy**
>
> `jh::pod::string_view` provides the semantic equivalent of `std::string_view`,
> constrained to a POD-safe ABI.
>
> Comparison and hashing are always **bytewise** to ensure deterministic
> cross-platform behavior.
>
> Unlike `bytes_view`, which represents arbitrary memory,
> `string_view` represents **immutable textual data** suitable for
> compile-time metaprogramming and symbol identifiers.
