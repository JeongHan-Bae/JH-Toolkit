# 🌀  **JH-Toolkit — Asynchronous Module Overview**

📁 **Module:** `<jh/async>`  
📦 **Namespace** `jh::async`  
📍 **Location:** `jh/asynchronous/`  
📅 **Version:** 1.4.0 (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::async` provides coroutine-based components with defined execution and
suspension behavior. Its interfaces encapsulate awaiter and promise types.

Users write coroutine bodies in lambdas or functions and use the generator,
fiber, and slot interfaces for their corresponding execution models.

---

## 🔹 Core Components

| Component                                      | Header                                       | Status   | Description                                                                |
|------------------------------------------------|----------------------------------------------|----------|----------------------------------------------------------------------------|
| <nobr>[`generator<T, U>`](generator.md)</nobr> | <nobr>`<jh/asynchronous/generator.h>`</nobr> | ✅ Stable | Coroutine generator with yield/send semantics.                             |
| [`fiber`](fiber.md)                            | `<jh/asynchronous/fiber.h>`                  | ✅ Stable | Lightweight cooperative threads (fibers) based on coroutines.              |
| [`slot`](slot.md)                              | `<jh/asynchronous/slot.h>`                   | ✅ Stable | Signal-slot mechanism built on coroutines for asynchronous event handling. |

---

## 🧩 Module Summary

* **Stability:** All listed components are considered **API-stable** as of v1.4.0.
* **Aggregated header `<jh/async>`:**
  Introduced in **v1.4.0** as the recommended entry point.
  Individual headers under `jh/asynchronous/` remain fully supported.

---

## 🧭 Navigation

|              Resource              |                                                     Link                                                     |
|:----------------------------------:|:------------------------------------------------------------------------------------------------------------:|
|       🏠 **Back to README**        | [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md) |
| 📘 **`generator<T, U>` Reference** |   [![Generator](https://img.shields.io/badge/Generator%20Reference-green?style=flat-square)](generator.md)   |
|      📗 **`fiber` Reference**      |         [![Fiber](https://img.shields.io/badge/Fiber%20Reference-green?style=flat-square)](fiber.md)         |
|      📙 **`slot` Reference**       |          [![Slot](https://img.shields.io/badge/Slot%20Reference-green?style=flat-square)](slot.md)           |
