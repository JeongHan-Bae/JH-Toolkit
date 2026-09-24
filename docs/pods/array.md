# 🧊 **JH Toolkit — `jh::pod::array` API Reference**

📁 **Header:** `<jh/pods/array.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::array<T, N>` is a **POD-compatible fixed-size array**,
behaving like `T[N]` but with **trivial layout**, **constexpr safety**,
and **ABI determinism**.

It provides all necessary type aliases and range traits
for integration with modern C++20 algorithms,
while maintaining a strict Plain Old Data (POD) model —
no constructors, no heap, no indirection.

---

## 🔹 Definition

```cpp
template<cv_free_pod_like T, std::size_t N>
requires (sizeof(T) * N <= 16 * 1024)
struct alignas(alignof(T)) array final;
```

| Parameter | Description                                                                                |
|-----------|--------------------------------------------------------------------------------------------|
| `T`       | Element type — must satisfy [`pod_like`](pod_like.md) and be free of `const` / `volatile`. |
| `N`       | Element count. Total memory (`sizeof(T) * N`) must not exceed **16 KB**.                   |

> 🧩 Invalid template arguments (non-POD, oversized, or const/volatile element)
> are **rejected at compile time** via `concept` and `requires` constraints.
> Clang/Clangd-based IDEs highlight these issues immediately during editing.
>
> 💡 Even if `T` itself satisfies `pod_like`,
> applying `const` or `volatile` qualifiers breaks the array's
> *trivial constructibility* and thus its **POD nature**.  
> To preserve strict POD semantics for the container-like buffer itself,
> `jh::pod::array` was upgraded in **v1.3.3+** to require
> `cv_free_pod_like<T>` — ensuring that the array object
> remains trivially constructible, destructible, and layout-stable.


---

## ⚙️ Member Aliases

`jh::pod::array` exposes standard container-like aliases,
mirroring `std::array` but restricted to POD semantics:

```cpp
using value_type      = T;
using size_type       = std::size_t;
using difference_type = std::ptrdiff_t;
using reference       = value_type&;
using const_reference = const value_type&;
using pointer         = value_type*;
using const_pointer   = const value_type*;
```

These allow template-based code and algorithms to deduce
the correct element, iterator, and reference types automatically —  
just like STL containers.

Example:

```cpp
template<typename A>
void fill_half(A& a, typename A::value_type v) {
    for (std::size_t i = 0; i < a.size() / 2; ++i)
        a[i] = v;
}

jh::pod::array<int, 6> xs{};
fill_half(xs, 42);
```

---

## 🧩 Template and Type Deduction

`jh::pod::array` supports **class template argument deduction (CTAD)**
for aggregate initialization — similar to `std::array`,
but with slightly stricter syntax to preserve full POD semantics.

### 🔹 Numeric Arrays

For numeric or trivially copyable element types,
you must use **double braces** for unambiguous deduction:

```cpp
jh::pod::array a = {{1, 2, 3}};  // ✅ deduces array<int, 3>
```

> ⚠️ A single brace (`{1, 2, 3}`) is interpreted as a braced-init-list,
> not an aggregate initializer — CTAD will fail.  
> Always use double braces `{{...}}` to deduce both `T` and `N`.

You can still specify template arguments explicitly:

```cpp
jh::pod::array<int, 3> a = {1, 2, 3};
```

### 🔹 String Deduction

`jh::pod::array` can also deduce string-like arrays:

```cpp
jh::pod::array s = {"hello\nworld"};  // ✅ deduces array<char, sizeof("hello\nworld")>
```

This creates a POD array of `char` **including the trailing null terminator**,
since in C++ the string literal `"hello\nworld"` already contains the final `'\0'`.

> ⚠️ **Do not use double braces** here:
>
> ```cpp
> jh::pod::array s = {{"hello"}};  // 🚫 deduces array<char*, 1>, not array<char, N>
> ```
>
> The initializer must be a single string literal inside `{...}`.

If you need an explicit `char` array:

```cpp
jh::pod::array<char, 6> s = {'h','e','l','l','o','\0'};
```

Any element type that can be streamed (`operator<<`) is printable
when `<jh/pods/stringify.h>` is included.

---

## 🧭 Design Motivation

