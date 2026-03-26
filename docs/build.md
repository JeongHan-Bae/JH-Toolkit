# 🧰 Build & Platform Guide

This document provides a complete reference for building **JH Toolkit**, including supported toolchains, Conan
packaging, CMake targets, and platform-specific notes.

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## 📦 Conan Packaging via GitHub Releases

Conan packages are distributed **as `.tar.gz` archives** attached to **GitHub Release Assets**.

**Available (v<VERSION>):**

* 🧩 `jh-toolkit-pod` — Header-only (platform independent)
* 🛠️ `jh-toolkit` — Full builds for:

    * Linux x86_64
    * macOS ARM64

---

### ⚙️ General Notes

* ✅ Uses **Conan 2.x** with modern profile & CMake toolchain support.
* 📦 **GitHub Packages** is **not used** (Conan 2.x incompatible).
* ⛔ **Windows builds excluded** — Conan 2.x under MSYS2/UCRT64 may inject MSVC dependencies.
* 🚫 **Linux ARM64 skipped in CI** — due to missing native runners or fully stable QEMU.

---

### 📦 Conan `.tar.gz` Archive — Usage

> All `.tar.gz` packages are pre-built via GitHub CI for each tagged release.

#### Dependency Matrix

| Package Name              | Platform Dependent | Compiler Dependent | Description                                  |
|---------------------------|--------------------|--------------------|----------------------------------------------|
| `jh-toolkit-pod`          | ❌                  | ❌                  | Header-only, platform-agnostic POD module    |
| `jh-toolkit-linux-x86_64` | ✅                  | ✅ (GCC 13+)        | Built on `ubuntu-latest` using GCC toolchain |
| `jh-toolkit-macos-arm64`  | ✅                  | ✅ (LLVM 20+)       | Built on `macos-latest` with Homebrew LLVM   |

#### Manual Cache Extraction

```bash
# Download from GitHub Releases
wget https://github.com/JeongHan-Bae/JH-Toolkit/releases/download/JH-Toolkit-<VERSION>/jh-toolkit-linux-x86_64-<VERSION>.tar.gz

# Inject into local Conan 2.x cache
mkdir -p ~/.conan2/p/jh-toolkit
tar -xzf jh-toolkit-linux-x86_64-<VERSION>.tar.gz -C ~/.conan2/p/jh-toolkit
```

> Replace `<VERSION>` with the desired release tag (e.g. `1.3.2`, `1.4.0`, etc.)
> Inspect cache layout using `conan list` or `conan cache path`.

