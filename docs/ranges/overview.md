# 🗂️ **JH Toolkit — Ranges Repository Overview**

📦 **Namespace:** `jh::ranges`  
📍 **Location:** `jh/ranges/`  
📅 **Version:** 1.3.4 (code freeze)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::ranges` is a **support repository** providing range adapters and view bases
that operate within the standard C++ ranges framework.

All components here are valid `std::ranges::range` models.
They exist to ensure interoperability and consistency between
JH Toolkit's lazy evaluation utilities and standard range algorithms.

Unlike higher-level modules, this repository exposes **no aggregated header**.
It serves as an internal infrastructure layer used primarily by `jh::ranges::views`.

---

## 🔹 Core Components

| Component                              | Header                        | Status                | Description                                                                                                              |
|----------------------------------------|-------------------------------|-----------------------|--------------------------------------------------------------------------------------------------------------------------|
| [`range_wrapper<T>`](range_wrapper.md) | `<jh/ranges/range_wrapper.h>` | ✅ Stable              | Wraps any `jh::concepts::sequence` into a standard `std::ranges::range`; provides sequence–range bridging.               |
| [`zip_view<...>`](zip_view.md)         | `<jh/ranges/zip_view.h>`      | 🟡 Minor              | Early implementation of C++23 `std::ranges::zip_view`; acts as a lazy tuple-based range combiner.                        |
| [`views/`](views/overview.md)          | `<jh/ranges/views/>`          | 🟡 Minor / Extensible | Extensible repository of range adaptors built atop this layer; each adaptor composes directly over `std::ranges::range`. |

---

## 🧩 Repository Summary

`jh::ranges` defines the **range-level infrastructure** of the JH Toolkit —
a lightweight repository of adapters and pre-standard range components.

`range_wrapper` bridges behavioral sequences into range-conformant forms,
while `zip_view` anticipates the standardized `std::ranges::zip_view`
to maintain forward compatibility across C++ versions.

This repository focuses purely on **structural consistency**
and **compatibility with the evolving C++ ranges model**.

---

## 🧭 Navigation

|            Resource             |                                                                       Link                                                                       |
|:-------------------------------:|:------------------------------------------------------------------------------------------------------------------------------------------------:|
|      🏠 **Back to README**      |                   [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                   |
|  📘 **Go to `range_wrapper`**   | [![Go to Range Wrapper Reference](https://img.shields.io/badge/Go%20to%20Range%20Wrapper%20Reference-green?style=flat-square)](range_wrapper.md) |
|     📗 **Go to `zip_view`**     |        [![Go to Zip View Reference](https://img.shields.io/badge/Go%20to%20Zip%20View%20Reference-green?style=flat-square)](zip_view.md)         |
| 🔭 **Go to `views` Repository** |          [![Go to Views Overview](https://img.shields.io/badge/Go%20to%20Views%20Overview-orange?style=flat-square)](views/overview.md)          |

---

## 🔮 1.3.5 Preview — `jh/ranges_ext` and Semantic Expansion

Starting from **version 1.3.5**, the `jh::ranges` repository will expand to include
a new module:  
**`<jh/ranges_ext>`**, designed to provide *semantic enhancers*
for range transformations and conversions — similar to `std::ranges::to` in C++23 and beyond.  

### **Highlights**

* **Fixes**: Addresses known limitations in `zip_view` and `jh::ranges::views`.  
* **New Header:** `<jh/ranges_ext>` — contains extended functional objects under `jh::ranges`.  
* **Purpose:** Implements enhanced "conversion-style" range operations (`to`, `collect`, etc.)
  with **wider input acceptance** and **duck-type compatibility**.  
* **Placement:** Parallel to `<jh/views>`, forming the dual extension layer of the JH Toolkit.

---

### **Component Stratification (Design Model)**

The standard `<ranges>` ecosystem (C++20–C++23) can be viewed as consisting of **three structural layers**:

| Layer                   | Representative Components                                                             | JH Toolkit Strategy                                                                               | Engineering Role                                                   |
|:------------------------|:--------------------------------------------------------------------------------------|:--------------------------------------------------------------------------------------------------|:-------------------------------------------------------------------|
| **Range Objects**       | `std::ranges::subrange`, **range concept**, `std::ranges::zip_view` (C++23+)          | Directly adopted or lightly extended for compatibility                                            | Core structural primitives defining range semantics                |
| **View Adaptors**       | `std::views::transform`, `std::views::zip` (C++23+), `std::views::enumerate` (C++23+) | Extended as `<jh/views>` to support broader sequence inputs and early adoption of C++23 behaviors | Lazy evaluation and pipeline composition layer                     |
| **Semantic Extensions** | `std::ranges::to` (C++23+), `std::ranges::repeat` (C++23+)                            | Introduced as `<jh/ranges_ext>`; implements pre-standard and duck-typed equivalents               | Semantic enhancement layer for conversion and range transformation |

---

**Design Summary:**

* **Range Objects** — Define *what a range is*; JH Toolkit follows STL semantics closely, using them as infrastructure.  
* **View Adaptors (`<jh/views>`)** — Define *how ranges transform*; extended to accept duck-typed `jh::sequence`.  
* **Semantic Extensions (`<jh/ranges_ext>`)** — Define *how ranges convert or materialize*; provide `to`-like and high-level conversion behaviors.

Thus,

> The `jh::ranges` repository acts as the **infrastructure layer** unifying these three semantics —
> bridging **range objects**, **views**, and **semantic extensions** into a consistent, pre-C++23-compatible ecosystem.  

> 🕓 *From 1.3.5 onward, the `ranges` repository becomes the foundation for
> both `jh::views` (lazy adaptors) and `jh::ranges_ext` (semantic extensions),
> providing a complete pre-C++23 range model ecosystem.* 