`jh::pod::array` is a **stack-based**, **trivially copyable** array
intended for deterministic low-level data storage.  
It eliminates the minor ABI variance and constructor semantics
of `std::array`, especially under MinGW or Windows-linked builds.

This makes it ideal for:

* Embedded systems or game engines using **no-RTTI / no-heap** policies
* Serialization and memory mapping (`mmap`, `arena`, `placement-new`)
* Predictable in-memory layout for binary I/O or IPC buffers

---

## 🔧 Member Summary

| Member               | Type          | Description                                      |
|----------------------|---------------|--------------------------------------------------|
| `T data[N]`          | Inline buffer | Raw storage, compatible with structured binding. |
| `operator[](size_t)` | Access        | Returns reference to element (unchecked).        |
| `at(size_t)`         | Checked access| Returns expected pointer or `out_of_bounds`.     |
| `begin()` / `end()`  | Iterators     | Return raw pointers, compatible with STL ranges. |
| `size()`             | Static        | Returns constant `N`.                            |
| `operator==`         | Comparison    | Performs element-wise comparison.                |

`at(i)` is `constexpr` and `noexcept`:

```cpp
auto result = values.at(i);
if (result)
    use(*result.value());
```

---

## 🧩 Initialization and Unpacking

Because it is a one-member aggregate, initialization matches `T[N]` semantics:

```cpp
jh::pod::array<int, 3> a = {1, 2, 3};
jh::pod::array<int, 3> b{{1, 2, 3}}; // equivalent
```

You can also unpack using structured bindings via `.data`:

```cpp
auto [x, y, z] = a.data;
```

Both forms are constexpr-safe and have zero construction overhead.

---

## 📦 Example — Basic Usage

```cpp
#include <jh/pods/array.h>
#include <iostream>

int main() {
    jh::pod::array<int, 4> a = {10, 20, 30, 40};

    for (auto v : a)
        std::cout << v << ' ';

    std::cout << "\nSize: " << a.size() << '\n';
}
```

**Output:**

```
10 20 30 40
Size: 4
```

---

## 🧩 Example — Range Compatibility

`jh::pod::array` satisfies `std::ranges::range`
and integrates directly with STL view pipelines:

```cpp
#include <ranges>
#include <jh/pod>

jh::pod::array<int, 3> arr = {1, 2, 3};
auto doubled = arr | std::views::transform([](int x){ return x * 2; });
```

---

## 🚀 Performance Notes

* `alignas(alignof(T))` guarantees optimal memory alignment.
* Predictable contiguous layout allows the compiler to perform **loop prefetching**.
* Access and iteration generate identical machine code to `T[N]`.
* No heap, no indirection, no RTTI → perfect cache predictability.

This property makes it suitable for numeric kernels,
binary IO, and any code path sensitive to memory locality.

---

## ⚠️ Safety and Diagnostics

| Behavior                 | Enforcement                                    |
|--------------------------|------------------------------------------------|
| `sizeof(T) * N > 16KB`   | Compile-time error (`requires` fails).         |
| Non-POD `T`              | Compile-time error (`pod_like` fails).         |
| `const` / `volatile` `T` | Compile-time error (`cv_free_pod_like` fails). |
| Out-of-bounds `operator[]` | Undefined (unchecked).                       |
| Out-of-bounds `at()`      | Returns `array<T, N>::error_code::out_of_bounds`. |

All static constraints are enforced at compile time.  
Clang-based toolchains and IDEs (via Clangd)
emit immediate diagnostics when constraints are violated.

---

## 🧩 Integration Notes

* Acts as a **container-like buffer** — a lightweight, inline POD data block
  with the same semantics as a native `T[N]`, but **without pointer decay**.  
  It preserves array identity and type information during template deduction,
  enabling safer generic programming and range interoperability.

* Represents **pure data**, not ownership —
  there is no heap allocation, lifetime management, or metadata.  
  Construction, copy, and destruction are all trivial.

* Can be **passed, reinterpreted, or serialized** directly
  (e.g., through `std::memcpy` or binary I/O)
  without auxiliary wrappers or translation layers.

* Fully **`constexpr`-compatible**,
  allowing compile-time instantiation and equality comparison.

* Natively satisfies `std::ranges::range`,
  so it integrates seamlessly with range-based for-loops
  and STL range pipelines.

---