If your system differs from the CI presets, you can always [build from source](#-building-from-source).

---

## 📋 Requirements & Recommended Toolchains

### 🧩 Requirements

| Requirement                  | Minimum Version | Notes                                          |
|------------------------------|-----------------|------------------------------------------------|
| **C++**                      | 20              | Mandatory                                      |
| **CMake (consumer)**         | **3.14+**       | Required when using `find_package(jh-toolkit)` |
| **CMake (building toolkit)** | **3.21+**       | Required when building JH Toolkit itself       |
| **Git**                      | Latest          | Required for Debug / FetchContent builds       |
| **System ABI**               | 64-bit          | 32-bit builds prohibited                       |

---

### 🧠 Recommended Toolchains

| Platform           | Recommended Compiler                          | Notes                                   |
|--------------------|-----------------------------------------------|-----------------------------------------|
| **Linux**          | **LLVM Clang 20+** or **GCC 13+**             | CI-tested (`ubuntu-latest`)             |
| **macOS (Darwin)** | **LLVM Clang 20+** via `brew install llvm@20` | Most stable; preferred over Apple Clang |
| **Windows**        | **MSYS2 UCRT64 (GCC 14+)**                    | Required for full C++20 compliance      |

> If you need to interact with other components on a Linux machine, especially a server, we recommend continuing to use
> GCC to fully support interactions within the Linux ecosystem.  
> If you're using it within a virtual machine or setting up entirely from scratch on a new server—meaning you have no
> need to interact with the ecosystem—  
> and your Linux system can install Homebrew, we recommend using LLVM@20.

#### ✅ Minimum Supported Versions

| Compiler             | Version Range     | Status                                                            | Notes                |
|----------------------|-------------------|-------------------------------------------------------------------|----------------------|
| **GCC**              | ≥ 13              | ✅ Supported                                                       | Recommended: GCC 14+ |
| **LLVM Clang**       | 15–16             | ✅ Supported                                                       | Fully compatible     |
| **LLVM Clang 17–18** | ⚠️ **Rejected**   | Known to cause unresolved linkage (`std::hash<std::string_view>`) |                      |
| **LLVM Clang 19**    | 🚫 Not Tested     | Unsupported                                                       |                      |
| **LLVM Clang 20+**   | ✅ **Recommended** | Most stable on Darwin; install via Homebrew                       |                      |
| **MSVC**             | ❌ Prohibited      | Incomplete C++20 and ABI inconsistencies                          |                      |

> ✅ **Summary:**
> Prefer **LLVM 20+** on macOS and **GCC 13+** on Linux.
> Clang 17–18 are **explicitly disallowed** due to reproducible linker instability.

---

## 📥 Installation

#### 🔸 **Option 1: Latest Stable Version (mainstream)**

```bash
git clone https://github.com/JeongHan-Bae/jh-toolkit.git
```

#### 🔹 **Option 2: Latest LTS Release (1.4.0+)**

```bash
git clone --branch 1.4.x-LTS --depth=1 https://github.com/JeongHan-Bae/jh-toolkit.git
```

👉 Or download from: **[JH Toolkit Latest LTS Release](https://github.com/JeongHan-Bae/JH-Toolkit/releases/latest)**

---

## ⚙️ Building from Source

### 🔹 Full Build (default)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

Installs both:

* `jh::jh-toolkit` — header-only interface (pure templates)
* `jh::jh-toolkit-static` — optimized static library for critical components

---

### 🔸 Header-Only Build (POD System)

```bash
cmake -B build-pod -DCMAKE_BUILD_TYPE=Release -DTAR=POD
cmake --build build-pod
sudo cmake --install build-pod
```

Installs only:

* `jh::jh-toolkit-pod` — pure header-only module

> Ideal for embedding or constrained deployment.

---

### 🧩 Modular Build Modes

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DTAR=POD,ALL
```

> **Note:** `TAR` is a comma-separated list of build modes, by default `ALL` (full build, no "Pod-only" mode).

| TAR Value | Description                                            |
|-----------|--------------------------------------------------------|
| `POD`     | Header-only POD-only module `jh::jh-toolkit-pod`       |
| `ALL`     | Full build: `jh::jh-toolkit` + `jh::jh-toolkit-static` |
| `POD,ALL` | Builds both; all targets available                     |

---

### 📦 Installed CMake Targets

| Mode          | Targets Installed                         | Description                                                               |
|---------------|-------------------------------------------|---------------------------------------------------------------------------|
| `TAR=ALL`     | `jh::jh-toolkit`, `jh::jh-toolkit-static` | Full toolkit: headers + optimized static objects                          |
| `TAR=POD`     | `jh::jh-toolkit-pod`                      | Header-only POD-only library                                              |
| `TAR=POD,ALL` | All of the above                          | Provides full modular access for development and distribution flexibility |

> **Note:**
> `TAR=POD` does **not** install `jh::jh-toolkit` or `jh::jh-toolkit-static`,  
> it intentionally provides only the POD module for users who want a minimal pod-only support.  
> `jh::jh-toolkit-pod` only guarantees the usage of one public header (`<jh/pod>`) and does not include the full API
> surface of `jh::jh-toolkit`.

---

### ⚙️ About `jh::jh-toolkit-static`

The **`jh::jh-toolkit-static`** target contains **precompiled implementations** for:

* `immutable_str`
* `runtime_arr` (bit-packed `bool` and byte-based variants)

These are built with the following strict optimization set:

```bash
-O3
-fno-rtti
-ftree-vectorize
-funroll-loops
-fno-omit-frame-pointer
-Wall -Wextra -Wpedantic
```

🧩 **Advantages:**

* Zero external dependencies
* Consistent performance across toolchains
* Protects against suboptimal user build flags
* Seamlessly interchangeable with header-only usage

---

## 🧠 Notes on Precompiled Translation Units & Platform Semantics

JH Toolkit is **primarily a template-based library**.
Most components are fully defined in headers and instantiated at the point of use.

As a result, the number of precompiled translation units (TUs) is intentionally minimal.

---

### Precompiled Components (Very Limited Scope)

At present, **only the following components provide precompiled TUs**:

- `immutable_str`
- `runtime_arr<bool>` (bit-packed / compressed representation)
- `runtime_arr<bool, runtime_arr_helper::bool_flat_alloc>`  
  (flat, non–bit-packed representation)

These correspond to **two concrete headers** with explicit instantiation support.

All other components are instantiated normally as templates.

---

### Performance Expectations

Because the library is predominantly template-based:

- The **performance difference between header-only usage and static linkage**
  is generally **small**.
- Static linkage mainly exists to:
    - stabilize optimization flags
    - avoid suboptimal user build configurations
    - provide predictable code generation for a few hot paths

If you encounter a performance bottleneck with:

```cpp
jh::runtime_arr<bool>   // bit-packed representation
````

you may consider:

1. Linking against `jh::jh-toolkit-static`, or
2. Switching to the flat representation:

```cpp
jh::runtime_arr<bool, jh::runtime_arr_helper::bool_flat_alloc>
```

If performance remains problematic after these adjustments,
it is usually an indication of **algorithmic or logical issues**,
rather than a limitation of the container itself.

---

### Notes on Interprocess Coordination (`jh::ipc`) and Windows

The **interprocess coordination subsystem (`jh::ipc`)**
is designed around **POSIX semantics**.

On Windows:

* The API is provided for **development and modeling purposes**
* Exact semantic equivalence with POSIX is **not guaranteed**

#### Permission Model Differences (Affects `jh::ipc`)

Windows enforces a split namespace:

* **Semaphores** typically depend on the **Local** namespace
* **Shared memory** typically depends on the **Global** namespace

Using incompatible combinations (for example,
`Global` semaphores or `Local` shared memory)
results in immediate failure.

As a consequence:

* Some interprocess coordination scenarios may require
  **administrator privileges**
* The Windows permission model is fundamentally different from POSIX
  and is not always expressive enough to emulate it faithfully

---

### Runtime Model Differences on Windows

Certain runtime behaviors on Windows differ significantly
from typical POSIX environments, especially under high concurrency.

Known limitations include:

* `std::weak_ptr::lock()` may succeed transiently
  on logically expired objects under heavy contention
* `std::shared_ptr`–related operations may occasionally stall or freeze
  due to scheduler jitter or runtime instability

These behaviors primarily affect components such as:

* `jh::observe_pool`

They are not design goals of the toolkit,
but limitations imposed by the underlying runtime environment.

---

### Platform Recommendation

If your project relies on the **full behavioral guarantees of JH Toolkit**:

* **Linux or macOS** are strongly recommended as final deployment platforms

> LLVM-Clang 20 is the most stable, smartest compiler for JH Toolkit on both Linux and macOS.  
> GCC 13+ is also fully supported on Linux, but may produce less optimal code in some cases.  
> If you work with Linux in a virtualized environment and LLVM 20 is available, it is recommended to use it instead of
> GCC.  
> Normally, if homebrew is available, you can install LLVM 20 with `brew install llvm@20` and use it by setting
> `brew link --force llvm@20` in your environment.

If you only rely on a **restricted subset**, such as:

* `jh-toolkit-pod`
* POD types and layout-stable utilities

then Windows usage is generally acceptable.

Some behaviors provided on Windows are **engineering approximations**,
intended for development convenience rather than semantic identity.

---

## 🧩 Unified CMake Target Semantics

Although JH Toolkit can be built and distributed in multiple forms
(header-only, static, or pod-only),
**these variants do not represent different user-facing libraries**.

From the user's perspective, they all expose the **same public API surface**.

### One API — Multiple Delivery Forms

The following build artifacts:

- `jh-toolkit`
- `jh-toolkit-pod`
- `jh-toolkit-static`

exist for **distribution, performance, and build-control reasons only**.

They do **not** require different usage patterns at the CMake level.

---

### Installed / Conan-Based Usage

When JH Toolkit is **installed** (via Conan, system install, or package manager),
it is consumed as a standard CMake package:

```cmake
find_package(jh-toolkit REQUIRED)

target_link_libraries(my_project PRIVATE jh::jh-toolkit)
````

This applies **equally** to:

* Header-only builds (which provide `jh::jh-toolkit` as a header-only target)
* POD-only builds (which provide `jh::jh-toolkit-pod` as a header-only target)
* Precompiled static builds (which provide `jh::jh-toolkit-static` as a static library target)

The specific internal composition is resolved automatically.

---

### FetchContent Usage (1.4.0+)

Starting from **v1.4.0**, JH Toolkit can also be consumed directly via
`FetchContent`, without prior installation:

```cmake
FetchContent_Declare(
        JH_Toolkit
        GIT_REPOSITORY https://github.com/JeongHan-Bae/JH-Toolkit.git
        GIT_TAG 1.4.x-LTS
)

FetchContent_MakeAvailable(JH_Toolkit)

target_link_libraries(my_project PRIVATE jh-toolkit)
```

In this mode:

* the target name is **`jh-toolkit`** (no `jh::` namespace)
* the toolkit is treated as an **in-tree dependency**
* no lookup via an installed package root is required

Despite the different target name, the **API surface and semantics are identical**
to the installed / Conan-based usage.

---

### Summary

| Acquisition Method | Target to Link   | Notes                                |
|--------------------|------------------|--------------------------------------|
| Install / Conan    | `jh::jh-toolkit` | Standard installed package semantics |
| FetchContent       | `jh-toolkit`     | In-tree / vendored usage (1.4.0+)    |

> **Do not choose targets based on API differences.**
> The choice only affects **how the toolkit is obtained**, not **how it is used**.

---

## ⚙️ CMake Integration Example

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(jh-toolkit REQUIRED)

add_executable(my_project main.cpp)
target_link_libraries(my_project PRIVATE jh::jh-toolkit)          # Header-only mode
# or
target_link_libraries(my_project PRIVATE jh::jh-toolkit-static)   # Optimized static linkage
```

---

## 🧩 Dual-Mode Headers

`jh-toolkit` provides a set of **dual-mode headers** under
[`include/jh/macros/`](macros/dual_mode_headers.md) — mainly:

* `header_begin.h`
* `header_end.h`

### ⚙️ Motivation & Advantages

With **dual-mode headers**, JH Toolkit eliminates code duplication between header-only and compiled modes.

> 🧠 **Write once, use twice.**

| Without Dual-Mode        | With Dual-Mode             |
|--------------------------|----------------------------|
| Two separate definitions | One unified source         |
| Divergent code paths     | Guaranteed synchronization |
| Manual inline management | Automatic via macros       |

### 🧰 Using Independently

1. Copy `header_begin.h` and `header_end.h`
2. Keep Apache 2.0 license notice
3. Optionally rename macro prefix `JH_`

---

## 🧪 Debug & FastDebug Modes

| Mode          | Optimization | Debug Info     | Use Case                        |
|---------------|--------------|----------------|---------------------------------|
| **Debug**     | `-O0`        | ✅ Full symbols | Traditional debug               |
| **FastDebug** | `-O2 -g`     | ✅ Partial      | CI-friendly performance testing |

> **FastDebug** helps reveal optimization-related bugs while keeping builds lightweight.

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=FastDebug
cmake --build build-debug
ctest --test-dir build-debug --output-on-failure
```

---

## 🧩 Platform & Compatibility Notes

### ❌ Unsupported Platforms

| Platform                  | Status                | Reason                                                        |
|---------------------------|-----------------------|---------------------------------------------------------------|
| **MSVC**                  | ❌                     | Incomplete `concepts`, `ranges`, and coroutine semantics      |
| **32-bit (x86, ARMv7)**   | ❌                     | `static_assert(sizeof(std::size_t) == 8)` ensures 64-bit only |
| **Windows ARM64 (MinGW)** | ⚠️ **Not Guaranteed** | Incomplete `std::ranges` and coroutine features               |

> ⚠️ For Windows ARM64, use **WSL2 + Ubuntu + GCC** for reliability.

### 📱 Mobile & Embedded

* ❌ Not intended for embedded or 32-bit.
* ✅ Android/iOS via `add_subdirectory()`.
* 📦 Use `jh::pod` for minimal deployment.

---

### 🧠 Notes on MinGW

* Older `mingw64` lacks full C++20 features.
* ✅ Fully supported with **MSYS2 UCRT64 + GCC 14+**.
* Header-only mode works even on older MinGW.

---

### 🧩 Verifying Installation

```cpp
#include <jh/immutable_str>
#include <iostream>

int main() {
    jh::observe_pool<jh::immutable_str> pool;
    const auto str = pool.acquire("Hello, Oree!");
    std::cout << str->view() << std::endl;
}
```

```bash
g++ -std=c++20 -I/usr/local/include main.cpp -o test
./test
```

---

## 🧾 Summary

| Category               | Recommended                         |
|------------------------|-------------------------------------|
| **Compiler (Linux)**   | LLVM 20 or GCC 13+                  |
| **Compiler (macOS)**   | LLVM 20+ (`brew install llvm@20`)   |
| **Compiler (Windows)** | MSYS2 UCRT64 (GCC 14+)              |
| **CMake**              | ≥ 3.20                              |
| **System**             | 64-bit only                         |
| **Distribution**       | Conan `.tar.gz` via GitHub Releases |

---

> 🚀 Build with confidence — JH Toolkit is tested, modern, and designed for clarity.
