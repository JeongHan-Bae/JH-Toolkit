# 🧩 **JH Toolkit — `jh::concepts::tuple_like` API Reference**

📁 **Header:** `<jh/conceptual/tuple_like.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/Back_to_README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/Back_to_Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Submodule Notice

`jh::concepts::tuple_like` defines the **structural rule set**
for detecting and validating tuple-compatible types.  
It generalizes `std::tuple`, `std::pair`, and similar structured aggregates,
while preserving compatibility with proxy-based tuple models such as
range zip proxies or nested view elements.

---

## 🧩 Introduction

`tuple_like` is a **structural concept**:  
a type `T` is tuple-like if it satisfies the standard tuple protocol —

1. `std::tuple_size<T>` is defined.
2. `std::tuple_element<I, T>` is defined for all indices `I`.
3. `get<I>(t)` is callable via ADL for each `I`.

Additionally, the return type of `get<I>(t)` must form a valid
`std::common_reference_t` with `std::tuple_element_t<I, T>`,
ensuring element-level semantic compatibility.

This design allows not only plain tuples and aggregates,
but also **proxy or view types** that model tuple behavior through
custom reference wrappers or structured bindings.

---

## 🔹 Core Concept

### `jh::concepts::tuple_like<T>`

Satisfied when:

```cpp
requires {
    typename std::tuple_size<std::remove_cvref_t<T>>::type;
    typename std::tuple_element_t<I, std::remove_cvref_t<T>>;
    get<I>(std::declval<T>());
    requires std::common_reference_t<
        decltype(get<I>(std::declval<T>())),
        std::tuple_element_t<I, std::remove_cvref_t<T>>
    >;
}
```

This validation is folded across all valid indices of `std::tuple_size_v<T>`.

The check is **tolerant** — it doesn't require exact type matches.  
Instead, it validates *semantic compatibility* through `std::common_reference_t`,
so proxy references, wrapped values, and nested tuple elements
can coexist safely under structured bindings.

---

## 🔹 Why `std::common_reference_t`

`std::common_reference_t` serves as the **semantic bridge**
between what `tuple_element` *declares* and what `get<I>()` *returns*.

This allows legitimate cases such as:

* Returning `std::reference_wrapper<T>` for an element declared as `T&`.
* Returning proxy objects convertible to the declared element type.
* Nested or layered proxies that ultimately behave as tuples.

As long as a valid `std::common_reference_t` exists between the two,
the concept considers them semantically equivalent.

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
