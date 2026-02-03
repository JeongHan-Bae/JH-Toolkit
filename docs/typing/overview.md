# 🧬 **JH Toolkit — Typing Subsystem Overview**

📁 **Module:** `<jh/typed>`  
📦 **Namespace:** `jh::typed`  
📍 **Location:** `jh/typing/`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::typed` defines the **Typing subsystem** —  
a group of components related to **type modeling** and **meta-programming primitives** in the JH Toolkit.

This subsystem provides minimal, header-only utilities that support
type traits, semantic placeholders, and structural typing used across generic programming contexts.

---

## 🌍 Overview

The Typing subsystem focuses on utilities that clarify or manipulate
types at the compile-time level.  
It includes both small self-contained primitives (like `monostate`)
and type-adapted constructs (like `null_mutex`) designed to integrate
smoothly with the Toolkit's concept and synchronization layers.

Future releases may extend this subsystem with meta-type reflection tools,
concept adapters, and POD-safe compile-time helpers.

---

## 🔹 Core Components

| Component    | Header                     |  Status  | Description                                                                                        |
|--------------|----------------------------|:--------:|----------------------------------------------------------------------------------------------------|
| `monostate`  | `<jh/typing/monostate.h>`  | ✅ Stable | POD-safe placeholder that explicitly represents "no value".                                        |
| `null_mutex` | `<jh/typing/null_mutex.h>` | ✅ Stable | Zero-overhead placeholder for `mutex_like` synchronization; <br> all locking functions are no-ops. |

---

## 🧠 Design Notes

* Focuses on **typing**, **templating**, and **meta-programming** facilities.
* Keeps all components **STL-minimal**, **header-only**, and **constexpr-safe**.
* Establishes common building blocks for type utilities and concept integration.
* Intended to evolve as the base layer for future meta-type and compile-time systems.

---

## 🧩 Module Summary

* **Scope:** typing helpers, concept adapters, and structural placeholders.
* **Integration:** used by concept and synchronization modules.
* **Design focus:** clarity of type semantics, composability, and zero runtime cost.
* **Extensibility:** open for new meta-programming and compile-time utilities.

---

## 🧭 Navigation

|         Resource          |                                                                  Link                                                                   |
|:-------------------------:|:---------------------------------------------------------------------------------------------------------------------------------------:|
|   🏠 **Back to README**   |              [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)               |
| 📘 **Go to `monostate`**  |   [![Go to Monostate Reference](https://img.shields.io/badge/Go%20to%20Monostate%20Reference-green?style=flat-square)](monostate.md)    |
| 📗 **Go to `null_mutex`** | [![Go to Null Mutex Reference](https://img.shields.io/badge/Go%20to%20Null%20Mutex%20Reference-green?style=flat-square)](null_mutex.md) |

---

> 📌 The **Typing subsystem** provides developers with concise, dependable tools
> for expressing type intent and structure — helping eliminate unnecessary runtime checks,
> avoid RTTI usage, and simplify optimization and generic programming workflows.
