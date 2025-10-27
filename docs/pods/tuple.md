# 🧊 **JH Toolkit — `jh::pod::tuple` API Reference**

📁 **Header:** `<jh/pods/tuple.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::tuple<Ts...>` is a **true POD-based variadic tuple** that
behaves like `std::tuple` while maintaining strict **POD layout**,
**constexpr safety**, and **zero overhead**.  

It supports:

* ✅ Structured bindings
* ✅ `make_tuple()` helper
* ✅ ADL-based `get<I>` access
* ✅ `std::tuple_size` / `std::tuple_element` interoperability
* ✅ Pythonic-style printing
* ✅ Integration with [`jh::pod::pair`](pair.md) and [`jh::pod::array`](array.md)

Unlike the transitional tuple (v1.3.0–1.3.3), this version is a **compositional POD implementation** —
built entirely through **aggregate composition**, not inheritance.

---

## 🔹 Definition

```cpp
template<cv_free_pod_like... Ts>
struct tuple final;
```

Each element is stored in a trivial wrapper `tuple_field<I, T>`
within a recursively defined aggregate, guaranteeing trivial layout and full POD compliance.

> 🧩 **Requirement:**
> All template arguments must satisfy [`cv_free_pod_like`](pod_like.md).  
> Each element must be trivially copyable, constructible, and standard layout.  

---

## ⚙️ Construction Helpers

### `make_tuple()`

```cpp
template<cv_free_pod_like... Ts>
constexpr auto make_tuple(Ts&&... args) noexcept;
```

Creates a POD-compatible tuple by forwarding each argument into the aggregate.  
Unlike `std::make_tuple`, this version preserves strict POD guarantees
and stores elements directly without indirection.

```cpp
auto t = jh::pod::make_tuple(7, 3.14f);
```

Equivalent to:

```cpp
jh::pod::tuple<int, float> t{{ {7}, {{3.14f}, {}} }};
```

### 💡 Notes

* Elements are **directly copied**, not reference-wrapped.  
* Works across all compilers; portable alternative to nested aggregate init.  
* GCC ≤ 13 may reject `tuple<int, float>{7, 3.14f}` — prefer `make_tuple()`.

---

## 🧩 Accessors

### `get<I>(tuple&)` and `get<I>(const tuple&)`

```cpp
template<std::size_t I, typename... Ts>
decltype(auto) get(tuple<Ts...>& t) noexcept;

template<std::size_t I, typename... Ts>
auto get(const tuple<Ts...>& t) noexcept;
```

Retrieves the `I`-th element by ADL lookup — this replaces the legacy `.get<I>()`.  

```cpp
auto t = jh::pod::make_tuple(42, 3.14f);
auto& x = get<0>(t);
auto  y = get<1>(t);
```

---

## 🧩 Structured Bindings Support

`jh::pod::tuple` fully supports **structured bindings**
and generic tuple-like interoperability with the STL.  

```cpp
#include <jh/pod>

int main() {
    auto t = jh::pod::make_tuple(10, 20, 30);
    auto [a, b, c] = t;    // by value
    auto& [x, y, z] = t;   // by reference
}
```

### 🔹 Binding Semantics

| Binding form              | Access type  | Description          |
|---------------------------|--------------|----------------------|
| `auto [a, b] = t;`        | by value     | Copies each element. |
| `auto& [a, b] = t;`       | by reference | Modifiable elements. |
| `const auto& [a, b] = t;` | read-only    | Immutable access.    |

Semantics are identical to `std::tuple` but preserve strict POD layout.

---

## 🧭 Design Philosophy & Principles

Starting with **v1.3.4**, `jh::pod::tuple` became a **true compositional POD** type,
replacing the transitional version formerly in `tools.h`.  

* Achieves `std::tuple`-like usability through **composition**, not inheritance.  
* Each element resides in a deducible aggregate (`tuple_field<I, T>`).  
* Compilers can often optimize `get<I>(t)` into a fixed offset load.  
* Layout is identical to a hand-written POD struct with unnamed sequential fields.  

> 🧠 Conceptually:
>
> ```cpp
> struct { int x; float y; };
> ```
>
> and
>
> ```cpp
> jh::pod::tuple<int, float>;
> ```
>
> have the same memory layout —
> the only difference is that tuple fields are accessed by **index**, not name.

