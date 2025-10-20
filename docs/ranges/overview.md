# 🗂️ **JH Toolkit — Ranges Repository Overview**

📦 **Namespace:** `jh::ranges`  
📍 **Location:** `jh/ranges/`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
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
| [`zip_view<...>`](zip_view.md)         | `<jh/ranges/zip_view.h>`      | ✅ Stable              | Early implementation of C++23 `std::ranges::zip_view`; acts as a lazy tuple-based range combiner.                        |
| [`views/`](views/overview.md)          | `<jh/ranges/views/>`          | ✅ Stable / Extensible | Extensible repository of range adaptors built atop this layer; each adaptor composes directly over `std::ranges::range`. |

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
