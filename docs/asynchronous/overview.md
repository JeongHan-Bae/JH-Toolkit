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

Currently (v1.3.x), the module provides `generator` — a bidirectional coroutine generator.
The upcoming **1.4.0-dev** branch extends this layer with advanced primitives including
**optimistic concurrency (OCC)**, **process synchronization**, and **process management**.

---

## 🔹 Core Components

| Component                          | Header                                 | Status       | Description                                                    |
|------------------------------------|----------------------------------------|--------------|----------------------------------------------------------------|
| [`generator<T, U>`](generator.md)  | `<jh/asynchronous/generator.h>`        | ✅ Stable     | Coroutine generator with yield/send semantics.                 |
| `occ_box<T>`                       | `<jh/asynchronous/occ_box.h>`          | 🚧 1.4.0-dev | Optimistic Concurrency Control box for atomic snapshots.       |
| `process_mutex<Name>`              | `<jh/asynchronous/process_mutex.h>`    | 🚧 1.4.0-dev | Cross-platform named mutex for inter-process synchronization.  |
| `process_launcher<Path, IsBinary>` | `<jh/asynchronous/process_launcher.h>` | 🚧 1.4.0-dev | Unified process launcher aligned with `std::thread` semantics. |

---

## 🧩 Module Summary

* **Current focus:** coroutine-based asynchronous iteration (`generator<T, U>`).
* **Upcoming (1.4.0-dev):** concurrency primitives — `occ_box`, `process_mutex`, `process_launcher`.
* **Aggregated header `<jh/async>`:** will be introduced in 1.4.0.
  Until then, include individual headers from `jh/asynchronous/`.

---

## 🧭 Navigation

|            Resource            |                                                                Link                                                                |
|:------------------------------:|:----------------------------------------------------------------------------------------------------------------------------------:|
|     🏠 **Back to README**      |            [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)            |
| 📘 **Go to `generator<T, U>`** | [![Go to Generator Reference](https://img.shields.io/badge/Go%20to%20Generator%20Reference-green?style=flat-square)](generator.md) |
