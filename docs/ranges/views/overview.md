# 🔭 **JH Toolkit — Views Module Overview**

📁 **Module:** `<jh/views>`  
📦 **Namespace:** `jh::views`  
📍 **Location:** `jh/ranges/views/`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)

</div>

---

## 🧭 Introduction

The **Views Module** (`jh::views`) provides range adaptors for C++20/23 pipelines.
Its adaptors express traversal, projection, consumption, and reentrancy behavior
through their interfaces.

---

## 🔹 Core Components

| Component Name             | Header                              | Status   | Description                                                                                    |
|----------------------------|-------------------------------------|----------|------------------------------------------------------------------------------------------------|
| `jh::views::zip`           | `<jh/ranges/views/zip.h>`           | ✅ Stable | Generalized, sequence-aware zip adaptor supporting multi-pipeline composition.                 |
| `jh::views::enumerate`     | `<jh/ranges/views/enumerate.h>`     | ✅ Stable | Index-value adaptor equivalent to Python-style enumerate; implemented via zip.                 |
| `jh::views::flatten`       | `<jh/ranges/views/flatten.h>`       | ✅ Stable | Observation-preserving flatten adaptor compatible with both consuming and non-consuming flows. |
| `jh::views::transform`     | `<jh/ranges/views/transform.h>`     | ✅ Stable | Unified transform adaptor that dispatches between consumptive and observational semantics.     |
| `jh::views::vis_transform` | `<jh/ranges/views/vis_transform.h>` | ✅ Stable | Explicitly non-consuming "visual" transform for reentrant, observation-only projections.       |
| `jh::views::common`        | `<jh/ranges/views/common.h>`        | ✅ Stable | Upgraded common adaptor with transparent forwarding and extended normalization support.        |

---

## 🔹 Design Semantics

### Key design features

* **Semantic augmentation** — strengthens and clarifies the intent of traversal and projection.
* **Cross-domain interoperability** — allows external or structurally limited iterable types
  to participate naturally in pipelines without conversion overhead.
* **Transparent forwarding** — avoids redundant wrapping of already conforming types,
  ensuring compile-time and runtime efficiency.
* **Behavioral preservation** — propagation of consumption or reentrancy semantics is explicit and deterministic.
* **Pipeline completeness** — provides missing pipe-form adaptors in certain standard library implementations (e.g.
  libc++).

---

## 🧩 Compatibility and Transition

| Version    | Behavior                                                                                                                                                                |
|------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **1.3.3+** | Provides both `<jh/views.h>` (**aggregated**) and `<jh/views>` (**forwarding**) headers. The aggregated form re-exports `jh::ranges::views` for backward compatibility. |
| **1.4.0**  | `<jh/views.h>` will be **deprecated and removed**. `<jh/views>` becomes the canonical **aggregate-forwarding entry**.                                                   |

In **v1.3.3+** and later:

```cpp
namespace jh::views {
    using namespace jh::ranges::views;
}
```

---

## 🧭 Navigation

|           Resource           |                                                                          Link                                                                          |
|:----------------------------:|:------------------------------------------------------------------------------------------------------------------------------------------------------:|
|    🗂️ **Back to Ranges**    |                      [![Back to Ranges](https://img.shields.io/badge/Back%20to%20Ranges-blue?style=flat-square)](../overview.md)                       |
|      📘 **Go to `zip`**      |                    [![Go to Zip Reference](https://img.shields.io/badge/Go%20to%20Zip%20Reference-green?style=flat-square)](zip.md)                    |
|   📗 **Go to `enumerate`**   |           [![Go to Enumerate Reference](https://img.shields.io/badge/Go%20to%20Enumerate%20Reference-green?style=flat-square)](enumerate.md)           |
|    📙 **Go to `flatten`**    |              [![Go to Flatten Reference](https://img.shields.io/badge/Go%20to%20Flatten%20Reference-green?style=flat-square)](flatten.md)              |
|   📘 **Go to `transform`**   |           [![Go to Transform Reference](https://img.shields.io/badge/Go%20to%20Transform%20Reference-green?style=flat-square)](transform.md)           |
| 📗 **Go to `vis_transform`** | [![Go to Visual Transform Reference](https://img.shields.io/badge/Go%20to%20Visual%20Transform%20Reference-green?style=flat-square)](vis_transform.md) |
|    📙 **Go to `common`**     |               [![Go to Common Reference](https://img.shields.io/badge/Go%20to%20Common%20Reference-green?style=flat-square)](common.md)                |
