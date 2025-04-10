# 🧱 JH Toolkit: `immutable_str` API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **Overview**

The `jh::immutable_str` class provides a **truly immutable string** in C++, ensuring that **once created, it cannot be modified**.  
It is designed to safely encapsulate C-strings and provide **ABI-level immutability**, **thread safety**, and **high-performance hashing and comparison**.

Unlike `const std::string`, which only restricts modification via the API, `immutable_str` guarantees:

- ❌ No modification (even via `const_cast`)
- ❌ No reallocation or resizing
- ✅ Thread-safe concurrent use
- ✅ C ABI compatibility (`const char*`)

---

## ✨ Key Features

- ✅ **Immutable at memory level** — guaranteed by internal design.
- ✅ **Zero-copy C-string support**
- ✅ **Thread-safe by nature** — safe for sharing without locking.
- ✅ **Optional automatic whitespace trimming**
- ✅ **Optimized hashing & comparison** — no deep string comparisons
- ✅ **Shared pointer friendly** — use with `std::shared_ptr` or `atomic_str_ptr`
- ✅ **Transparent lookup** — directly use string literals as map keys
- ✅ **Poolable** — works with `jh::pool` allocator
- ✅ **Seamless access** — `c_str()`, `view()`, `str()`

---

## 🚀 Best Practices

| Use Case                      | Recommended Practice                               |
|-------------------------------|----------------------------------------------------|
| Immutable shared string       | `std::shared_ptr<immutable_str>` via `make_atomic` |
| Thread-safe string assignment | `safe_from(view, mutex)`                           |
| Lookup in hash containers     | Use `const char*` literal directly via `find()`    |
| Temporary string use          | Call `.view()` or `.str()` explicitly              |

---

## 🛡️ Design Philosophy

- `immutable_str` is a **holder**, not a view.
- All logic involving content manipulation should be performed on `.view()` or `.str()` explicitly.
- The class is `final` to prevent unintended mutation via inheritance.
- It is **not intended for ordered containers** (e.g., `std::set`) — no `operator<` provided.
- `.view()` is not cached — `std::string_view` is trivial to construct.

---

## 🧩 Interface Summary

### ✅ Constructor

```c++
explicit immutable_str(const char* str);
immutable_str(std::string_view sv, std::mutex& mtx);
```

- Automatically trims whitespace if `auto_trim == true`
- `explicit` to avoid accidental conversions

---

### ❌ Deleted Operations

```c++
immutable_str(const immutable_str&) = delete;
immutable_str(immutable_str&&) = delete;
immutable_str& operator=(const immutable_str&) = delete;
immutable_str& operator=(immutable_str&&) = delete;
```

- **Immutability is enforced by deleting all copy and move operations.**
- Use `shared_ptr<immutable_str>` (`atomic_str_ptr`) to share instances.

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
- `.str()` → Allocates and returns a new `std::string`
- `.size()` → Returns number of characters (i.e., bytes for UTF-8)

📌 `.view()` is **not cached**, as `std::string_view` is trivial to construct.

---

### 🔁 Equality & Hashing

```c++
bool operator==(const immutable_str& other) const noexcept;
std::uint64_t hash() const noexcept;
```

