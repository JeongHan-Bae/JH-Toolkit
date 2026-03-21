# 🍯 **JH Toolkit — Serialization I/O Module Overview**

📁 **Module:** `<jh/serio>`  
📦 **Namespace:** `jh::serio`  
📍 **Location:** `jh/serialize_io/`  
📅 **Version:** **1.4.x** (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::serio` provides the **Serialization I/O utilities** of the JH Toolkit.

The module contains **standalone codecs** for transforming data between
representations, with a focus on:

* explicit behavior
* deterministic decoding
* binary correctness
* minimal abstraction

It does **not** provide object serialization frameworks or schema-based formats.

---

## 🔹 Core Submodules

| Submodule               | Header                        |  Status  | Description                                                                                                                                                                 |
|-------------------------|-------------------------------|:--------:|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`base64`](base64.md)   | `<jh/serialize_io/base64.h>`  | ✅ Stable | Binary-to-text encoding using Base64 / Base64URL (RFC 4648). Output is ASCII-safe and **universally decodable** by any compliant Base64 implementation.                     |
| [`uri`](uri.md)         | `<jh/serialize_io/uri.h>`     | ✅ Stable | URI percent-encoding and decoding utilities (RFC 3986). Provides **basic APIs** and **UTF-8 validated APIs** for safely encoding textual data within URI components.        |
| [`huffman`](huffman.md) | `<jh/serialize_io/huffman.h>` | ✅ Stable | Binary compression codec implementing standard and canonical Huffman algorithms. Streams are **signature-bound** and must be decoded using the same `Signature` definition. |

---

## 🔐 Decoding and Recoverability

The codecs in `jh::serio` differ intentionally in **who can recover the data**.

### `base64`

* Output is standard-compliant ASCII text
* Can be decoded by **any Base64 / Base64URL implementation**
* No library- or project-specific metadata is required
* Designed for interoperability and transport

### `uri`

* Output is standard URI percent-encoded text
* Can be decoded by **any RFC 3986-compliant percent-decoder**
* No library-specific metadata is required
* Designed for URI/URL text escaping and transport safety

### `huffman`

* Output is a **custom binary format**
* Stream begins with a compile-time `Signature`
* Decoding **requires the same signature definition**
* Intended for controlled environments using this library

A mismatched or missing signature causes decompression to fail explicitly.

---

## 🧠 Design Notes

* Each submodule exposes a **self-contained codec**.
* All decoding errors are reported via exceptions.
* Binary formats perform structural validation before decoding.
* No implicit format detection or fallback behavior is provided.

---

## 🧭 Navigation

| Resource               |                                                             Link                                                             |
|------------------------|:----------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**  |         [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)         |
| 📗 **Go to `base64`**  |  [![Go to Base64 Reference](https://img.shields.io/badge/Go%20to%20Base64%20Reference-green?style=flat-square)](base64.md)   |
| 📙 **Go to `uri`**     |       [![Go to URI Reference](https://img.shields.io/badge/Go%20to%20URI%20Reference-green?style=flat-square)](uri.md)       |
| 📘 **Go to `huffman`** | [![Go to Huffman Reference](https://img.shields.io/badge/Go%20to%20Huffman%20Reference-green?style=flat-square)](huffman.md) |

---

> 📌 `jh::serio` defines **low-level, explicit serialization codecs**.
> Recoverability, interoperability, and validation behavior are determined
> by the chosen submodule.