> 🧠 **Conceptually:**
> `jh::pod::array` is equivalent to a fixed-size C array that
> *does not decay into a pointer*, ensuring deterministic layout,
> safe range compatibility, and optimal compiler optimization.

---

## 🧾 Debug Stringification

The `jh::pod::array` type provides **debug-friendly stream output**
when printing helpers from

```cpp
#include <jh/pods/stringify.h>
```

are available.

If you include only

```cpp
#include <jh/pods/array.h>
```

then `operator<<` is **not defined** —
printing requires the `stringify` header.

> ✅ **Recommended include for users:**
>
> ```cpp
> #include <jh/pod>
> ```
>
> This aggregate header automatically brings in `<jh/pods/stringify.h>`
> and all related POD utilities, ensuring printing just works.

---

### 🔹 Example

```cpp
#include <jh/pod>
#include <iostream>

int main() {
    jh::pod::array<int, 3> a = {{1, 2, 3}};
    std::cout << a << '\n';  // → [1, 2, 3]

    jh::pod::array s = {"hello\nworld"};
    std::cout << s << '\n';  // → "hello\nworld"
}
```

---

### 🔹 Output Rules

| Element type              | Output format                    |
|---------------------------|----------------------------------|
| `char`                    | Escaped string literal (`"..."`) |
| Other printable POD types | List form (`[1, 2, 3]`)          |

Printing is available only if each element type defines a valid `operator<<`;
the constraint is statically enforced.

---

### ⚙️ Notes on Overriding Behavior

All `operator<<` overloads in `stringify.h` are declared **`inline`**,
which gives them *weak linkage* — they can be replaced or shadowed safely.

If you want to customize printing:

* **Define a non-`inline` global overload**
  to override the weak inline definition:

  ```cpp
  std::ostream& operator<<(std::ostream& os, const jh::pod::array<int, 3>& a) {
      os << "MyArray(" << a[0] << "," << a[1] << "," << a[2] << ")";
      return os;
  }
  ```

* **Or import your own overload into scope** before printing:

  ```cpp
  using my_namespace::operator<<;
  ```

Argument-dependent lookup (ADL) automatically finds
`jh::pod::operator<<` for all `jh::pod` types,
so no extra `using` directives are normally required.

---

## 🧩 Structured Bindings Support

`jh::pod::array` supports **structured bindings** when
`<jh/pods/tuple.h>` or the unified header `<jh/pod>` is included.  
This provides tuple-like unpacking semantics consistent with `jh::pod::tuple`.

### 🔹 Binding Semantics

| Binding form                | Access type  | Description                 |
|-----------------------------|--------------|-----------------------------|
| `auto [a, b] = arr;`        | by value     | Copies each element.        |
| `auto& [a, b] = arr;`       | by reference | Elements remain modifiable. |
| `const auto& [a, b] = arr;` | read-only    | Safe const access.          |

### 🔹 Combined Usage

Structured bindings and printing can be used together:

```cpp
#include <jh/pod>
#include <iostream>

int main() {
    jh::pod::array<int, 3> rgb = {128, 200, 255};
    auto& [r, g, b] = rgb;

    std::cout << "RGB: " << rgb << '\n';
    g = 180;  // direct reference access
    std::cout << "Modified: " << rgb << '\n';
}
```

> 💡 Include `<jh/pod>` to enable all features:
>
> * Structured bindings (tuple integration)
> * Stream printing (`stringify`)
> * Full POD safety and constexpr compatibility

---

## 🧠 Summary

| Aspect         | Description                                  |
|----------------|----------------------------------------------|
| Category       | POD buffer (container-like)                  |
| ABI            | Deterministic, portable                      |
| Storage        | Inline contiguous buffer                     |
| Initialization | Aggregate (C array-compatible)               |
| Deduction      | CTAD with `{{...}}`, full type alias support |
| Range support  | Satisfies `std::ranges::range`               |
| Performance    | Prefetch/vectorization-friendly              |

---

> 📌 **Design Philosophy**
>
> `jh::pod::array` is a **predictable, low-level** array abstraction —
> not a safer `std::array`, but a more *transparent* one.  
> It gives you exactly what `T[N]` provides,
> while ensuring strong compile-time validation,
> modern range interoperability, and optimal layout alignment.
