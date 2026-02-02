# 🛰️ JH Toolkit — `jh::sync::ipc::limits` API Reference

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/ipc_limits.h>`  
📦 **Namespace:** `jh::sync::ipc::limits`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Sync](https://img.shields.io/badge/%20Back%20to%20Sync-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Overview

`jh::sync::ipc::limits` provides **compile-time (`consteval`) validation utilities** for
inter-process communication (IPC) object names and POSIX-style relative paths.

The utilities are designed to enforce **platform-aware constraints** at compile time,
ensuring that invalid IPC identifiers never reach runtime.

Validated targets include:

- IPC object names (semaphores, shared memory, condition variables, etc.)
- POSIX-style *relative* paths used for file-based or namespace-based IPC

All checks are evaluated during template instantiation.  
**No runtime code, branches, or overhead are generated.**

---

## 🧱 Platform-Specific Limits

The maximum IPC object name length is selected automatically at compile time
based on the target platform:

| Platform / Mode                | Maximum length                                                      |
|--------------------------------|---------------------------------------------------------------------|
| macOS (Darwin) / FreeBSD       | **30** characters (BSD POSIX limit: 31 bytes including leading `/`) |
| Linux / Windows / WASM         | **128** characters (portable, conservative limit)                   |
| `JH_FORCE_SHORT_SEM_NAME == 1` | **30** characters on all platforms                                  |

The detection logic relies on `jh/macros/platform.h`.

---

## 🧰 API Detail

### `max_name_length`

```cpp
inline constexpr std::uint64_t max_name_length;
```

The compile-time maximum length for IPC object names.

* Value is **30** on BSD-derived systems or when `JH_FORCE_SHORT_SEM_NAME == 1`
* Value is **128** on Linux, Windows, and WASM
* Intended for use in `static_assert`, `requires` clauses, or template constraints

---

### `valid_object_name<S, MaxLen>()`

```cpp
template<jh::meta::TStr S, std::uint64_t MaxLen = max_name_length>
consteval bool valid_object_name();
```

Compile-time validation for IPC object names.

#### Rules

* Length must be in range **[1, MaxLen]**
* Allowed characters:

  ```
  [A–Z a–z 0–9 _ . -]
  ```
* No leading `'/'`
  (the operating system automatically prefixes the namespace)

#### Notes

* `S` must be a compile-time string (`jh::meta::TStr`)
* `MaxLen` defaults to `max_name_length`
* Returns a `constexpr bool`
* Because the function is `consteval`, invalid names **fail compilation**

#### Typical usage

```cpp
static_assert(
    jh::sync::ipc::limits::valid_object_name<"my_semaphore">()
);
```

---

### `valid_relative_path<S>()`

```cpp
template<jh::meta::TStr S>
consteval bool valid_relative_path();
```

Compile-time validation for POSIX-style **relative** paths.

#### Rules

* Length must be in range **[1, 128]**
* Absolute paths are forbidden (no leading `'/'`)
* `"./"` segments are forbidden
* `".."` segments:

    * When `JH_ALLOW_PARENT_PATH == 0` (default): **forbidden**
    * When `JH_ALLOW_PARENT_PATH == 1`:

        * Allowed **only as leading `"../"` segments**
        * Path cannot consist solely of `"../"`
        * No `".."` may appear after non-parent content begins
* Allowed characters:

  ```
  [A–Z a–z 0–9 _ . - /]
  ```

#### Notes

* Designed for deterministic IPC path handling
* Ensures no directory traversal or ambiguous path semantics
* Entirely compile-time enforced

#### Typical usage

```cpp
static_assert(
    jh::sync::ipc::limits::valid_relative_path<"ipc/demo">()
);
```

---

## 🔐 Configuration Macros

The following macros control compile-time validation behavior.
They are evaluated **during header inclusion**, so their values must be fixed
*before* any IPC-related headers are included.

There are **two supported ways** to override these macros.

---

### `JH_FORCE_SHORT_SEM_NAME`

```cpp
#define JH_FORCE_SHORT_SEM_NAME 0
```

Forces the BSD-safe IPC object name length limit (**30 characters**)
regardless of the detected platform.

#### Effect

* `0` (default)
  Platform-dependent limit is used:

    * 30 on macOS / FreeBSD
    * 128 on Linux / Windows / WASM

* `1`
  Always enforce the BSD POSIX limit (30 characters)

This is recommended when:

* Targeting unknown or BSD-like systems
* Developing cross-platform code that must remain BSD-compatible
* Ensuring identical IPC namespaces across all builds

---

### `JH_ALLOW_PARENT_PATH`

```cpp
#define JH_ALLOW_PARENT_PATH 0
```

Controls whether leading parent-directory segments (`"../"`) are permitted
in POSIX-style relative paths.

#### Effect

* `0` (default)
  All `".."` segments are forbidden

* `1`
  Leading `"../"` segments are allowed **with strict constraints**:

    * `"../"` may appear **only at the beginning**
    * The path **cannot consist solely of `"../"`**
    * After non-parent content begins, **no additional `".."` segments are allowed**

---

## 🧩 How to Override Configuration Macros

These macros can be overridden in **two supported ways**.

### 1️⃣ Override Before Including IPC Headers

Define the macros **before** including any JH IPC-related headers.
This must occur **before all** the following (and similar) includes:

* `#include <jh/ipc>`
* `#include <jh/sync>`

Example:

```cpp
#define JH_FORCE_SHORT_SEM_NAME 1
#define JH_ALLOW_PARENT_PATH 1

#include <jh/ipc>
```

⚠️ **Important**

* The macro definitions must appear in *every translation unit*
  that includes IPC headers
* Defining them after inclusion has **no effect**
* This method is best suited for small projects or explicit TU-level control

---

### 2️⃣ Override via Compiler / Build System (Recommended)

Define the macros globally using compiler definitions.
This ensures consistent behavior across all translation units.

#### CMake example

```cmake
target_compile_definitions(my_target
    PRIVATE
        JH_FORCE_SHORT_SEM_NAME=1
        JH_ALLOW_PARENT_PATH=1
)
```

Or globally:

```cmake
add_compile_definitions(
    JH_FORCE_SHORT_SEM_NAME=1
    JH_ALLOW_PARENT_PATH=1
)
```

#### Advantages

* Applies uniformly to the entire target or project
* Prevents accidental mismatches between translation units
* Recommended for libraries, large projects, and CI builds

---

## 🧠 Design Philosophy

* **Fail early**: invalid IPC names or paths cause compile-time errors
* **No runtime cost**: all checks are `consteval`
* **Portable and deterministic**: identical behavior across compilers and platforms
* **Explicit constraints**: no hidden OS-specific surprises

This guarantees that IPC namespaces behave consistently and safely
on all supported platforms.

---

## Summary

* `jh::sync::ipc::limits` enforces IPC naming and path rules at compile time
* Use `valid_object_name` for semaphores, shared memory, and synchronization primitives
* Use `valid_relative_path` for file-based or namespace-based IPC helpers
* Adjust `JH_FORCE_SHORT_SEM_NAME` and `JH_ALLOW_PARENT_PATH` to match deployment needs
* All utilities are available via `<jh/synchronous/ipc/ipc_limits.h>` or the umbrella headers
