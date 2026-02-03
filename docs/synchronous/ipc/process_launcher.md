# 🛰️ **JH Toolkit — `jh::sync::ipc::process_launcher` API Reference**

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/process_launcher.h>`  
📦 **Namespace:** `jh::sync::ipc`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Sync](https://img.shields.io/badge/%20Back%20to%20Sync-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Overview

`jh::sync::ipc::process_launcher<Path, IsBinary>` is a **cross-platform process launcher**
that exposes **`std::thread`-aligned semantics** for spawning and joining child processes.

It encapsulates the fundamental platform differences between:

* **POSIX**: `fork()` + `exec*()` + `waitpid()`
* **Windows**: `CreateProcess()` + `WaitForSingleObject()`

while enforcing **compile-time path validation**, **static identity**, and
**strict lifetime ownership**.

> `process_launcher` is a *launcher*, not a process manager.
> It deliberately provides **no kill, stop, detach, or signal APIs**.

---

## 🎯 Standard Library Correspondence

`process_launcher` is conceptually aligned with:

> **`std::thread`**

### Conceptual mapping

| Aspect            | `std::thread`                | `process_launcher`             |
|-------------------|------------------------------|--------------------------------|
| Unit of execution | Thread                       | Process                        |
| Start             | Constructor                  | `start()`                      |
| Join              | `join()`                     | `wait()`                       |
| Detach            | `detach()`                   | ❌ (intentionally absent)       |
| Destructor rule   | `std::terminate` if joinable | `std::terminate` if not waited |
| Ownership         | Move-only                    | Move-only                      |
| Identity          | Runtime                      | Compile-time (path-bound)      |

In short:

> **`process_launcher` treats processes as threads with stricter identity and security rules.**

---

## 🧱 Compile-Time Identity

Each instantiation of:

```cpp
process_launcher<"path/to/executable", IsBinary>
```

defines a **unique launcher type**, bound at compile time to:

* a specific executable path
* a specific binary/script policy

This identity is:

* **static**
* **non-substitutable**
* **non-polymorphic**

There is no generic "process handle" type by design.

---

## 🛣️ Path Rules and Validation

The template parameter `Path` must be a **POSIX-style relative path**,
validated **entirely at compile time** via `ipc::limits::valid_relative_path`.

### Core constraints

* **Relative paths only**

    * No leading `'/'`
    * No absolute paths
* **No `"./"` segments**
* **Length**: `[1, 128]`
* **Allowed characters**:

  ```
  [A–Z a–z 0–9 _ . - /]
  ```

---

### Parent path policy (`JH_ALLOW_PARENT_PATH`)

| Setting       | Behavior                              |
|---------------|---------------------------------------|
| `0` (default) | Any `".."` segment is forbidden       |
| `1`           | Only leading `"../"` prefixes allowed |

When enabled:

* One or more leading `"../"` are allowed
* The path **must contain additional content**
* Once non-parent content appears, `".."` is forbidden

This prevents directory traversal while still enabling controlled layouts.

---

### Cross-platform normalization

* **POSIX**: path is used as-is, relative to `cwd`
* **Windows**:

    * `'/'` is translated automatically
    * `'\\'` is never required

---

## ⚙️ Binary Policy (`IsBinary`)

The `IsBinary` template parameter exists to simplify **cross-platform build workflows**.

### Behavior

| Platform | `IsBinary == true`              | `IsBinary == false` |
|----------|---------------------------------|---------------------|
| POSIX    | Path used directly              | Path used directly  |
| Windows  | `".exe"` appended automatically | Path used as-is     |

Typical usage:

* `IsBinary == true`: CMake-built executables
* `IsBinary == false`: scripts (`.bat`, `.ps1`, shebang scripts on POSIX)

---

## 🔐 Handle Semantics

Each call to `start()` returns a **move-only handle** representing one running process.

### Ownership rules

* The handle **must** be explicitly `wait()`-ed
* Destroying an active handle triggers `std::terminate()`
* Move transfers exclusive ownership
* Assigning into an active handle also terminates the program

These rules are **intentionally identical to `std::thread`**.

---

### Why no detach?

Detached processes:

* obscure lifetime
* complicate error handling
* undermine structured IPC coordination

Therefore:

> **Every launched process must be joined.**

---

## 🔒 Static Security Binding

Each `process_launcher<Path, IsBinary>` defines its **own handle type**.

This has important consequences:

* A handle is statically bound to its executable
* Handles cannot be mixed, reused, or forged
* Runtime path substitution is impossible

### Rationale

This design:

* Preserves compile-time validation guarantees
* Prevents runtime path injection
* Eliminates an entire class of security vulnerabilities

A generic, runtime-configurable launcher was **explicitly rejected**.

---

## 🧠 Semantics Summary

* Launching is **explicit**
* Lifetime is **strictly owned**
* Identity is **compile-time**
* No implicit cleanup
* No runtime polymorphism

> **If a process is started, it must be waited.
> If it is not waited, the program is wrong.**

---

## 🧪 Typical Usage Pattern

```cpp
using launcher_t = jh::sync::ipc::process_launcher<"process_lock/writer">;

auto h = launcher_t::start();
// ...
h.wait();
```

Multiple launches yield independent handles, all of which must be waited.

---

## ⚠️ Error Handling

* Process creation failures throw `std::runtime_error`
* Handle misuse results in `std::terminate()`
* These are **programming errors**, not recoverable conditions

This mirrors `std::thread`’s philosophy.

---

## 🧩 Design Philosophy

* Static identity over runtime flexibility
* Determinism over convenience
* Ownership must be explicit
* IPC orchestration must be auditable

> **Processes are not tasks.
> They are resources with global side effects.**

---

## Summary

* `process_launcher` is a `std::thread`-like launcher for processes
* Compile-time validated, relative-only paths
* Strict handle ownership and lifetime rules
* No detach, no kill, no runtime substitution
* Designed for deterministic IPC orchestration
