# 🧊 **JH Toolkit — `jh::pod::optional` API Reference**

📁 **Header:** `<jh/pods/optional.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::optional<T>` is a **POD-safe, raw-storage form** of `std::optional<T>` —
it preserves identical logical semantics (presence, equality, fallback),
but executes entirely without constructors, destructors, or lifetime management.

The type is **trivially copyable**, **ABI-stable**, and safe for
**serialization**, **mmap-backed regions**, or **static aggregates**.

---

## 🔹 Definition

```cpp
template<cv_free_pod_like T>
struct optional final {
    alignas(alignof(T))
    std::byte storage[sizeof(T)];
    bool has_value;
};
```

### Key Properties

| Aspect    | Description                                                     |
|-----------|-----------------------------------------------------------------|
| Layout    | Flat POD layout — one byte array + one flag.                    |
| Alignment | Explicitly `alignas(alignof(T))` for correct pointer alignment. |
| Size      | ≥ `sizeof(T) + 1` (compiler may add padding).                   |
| ABI       | Stable across all compilers.                                    |

### Why `cv_free_pod_like<T>`?

`cv_free_pod_like` guarantees `T` has **no const/volatile qualifiers**,
ensuring the optional itself remains *trivially assignable*.

If `T` were `const` or `volatile`, the implicit assignment of the optional
would cease to be trivial, breaking its POD guarantees.
Thus, this qualifier-free restriction is **structural**, not semantic.

---

## 🔬 API Breakdown

### 🔹 `store(const T& value)`

Copies the value bytes into storage and sets presence flag.

| Aspect   | Description                                             |
|----------|---------------------------------------------------------|
| Behavior | `memcpy(storage, &value, sizeof(T)); has_value = true;` |
| Safety   | Safe for all valid `T`.                                 |
| UB       | None.                                                   |

---

### 🔹 `clear()`

Marks the optional as empty (does not zero storage).

| Aspect    | Description               |
|-----------|---------------------------|
| Operation | Sets `has_value = false`. |
| Safety    | Always safe.              |

---

### 🔹 `get()` / `get() const`

Returns pointer to contained object.

| Aspect | Description                                    |
|--------|------------------------------------------------|
| Return | Pointer to `T`.                                |
| Safety | Caller must check `.has()` before dereference. |
| UB     | Dereferencing when empty.                      |

---

### 🔹 `ref()` / `ref() const`

Returns reference to contained object.

| Aspect | Description                     |
|--------|---------------------------------|
| Return | Reference to stored value.      |
| UB     | Undefined if `.has() == false`. |

---

### 🔹 `has()` / `empty()`

| Function  | Return | Description               |
|-----------|--------|---------------------------|
| `has()`   | `bool` | True if contains a value. |
| `empty()` | `bool` | True if value is absent.  |

---

### 🔹 `value_or(T fallback) const`

Returns contained value or fallback if empty.

| Aspect | Description                       |
|--------|-----------------------------------|
| Return | Copy of stored or fallback value. |
| Safety | Always safe.                      |

---

### 🔹 `operator==(const optional&) const`

Compares two optionals for logical equivalence.

| Aspect          | Description                                                                                                                                   |
|-----------------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| Presence        | If `has()` differs → result is `false`.                                                                                                       |
| Both empty      | Always `true`, regardless of raw storage.                                                                                                     |
| Both have value | Performs bytewise comparison of storage (`memcmp`).                                                                                           |
| Behavior        | Never calls `T::operator==`. Even for types like `jh::pod::string_view`, comparison is **pointer-and-length equality**, not content equality. |

```cpp
jh::pod::optional<jh::pod::string_view> a, b;
// even if they point to equal text, different addresses → not equal
bool eq = (a == b);
```

This design keeps comparison **ABI-deterministic** and purely **byte-based**,
never deferring to user-defined logic.

---

### 🔹 `jh::pod::make_optional(const T& value)`

Factory helper for constructing a filled optional.

