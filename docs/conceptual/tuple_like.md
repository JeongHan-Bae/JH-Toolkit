# 🧩 **JH Toolkit — `jh::concepts::tuple_like` API Reference**

📁 **Header:** `<jh/conceptual/tuple_like.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.4.0 (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/Back_to_README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/Back_to_Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Submodule Scope

`jh::concepts::tuple_like` defines the **formal structural contract**
used across the JH Toolkit to recognize **explicit tuple-protocol types**.

This header does **not** introduce tuple semantics.
It only validates whether a type already conforms to the tuple model
used by:

* structured bindings
* associative container insertion
* range and view adaptors
* generic tuple-based metaprogramming

Implicit aggregate decomposition is intentionally excluded.

---

## 🧩 Introduction

A type `T` is considered *tuple-like* if it satisfies the standard tuple protocol:

* `std::tuple_size<T>` is defined
* `std::tuple_element<I, T>` is defined for all valid indices
* `get<I>(t)` is available via ADL for all valid indices

In addition, `tuple_like` enforces a **semantic compatibility rule**:

> For each index `I`,
> `decltype(get<I>(t))` must form a valid
> `std::common_reference_t` with
> `std::tuple_element_t<I, T>`.

This allows proxy references, wrapped values, and view-based tuple elements
to participate safely in generic tuple algorithms.

---

## 🔹 Core Concept

### `jh::concepts::tuple_like<T>`

Satisfied when:

```pseudocode
std::tuple_size<std::remove_cvref_t<T>> is defined
AND
for all valid indices I:
    get<I>(t) is ADL-callable
    AND
    std::common_reference_t<
        decltype(get<I>(t)),
        std::tuple_element_t<I, T>
    > is well-formed
```

This validation is folded at compile time across all indices.

### Properties

* purely structural
* constexpr-only
* zero runtime cost
* proxy-friendly
* excludes implicit aggregates by design

---

## 🔹 Why `std::common_reference_t` Is Required

`std::common_reference_t` acts as the **semantic compatibility gate**
between:

* what `tuple_element<I, T>` *declares*
* what `get<I>(t)` *returns*

Exact type equality is not required.
Semantic equivalence is.

This enables:

* proxy references
* `std::reference_wrapper`
* layered or transformed tuple views
* lazy or deferred element access

If no valid common reference exists, the type is considered structurally inconsistent
and is rejected.

---

## ⚙️ Proxy or Nested Tuple-like Types

If your type acts as a **tuple proxy**, additional setup is only required
when it is *contained by another tuple-like type* whose
`std::tuple_element<I, T>` **declares the element as an equivalent `std::tuple<...>`**
rather than the proxy itself.

In that case, you must provide:

1. An **implicit conversion** to the declared tuple type:

   ```cpp
   operator std::tuple<...>() const noexcept;
   ```
2. A matching set of **`std::common_reference` specializations** bridging
   your proxy and the declared `std::tuple` form.

These make

```cpp
std::common_reference_t<
    decltype(get<I>(outer)),
    std::tuple_element_t<I, outer>
