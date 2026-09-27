# ⚗️ **JH Toolkit — `jh::meta::flatten_proxy` API Reference**

📁 **Header:** `<jh/metax/flatten_proxy.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::flatten_proxy` defines the **tuple flattening core** for Meta utilities.  
It provides a constexpr-safe mechanism to expand arbitrarily nested [`tuple_like`](../conceptual/tuple_like.md)
objects into a single flat `std::tuple` and a proxy wrapper that exposes this flattened view.  

This facility allows **structured binding**, **compile-time element mapping**,
and **transparent recursion across tuple-like hierarchies**.

---

## 🔹 Overview

| Aspect                   | Description                                                                   |
|--------------------------|-------------------------------------------------------------------------------|
| **Purpose**              | Flatten nested `tuple_like` hierarchies into a single-level tuple.            |
| **Implementation Model** | Fully `constexpr`; no dynamic allocation, pure template metaprogramming.      |
| **Input Constraint**     | Input must satisfy [`jh::concepts::tuple_like`](../conceptual/tuple_like.md). |
| **Integration**          | Supports structured binding and implicit conversion to `std::tuple`.          |
| **Output Form**          | A `std::tuple` or proxy convertible to `std::tuple`.                          |

---

## 🔹 Core Components

| Symbol                | Type     | Description                                                   |
|-----------------------|----------|---------------------------------------------------------------|
| `tuple_materialize()` | Function | Recursively expands any nested `tuple_like` object.           |
| `flatten_proxy`       | Struct   | Proxy exposing a flattened `get<I>` interface and tuple view. |

---

## 🔹 Flattening Model

The flattening process is a **compile-time recursive expansion** based on the
[`tuple_like`](../conceptual/tuple_like.md) concept — any type that defines
`std::tuple_size`, `std::tuple_element`, and supports ADL-based `get<I>`
is recognized as *tuple-like*.

---

### 🌐 Step-by-step logic

1. **Input requirement**  
   The argument must be a `tuple_like` object `T`.  
   Non-`tuple_like` types are not accepted.

2. **Recursive expansion**  
   For `T`, we enumerate indices `[0, tuple_size_v<T>)`
   and recursively apply `get<I>(T)`.

3. **Element classification**

    * If `get<I>(T)` is *not* `tuple_like`, it is treated as a leaf value and contributes
      one entry to the flattened result.
    * If `get<I>(T)` *is* `tuple_like`, it is **recursively expanded**
      until all nested tuple layers are resolved.

4. **Index mapping**  
   During flattening, each sub-tuple's expansion contributes a compile-time known
   number of elements `N`. The next element's expansion continues from index `N`.  
   The mapping between original nested indices and flattened indices
   is computed entirely at **compile time**.

5. **Empty tuple collapsing**  
   Empty tuples are eliminated:  

    * `tuple(tuple(), tuple(tuple()))` → `tuple()`
    * `tuple(ele1, tuple(), ele2)` → `tuple(ele1, ele2)`

6. **Reference and value-category preservation**
   Reference elements are forwarded as references. An lvalue proxy exposes
   lvalue elements; an rvalue proxy exposes rvalue elements. The source proxy
   may contain `std::reference_wrapper`, but a materialized tuple cannot retain it.
Moving a proxy moves owned values; referenced values remain lvalues, so they
are copied into value elements or retained as references in the output tuple.
An rvalue proxy cannot produce reference elements that would point into its
owned storage after the proxy is destroyed.
After conversion, later operations act on the materialized tuple's values or
references: its values are independent copies or moved objects, while its
reference elements still alias their sources.

---

### 🧩 Tuple-like Concept and Recognized Types

In JH Toolkit, **tuple-likeness** is defined purely by *structure declaration*,
not by inheritance or explicit tagging.  

A type is considered `tuple_like` **if and only if** it provides:

* `std::tuple_size<T>`
* `std::tuple_element<I, T>`
* ADL-resolvable `get<I>(T)`

Once these are present, the type explicitly declares

> "I support structured binding — I am tuple_like."

---

#### ✅ Tuple-like types in JH Toolkit

Within the standard layer (`jh` and `std` namespaces):

| Category                    | Types                             |
|-----------------------------|-----------------------------------|
| **Standard tuples**         | `std::tuple`, `std::pair`         |
| **POD tuples**              | `jh::pod::tuple`, `jh::pod::pair` |
| **Array-like (fixed size)** | `std::array`, `jh::pod::array`    |
| **Range proxies**           | `jh::ranges::zip_reference_proxy` |

