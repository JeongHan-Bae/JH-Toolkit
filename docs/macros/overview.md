# 🔮 **JH Toolkit — Macros Repository Overview**

📦 **Namespace:** `jh::macro`  
📍 **Location:** `jh/macros/`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::macro` is the **foundation repository** for all compiler- and preprocessor-level
features of the JH Toolkit.  
It centralizes every macro-based mechanism to ensure **platform portability**,
**compiler feature detection**, and **header-level consistency**.  

This repository consolidates all **macro magic** into a controlled environment —  
preventing external leakage of preprocessor definitions and isolating all
cross-compiler workarounds from higher-level headers.

All other repositories in the toolkit implicitly depend on `jh::macro`.  
It acts as the **lowest-level layer** beneath `jh::pod`, `jh`, etc.

---

## 🔹 Core Components

| Component                                   | Header                                                    | Status   | Description                                                                                 |
|---------------------------------------------|-----------------------------------------------------------|----------|---------------------------------------------------------------------------------------------|
| [`platform`](platform.md)                   | `<jh/macros/platform.h>`                                  | ✅ Stable | Unified compiler / OS / architecture detection layer used across all repositories.          |
| [`type_name`](type_name.md)                 | `<jh/macros/type_name.h>`                                 | ✅ Stable | Compile-time type name extraction without RTTI; used in diagnostic and debug-only contexts. |
| [`dual-mode headers`](dual_mode_headers.md) | `<jh/macros/header_begin.h>` / `<jh/macros/header_end.h>` | ✅ Stable | Defines consistent header entry/exit guards for dual C/C++ linkage and interoperability.    |

---

## 🧩 Repository Summary

`jh::macro` defines the **meta-infrastructure layer** of the toolkit —
containing every cross-platform, compile-time, and header-level construct
required by the upper modules.  

The *Dual-Mode Header System* ensures safe inclusion for both C and C++ environments.  
`platform` standardizes feature detection macros,
and `type_name` provides compile-time reflection support for POD debugging.  

Together, these components form a **closed macro domain**,
ensuring that preprocessor logic never escapes into user space.  

---

## 🧭 Navigation

|            Resource            |                                                                             Link                                                                             |
|:------------------------------:|:------------------------------------------------------------------------------------------------------------------------------------------------------------:|
|     🏠 **Back to README**      |                         [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                         |
|    📘 **Go to `platform`**     |               [![Go to Platform Reference](https://img.shields.io/badge/Go%20to%20Platform%20Reference-green?style=flat-square)](platform.md)                |
|    📗 **Go to `type_name`**    |             [![Go to Type Name Reference](https://img.shields.io/badge/Go%20to%20Type%20Name%20Reference-green?style=flat-square)](type_name.md)             |
| 📙 **Go to Dual-Mode Headers** | [![Go to Dual Mode Header Reference](https://img.shields.io/badge/Go%20to%20Dual%20Mode%20Header%20Reference-green?style=flat-square)](dual_mode_headers.md) |

---

> **Note:**
> `jh::macro` is a **meta-level subsystem**, not a runtime library.  
> It provides no symbols, only compiler directives and structural scaffolding.  
> Every other repository begins its inclusion chain here.
