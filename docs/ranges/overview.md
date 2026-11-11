# 🗂️ **JH-Toolkit — Ranges Repository Overview**

📦 **Namespace:** `jh::ranges`  
📍 **Location:** `jh/ranges/`  
📅 **Version:** 1.3.5 (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::ranges` is the **core repository** of the JH-Toolkit’s range ecosystem.<br>
It defines the **foundational range infrastructure** that unifies:

* the **view adaptor layer** (`<jh/views>`), and
* the **semantic extension layer** (`<jh/ranges_ext>`).

It is a modular range library providing view and extension interoperability
across the entire JH-Toolkit.

The library adapts JH sequences to standard ranges and supplies C++20
fallbacks for selected C++23 facilities. Each adaptor documents its own
input constraints and supported iterator operations.

Iterator traversal category and semantic reentrancy answer different
questions. The former describes available iterator operations; the latter
requires a source and any applied callable to support repeatable observation.

---

## 🔹 Repository Composition

| Module / Component   | Header / Path                      | Status   | Role                                                                                     |
|----------------------|------------------------------------|----------|------------------------------------------------------------------------------------------|
| `<jh/views>`         | `jh/ranges/views/`                 | ✅ Stable | Range adaptor layer — defines transform, zip, enumerate, flatten, etc.                   |
| `<jh/ranges_ext>`    | `jh/ranges/`                       | ✅ Stable | Semantic extension layer — implements `adapt`, `collect`, `to` for conversion semantics. |
| `range_adaptor`      | `<jh/ranges/range_adaptor.h>`      | ✅ Stable | Bridges any `jh::concepts::sequence` into a valid `std::ranges::range`.                  |
| `zip_view`           | `<jh/ranges/zip_view.h>`           | ✅ Stable | C++20-compatible fallback for `std::ranges::zip_view`.                                   |
| `vis_transform_view` | `<jh/ranges/vis_transform_view.h>` | ✅ Stable | Non-consuming observational transform view backing `jh::views::vis_transform`.           |

---

## 🧩 Structural Overview

The `jh::ranges` repository represents the **complete range model** of JH-Toolkit:

| Layer                    | Representative Components                                       | Purpose and Design Role                                            |
|--------------------------|-----------------------------------------------------------------|--------------------------------------------------------------------|
| **Range Infrastructure** | `range_adaptor`, `zip_view`, `vis_transform_view`               | Core primitives ensuring compatibility and structural consistency. |
| **View Adaptors**        | `<jh/views>` — transform, zip, enumerate, flatten, common, etc. | Lazy evaluation and pipeline composition.                          |
| **Semantic Extensions**  | `<jh/ranges_ext>` — adapt, collect, to                          | Materialization and conversion semantics beyond standard adaptors. |

Together, these layers connect lazy traversal, explicit materialization, and
container construction. Consult each adaptor's contract for its supported
input types and behavior.

---

## 🧭 Navigation

|               Resource                |                                                                             Link                                                                              |
|:-------------------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------:|
|         🏠 **Back to README**         |                         [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                          |
|   📗 **Go to `<jh/views>` Module**    |                 [![Go to Views Overview](https://img.shields.io/badge/Go%20to%20Views%20Overview-green?style=flat-square)](views/overview.md)                 |
| 📘 **Go to `<jh/ranges_ext>` Module** |             [![Go to Ranges Ext Overview](https://img.shields.io/badge/Go%20to%20Ranges%20Ext%20Overview-green?style=flat-square)](range_ext.md)              |
|     📙 **Go to `range_adaptor`**      |       [![Go to Range Adaptor Reference](https://img.shields.io/badge/Go%20to%20Range%20Adaptor%20Reference-green?style=flat-square)](range_adaptor.md)        |
|        📗 **Go to `zip_view`**        |               [![Go to Zip View Reference](https://img.shields.io/badge/Go%20to%20Zip%20View%20Reference-green?style=flat-square)](zip_view.md)               |
|   📘 **Go to `vis_transform_view`**   | [![Go to Visual Transform View Reference](https://img.shields.io/badge/Go%20to%20Vis%20Transform%20Reference-green?style=flat-square)](vis_transform_view.md) |
