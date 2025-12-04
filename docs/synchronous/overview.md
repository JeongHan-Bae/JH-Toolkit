# ⏱️ **JH Toolkit — Synchronization Module Overview**

📁 **Module:** `<jh/sync>`  
📦 **Namespace:** `jh::sync`  
📍 **Location:** `jh/synchronous/`  
📅 **Version:** 1.3.5 → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::sync` defines the **synchronization layer** of the JH Toolkit —
a collection of **lightweight synchronization primitives**
that support safe coordination between threads or processes.

This module provides **building-block components** rather than frameworks:
each primitive is self-contained, composable, and designed
for deterministic behavior with minimal runtime cost.

---

## 🔹 Core Components

| Component           | Header                          |           Status            | Description                                                                                                       |
|---------------------|---------------------------------|:---------------------------:|-------------------------------------------------------------------------------------------------------------------|
| `const_lock<Mutex>` | `<jh/synchronous/const_lock.h>` |          ✅ Stable           | Scope-based const-locking primitive enforcing immutability barriers for mutex-like synchronization.               |
| `ipc` (submodule)   | `<jh/synchronous/ipc.h>`        | 🕓 Pending <br> (1.4.0-dev) | Inter-process synchronization primitives, including process-level mutexes, counters, and shared memory utilities. |

---

## 🧩 Module Summary

* **Current scope:** const-correct synchronization through `const_lock<Mutex>`.
* **Under development:** `jh::sync::ipc` for inter-process synchronization primitives.
* **Design focus:** standalone, composable primitives with predictable semantics.
* **Planned for v1.4.0:** aggregated header `<jh/sync>` and IPC integration.

---

## 🧭 Navigation

| Resource                         |                                                                  Link                                                                   |
|:---------------------------------|:---------------------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**            |              [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)               |
| 📘 **Go to `const_lock<Mutex>`** | [![Go to Const Lock Reference](https://img.shields.io/badge/Go%20to%20Const%20Lock%20Reference-green?style=flat-square)](const_lock.md) |

---

> 📌 `jh::sync` provides a **foundation of synchronization primitives** —  
> minimal, composable, and semantically precise tools
> for thread-safe and process-safe coordination within the Toolkit.