All the above types are **semantically tuple_like**
and will be **automatically flattened** by `flatten_proxy`.

---

#### 🚫 How to avoid array flattening

If you do **not** want arrays to be flattened:  
use types that **do not** define `std::tuple_size` / `std::tuple_element`, such as:

* `std::vector`
* `jh::runtime_arr`
* or a custom container without structured-binding declarations.

By design, `flatten_proxy` cannot accidentally flatten a type —
`std::tuple_size` and `std::tuple_element` cannot be defined by accident.

---

## 🔹 API Reference

### `tuple_materialize()`

```cpp
template<typename Tuple>
constexpr auto tuple_materialize(const Tuple &t);
```

**Description:**
Flattens a nested `tuple_like` object `t` into a fully materialized single-level `std::tuple`.  
All nested tuple layers are expanded according to the recursive flattening model above.

| Parameter | Type           | Description                            |
|-----------|----------------|----------------------------------------|
| `t`       | `const Tuple&` | A `tuple_like` object to be flattened. |

**Returns:**  
A single-level `std::tuple` containing all recursively expanded elements.  

**Properties:**

* Compile-time deterministic mapping.
* Eliminates empty sub-tuples.
* Produces value elements; referenced source elements are read and copied.
* `flatten_proxy` preserves element references and rejects materialized tuples containing `std::reference_wrapper`.

---

### `flatten_proxy`

```cpp
template<typename Tuple>
struct flatten_proxy {
    Tuple tuple;

    template<typename Source>
    constexpr explicit flatten_proxy(Source&& source);

    template<std::size_t I>
    [[nodiscard]] constexpr auto get() const noexcept;

    template<typename... Ts>
    constexpr operator std::tuple<Ts...>() &;

    template<typename... Ts>
    constexpr operator std::tuple<Ts...>() const &;

    template<typename... Ts>
    constexpr operator std::tuple<Ts...>() &&;

    template<typename... Ts>
    constexpr operator std::tuple<Ts...>() const &&;
};
```

**Description:**  
`flatten_proxy` is a **lazy view** of a flattened `tuple_like` object.  
It behaves like a flattened tuple, supports **structured binding**,
and can be **implicitly converted** to a fully materialized `std::tuple`.  

Constructing from an lvalue stores a reference to the source; constructing from
an rvalue stores it by value. Conversion forwards flattened elements directly
to the destination tuple, so it does not create an intermediate value tuple.
`auto result = proxy` keeps the proxy type and does not materialize a tuple;
declare a `std::tuple<...>` target to request the conversion.
Fully value-owned nested input can be materialized in a constant expression:

```cpp
constexpr std::tuple<int, int, int> flat =
    jh::meta::flatten_proxy{
        std::tuple{std::tuple{1, std::tuple{2}}, std::tuple{3}}
    };
static_assert(flat == std::tuple{1, 2, 3});
```

**Key Traits:**

* Lightweight proxy — lvalue inputs are held by reference and rvalue inputs are owned.
* Reference-safe — preserves reference elements and rejects materialized tuples containing `std::reference_wrapper`.
* Fully usable in `constexpr` / `consteval` contexts.
* Provides structural compatibility with `std::tuple` via specialization.

---

## 🔹 Design Notes

| Design Aspect                | Description                                                                    |
|------------------------------|--------------------------------------------------------------------------------|
| **Recursive tuple mapping**  | Each nested `tuple_like` contributes elements to a compile-time mapping table. |
| **Compile-time determinism** | Flattening topology resolved entirely during template instantiation.           |
| **Empty-collapse semantics** | Nested empty tuples collapse automatically.                                    |
| **Tuple-like openness**      | Any type declaring `tuple_size`, `tuple_element`, and `get<I>` is accepted.    |
| **Constexpr compliance**     | Entire pipeline works at compile time.                                         |
| **Zero overhead**            | No runtime recursion or heap allocation.                                       |

---

## 🧩 Summary

`jh::meta::flatten_proxy` provides the **compile-time flattening infrastructure**
for tuple-like meta utilities in JH Toolkit.

It ensures deterministic, constexpr-resolved flattening
and a consistent structural interface for meta-level tuple manipulation.

| Component           | Description                                                                |
|---------------------|----------------------------------------------------------------------------|
| `tuple_materialize` | Compile-time recursive flattening into `std::tuple`.                       |
| `flatten_proxy`     | Lazy flattened view with structured binding and implicit tuple conversion. |
