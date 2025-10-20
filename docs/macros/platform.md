# 🔮 **JH Toolkit — `platform` Reference**

📁 **Header:** `<jh/macros/platform.h>`  
📦 **Namespace:** `/` *(pure macro scope)*  
📅 **Version:** 1.3.3  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

> **Note:**  
> `platform` is a **submodule**, not a class or namespace.  
> It defines **compiler-, architecture-, OS-, and endianness-detection macros**
> used globally across the entire JH Toolkit.  
> This layer ensures consistent behavior and ABI safety before any other header is parsed.

---

## 🏷️ Overview

`platform.h` is the **foundation of all environment detection** within the JH Toolkit.  
It validates the toolchain, compiler, and architecture **before any code compiles**,
preventing unsupported builds at preprocessing stage.

This submodule provides:

* Minimal and consistent `IS_*` macros for platform traits
* 64-bit architecture enforcement via `sizeof(std::size_t)`
* Explicit rejection of **MSVC** toolchains
* Unified detection of POSIX, Darwin, Linux, Windows, and WASM targets

It is **implicitly included** by all internal components.

> **Why not use `JH_` prefixes?**  
> All macros in this header describe **objective environmental facts** (e.g. `IS_LINUX == 1`).  
> They are **not configuration flags**, but **truth indicators** guaranteed to be consistent across compilers.
>
> Therefore, redefinition with the **same value** is harmless and does not violate ODR.  
> However, redefining them with **different values** indicates a deeper environment inconsistency —
> in such cases, **behavior is undefined** and the issue lies within the toolchain itself,
> not within the JH Toolkit.

---

## 🔹 Detection Coverage

| Category             | Macros                                                                     | Description                                 |
|----------------------|----------------------------------------------------------------------------|---------------------------------------------|
| **Compiler**         | `IS_CLANG`, `IS_GCC`, `IS_MSVC`, `IS_MINGW`, `IS_CLANG_ON_WINDOWS`         | Detects the active compiler frontend.       |
| **Architecture**     | `IS_AMD64`, `IS_X86`, `IS_AARCH64`, `IS_X86_FAMILY`                        | Identifies CPU architecture.                |
| **Operating System** | `IS_LINUX`, `IS_WINDOWS`, `IS_APPLE`(`IS_DARWIN`), `IS_FREEBSD`, `IS_WASM` | Determines target OS.                       |
| **Platform Family**  | `IS_POSIX`, `HAS_POSIX_1B`, `IS_MACOS`(`IS_OS_X`)                          | Groups environments and toolchain variants. |
| **Endianness**       | `IS_BIG_ENDIAN`, `IS_LITTLE_ENDIAN`                                        | Detects byte order via compiler macros.     |

---

## 🔹 Enforcement Rules

`platform.h` is not merely descriptive — it **actively enforces** runtime constraints at compile time.

| Enforcement           | Mechanism                                 | Description                                                    |
|-----------------------|-------------------------------------------|----------------------------------------------------------------|
| **64-bit only**       | `static_assert(sizeof(std::size_t) == 8)` | Rejects 32-bit platforms immediately.                          |
| **No MSVC**           | Preprocessor guard on `_MSC_VER`          | If detected without **`clang`** or **`GNUC`**, triggers error. |
| **Trusted ABI width** | `sizeof(std::size_t)` check               | Ensures actual ABI matches claimed architecture macros.        |

MSVC detection logic ensures that **MinGW-w64** and **Clang-cl** remain valid:

```cpp
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
#  define IS_MSVC 1
#else
#  define IS_MSVC 0
#endif
```

Thus, only **pure MSVC** (non-GNU, non-Clang) compilers are rejected.

---

## 🔹 POSIX and Platform Extensions

`platform.h` includes **POSIX compatibility macros** to differentiate runtime capabilities.

| Macro          | Description                                                                                    |
|----------------|------------------------------------------------------------------------------------------------|
| `IS_POSIX`     | Defined for all Unix-like systems (Linux, macOS, BSD).                                         |
| `HAS_POSIX_1B` | Set to 1 if `_POSIX_TIMEOUTS >= 200112L`, meaning POSIX.1b real-time extensions are available. |

