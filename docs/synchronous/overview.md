# ⏱️ **JH Toolkit — Synchronization Module Overview**

📁 **Module:** `<jh/sync>`  
📦 **Namespace:** `jh::sync`  
📍 **Location:** `jh/synchronous/`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::sync` defines the **synchronization layer** of the JH Toolkit.

It provides a set of **low-level, engineering-oriented synchronization primitives**
designed for:

* deterministic behavior
* minimal abstraction overhead
* composability
* predictable lifetime and memory semantics

This module intentionally avoids frameworks or policy-heavy designs.
Each component is a **standalone building block** that can be composed into
larger systems without hidden coordination logic.

---

## 🔹 Core Components (Stable)

| Component           | Header                                |  Status  | Description                                                                                                                                                                                                             |
|---------------------|---------------------------------------|:--------:|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `const_lock<Mutex>` | `<jh/synchronous/const_lock.h>`       | ✅ Stable | RAII-based **const-oriented lock guard** enforcing immutability barriers for mutex-like synchronization objects.                                                                                                        |
| `control_buf<T>`    | `<jh/synchronous/control_buf.h>`      | ✅ Stable | **Block-allocated control container** for non-copyable, non-movable synchronization primitives with strict address stability guarantees.                                                                                |
| `strong_lock`       | `<jh/synchronous/strong_lock.h>`      | ✅ Stable | **Ordering-strengthened RAII lock adapters** (`strong_unique_lock`, `strong_shared_lock`) that insert `seq_cst` fences at lock boundaries to reduce cross-domain ordering anomalies when mixing locks and atomic types. |
| `ipc` (submodule)   | `<jh/synchronous/ipc.h>` / `<jh/ipc>` | ✅ Stable | **Inter-process synchronization primitives** built on OS semaphores and shared memory (mutexes, condition variables, counters, POD storage, process launchers).                                                         |

---

## 🧩 Module Scope

`jh::sync` currently covers:

* **const-correct synchronization semantics** (`const_lock`)
* **stable-address control-object storage** (`control_buf`)
* **ordering-strengthened lock adapters** (`strong_lock`)
* **process-level coordination via shared memory** (`ipc`)

---

## 🧭 Navigation

| Resource                       |                                                                  Link                                                                   |
|--------------------------------|:---------------------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**          |              [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)               |
| 📗 **`const_lock` Reference**  |                 [![Go to Const Lock](https://img.shields.io/badge/Const%20Lock-green?style=flat-square)](const_lock.md)                 |
| 📘 **`control_buf` Reference** |               [![Go to Control Buf](https://img.shields.io/badge/Control%20Buf-green?style=flat-square)](control_buf.md)                |
| 📙 **IPC Overview**            | [![Go to InterProcess Coordination](https://img.shields.io/badge/InterProcess%20Coordination-green?style=flat-square)](ipc/overview.md) |

---

> 📌 **Summary**
> `jh::sync` is a **foundation module**:
> minimal primitives, explicit semantics, and predictable behavior —
> suitable for both thread-level and process-level coordination without framework lock-in.