### 💡 ADL Access Rule

Use **`get<I>(tuple)`**, not `.get<I>()`.  
All accessors are free functions resolved by ADL.  

### 🔹 Value vs Reference Unpacking

* `auto` / `const auto&` → by value (safe, immutable)
* `auto&` → by reference (modifiable)

No unsafe aliasing or `const_cast` is possible — all bindings are safe and constexpr-valid.

---

## 🧾 Debug Stringification

When `<jh/pods/stringify.h>` or `<jh/pod>` is included,
`jh::pod::tuple` gains **stream output** with **Pythonic formatting**:  

```cpp
#include <jh/pod>
#include <iostream>

int main() {
    std::cout << jh::pod::make_tuple()           << '\n'; // ()
    std::cout << jh::pod::make_tuple(1)          << '\n'; // (1,)
    std::cout << jh::pod::make_tuple(1, 2, 3.0f) << '\n'; // (1, 2, 3)
}
```

### 🔹 Pythonic Print Style

The printing syntax mirrors Python tuples:

| Case           | Example             | Output      |
|----------------|---------------------|-------------|
| Empty tuple    | `make_tuple()`      | `()`        |
| Single element | `make_tuple(1)`     | `(1,)`      |
| Multi-element  | `make_tuple(1,2,3)` | `(1, 2, 3)` |

> This design is intentional — it improves readability in debugging
> and keeps tuple printing consistent with `pair` and `array` outputs.

Printing is provided by inline overloads in `<jh/pods/stringify.h>`.
They are weakly linked and can be safely overridden if desired.

---

## 🚀 Performance Notes

* Memory layout identical to `{T0 a; T1 b; T2 c; ...}`
* Fully trivial, standard-layout, and `memcpy`-safe
* No heap, RTTI, or virtual dispatch
* Access optimizes to compile-time offsets
* Ideal for serialization, I/O, and embedded systems

---

## ⚠️ Initialization Notes

| Compiler          | Behavior                              | Recommended form              |
|-------------------|---------------------------------------|-------------------------------|
| **Clang ≥ 15**    | Accepts direct `{}` init              | ✅ `tuple<int,float>{7,3.14f}` |
| **GCC ≤ 13**      | May reject as “too many initializers” | ✅ use `make_tuple()`          |
| **All compilers** | Aggregate-safe with nested braces     | ✅ portable but verbose        |

Prefer `jh::pod::make_tuple()` for clarity and portability.

---

## ⚙️ Comparison Operators

```cpp
template<typename... Ts>
constexpr bool operator==(const tuple<Ts...>&, const tuple<Ts...>&) noexcept;
template<typename... Ts>
constexpr bool operator!=(const tuple<Ts...>&, const tuple<Ts...>&) noexcept;
```

Element-wise equality; both are `constexpr` and trivially inlined.

---

## 🧩 Integration Summary

| Feature               | Provided by             | Description                         |
|-----------------------|-------------------------|-------------------------------------|
| Structured bindings   | Built-in via ADL `get`  | Works for `tuple`, `pair`, `array`. |
| Printing / stringify  | `<jh/pods/stringify.h>` | Pythonic-style stream output.       |
| Pair / array bridging | `<jh/pods/tuple.h>`     | Unified `get<I>` overloads.         |
| Unified include       | `<jh/pod>`              | Brings all POD core utilities.      |

---

## 🧠 Summary

| Aspect             | Description                        |
|--------------------|------------------------------------|
| Category           | Variadic POD aggregate             |
| Layout             | Trivial, standard layout           |
| ABI                | Deterministic and portable         |
| Access             | `get<I>(tuple)` (ADL)              |
| Construction       | `make_tuple()` / aggregate init    |
| Structured binding | Supported (Clang, GCC)             |
| Printing           | Pythonic `()` / `(a,) / (a, b, c)` |
| Equality           | `operator==`, `operator!=`         |

---

> 📌 **Design Philosophy**
>
> `jh::pod::tuple` is a *compositional POD tuple* —
> it offers modern C++ convenience (`make_tuple`, structured bindings)
> with deterministic, fully trivial memory layout.  
>
> Its **Pythonic printing style** provides clear, readable debugging output,
> unifying the behavior of all POD containers in the toolkit.  

