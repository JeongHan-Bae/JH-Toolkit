# 🧊 **JH Toolkit — `jh::pod::span` API Reference**

📁 **Header:** `<jh/pods/span.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::span<T>` is a **POD-safe, non-owning view**
over a contiguous sequence of elements of type `T`.  

It resembles `std::span<T>` in interface,
but applies **two critical restrictions** to ensure ABI determinism and constexpr-safe behavior:

1. **Type constraint:**
   `T` must satisfy `pod_like` — trivially copyable, trivially destructible, standard layout.
2. **Memory constraint:**
   The observed region must be **physically contiguous**,
   and its iterators are always **raw pointers (`T*`)**,
   never proxy or user-defined iterator types.

These design limits make `jh::pod::span` a **predictable binary observer**
ideal for inspecting POD containers, static buffers, or memory-mapped data.

---

## 🔹 Definition

```cpp
template<pod_like T>
struct span final {
    T*            data;
    std::uint64_t len;
};
```

| Field  | Type            | Description                 |
|--------|-----------------|-----------------------------|
| `data` | `T*`            | Pointer to first element.   |
| `len`  | `std::uint64_t` | Number of elements in view. |

### Key Properties

| Aspect        | Description                             |
|---------------|-----------------------------------------|
| Layout        | Pure POD (two fields, no hidden state). |
| Iterators     | Always raw pointers (`T*`).             |
| Const support | `span<const T>` fully supported.        |
| ABI           | Deterministic (no allocator or proxy).  |
| Lifetime      | Caller-managed; must not dangle.        |

---

## 🔬 API Breakdown

### 🔹 `operator[](std::uint64_t index) const noexcept`

Access an element by index (unchecked).

| Aspect | Description                                       |
|--------|---------------------------------------------------|
| Bounds | No checking. Caller must ensure `index < size()`. |
| UB     | Out-of-range access is undefined.                 |

```cpp
auto v = sp[i];
```

---

### 🔹 `begin()` / `end()`

Return raw pointers suitable for iteration or range-for loops.

| Function  | Returns                                |
|-----------|----------------------------------------|
| `begin()` | `const T*` — first element.            |
| `end()`   | `const T*` — one-past-the-end pointer. |

```cpp
for (auto& v : sp) std::cout << v << ' ';
```

---

### 🔹 `size()` / `empty()`

| Function  | Return          | Description         |
|-----------|-----------------|---------------------|
| `size()`  | `std::uint64_t` | Number of elements. |
| `empty()` | `bool`          | True if `len == 0`. |

---

### 🔹 `sub(offset, count = 0)`

Create a sub-span starting at `offset`, optionally limited by `count`.

| Aspect   | Description                               |
|----------|-------------------------------------------|
| Default  | If `count == 0`, extends to end.          |
| OOB      | Returns `{nullptr, 0}` if `offset > len`. |
| Behavior | Non-allocating, zero-cost slice.          |

```cpp
auto tail = sp.sub(2);      // from element 2 to end
auto mid  = sp.sub(1, 3);   // view of 3 elements
```

---

### 🔹 `first(count)` / `last(count)`

Return the first or last `count` elements as a new `span`.

| Function       | Description                                          |
|----------------|------------------------------------------------------|
| `first(count)` | Returns prefix of length `count` (clamped to range). |
| `last(count)`  | Returns suffix of length `count` (clamped to range). |

```cpp
auto head = sp.first(4);
auto tail = sp.last(2);
```

#### Boundary Behavior

These functions **never trigger out-of-bounds access**:
if `count` exceeds the span length, it is **automatically truncated**.

| Case           | Behavior                                                 |
|----------------|----------------------------------------------------------|
| `count == 0`   | Returns `{nullptr, 0}` (empty span).                     |
| `count >= len` | `first()` returns full span, `last()` returns full span. |
| `count < len`  | Returns the corresponding prefix/suffix view.            |

> ✅ Always safe — truncation ensures that `begin()` and `end()` remain valid.  
> ❌ No dynamic allocation or copying occurs.

---

### 🔹 `operator==`

Two spans are equal if and only if they reference the **same memory region**:

```cpp
constexpr bool operator==(const span& rhs) const = default;
```

This comparison checks **pointer + length equality**,
not element value equality.

