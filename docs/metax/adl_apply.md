# ⚗️ **JH Toolkit — `jh::meta::adl_apply` API Reference**

📁 **Header:** `<jh/metax/adl_apply.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::adl_apply` provides a **constexpr-safe, ADL-enabled universal tuple invocation**
compatible with any type modeling [`jh::concepts::tuple_like`](../conceptual/tuple_like.md#-core-concept).

It is functionally equivalent to `std::apply`,
but extends applicability to all tuple-like structures by resolving `get<I>` via **unqualified lookup (ADL)**.

For standard tuple types (`std::tuple`, `std::pair`, `std::array`),
`adl_apply` behaves identically to `std::apply`, since their `get<I>` functions
reside in namespace `std` and naturally participate in ADL.

---

## 🔹 Overview

| Aspect                   | Description                                                                          |
|--------------------------|--------------------------------------------------------------------------------------|
| **Purpose**              | Invoke a callable with unpacked elements of a tuple-like object.                     |
| **Scope**                | Works with all `tuple_like` types, including proxies and views.                      |
| **Expansion Depth**      | Opens **only one structural layer** — for recursive flattening, use `flatten_proxy`. |
| **Implementation Model** | Unqualified `get<I>` lookup to preserve ADL participation.                           |
| **Compatibility**        | Syntax and semantics identical to `std::apply`.                                      |

---

## 🔹 Core Component

| Symbol        |   Type   | Description                                                     |
|---------------|:--------:|-----------------------------------------------------------------|
| `adl_apply()` | Function | ADL-aware universal callable invocation for tuple-like objects. |

---

## 🔹 Expansion and Lookup Model

`adl_apply` performs **single-level expansion** of a tuple-like object and
invokes a callable with its directly contained elements.  
It does **not** recursively flatten nested tuple-likes —
for that, combine it with [`jh::meta::flatten_proxy`](flatten_proxy.md)
or call `tuple_materialize()` beforehand.

### 🌐 Step-by-step logic

1. **Input requirement**  
   The argument `t` must satisfy [`jh::concepts::tuple_like`](../conceptual/tuple_like.md).
   It must define `std::tuple_size`, `std::tuple_element`,  
   and provide an ADL-visible `get<I>`.

2. **Compile-time expansion**  
   An index sequence `[0, std::tuple_size_v<T>)` is generated for expansion.

3. **ADL-based element access**  
   Each element is retrieved using unqualified `get<I>(t)`,
   allowing lookup to resolve both standard and user-defined overloads.

4. **Callable invocation**  
   The callable is invoked as:

   ```cpp
   std::invoke(std::forward<F>(f), get<0>(t), get<1>(t), ..., get<N-1>(t));
   ```

5. **Depth limitation**  
   Only the *current layer* of `t` is expanded.  
   If an element itself is tuple-like, its internal structure remains intact until explicitly flattened.

6. **Forwarding and noexcept propagation**  
   Both the callable and tuple-like object are perfectly forwarded;  
   `constexpr` and exception guarantees are fully preserved.

---

### 🔁 Recursion Note

`adl_apply` does **not** recursively traverse nested tuple-likes.
It opens one structural layer at a time.  
For deeply nested structures, use:

```cpp
jh::meta::adl_apply(f, jh::meta::flatten_proxy{nested_tuple});
```

This flattens the hierarchy first, then performs ADL-based invocation.

---

## 🔹 Recognized Tuple-like Types

`adl_apply` supports all structures modeling [`tuple_like`](../conceptual/tuple_like.md):

| Category            | Example Types                                                      |
|---------------------|--------------------------------------------------------------------|
| **Standard tuples** | `std::tuple`, `std::pair`, `std::array`                            |
| **POD tuples**      | `jh::pod::tuple`, `jh::pod::pair`, `jh::pod::array`                |
| **Range proxies**   | `jh::ranges::zip_reference_proxy`                                  |
| **User-defined**    | Any type declaring `tuple_size`, `tuple_element`, and ADL `get<I>` |

Because `std::get` participates in ADL for standard types,
`jh::meta::adl_apply` can always replace `std::apply` transparently
without altering semantics or code generation.

---

## 🔹 API Reference

### `adl_apply()`

```cpp
template<class F, jh::concepts::tuple_like T>
constexpr decltype(auto) adl_apply(F&& f, T&& t)
    noexcept(noexcept(
        std::invoke(
            std::forward<F>(f),
            get<I>(std::forward<T>(t))... // via ADL lookup
        )
    ));
```

**Description:**  
Invokes a callable `f` with the unpacked elements of a tuple-like `t`,
resolving `get<I>` through ADL to support user-defined tuple-like types.

| Parameter | Type  | Description                  |
|-----------|-------|------------------------------|
| `f`       | `F&&` | Callable object to invoke.   |
| `t`       | `T&&` | Tuple-like object to unpack. |

**Returns:**  
Result of invoking `f` with the unpacked elements of `t`.

**Properties:**

* `constexpr` and `noexcept` propagation identical to `std::apply`.
* Perfect forwarding for both callable and tuple object.
* No temporary tuples or allocations.
* Fully compatible with ADL-defined `get<I>`.

---

## 🔹 Usage Syntax Equivalence

`jh::meta::adl_apply` shares the same syntax as `std::apply`,
while extending support to ADL-visible `get<I>` implementations.

**Fixed arity example**

```cpp
jh::meta::adl_apply([](auto&& x, auto&& y) {
    std::cout << x << ", " << y << '\n';
}, pair_proxy);
```

**Variable arity (generic) example**

```cpp
jh::meta::adl_apply(
    [&](auto&&... args) {
        // do something generic
    },
    some_tuple_like);
```

Both forms compile and behave exactly like `std::apply`,
except that `adl_apply` can also unpack user-defined tuple-like proxies
(e.g., `zip_reference_proxy`, or custom structured bindings).

---

## 🔹 Design Notes

| Design Aspect              | Description                                                                                 |
|----------------------------|---------------------------------------------------------------------------------------------|
| **ADL correctness**        | Uses unqualified `get<I>` to enable full argument-dependent lookup.                         |
| **Single-level expansion** | Expands only the outermost tuple-like layer; recursive flattening requires `flatten_proxy`. |
| **Standard compatibility** | Identical semantics and code generation to `std::apply` for STL tuple types.                |
| **Language conformance**   | Avoids illegal specialization of `std::get`; 100% standard-conforming.                      |
| **Constexpr safety**       | Usable in compile-time and `consteval` contexts.                                            |
| **Zero overhead**          | Inline expansion at compile time; no runtime cost.                                          |

---

## 🧩 Summary

`jh::meta::adl_apply` is a **universal, ADL-aware alternative to `std::apply`**
that safely generalizes tuple invocation to any structure modeling `tuple_like`.  

It opens exactly one tuple-like layer per call, preserving constexpr and noexcept guarantees.  
For recursive expansion, combine it with `flatten_proxy`.

| Component   | Description                                                                      |
|-------------|----------------------------------------------------------------------------------|
| `adl_apply` | ADL-enabled, constexpr-safe universal tuple invocation (single-level expansion). |