>
```

resolvable and semantically valid.

---

### ✅ When You **Do Not** Need Any Specialization

* The proxy is not nested or contained by other tuple-like types.
* Its `tuple_element<I>` entries already declare itself (`proxy`).
* It is only used as a top-level tuple-like type.

Such cases work out of the box —
no conversion operator or bridge specialization is required.

---

### ⚙️ When You **Do** Need Bridge Definitions

You need bridge definitions only when:

* The proxy is **nested** in another tuple-like type, such as
  `other_proxy_t<proxy, ...>` or `proxy<proxy, ...>`.
* The **outer** `tuple_element<I, T>` declares the element
  as an *equivalent* `std::tuple<...>` rather than the proxy itself.

Then you should define both:

* `operator std::tuple<...>() const noexcept;`
* `std::common_reference` specializations:

| Relationship                                 | Purpose                                           |
|----------------------------------------------|---------------------------------------------------|
| `proxy` ↔ `tuple`                            | Logical equivalence bridge                        |
| `proxy&` ↔ `tuple&`                          | Reference-level bridge                            |
| *(optional)* `const proxy&` ↔ `const tuple&` | Only if `get<I>()` can't auto-deduce const access |
| *(optional)* `proxy&&` ↔ `tuple&&`           | For move-enabled proxies                          |
| *(optional)* `proxy` ↔ `proxy`               | Self-equivalence optimization                     |

---

### 🧠 Practical Notes

If your proxy's `get<I>()` is written using `auto` or perfect forwarding,
it already supports both const and non-const overloads automatically —
the `const proxy&` bridge is unnecessary and generally not recommended.

You only need the const version when your proxy's getter explicitly
differs between const / non-const overloads or lacks deducible reference return.

---

## 🔹 Strict Pair Semantics for Associative Containers

### `jh::concepts::pair_like_for<P, K, V>`

`pair_like_for` is a **strict specialization** of `tuple_like`
designed specifically for **associative container insertion**.

It is inspired by **Clang's permissive associative-container insert rules**
for tuple-like arguments.

#### Motivation

In practice, associative containers:

* do **not** construct their elements by copying or moving a `pair` object
* instead, they **extract the key and value separately**
* and construct internal storage from those extracted components

This is true even when the input object *is* an actual `std::pair`.

As a result:

* the input type only needs to *behave like* a `(key, value)` tuple
* not necessarily be constructible as a `pair` itself

This observation motivated Clang's acceptance of tuple-like objects
in `map::insert` and related APIs.

---

### Concept Definition

`pair_like_for<P, K, V>` is satisfied when:

* `P` satisfies `tuple_like`
* `std::tuple_size_v<P> == 2`
* `get<0>(p)` has **exact type** `K` after `remove_cvref`
* `get<1>(p)` has **exact type** `V` after `remove_cvref`
* no implicit conversions are permitted

Semantic compatibility is **not sufficient** here;
exact type identity is required.

---

### Where It Is Used

This concept is used by JH associative containers, including:

* [`jh::ordered_map`](../core/ordered_map.md)
* [`jh::flat_multimap`](../core/flat_multimap.md)

These containers follow the same construction model:

* extract key and mapped value independently
* construct internal storage from those components
* never copy or move a `pair` object as a whole

---

### Why Exact Matching Is Required

Unlike `tuple_like`, `pair_like_for` enforces **exact element types** because:

* keys must not undergo implicit conversion
* mapped values must not be reinterpreted
* container invariants depend on precise type identity

This ensures deterministic behavior for insertion APIs
while still allowing tuple-like flexibility at the call site.

---

## 🧩 Summary of Bridge Requirements

| Case                                                | Action                                |
|-----------------------------------------------------|---------------------------------------|
| Standalone tuple-like struct                        | Works automatically                   |
| Proxy not contained in another tuple                | No bridge needed                      |
| Proxy nested, `tuple_element` declares itself       | No bridge needed                      |
| Proxy nested, `tuple_element` declares `std::tuple` | Add full bridge and conversion        |
| Proxy lacks const-deducible `get<I>()`              | Optionally add const reference bridge |
| Proxy supports move semantics                       | Optionally add `&&` bridge            |

---

## 🔹 Design Rationale

This mechanism is **not strict**, but **structurally protective**.

If you explicitly declare `tuple_element<I, T>` and `get<I>(t)`
to yield semantically different types —   
for example, declaring an element as a value type but returning a proxy object —
then you must prove that a `std::common_reference_t` exists between them.

If you fail to provide such a bridge,
the type may still bypass `tuple_like` detection,
but later **unpacking templates** (e.g. in range or collection adaptors)
will fail with more explicit and harder errors.

Thus, the `tuple_like` contract ensures that your tuple declarations
are **semantically self-consistent** across nested and proxy contexts.

---

## ✅ Tuple-like Types in JH Toolkit

Within the standard layer (`jh` and `std` namespaces):

| Category                    | Types                             |
|-----------------------------|-----------------------------------|
| **Standard tuples**         | `std::tuple`, `std::pair`         |
| **POD tuples**              | `jh::pod::tuple`, `jh::pod::pair` |
| **Array-like (fixed size)** | `std::array`, `jh::pod::array`    |
| **Range proxies**           | `jh::ranges::zip_reference_proxy` |

---

## ⚙️ Detection Scope

This concept only checks **explicitly structured binding–capable** types,
i.e. those that provide **all three** of:

* `std::tuple_size<T>`
* `std::tuple_element<I, T>`
* ADL-resolvable `get<I>(t)`

Plain aggregates that happen to support structured bindings by decomposition
are *not* recognized as `tuple_like` —
they lack the formal tuple protocol and are excluded by design.

This ensures strict structural consistency while preventing false positives.

---

## 🧩 Summary

* `tuple_like` detects explicit, protocol-compliant tuple structures.
* It validates element semantics via `std::common_reference_t`.
* Proxy and nested tuple-like types can cooperate safely through
  conversion and `common_reference` bridges when required.
* Aggregates with implicit bindings are deliberately excluded.
* Built-in JH types (`jh::pod::tuple`, `jh::ranges::zip_reference_proxy`)
  fully conform to this model.