These checks ensure correct feature gating for systems relying on POSIX APIs.

---

## 🔹 Architecture Mapping

| Macro           | Meaning                                                    |
|-----------------|------------------------------------------------------------|
| `IS_AMD64`      | Target architecture is x86-64 (AMD64).                     |
| `IS_X86`        | 32-bit x86 (Intel 80386 family).                           |
| `IS_AARCH64`    | ARMv8+ 64-bit (Apple Silicon, ARM64).                      |
| `IS_X86_FAMILY` | Equivalent to <code>(IS_X86 &#124;&#124; IS_AMD64)</code>. |

---

## 🔹 Platform Group Mapping

| Family             | Typical Targets              | Description                                                                                                                                               |
|--------------------|------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| **POSIX**          | Linux, macOS, BSD            | Detected when **`linux`**, **`unix`**, or **`APPLE`** is defined.                                                                                         |
| **Windows**        | Win32 / Win64, MinGW-w64     | `IS_WINDOWS == 1`. If GCC-based, `IS_MINGW == 1`. <br/> `IS_CLANG_ON_WINDOWS == 1` when Clang is the active frontend.                                     |
| **Apple / Darwin** | macOS-specific Darwin builds | `IS_APPLE` marks Apple vendor; `IS_DARWIN` aliases it. <br/> `IS_MACOS` (or `IS_OS_X`) is set **only** for macOS host builds (non-cross-compiled Darwin). |
| **FreeBSD**        | FreeBSD family               | `IS_FREEBSD == 1`.                                                                                                                                        |
| **WebAssembly**    | Emscripten / WASM            | `IS_WASM == 1`.                                                                                                                                           |

> 🧩 **Darwin clarification:**
>
> * `IS_APPLE` = vendor (Apple toolchain).  
> * `IS_DARWIN` = underlying kernel family.  
> * `IS_MACOS` / `IS_OS_X` = actual macOS platform (host build).  
    >   Other Darwin variants (iOS, tvOS, etc.) are not specialized in this header.

---

## 🔹 Endianness Detection

| Macro              | Meaning                                             |
|--------------------|-----------------------------------------------------|
| `IS_BIG_ENDIAN`    | Defined if **`BYTE_ORDER == ORDER_BIG_ENDIAN`**.    |
| `IS_LITTLE_ENDIAN` | Defined as the logical negation of `IS_BIG_ENDIAN`. |

This detection relies purely on compiler-provided constants
and is safe on all modern GCC/Clang targets — no `<endian.h>` dependency required.

---

## ⚙️ Design Notes

* No redefinitions — macros are pure constants.  
* No dependencies beyond `<cstddef>`.  
* No namespace exposure — global-only scope.  
* Prefer preprocessor over `constexpr` for compile-time branching.  
* Side-effect free: multiple inclusions are harmless.  

```cpp
#if IS_GCC
#   pragma message("Compiling under GCC")
#elif IS_CLANG
#   pragma message("Compiling under Clang")
#endif
```

---

## 🧠 Summary

| Aspect              | Description                               |
|---------------------|-------------------------------------------|
| Category            | Compiler and platform detection submodule |
| Kind                | Pure macro header                         |
| Namespace           | `/` (global)                              |
| Supported Compilers | GCC, Clang                                |
| Unsupported         | MSVC (pure), 32-bit targets               |
| Architecture        | Enforces 64-bit only                      |
| Dependencies        | `<cstddef>`                               |
| POSIX Support       | Detects POSIX.1b extensions               |
| ABI Impact          | None                                      |
| Side Effects        | None                                      |

---

> **Design Philosophy**  
> `platform` serves as the first line of validation across the toolkit.  
> It ensures **deterministic compilation**, a **verified ABI environment**,
> and prevents undefined cross-platform states before any targets as `jh::jh-toolkit-pod`, `jh::jh-toolkit`,
> or `jh::jh-toolkit-static` target is compiled.  
>
> All subsequent layers rely on the guarantees established by this submodule.
