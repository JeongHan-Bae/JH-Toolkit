# 🌀  **JH Toolkit — Asynchronous Module Overview**

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

`jh::async` is built on top of C++ coroutines, but does **not** expose them in their fully unrestricted form.
Instead of offering raw coroutine primitives directly, it deliberately **constrains the coroutine boundary**
and **narrows the semantic surface**.

By fixing execution boundaries, contracting semantics, and encapsulating complex coroutine behaviors
— especially those involving awaiter types, promise types, and suspension mechanics —
`jh::async` presents users with **well-defined, outer-layer semantic structures only**.

The goal is not maximal flexibility, but to provide **engineering-oriented asynchronous components**
that are easier to reason about, compose, and adopt in real-world codebases.

Users can declare their coroutine logic within lambda or asynchronous functions, while more complex encapsulated
components are handled by libraries. Coroutine logic is subject to certain constraints, yet its expressive power remains
sufficient (for example, when constructing a generator, your coroutine body should behave as expected for a generator).
We have abstracted common coroutine business logic into pre-built models for users.

---

## 🔹 Core Components

| Component                                      | Header                                       | Status   | Description                                                                |
|------------------------------------------------|----------------------------------------------|----------|----------------------------------------------------------------------------|
| <nobr>[`generator<T, U>`](generator.md)</nobr> | <nobr>`<jh/asynchronous/generator.h>`</nobr> | ✅ Stable | Coroutine generator with yield/send semantics.                             |
| [`fiber`](fiber.md)                            | `<jh/asynchronous/fiber.h>`                  | ✅ Stable | Lightweight cooperative threads (fibers) based on coroutines.              |
| [`slot`](slot.md)                              | `<jh/asynchronous/slot.h>`                   | ✅ Stable | Signal-slot mechanism built on coroutines for asynchronous event handling. |

---

## 🧩 Module Summary

* **Design goal:** Provide a unified coroutine-based abstraction for asynchronous flow and cooperative concurrency.
* **Core primitives:** `generator<T, U>`, `fiber`, and `slot`, all implemented as **header-only coroutine constructs**.
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
