# 🧩 **JH Toolkit — POD Concepts Overview**

📁 **Header:** `<jh/conceptual/pod_concepts.h>`  
📦 **Namespace:** `jh::concepts`  
📅 **Version:** **1.4.x** (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`<jh/conceptual/pod_concepts.h>` is the **conceptual aggregation header**
for all **POD-related concepts, traits, and constants** exposed under
the `jh::concepts` namespace.

This header introduces **no new logic**.
It exists solely to **re-export POD contracts** defined in <code>&lt;jh/pods/*.h&gt;</code>,
so that other modules may depend on `<jh/concepts>` without including
implementation headers.

---

## Header Usage Policy

`<jh/conceptual/pod_concepts.h>` is **not intended for direct inclusion**.

This header exists solely as a **conceptual aggregation layer** and should be
consumed only through higher-level umbrella headers:

* **If you need POD implementations or containers**
  use:

  ```cpp
  #include <jh/pod>
  ```

* **If you need concepts (including POD concepts) exposed under `jh::concepts`**
  use:

  ```cpp
  #include <jh/concepts>
  ```

Directly including `<jh/conceptual/pod_concepts.h>` bypasses the intended
module boundaries and is **not part of the public include contract**.

This rule ensures:

* clear separation between **implementation** (`jh::pod`)
* unified access to **conceptual interfaces** (`jh::concepts`)
* stable include behavior across the entire toolkit

---

## 🌍 What Is Exposed

All symbols below are **direct aliases** of definitions in <code>&lt;jh/pods/*.h&gt;</code>.
Links point to the authoritative POD documentation.

| Symbol (navigation)                         | Origin            |
|---------------------------------------------|-------------------|
| [`pod_like`](../pods/pod_like.md)           | `jh::pod`         |
| [`cv_free_pod_like`](../pods/pod_like.md)   | `jh::pod`         |
| [`max_pod_array_bytes`](../pods/array.md)   | `jh::pod`         |
| [`max_pod_bitflags_bytes`](../pods/bits.md) | `jh::pod`         |
| [`trivial_bytes`](../pods/bytes_view.md)    | `jh::pod`         |
| [`linear_container`](../pods/span.md)       | `jh::pod::detail` |
| [`linear_status`](../pods/span.md)          | `jh::pod::detail` |
| [`streamable`](../pods/stringify.md)        | `jh::pod`         |
| [`streamable_pod`](../pods/stringify.md)    | `jh::pod`         |

> `detail`-scoped symbols are re-exported intentionally for
> **advanced compile-time and serialization tooling**.
> Their presence here does not imply general-purpose use.

---

## 🔹 Integration Pattern

* Generic constraints should use `jh::concepts::pod_like<T>`
* Buffer sizing and layout limits come from <code>max_pod_*_bytes</code>
* Streaming and serialization layers gate behavior via
  `streamable` / `streamable_pod`

---

## 🧠 Structure Policy

* `jh::concepts` **depends on** `jh::pod`
* `jh::pod` **never depends on** `jh::concepts`
* This guarantees:

    * independent compilation of POD utilities
    * a stable conceptual surface
    * zero leakage of implementation details into generic code

---

## 🧩 Summary

* `<jh/conceptual/pod_concepts.h>` is a **pure aggregation layer**
* It provides **navigation and unification**, not abstraction
* All symbols are **aliases**, not wrappers
* Documentation links live on the **symbols themselves**
* The file exists to keep **compile-time contracts clean and centralized**
