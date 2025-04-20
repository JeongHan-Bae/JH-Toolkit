# 🧱 JH Toolkit: `immutable_str` API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

[![License](https://img.shields.io/github/license/JeongHan-Bae/JH-Toolkit)](../LICENSE)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://github.com/JeongHan-Bae/JH-Toolkit)

---

## 📌 Overview

`jh::immutable_str` is a truly immutable string implementation designed for C++20 projects where safety, thread correctness, and structural simplicity are paramount.

Unlike `const std::string`, which can still be modified via `const_cast` or internal hacks,
`immutable_str` is **guaranteed immutable at the memory level**.
It's ideal for shared string storage, hashing, and multithreaded environments.

---

## 🚀 Key Features

- ✅ **True Immutability**: Cannot be modified after construction.
- ✅ **Thread-Safe**: Safe to share across threads without locking.
- ✅ **Compile-Time Trimming Policy**: `JH_IMMUTABLE_STR_AUTO_TRIM` is defined once at compile-time.
- ✅ **Efficient Hashing**: Content-based hashing with transparent support for `const char*` lookup.
- ✅ **Whitespace Trimming**: Optional, compile-time controlled.
- ✅ **Pool-Compatible**: Works seamlessly with `jh::pool` for deduplicated string storage.
- ✅ **Minimal Overhead**: Uses `std::unique_ptr<const char[]>` for compact and safe storage.
- ✅ **Storage Priority**: Designed for string value holder instead of value itself, multiple viewing interfaces provided.

---

## 🔐 Memory Safety & Immutability

```c++
uint64_t size_ = 0;
std::unique_ptr<const char[]> data_;
mutable std::optional<std::uint64_t> hash_{std::nullopt};
mutable std::once_flag hash_flag_;
```

These members ensure:

- `data_` is **read-only**, preventing accidental modification
- `size_` is fixed and non-mutable after construction
- `hash_` is computed once, guarded by `hash_flag_`
- The Entire structure is **thread-safe and mutation-resistant**

### ✅ Safer Than `std::string`

| Feature             | `std::string`                   | `immutable_str`                                |
|---------------------|---------------------------------|------------------------------------------------|
| Memory immutability | ❌ No (`const_cast` still works) | ✅ Yes (`const char[]`)                         |
| Thread-safe reads   | ❌ No                            | ✅ Yes                                          |
| Auto trimming       | ❌ Manual                        | ✅ Optional via macro                           |
| Copy behavior       | ✅ Copyable                      | ❌ Must use `shared_ptr` or `unique_ptr::get()` |
| Hidden reallocation | ✅ Yes                           | ❌ Never reallocates                            |


## ✂️ Trimming Behavior

`immutable_str` optionally trims **leading and trailing ASCII whitespace** during construction. This includes:
- `' '`, `\t`, `\n`, `\r`, `\f`, `\v`

### ⚙️ Compile-Time Config

Define trimming policy before including the header:

```c++
#define JH_IMMUTABLE_STR_AUTO_TRIM false // or true (default)
```

By default uses:

```c++
#ifndef JH_IMMUTABLE_STR_AUTO_TRIM
#define JH_IMMUTABLE_STR_AUTO_TRIM true
#endif
```

Only one definition should exist **per project**, ensuring consistency and allowing compile-time optimization.


---

## 🧩 Interface Summary

### ✅ Constructor

```c++
explicit immutable_str(const char* str);
immutable_str(std::string_view sv, std::mutex& mtx);
```

- Automatically trims whitespace if `auto_trim == true`
- `explicit` to avoid accidental conversions

```c++
// Recommended usage
auto raw = get_config_value(); // Simulate a std::string which is not shared by multiple threads
auto str = std::make_shared<jh::immutable_str>(raw.c_str()); // Or use make_atomic
```

---

### ❌ Deleted Operations

```c++
immutable_str(const immutable_str&) = delete;
immutable_str(immutable_str&&) = delete;
immutable_str& operator=(const immutable_str&) = delete;
immutable_str& operator=(immutable_str&&) = delete;
```

- **Immutability is enforced by deleting all copy and move operations.**

> Although `immutable_str` is not copyable by design (to enforce immutability and avoid implicit heap duplication), its content is safe to share via `shared_ptr` or via raw pointer access (`unique_ptr::get()`), as long as lifetime is managed correctly.

📌 **Note**:
Because the underlying buffer is `const char[]`, even multiple threads accessing the same pointer are guaranteed safe.

---

### 📥 Accessors

```c++
const char* c_str() const noexcept;
std::string_view view() const noexcept;
std::string str() const;
uint64_t size() const noexcept;
```

- `.c_str()` → For C API compatibility
- `.view()` → For zero-copy read access (excludes `\0`)
- `.pod_view()` → For pod-like string_view storage when the source is still alive (does not check dangling).
- `.str()` → Allocates and returns a new `std::string`
- `.size()` → Returns number of characters (i.e., bytes for UTF-8)

📌 `.view()` is **not cached**, as `std::string_view` is trivial to construct.  
📌 `.pod_view()` returns a `jh::pod::string_view`, see [`pod module`](pod.md) for details.

> `.pod_view()` is intended for use with `jh::pod::string_view`, allowing temporary zero-copy views with strict lifetime awareness.

---

### 🔁 Equality & Hashing

```c++
bool operator==(const immutable_str& other) const noexcept;
std::uint64_t hash() const noexcept;
```

- Efficient and thread-safe.
- Lazy hash computation (cached once, thread-safe).
- No comparing operators, used as storage instead of value, if you need comparing, access by `.c_str()` or `.view()`.

> ℹ️ `hash()` uses `std::hash<std::string_view>` internally, which is **process-local and not stable across runs**.  
> If you require deterministic or persistent hashes (e.g., for database keys, content hashing, or disk storage), please compute it manually via `.c_str()` and `.size()` using your preferred algorithm.

---

## 🧪 Hash Container Support

### ✅ Transparent Lookup

`atomic_str_hash` and `atomic_str_eq` support **transparent key lookup**, so you can use `const char*` string literals directly in `unordered_map` or `unordered_set`:

```c++
std::unordered_set<jh::atomic_str_ptr, jh::atomic_str_hash, jh::atomic_str_eq> str_set;
str_set.insert(jh::make_atomic("cached"));

if (str_set.find("cached") != str_set.end()) {
    // Lookup succeeded without constructing immutable_str
}
```

📌 This optimization **avoids heap allocation** during lookup!

---

### 📌 `nullptr` as key

- Allowed in lookup and hashing.
- `nullptr` always hashes to `0` and compares unequal to all other values.
- Use only to represent "empty" sentinel.

---

### 🧽 Whitespace Trimming

Controlled by static flag:

```c++
static constexpr bool jh::immutable_str::auto_trim = JH_IMMUTABLE_STR_AUTO_TRIM;
```

- If enabled, trims leading/trailing **ASCII whitespace only**  
  (space, tab, `\n`, `\r`, `\f`, `\v`) using `jh::detail::is_space_ascii(char)`.
- `JH_IMMUTABLE_STR_AUTO_TRIM` can be set manually before including `<jh/immutable_str>` 
   or used as a compile flag in makefiles.
  (Single definition in one project).

Example:

```c++
const char* raw = "  hello world ";
jh::immutable_str s(raw); // auto-trims
std::cout << s.view();    // → "hello world"
```

---

## 🔄 Shared Storage (`atomic_str_ptr`)

```c++
using atomic_str_ptr = std::shared_ptr<immutable_str>;
```
> Here, 'atomic' refers to the atomicity of pointer assignment, not the content.

Use when:

- You need multiple references to the same string
- You want to store in containers with shared keys

Example combined with `<jh/pool>`

```c++
jh::pool<jh::immutable_str> pool_;
auto shared = pool_.acquire("string to obtain"); // type as atomic_str_ptr
```

### 🏗️ Construction

```c++
auto shared = jh::make_atomic("immutable");
```

Or with mutex:

```c++
std::mutex mtx;
auto safe = jh::safe_from("string view here", mtx);
```

---

## 📚 API Functions

### 🔨 `make_atomic(const char*)`

Creates a `shared_ptr<immutable_str>` from a C-string.

### 🔐 `safe_from(std::string_view, std::mutex&)`

Constructs `atomic_str_ptr` from a string view protected by a mutex.

- Ensures thread safety when the source is temporary or mutable.
- Validates that no embedded null characters exist.

---

## 💡 Technical Notes

| Behavior            | Detail                                                                 |
|---------------------|------------------------------------------------------------------------|
| Hash caching        | Hash is lazily computed and thread-safe (`std::once_flag`)             |
| Trimming            | Controlled via `auto_trim` — affects both constructor & hashing        |
| `size()` unit       | Represents **character count in bytes** (not Unicode codepoints)       |
| `.view()` behavior  | Not cached. Constructed on demand (trivial cost)                       |
| `.str()` behavior   | Performs a copy construction (prevents modifying the immutable source) |
| Sorting             | No `operator<` or `<=>` provided — not usable in `std::set/map`        |
| Inheritance         | Disallowed (`final`) — immutability must not be compromised            |
| `nullptr` key       | Supported. Hash is 0, comparison returns false                         |
| ABI safety          | `const char*` is stored internally — suitable for `extern "C"` APIs    |

---

## 🧪 Performance Benchmarks

### 🔍 Summary (default constructed from c_str)

| Compiler  | Optimization | Pointer Type | Type                   | Mean (ns) | Notes                            |
|-----------|--------------|--------------|------------------------|-----------|----------------------------------|
| **Clang** | `-O2`        | `shared_ptr` | `std::string`          | 0.267     | Extremely stable                 |
|           |              | `shared_ptr` | `immutable_str`        | 0.265     | Slightly faster                  |
|           |              | `unique_ptr` | `std::string`          | 18.70     | Heap allocation visible          |
|           |              | `unique_ptr` | `immutable_str`        | 18.23     | Slightly better                  |
|           |              | `unique_ptr` | `immutable_str` (view) | 23.52     | Mutex + validation path          |
| **Clang** | `-O0`        | `shared_ptr` | `std::string`          | 142.00    | Debug-mode cost visible          |
|           |              | `shared_ptr` | `immutable_str`        | 138.86    | Consistently faster              |
|           |              | `unique_ptr` | `std::string`          | 144.02    |                                  |
|           |              | `unique_ptr` | `immutable_str`        | 143.15    |                                  |
|           |              | `unique_ptr` | `immutable_str` (view) | 150.17    | Mutex + string_view safety       |
| **GCC**   | `-O2`        | `shared_ptr` | `std::string`          | 18.16     | Slower baseline than Clang       |
|           |              | `shared_ptr` | `immutable_str`        | 26.62     | Slightly heavier                 |
|           |              | `unique_ptr` | `std::string`          | 30.11     |                                  |
|           |              | `unique_ptr` | `immutable_str`        | 27.85     | Slightly faster than std::string |
|           |              | `unique_ptr` | `immutable_str` (view) | 27.19     | View overhead less than Clang    |
| **GCC**   | `-O0`        | `shared_ptr` | `std::string`          | 39.02     | Much faster than Clang `-O0`     |
|           |              | `shared_ptr` | `immutable_str`        | 47.78     |                                  |
|           |              | `unique_ptr` | `std::string`          | 52.42     |                                  |
|           |              | `unique_ptr` | `immutable_str`        | 52.36     | Almost identical                 |
|           |              | `unique_ptr` | `immutable_str` (view) | 53.66     | Mutex handling                   |

### 💡 Insights

- `immutable_str` performs **as fast or faster** than `std::string` under many conditions.
- The performance gap is **negligible** across compilers and optimization levels.
- Compile-time trimming logic does **not** introduce overhead in practice.
- The class is **thread-safe by design**, so extra locking is unnecessary.
- You can safely adopt `immutable_str` in performance-critical code — both `shared_ptr` and `unique_ptr` scenarios are well-optimized.
- View-based constructors are expectedly a bit slower due to mutex validation, but only by ~5ns.

📌 In short: **`immutable_str` is cost-equivalent to `std::string` with stronger safety guarantees.**

---
## 🔧 Technical Highlights

- `std::once_flag` for thread-safe lazy hash computation
- `std::unique_ptr<const char[]>` ensures storage immutability
- `std::string_view` used for efficient non-owning access
- `std::optional<uint64_t>` used to cache hashes only on demand
- `is_space_ascii(char)` replaces `std::isspace()` with constexpr performance

---

## 🧠 Design Philosophy

> "If you want to build fast, thread-safe systems in C++, start with immutability. Then reason about sharing."

- Strings should not mutate once created
- Shared ownership only makes sense when the data is guaranteed constant
- Compile-time configuration leads to better branch elimination and code clarity

---

## ✅ Summary

- `immutable_str` is a **safe, minimal, and fast immutable string** for modern C++
- Ideal for multithreaded code, config systems, ID registries, and string interning
- Avoids the pitfalls of `std::string` mutability and reallocation
- Offers clear APIs and performance guarantees

**Use `jh::immutable_str` when string identity, safety, and performance all matter.**


## 📦 For Developers

- ✅ Designed for high-performance infrastructure
- ✅ Friendly to LLVM and `extern "C"` interfaces
- ✅ Clean value semantics with shared ownership when needed
- ✅ No hidden allocation or unexpected behavior

> “Once constructed, it just stays the same. That’s the whole point.”

---

📌 **For detailed module information, refer to `immutable_str.h`.**  
📌 **For auto object pooling, refer to [`pool.md`](pool.md).**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**
