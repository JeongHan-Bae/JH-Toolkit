# 🛰️ **JH Toolkit — InterProcess Coordination Module Overview**

📁 **Module:** `<jh/synchronous/ipc.h>`  
📦 **Namespace:** `jh::sync::ipc`  
📍 **Location:** `jh/synchronous/ipc/`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Sync](https://img.shields.io/badge/%20Back%20to%20Sync-green?style=flat-square)](../overview.md)

</div>

---

## 🧭 Introduction

`jh::sync::ipc` is the **synchronous IPC subsystem** of the JH Toolkit.  
It exposes a set of **named, globally addressable primitives** (mutexes, conditions, counters, shared memory objects,
and process launchers) that coordinate inter-process workflows without any runtime registries or supervising daemons.

Every primitive is bound to a **compile-time name** and uses OS-level semaphores, shared memory, or Win32 handles to
enforce deterministic behavior.

The module is available through two headers:

* `#include <jh/synchronous/ipc.h>` — raw namespace `jh::sync::ipc` with the individual helpers.
* `#include <jh/ipc>` — convenience alias (`jh::ipc`) that re-exports the same helpers under a shorter namespace.

This documentation lives in `docs/synchronous/ipc/…` and describes both the module and each primitive.

---

## 🔹 Core Components

| Component                                         | Header                                        | Description                                                                                        |
|---------------------------------------------------|-----------------------------------------------|----------------------------------------------------------------------------------------------------|
| [`limits`](ipc_limits.md)                         | `<jh/synchronous/ipc/ipc_limits.h>`           | Compile-time name and path validation (length, characters, parent-path rules).                     |
| [`process_mutex`](process_mutex.md)               | `<jh/synchronous/ipc/process_mutex.h>`        | Named, timed mutex built on OS semaphores, exposing lock/try-lock/unlink.                          |
| [`process_cond_var`](process_cond_var.md)         | `<jh/synchronous/ipc/process_cond_var.h>`     | Process-visible condition variable with manual wait/notify and optional unlink.                    |
| [`process_counter`](process_counter.md)           | `<jh/synchronous/ipc/process_counter.h>`      | Shared 64-bit counter with relaxed/strong/locked loads plus RMW helpers.                           |
| [`process_shm_obj`](process_shm_obj.md)           | `<jh/synchronous/ipc/process_shm_obj.h>`      | Single POD object mapped into shared memory with fences and explicit locking.                      |
| [`shared_process_mutex`](shared_process_mutex.md) | `<jh/synchronous/ipc/shared_process_mutex.h>` | Reader/writer primitive composed from the other helpers, with optional upgrade support.            |
| [`process_launcher`](process_launcher.md)         | `<jh/synchronous/ipc/process_launcher.h>`     | `std::thread`-like process launcher enforcing compile-time relative paths and ownership semantics. |

---

## 🧩 Cross-Platform Policies

* **Compile-time naming** — Every primitive requires a `jh::meta::TStr` literal name. Names are validated via
  `limits::valid_object_name`; invalid or over-length identifiers fail at compile time.
* **Path safety** — `process_launcher` accepts only POSIX-like relative paths. `<jh/synchronous/ipc/ipc_limits.h>`
  enforces the absence of internal `".."` segments unless `JH_ALLOW_PARENT_PATH == 1`, ensures no leading slash, and
  limits length to 128 (or 30 on BSD/when `JH_FORCE_SHORT_SEM_NAME` is forced).
* **Windows privilege** — Shared-memory-backed primitives (`process_counter`, `process_cond_var`, `process_shm_obj`,
  `shared_process_mutex`) use the `Global\\` namespace and therefore **require administrator rights** to create or open.
  `process_mutex` (Local namespace) and the launcher do not.
* **High-privilege operations** — Each class supplies a `HighPriv` boolean parameter that gates `unlink()` and
  upgrade-related helpers. Pass `true` only when the process owns the right to remove global objects.

### Windows-Specific Notes and Limitations

#### Privilege and Namespace Constraints

On **Windows**, IPC behavior differs fundamentally from POSIX systems due to OS-level namespace and privilege rules:

* **All semaphores must be created in the `Global\\` namespace** to be visible across processes.
* **All shared-memory objects (SHM) must be created in the `Local\\` namespace**; placing them in `Global\\` makes them
  inaccessible.
* As a result, **using this module on Windows requires administrator privileges** in order to successfully create or
  open the required IPC objects.

This is a constraint imposed by the Windows kernel and is not abstracted away by the library.

---

#### Design Intent and Platform Priority

The `jh::sync::ipc` module is **designed primarily for POSIX-based systems**, including:

* Unix systems (Darwin)
* Unix-like systems (Linux, BSD)

Its **core philosophy, semantics, and guarantees are rooted in POSIX IPC behavior**.

On Windows:

* The platform is supported **only via API-level emulation**
* Behavior is **best-effort**, not guaranteed to be identical
* The implementation attempts to *approximate* POSIX semantics, but **perfect alignment is neither assumed nor promised
  **

Therefore:

> **Windows support exists for feasibility, not parity.**

---

#### Intended Use on Windows

For projects that **must** be developed or tested on Windows:

* Windows support is intended **only for early-stage prototyping and initial validation**
* Long-term production use on Windows is **not recommended**, especially if administrator privileges cannot be
  guaranteed

Notably:

* The provided **MinGW-GCC toolchain under MSYS2 runs with administrator privileges by default**
* Therefore, **examples and tests will reliably succeed in that environment**

However:

> **Final projects should not depend on administrator privileges.**

---

#### Recommended Path Forward

For Windows-based development workflows, the recommended solution is:

* **Migrate to WSL2 + GCC**

This provides:

* Native POSIX semantics
* Predictable IPC behavior
* Alignment with the design assumptions of `jh::ipc`

---

#### Platform Status Statement

In summary:

> **Windows is a second-class platform in `jh::ipc`.**

Support is provided pragmatically, but the module’s guarantees, philosophy, and long-term expectations are **POSIX-first
**.

---

## 🧠 Summary

* `jh::sync::ipc` offers deterministic, compile-time-named IPC primitives based on OS semaphores, shared memory, and
  Win32 handles.
* Naming, hashing, and validation are all enforced by `<jh/synchronous/ipc/ipc_limits.h>`.
* `<jh/ipc>` merely re-exports this module under `jh::ipc`; include it when you want a shorter namespace.
* Every primitive in this module is documented below; follow the navigation links to drill into usage rules.

## 🧭 Navigation

| Resource                          |                                                              Link                                                               |
|-----------------------------------|:-------------------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**             |         [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../../README.md)         |
| 📘 **Back to Sync Overview**      |  [![Back to Sync Overview](https://img.shields.io/badge/Back%20to%20Sync%20Overview-green?style=flat-square)](../overview.md)   |
| 📗 **Go to Limits**               |        [![Go to IPC Limits](https://img.shields.io/badge/Go%20to%20IPC%20Limits-green?style=flat-square)](ipc_limits.md)        |
| 📙 **Go to Process Mutex**        |   [![Go to Process Mutex](https://img.shields.io/badge/Go%20to%20Process%20Mutex-green?style=flat-square)](process_mutex.md)    |
| 📘 **Go to Condition Variable**   |       [![Go to Cond Var](https://img.shields.io/badge/Go%20to%20Cond%20Var-green?style=flat-square)](process_cond_var.md)       |
| 📗 **Go to Process Counter**      |    [![Go to Counter](https://img.shields.io/badge/Go%20to%20Process%20Counter-green?style=flat-square)](process_counter.md)     |
| 📙 **Go to Shared Memory Object** |     [![Go to SHM Object](https://img.shields.io/badge/Go%20to%20SHM%20Object-green?style=flat-square)](process_shm_obj.md)      |
| 📘 **Go to Shared Process Mutex** | [![Go to Shared Mutex](https://img.shields.io/badge/Go%20to%20Shared%20Mutex-green?style=flat-square)](shared_process_mutex.md) |
| 📗 **Go to Process Launcher**     |        [![Go to Launcher](https://img.shields.io/badge/Go%20to%20Launcher-green?style=flat-square)](process_launcher.md)        |
