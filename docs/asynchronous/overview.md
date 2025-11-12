# 🌀  **JH Toolkit — Asynchronous Module Overview**

📁 **Module:** `<jh/async>`  
📦 **Namespace** `jh::async`  
📍 **Location:** `jh/asynchronous/`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::async` defines the **asynchronous and concurrency foundation** of the JH Toolkit.
It integrates **coroutine-level asynchronous flow** and **system-level concurrency control**
under a consistent, header-only design.

---

## 🔹 Core Components

| Component                          | Header                                 | Status       | Description                                                    |
|------------------------------------|----------------------------------------|--------------|----------------------------------------------------------------|
| [`generator<T, U>`](generator.md)  | `<jh/asynchronous/generator.h>`        | ✅ Stable     | Coroutine generator with yield/send semantics.                 |

---

## 🧩 Module Summary

* **Current focus:** coroutine-based asynchronous iteration (`generator<T, U>`).
* **Aggregated header `<jh/async>`:** will be introduced in 1.4.0.
  Until then, include individual headers from `jh/asynchronous/`.

---

## 🧭 Navigation

|            Resource            |                                                                Link                                                                |
|:------------------------------:|:----------------------------------------------------------------------------------------------------------------------------------:|
|     🏠 **Back to README**      |            [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)            |
| 📘 **Go to `generator<T, U>`** | [![Go to Generator Reference](https://img.shields.io/badge/Go%20to%20Generator%20Reference-green?style=flat-square)](generator.md) |
