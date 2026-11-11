# 🛰️ **JH-Toolkit — `jh::ipc` Entry Overview**

### InterProcess ~~Communication~~ Coordination

📁 **Header:** `<jh/ipc>`  
📦 **Namespace:** `jh::ipc`  
📍 **Location:** `jh/synchronous/ipc.h` (flattened entry)  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

<div align="center" style="margin-top: -32px;">
  <img src="https://raw.githubusercontent.com/JeongHan-Bae/JH-Toolkit/main/docs/img/Ojing.svg" 
       alt="Ojing mascot"
       width=96px;>
</div>

---

## 🧭 Introduction

`<jh/ipc>` is the **flattened, user-facing include** for the IPC primitives implemented under `jh::sync::ipc`.  
It simply includes `<jh/synchronous/ipc.h>` and re-exports every symbol from `jh::sync::ipc` inside `jh::ipc`, giving
consumers a shorter namespace (and a shorter include path) when they do not need to qualify with `jh::sync::`.

If you need the complete `jh::sync` module, include `<jh/sync>` directly.
If you only need IPC components, just write `#include <jh/ipc>` and use as `jh::ipc::process_mutex<"foo">`.

**IPC here is intentionally non-intimidating**: when you restrict yourself to OS-backed semaphore and shared-memory
wrappers (`jh::ipc`), it behaves like a *fully grilled squid* — visually aggressive, but safe and familiar once used.
The syntax mirrors ordinary synchronous primitives, the cognitive load is low, and synchronization remains cleanly
decoupled from business logic.

---

## 🔹 Entry Points

| Include     | Brings in...                                                                                                                                     |
|-------------|--------------------------------------------------------------------------------------------------------------------------------------------------|
| `<jh/sync>` | All `jh::sync::*` modules, including `jh::sync::ipc::*`                                                                                          |
| `<jh/ipc>`  | Only `<jh/synchronous/ipc.h>`, with a `namespace jh::ipc { using namespace jh::sync::ipc; }` alias so the helpers live directly under `jh::ipc`. |

## 🧩 Component Surface

| Component                             | Description                                                                                |
|---------------------------------------|--------------------------------------------------------------------------------------------|
| `jh::sync::ipc::limits`               | Compile-time validation of IPC object names and POSIX-style paths (see `ipc/limits.md`).   |
| `jh::sync::ipc::process_mutex`        | Named, timed process mutex backed by OS semaphores.                                        |
| `jh::sync::ipc::process_cond_var`     | Process-shared condition variable built on `process_mutex` + shared memory.                |
| `jh::sync::ipc::process_counter`      | Shared 64-bit counter with lock-protected RMW operations and optional strong loads.        |
| `jh::sync::ipc::process_shm_obj`      | Single POD object mapped into shared memory with explicit fences and mutex guards.         |
| `jh::sync::ipc::shared_process_mutex` | Reader/writer primitive with optional upgrade support, composed from the other primitives. |
| `jh::sync::ipc::process_launcher`     | `std::thread`-like process launcher with compile-time relative path validation.            |

---

## 🧠 Summary

* `<jh/ipc>` is a **pure forwarding entry** that flattens `jh::sync::ipc` into a shorter namespace.
* No additional abstraction, policy, or lifecycle logic is introduced at this level.
* All IPC semantics, guarantees, and constraints are defined exclusively in `jh::sync::ipc`.
* This header exists solely to reduce include depth and namespace verbosity for end users.
* Treat `<jh/ipc>` as a **convenience alias**, not a conceptual boundary.

---

## 🧠 Philosophy — What `jh::ipc` Represents

`jh::ipc` represents a **shared-memory synchronization philosophy**, not a communication framework.

The model is based on the following principles:

### 1. IPC is not messaging

`jh::ipc` does **not** implement:

* message queues
* RPC
* serialization-based transport
* semantic object graphs

Instead, it provides **process-shared synchronization primitives** analogous to the C++ standard library:

* `std::timed_mutex` → `process_mutex`
* `std::shared_timed_mutex` → `shared_process_mutex`
* `std::condition_variable` → `process_cond_var`
* `std::atomic<uint64_t>` → `process_counter`
* POD storage → `process_shm_obj`

The interaction model is **coordination through shared state**, not communication through messages.

---

### 2. Shared data is unopinionated

In `jh::ipc`:

* synchronization objects define **ordering**
* shared memory defines **storage**
* **meaning is external**

`process_shm_obj<S, T>` stores **POD data only**.
How that data is interpreted is entirely the responsibility of the reader.

This explicitly separates:

* **data** (raw, shared, POD)
* **data entity / semantics** (local, typed, contextual)

This avoids semantic coupling across processes.

---

### 3. Compile-time identity replaces runtime registries

All IPC objects are:

* named at compile time
* globally addressable
* resolved directly to OS-managed resources

There is:

* no runtime registration
* no central broker
* no supervising daemon
* no ownership graph

The **name itself is the contract**.

If two binaries instantiate the same template literal, they refer to the same underlying system object.

---

### 4. IPC objects are handles, not shared C++ objects

Each IPC type is a **cross-process singleton**, accessed via `X::instance()`.

Important distinction:

* Inside one process → one C++ object
* Across processes → multiple C++ handles
* Underneath → one OS-controlled resource

This is conceptually identical to file descriptors or named semaphores.

The library makes this explicit instead of hiding it behind object semantics.

---

### 5. Engineering over abstraction

Compared to Boost.Interprocess:

* Boost models **semantic shared objects**
* `jh::ipc` models **synchronization + raw state**

This results in:

* simpler mental model
* fewer failure modes
* easier debugging
* clearer ownership
* deterministic topology

The design intentionally avoids "feature richness" in favor of **predictable engineering behavior**.

---

## 🧭 Navigation

| Resource                          |                                                         Link                                                          |
|-----------------------------------|:---------------------------------------------------------------------------------------------------------------------:|
| 🏠 **Back to README**             |     [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)      |
| 📘 **Go to Synchronous Overview** | [![Go to Sync Overview](https://img.shields.io/badge/Go%20to%20Sync%20Overview-green?style=flat-square)](overview.md) |
| 📗 **Go to IPC Module Overview**  | [![Go to IPC Module](https://img.shields.io/badge/Go%20to%20IPC%20Overview-green?style=flat-square)](ipc/overview.md) |
