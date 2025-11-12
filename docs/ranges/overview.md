# 🗂️ **JH Toolkit — Ranges Repository Overview**

📦 **Namespace:** `jh::ranges`  
📍 **Location:** `jh/ranges/`  
📅 **Version:** 1.3.5 (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::ranges` is the **core repository** of the JH Toolkit’s range ecosystem.  
It defines the **foundational range infrastructure** that unifies:

* the **view adaptor layer** (`<jh/views>`), and
* the **semantic extension layer** (`<jh/ranges_ext>`).

This repository is **not a single header** like `<ranges>` in STL;  
it is a **modular repository** providing range–view–extension interoperability
across the entire JH Toolkit.

All components here conform to `std::ranges::range` semantics
and ensure full compatibility with both C++20 and C++23 range algorithms.

---

## 🔹 Repository Composition

| Module / Component   | Header / Path                      | Status   | Role                                                                                     |
|----------------------|------------------------------------|----------|------------------------------------------------------------------------------------------|
| `<jh/views>`         | `jh/ranges/views/`                 | ✅ Stable | Range adaptor layer — defines transform, zip, enumerate, flatten, etc.                   |
| `<jh/ranges_ext>`    | `jh/ranges/`                       | ✅ Stable | Semantic extension layer — implements `adapt`, `collect`, `to` for conversion semantics. |
| `range_wrapper`      | `<jh/ranges/range_wrapper.h>`      | ✅ Stable | Bridges any `jh::concepts::sequence` into a valid `std::ranges::range`.                  |
| `zip_view`           | `<jh/ranges/zip_view.h>`           | ✅ Stable | C++20-compatible fallback for `std::ranges::zip_view`.                                   |
| `vis_transform_view` | `<jh/ranges/vis_transform_view.h>` | ✅ Stable | Non-consuming observational transform view backing `jh::views::vis_transform`.           |

---

## 🧩 Structural Overview

The `jh::ranges` repository represents the **complete range model** of JH Toolkit:

| Layer                    | Representative Components                                       | Purpose and Design Role                                            |
|--------------------------|-----------------------------------------------------------------|--------------------------------------------------------------------|
| **Range Infrastructure** | `range_wrapper`, `zip_view`, `vis_transform_view`               | Core primitives ensuring compatibility and structural consistency. |
| **View Adaptors**        | `<jh/views>` — transform, zip, enumerate, flatten, common, etc. | Lazy evaluation and pipeline composition.                          |
| **Semantic Extensions**  | `<jh/ranges_ext>` — adapt, collect, to                          | Materialization and conversion semantics beyond standard adaptors. |

Together, they provide a **pre-C++23-complete range ecosystem**
that bridges lazy transformation, semantic extension, and structural normalization.

---

## 🧭 Navigation

|               Resource                |                                                                             Link                                                                              |
|:-------------------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------:|
|         🏠 **Back to README**         |                         [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                          |
|   📗 **Go to `<jh/views>` Module**    |                 [![Go to Views Overview](https://img.shields.io/badge/Go%20to%20Views%20Overview-green?style=flat-square)](views/overview.md)                 |
| 📘 **Go to `<jh/ranges_ext>` Module** |             [![Go to Ranges Ext Overview](https://img.shields.io/badge/Go%20to%20Ranges%20Ext%20Overview-green?style=flat-square)](range_ext.md)              |
|     📙 **Go to `range_wrapper`**      |       [![Go to Range Wrapper Reference](https://img.shields.io/badge/Go%20to%20Range%20Wrapper%20Reference-green?style=flat-square)](range_wrapper.md)        |
|        📗 **Go to `zip_view`**        |               [![Go to Zip View Reference](https://img.shields.io/badge/Go%20to%20Zip%20View%20Reference-green?style=flat-square)](zip_view.md)               |
|   📘 **Go to `vis_transform_view`**   | [![Go to Visual Transform View Reference](https://img.shields.io/badge/Go%20to%20Vis%20Transform%20Reference-green?style=flat-square)](vis_transform_view.md) |