- Efficient and thread-safe.
- Lazy hash computation (cached once, thread-safe).

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
static inline bool jh::immutable_str::auto_trim = true;
```

- If enabled, trims leading/trailing **ASCII whitespace only**  
  (space, tab, `\n`, `\r`, `\f`, `\v`) using `std::isspace(unsigned char)`.

---

## 🔄 Shared Storage (`atomic_str_ptr`)

```c++
using atomic_str_ptr = std::shared_ptr<immutable_str>;
```
> Here, ‘atomic’ refers to the atomicity of pointer assignment, not the content.

Use when:

- You need multiple references to the same string
- You want to store in containers with shared keys

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

| Behavior         | Detail                                                                 |
|------------------|------------------------------------------------------------------------|
| Hash caching     | Hash is lazily computed and thread-safe (`std::once_flag`)             |
| Trimming         | Controlled via `auto_trim` — affects both constructor & hashing        |
| `size()` unit    | Represents **character count in bytes** (not Unicode codepoints)       |
| `.view()` cache  | Not cached. Constructed on demand (trivial cost)                       |
| Sorting          | No `operator<` or `<=>` provided — not usable in `std::set/map`        |
| Inheritance      | Disallowed (`final`) — immutability must not be compromised            |
| `nullptr` key    | Supported. Hash is 0, comparison returns false                         |
| ABI safety       | `const char*` is stored internally — suitable for `extern "C"` APIs    |

---

## ⚙️ Ownership Model in JH Toolkit

JH Toolkit promotes **explicit, predictable ownership** in modern C++20 applications.  
Unlike garbage-collected languages, C++ requires clear and manual resource management — and our design reflects that.

---

### ✅ Design Philosophy

> **“Don't abuse `shared_ptr`. C++ is not a garbage-collected language.”**

Instead of treating `shared_ptr` as a default, **JH Toolkit encourages a disciplined ownership model**, emphasizing clarity and correctness based on **object mutability and lifetime semantics**.

---

### 🔹 1. `unique_ptr<T>` — for mutable or exclusive ownership

- Use `std::unique_ptr<T>` to express **exclusive, movable ownership**.
- This provides:
  - **Controlled mutation**: use `T*` or `T&` only within known lifetimes
  - **Const-safe access**: multiple readers can access `const T*` concurrently
  - **Manual sharing** where appropriate (but carefully scoped)

📌 **Shared mutable state is discouraged**, unless safely wrapped (e.g., `std::mutex`, `std::atomic`, or specialized wrappers).

---

### 🔹 2. `shared_ptr<T>` — for immutable objects only

- **Preferred for immutable objects**, including:
  - `jh::immutable_str`
  - Structurally stable data snapshots
  - Pure data types without side effects
- This guarantees:
  - **No data races** (object is never mutated)
  - **Safe concurrent sharing**
  - **Atomic reference semantics** — ideal for cache updates and broadcast-style use cases

💡 In JH Toolkit, `shared_ptr<T>` is viewed as a **safe reference distribution tool**, not a general-purpose lifetime manager.

---

### 🔹 3. Pooling (`jh::pool<T>`) — for high reuse of immutable or ephemeral objects

JH Toolkit's `jh::pool<T>` is optimized for **immutable object reuse** via **shared ownership + weak tracking**.

#### ✅ Strategy: *“Construct first, then insert”*

- The pool **always constructs the object immediately**, with no prior lookup.
- The resulting `shared_ptr<T>` is cached internally using a `weak_ptr<T>`.
- This design avoids:
  - Complex template deduction or multi-parameter hashing
  - Runtime overhead of matching constructor arguments
  - Cache miss penalties during lookup under contention

> ⚠️ Avoid premature optimization: **constructing then inserting** is often faster than lookup+construct+insert, especially when construction cost is low.

📌 **Best suited for:**
- Immutable or trivially copyable types
- Lightweight identifiers (e.g., `immutable_str`)
- Pooled message headers or protocol keys
- High-frequency hash lookups with infrequent churn

---

### 🧱 Ownership Summary Table

| Object Type         | Ownership          | Suggested Holder     | Notes                                         |
|---------------------|--------------------|----------------------|-----------------------------------------------|
| Mutable & exclusive | Single owner       | `std::unique_ptr<T>` | Clear lifetime, manual mutation control       |
| Shared, immutable   | Safe shared access | `std::shared_ptr<T>` | Thread-safe by design, zero locking required  |
| Reused, immutable   | Weak-tracked reuse | `jh::pool<T>`        | Optimized for fast creation + auto expiration |
| External/unsafe     | View only          | `T*`, `std::span<T>` | No ownership — validity managed externally    |

---

> 💡 **Rule of Thumb:**  
> If the object can change — own it.  
> If it never changes — share it.  
> If it's reused frequently — pool it.

---

### 🧠 Key Insights

- **Mutability implies control**: if you're going to mutate, **you must own**.
- **Immutability enables sharing**: immutable objects are **cheap to share and safe to reuse**.
- **`shared_ptr` is a synchronization mechanism**, not a garbage collector.

---

> 💡 “If you want to build fast, thread-safe systems in C++, start with immutability. Then reason about sharing.”


## ✅ Summary

- `immutable_str` is a **system-level, immutable string abstraction**.
- It provides **hashable, comparable, thread-safe string identity**, backed by minimal memory.
- Ideal for:
  - Global string registries
  - Caching layers
  - Configuration identifiers
  - Thread-safe shared data

---

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
