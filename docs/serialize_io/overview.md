# 🍯 **JH Toolkit — Serialization I/O Module Overview**

📁 **Module:** `<jh/serio>`  
📦 **Namespace:** `jh::serio`  
📍 **Location:** `jh/serialize_io/`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::serio` defines the **Serialization I/O layer** of the JH Toolkit —
a collection of **codec utilities** for data serialization and deserialization.  
It provides portable, type-safe tools for converting between binary and text forms.

---

## 🌍 Overview

The `jh::serio` module focuses on creating a consistent and safe interface
for encoding and decoding operations across formats.  
It emphasizes correctness, constexpr verification,
and compatibility with standard binary–text representations.

---

## 🔹 Core Submodules

| Submodule             | Header                       |  Status  | Description                                                                                                                                                                                           |
|-----------------------|------------------------------|:--------:|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`base64`](base64.md) | `<jh/serialize_io/base64.h>` | ✅ Stable | Implements both **Base64** (RFC 4648 §4) and **Base64URL** (RFC 4648 §5). <br> Two namespaces — `jh::serio::base64` and `jh::serio::base64url` <br> — share a unified API for binary–text conversion. |

---

## 🧠 Design Notes

* Provides a unified interface for serialization codecs.
* Ensures type safety and constexpr-driven validation.
* Uses clear exception signaling for all decoding errors.
* Designed for predictable and cross-language interoperability.

---

## 🧭 Navigation

|       Resource        |                                                           Link                                                            |
|:---------------------:|:-------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README** |       [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)        |
| 📘 **Go to `base64`** | [![Go to Base64 Reference](https://img.shields.io/badge/Go%20to%20Base64%20Reference-green?style=flat-square)](base64.md) |

---

> 📌 The `jh::serio` module forms the foundation
> for all serialization and deserialization tools in the JH Toolkit.