Use `std::equal(sp1.begin(), sp1.end(), sp2.begin())`
for content-wise comparison.

---

### 🔹 Construction Helpers

```cpp
jh::pod::to_span(T(&arr)[N]);
jh::pod::to_span(const T(&arr)[N]);
jh::pod::to_span(container);
```

| Source            | Result                                           |
|-------------------|--------------------------------------------------|
| Raw array         | Creates `span<T>` directly.                      |
| `const` raw array | Creates `span<const T>`.                         |
| Linear container  | Deduces element type from `.data()` + `.size()`. |

```cpp
int arr[4] = {1, 2, 3, 4};
auto s1 = jh::pod::to_span(arr);

std::vector<int> v{5, 6, 7};
auto s2 = jh::pod::to_span(v);
```

---

## 🧩 Integration Notes

* `jh::pod::span` is a **true range type** — it satisfies `std::ranges::range` and can be used in any range-based context.  
  In contrast, `jh::pod::bytes_view` is **not iterable** and operates purely on byte offsets.  
* `span<T>` may safely wrap `const T` elements since `const T*` does not affect POD assignability.  
* Like all non-owning views, spans **must not dangle** once their source memory is destroyed.  
* The helper `to_span()` works automatically with STL-style containers exposing `.data()` and `.size()`
  (e.g., `std::vector`, `std::array`, custom linear containers).  
  It **cannot** be used with `jh::pod::array`, since that type exposes
  a **public field** `.data` rather than a `.data()` accessor.  

> 🧩 **Tip:**
> For `jh::pod::array`, create spans directly:
>
> ```cpp
> jh::pod::span sp{ arr.data, arr.size() };
> ```
>
> This is functionally equivalent to `to_span()` and incurs no overhead.

---

## 🧾 Debug Stringification

When streamed to an `std::ostream`,
`jh::pod::span` prints in a debug-friendly form:

```
span<T>[e₀, e₁, e₂, ...]
```

Example:

```cpp
#include <jh/pods/span.h>
#include <jh/pods/stringify.h>

int arr[4] = {1, 2, 3, 4};
auto sp = jh::pod::to_span(arr);
std::cout << sp;
// → span<int>[1, 2, 3, 4]
```

Type names are resolved via `jh::macro::type_name<T>()`,
which extracts the unmangled name from `__PRETTY_FUNCTION__`
(Clang/GCC) without RTTI.

> ⚙️ **Behavior Summary**
>
> * Uses compile-time extraction — no RTTI or runtime reflection.  
> * Works under `-fno-rtti`.  
> * Output may vary by compiler version; use only for debugging/logging.  
> * If type detection fails, `"unknown"` is displayed.  

> ✅ **Recommended include for printing:**
>
> ```cpp
> #include <jh/pods/stringify.h>
> ```
>
> or
>
> ```cpp
> #include <jh/pod>
> ```
>
> which brings all stringify helpers into scope.

---

## 🧠 Summary

| Aspect     | Description                                                          |
|------------|----------------------------------------------------------------------|
| Category   | POD contiguous range view                                            |
| Ownership  | Non-owning                                                           |
| Concept    | `pod_like<T>`                                                        |
| Iterators  | Raw pointers                                                         |
| Range      | Fully satisfies `std::ranges::range`                                 |
| Comparison | Pointer + length equality                                            |
| Printing   | `span<T>[...]` via `jh::macro::type_name`                            |
| ABI        | 2-field deterministic POD layout                                     |
| Consteval  | Technically allowed but meaningless — spans reference runtime memory |

> ⚙️ **Explanation:**
> Unlike `bytes_view` or `optional`, `span` does not perform any reinterpretation or laundering.  
> It simply stores a pointer and a count.  
> However, since the concept of a "view" depends on the existence of live memory,
> using it in a `consteval` context has no semantic value.

---

> 📌 **Design Philosophy**
>
> `jh::pod::span` provides a **minimal, trivially copyable span abstraction**
> for raw contiguous memory.  
> It mirrors the usability of `std::span<T>` but constrains its domain
> to POD-compatible, pointer-linear data,
> ensuring strict ABI predictability and full interoperability
> with other `jh::pod` structures such as `array`, `bytes_view`, and `optional`.
