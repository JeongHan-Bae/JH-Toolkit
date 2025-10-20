# 🧰 Build & Platform Guide

This document provides a complete reference for building **JH Toolkit**, including supported toolchains, Conan packaging, CMake targets, and platform-specific notes.

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
| `jh-toolkit-linux-x86_64` | ✅                  | ✅ (GCC 12+)        | Built on `ubuntu-latest` using GCC toolchain |
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

| Requirement    | Minimum Version | Notes                               |
|----------------|-----------------|-------------------------------------|
| **C++**        | 20              | Mandatory                           |
| **CMake**      | 3.14+           | For library usage                   |
| **CMake**      | 3.20+           | For full compilation & installation |
| **Git**        | Latest          | Required for Debug builds           |
| **System ABI** | 64-bit          | 32-bit builds prohibited            |

---

### 🧠 Recommended Toolchains

| Platform           | Recommended Compiler                          | Notes                                   |
|--------------------|-----------------------------------------------|-----------------------------------------|
| **Linux**          | **GCC 12+**                                   | CI-tested (`ubuntu-latest`)             |
| **macOS (Darwin)** | **LLVM Clang 20+** via `brew install llvm@20` | Most stable; preferred over Apple Clang |
| **Windows**        | **MSYS2 UCRT64 (GCC 13+)**                    | Required for full C++20 compliance      |

#### ✅ Minimum Supported Versions

| Compiler             | Version Range     | Status                                                            | Notes                |
|----------------------|-------------------|-------------------------------------------------------------------|----------------------|
| **GCC**              | ≥ 11              | ✅ Supported                                                       | Recommended: GCC 12+ |
| **LLVM Clang**       | 15–16             | ✅ Supported                                                       | Fully compatible     |
| **LLVM Clang 17–18** | ⚠️ **Rejected**   | Known to cause unresolved linkage (`std::hash<std::string_view>`) |                      |
| **LLVM Clang 19**    | 🚫 Not Tested     | Unsupported                                                       |                      |
| **LLVM Clang 20+**   | ✅ **Recommended** | Most stable on Darwin; install via Homebrew                       |                      |
| **MSVC**             | ❌ Prohibited      | Incomplete C++20 and ABI inconsistencies                          |                      |

> ✅ **Summary:**
> Prefer **LLVM 20+** on macOS and **GCC 12+** on Linux.
> Clang 17–18 are **explicitly disallowed** due to reproducible linker instability.

---

## 📥 Installation

#### 🔸 **Option 1: Latest Stable Version (mainstream)**

```bash
git clone https://github.com/JeongHan-Bae/jh-toolkit.git
```

#### 🔹 **Option 2: Latest LTS Release (1.3.0+)**

```bash
git clone --branch 1.3.x-LTS --depth=1 https://github.com/JeongHan-Bae/jh-toolkit.git
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

| TAR Value | Description                                            |
|-----------|--------------------------------------------------------|
| `POD`     | Header-only POD module                                 |
| `ALL`     | Full build: `jh::jh-toolkit` + `jh::jh-toolkit-static` |
| `POD,ALL` | Builds both; all targets available                     |

---

### 📦 Installed CMake Targets

| Mode          | Targets Installed                         | Description                                                               |
|---------------|-------------------------------------------|---------------------------------------------------------------------------|
| `TAR=ALL`     | `jh::jh-toolkit`, `jh::jh-toolkit-static` | Full toolkit: headers + optimized static objects                          |
| `TAR=POD`     | `jh::jh-toolkit-pod`                      | Header-only POD library                                                   |
| `TAR=POD,ALL` | All of the above                          | Provides full modular access for development and distribution flexibility |

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
[`include/jh/macros/`](../include/jh/macros/) — mainly:

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
* ✅ Fully supported with **MSYS2 UCRT64 + GCC 13+**.
* Header-only mode works even on older MinGW.

---

### 🧩 Verifying Installation

```cpp
#include <jh/immutable_str>
#include <iostream>

int main() {
    jh::pool<jh::immutable_str> pool;
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
| **Compiler (Linux)**   | GCC 12+                             |
| **Compiler (macOS)**   | LLVM 20+ (`brew install llvm@20`)   |
| **Compiler (Windows)** | MSYS2 UCRT64 (GCC 13+)              |
| **CMake**              | ≥ 3.20                              |
| **System**             | 64-bit only                         |
| **Distribution**       | Conan `.tar.gz` via GitHub Releases |

---

> 🚀 Build with confidence — JH Toolkit is tested, modern, and designed for clarity.