| Aspect   | Description                                                               |
|----------|---------------------------------------------------------------------------|
| Behavior | Returns a fully initialized `jh::pod::optional<T>` with `.has() == true`. |
| Notes    | Calls `.store(value)` internally. Type deduction is supported.            |

Because of `jh::pod::optional`'s **explicit raw-storage layout**,
it **cannot be directly aggregate-initialized** (e.g. `{ value, true }` is meaningless and unsafe).

Instead, construct an empty optional by default:

```cpp
jh::pod::optional<MyType> opt{}; // empty
```

and then fill it via `jh::pod::make_optional()` or `.store()`:

```cpp
auto a = jh::pod::make_optional(MyType{1, 2});
auto b = jh::pod::make_optional<int>(42);
```

> 🧩 **Note:** `jh::pod::make_optional()` supports CTAD (class template argument deduction)
> and automatically deduces `T` from the provided argument.

---

## 🔹 Behavior Summary

| Function          | Checked | UB Possible | Notes                                          |
|-------------------|---------|-------------|------------------------------------------------|
| `store()`         | ✅       | ❌           | Copies bytes                                   |
| `clear()`         | ✅       | ❌           | Marks empty                                    |
| `get()` / `ref()` | ❌       | ✅           | Must check `.has()`                            |
| `value_or()`      | ✅       | ❌           | Safe fallback                                  |
| `operator==()`    | ✅       | ❌           | Requires `has()` match; bytewise equality only |
| `make_optional()` | ✅       | ❌           | Factory helper                                 |

---

## 🧩 Integration Notes

* `jh::pod::optional<T>` is **POD-stable** — safe for embedding in any other `jh::pod` type.
* Acts as a **direct ABI substitute** for `std::optional<T>` in binary or mapped memory.
* Access via `.get()` / `.ref()` is valid only if `.has() == true`.
* Avoid using inside `consteval` contexts; reinterpretation is runtime-only.
* Use `jh::pod::pair<T, bool>` instead for constexpr logic.
* For serialization or file I/O, always use `sizeof(optional<T>)` (not `sizeof(T) + 1`).
* Equality is **bytewise** — guarantees deterministic comparison, never invokes `T::operator==`.

---

## 🧾 Debug Stringification

The `jh::pod::optional` type supports **debug-friendly printing**
when streaming helpers from

```cpp
#include <jh/pods/stringify.h>
```

are available.

If the optional holds a value (`.has() == true`),
its contained element is streamed using its own `operator<<`.
If empty, the output is the literal:

```
nullopt
```

For example:

```cpp
jh::pod::optional<int> a = jh::pod::make_optional(42);
jh::pod::optional<int> b;
std::cout << a << ", " << b << '\n';
```

produces:

```
42, nullopt
```

If you include only:

```cpp
#include <jh/pods/optional.h>
```

then the `operator<<` overload is **not defined** —
printing support requires the `stringify` header.

> ✅ **Recommended include for users:**
>
> ```cpp
> #include <jh/pod>
> ```
>
> This umbrella header automatically includes
> `<jh/pods/stringify.h>` and all related stream helpers,
> ensuring consistent and convenient debug printing.

---

## 🧠 Summary

| Aspect      | Description                                 |
|-------------|---------------------------------------------|
| Category    | POD optional wrapper                        |
| Storage     | Byte buffer + flag                          |
| ABI         | Flat layout, alignment preserved            |
| Size        | ≥ `sizeof(T) + 1` (depends on padding)      |
| Concept     | `cv_free_pod_like<T>`                       |
| Semantics   | Equivalent to `std::optional<T>`            |
| Comparison  | Requires `has()` match; bytewise data check |
| Consteval   | Not supported                               |
| Integration | Fully compatible with other `jh::pod` types |

---

> 📌 **Design Philosophy**
>
> `jh::pod::optional<T>` provides the same logical contract as `std::optional<T>`
> but with a **pure POD ABI**. It trades constructor semantics for direct memory layout control,
> ensuring deterministic, byte-level behavior for serialization, embedding, and binary transport.  
>
> All equality and access are strictly byte-driven, guaranteeing stability
> even when used with complex POD-like types such as `jh::pod::string_view`.
