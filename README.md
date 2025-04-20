# JH Toolkit

### **version: 1.3.0-dev**

**A Modern C++20 Utility Library with Coroutine-based Generators, Behavior-defined Concepts, Immutable Strings, and Weak pointer-based Object Pooling.**

<!-- ✅ Language & Standard Support -->
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-violet.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-Only](https://img.shields.io/badge/header--only-partial-green)](#)
[![Coroutines](https://img.shields.io/badge/coroutines-supported-brightgreen)](https://en.cppreference.com/w/cpp/language/coroutines)
[![Concepts](https://img.shields.io/badge/concepts-supported-brightgreen)](https://en.cppreference.com/w/cpp/language/constraints)

<!-- ✅ Feature Highlights -->
[![Object Pool](https://img.shields.io/badge/object--pooling-weak_ptr_based-brown)](#)
[![Immutable Strings](https://img.shields.io/badge/immutable--strings-truly_immutable-brown)](#)
[![Generators](https://img.shields.io/badge/generators-coroutine_driven-brown)](#)

---

🚀 **JH Toolkit 1.3.0-dev - Expanding Core Features!**

## 📌 Requirements

- **C++20** (mandatory)
- **CMake 3.14+** (for library usage)
- **CMake 3.20+** (for library compilation and installation)
- **Git** (required for debugging mode compilation)
- **A modern C++20 compiler** (GCC 10+, Clang 10+)
  > **GCC(MinGW)**, **Clang** are tested and supported in every release.
  > **MSVC is not supported** due to its limited support for C++20 features. We do not plan to downgrade to match MSVC's capabilities, so users should use GCC or Clang instead.
  > For **Windows ARM64**, native support for MinGW is limited. We recommend using **GCC within WSL2** to achieve near-native performance.
  
Ensure your project enables **C++20**, otherwise `jh-toolkit` will not compile:
```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

> ⚠️ **Not Designed for Embedded or 32-bit Platforms**  
> JH Toolkit is designed for **modern 64-bit systems** (x86_64, ARM64). All container internals use  
> `uint64_t` for `size_`, and STL compatibility **requires `size()` to match `uint64_t` semantics**.  
> As a result, **embedded targets or 32-bit builds are not supported**.

> 📱 **Mobile (Android/iOS) and Cross-Compiling Notes**  
> When targeting **mobile platforms**, we strongly recommend **project-local integration** (e.g., `add_subdirectory(jh-toolkit)`)  
> instead of precompiled `install` + `find_package()`.
>
> This avoids **ABI mismatch issues** when pulling desktop-compiled binaries into mobile toolchains  
> (e.g., NDK, Xcode, etc.), and allows tight control over architecture-specific builds.

> 🧰 **CMake < 3.20 Systems**  
> If your system does not support `CMake 3.20+`, you can still:
> - Include the source directly in your CMake tree.
> - Use `#include "jh/header"` instead of `#include <jh/header>`.
> - Manually link `.cpp` sources for non-header-only modules.

> ✅ Local embedding works well on modern Linux/macOS desktops, CI runners, or mobile SDKs —  
> just **not embedded** or **resource-constrained systems**.

---

## 📥 Installation

#### 🔸 **Option 1: Latest Development Version (1.2+ - Ongoing Updates)**
```sh
git clone https://github.com/JeongHan-Bae/jh-toolkit.git
```

#### 🔹 **Option 2: Latest LTS Release (1.2.3+)**
```sh
git clone --branch 1.2.x-LTS --depth=1 https://github.com/JeongHan-Bae/jh-toolkit.git
```
👉 Or download from: **[JH Toolkit Latest LTS Release](https://github.com/JeongHan-Bae/JH-Toolkit/releases/latest)**

---

### 1️⃣ Build and Install
---

#### 🔹 Full Version (Default)

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

This installs the complete **`jh-toolkit`**:

- Provides full header sets and implementation.
- Use `jh::jh-toolkit` and `jh::jh-toolkit-impl`.

Recommended on development machines or when full functionality is needed.

---

#### 🔸 Minimal Version (Header-only POD system)

```sh
cmake -B build-pod -DCMAKE_BUILD_TYPE=Release -DTAR=POD
cmake --build build-pod
sudo cmake --install build-pod
```

This installs only the **POD system**:

- A lightweight, header-only C++20 template library.
- Use `jh::jh-toolkit-pod`.

Recommended for minimal deployment, embedding, or when implementation is not needed.

📖 See [docs/pod.md](docs/pod.md) for usage details.

---

#### 🧩 Custom Build Modes for Modular Deployments

You can selectively build and install specific components using:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DTAR=POD,ALL
```

- The `TAR` option accepts a comma-separated list of target modules to include.
- Currently supported values: `POD`, `ALL`
- Targets are **individually downloadable and usable** depending on your deployment needs.

Use case examples:

| Usage Context       | Suggested `TAR` Value | What It Builds                       |
|---------------------|-----------------------|--------------------------------------|
| Minimal runtime     | `POD`                 | Header-only POD system               |
| Development machine | `ALL`                 | Everything: headers + implementation |
| Shared dependency   | `POD,ALL`             | All targets available in local build |

> ✅ `TAR=POD,ALL` makes all targets available without restricting modularity.  
> This allows dependent projects to only link what they need.

---

#### 📦 Installed Targets by Build Mode

| Build Mode           | CMake Targets Available                 | Description                               |
|----------------------|-----------------------------------------|-------------------------------------------|
| `TAR=ALL` (default)  | `jh::jh-toolkit`, `jh::jh-toolkit-impl` | Full header set + implementation          |
| `TAR=POD`            | `jh::jh-toolkit-pod`                    | Header-only POD module                    |
| `TAR=POD,ALL`        | All above targets                       | Maximum availability, optional usage      |

All modes are compatible with:

```cmake
find_package(jh-toolkit REQUIRED)
```

Then link only the targets your project needs.

---

### 2️⃣ Verify Installation

If installation is successful, you should be able to include and use `jh-toolkit` in a minimal program:

```c++
#include <jh/immutable_str> // or <jh/immutable_str.h>
#include <iostream>

int main() {
    auto pool = jh::pool<jh::immutable_str>();
    const auto str = pool.acquire("Hello, JH Toolkit!");
    std::cout << str->view() << std::endl;
    return 0;
}
```

If this compiles and runs successfully, `jh-toolkit` is correctly installed.

## 📦 Usage

### CMake Integration

Add `jh-toolkit` to your project:

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(jh-toolkit REQUIRED)

add_executable(my_project main.cpp)
target_link_libraries(my_project PRIVATE jh::jh-toolkit) # For header-only modules
target_link_libraries(my_project PRIVATE jh::jh-toolkit-impl) # For compiled components
```

---

### ⚙️ Debug Build Setup

Build and test the project using standard `Debug` or performance-friendly `FastDebug` modes:

```sh
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
```
**or**
```sh
cmake -B build-debug -DCMAKE_BUILD_TYPE=FastDebug
```
**then**
```sh
cmake --build build-debug
ctest --test-dir build-debug --output-on-failure
```

---

#### 🧪 Fast, CI-friendly Testing

- All unit tests are registered with `ctest`.
- Most modules are mature and run limited randomized iterations (e.g., 128 rounds).
- Suitable for rapid regression checks and automation pipelines.

---

#### 🛠️ FastDebug vs Debug

| Mode          | Optimization | Debug Info | Use Case                                             |
|---------------|--------------|------------|------------------------------------------------------|
| **Debug**     | `-O0`        | ✅ `-g`     | Traditional debug builds; full symbolic debugging    |
| **FastDebug** | `-O2`        | ✅ `-g`     | Faster test execution with minimal optimization bias |

---

#### 🚫 What FastDebug *Does Not* Do

FastDebug **only** applies `-O2 -g` and intentionally avoids more aggressive compiler behaviors:

- ❌ No `-march=native`
- ❌ No auto-vectorization (`-ftree-vectorize`)
- ❌ No loop unrolling (`-funroll-loops`)
- ❌ No SIMD-only code paths

This ensures consistent runtime behavior and portability across CI runners or developer machines.

> ✅ **FastDebug mode not only speeds up testing, but also helps identify bugs that only appear under compiler optimizations (e.g., undefined behavior, aliasing issues, optimization-elided side effects).**

---

### 📌 Notes

- 🧪 All modules are covered by unit tests for **behavioral correctness**.
  - Modules like `generator` and `pool` are stable but **not yet micro-optimized**.
  - Core utility types such as `immutable_str` and `runtime_arr` are **micro-optimized** and designed for practical performance without sacrificing safety or clarity.
- 🧩 Modules like `data_sink` and `pod_stack` were previously benchmarked
  but are now deprecated due to consistently poor performance under optimization.  
  They have been **removed from test coverage and performance tracking**.

---

### ⚠️ Deprecated Experimental Modules

- The previously benchmarked **`data_sink`** and **`pod_stack`** have been moved to the `/experimental` directory:
  - While they performed well under **Clang `-O0`/`-O1`**, they **underperformed STL by 10–20% at `-O2`**.
  - On **GCC**, performance dropped further due to missed optimization opportunities.
  - These modules are no longer maintained, and **no benchmark results will be provided going forward**.
- All `/experimental` modules:
  - Are **not part of the public API**.
  - May be removed or changed **without notice**.
  - Have **no performance or stability guarantees**.

---

### ⚙️ On Lightweight Benchmarking

Some core modules include **lightweight, embedded benchmarks** in their test suites to validate real-world performance without requiring a full benchmarking framework.

- 🧪 Modules with benchmarks:
  - `immutable_str`: A fully immutable, thread-safe string type optimized for hashing and interning.
  - `runtime_arr`: A fixed-size, non-resizable heap array with raw-layout speed and STL compatibility.
- 🧪 Modules **without active benchmarks**:
  - `generator`, `pool`: Stable but not micro-optimized yet.
  - `data_sink`, `pod_stack`: Deprecated and removed from performance consideration.

#### 📈 Results Overview

- Benchmarks demonstrate that optimized modules are typically **comparable to STL** in performance.
- In specific cases, custom structures even **outperform STL** by reducing allocation or indirection overhead.
- Overhead remains **predictable and minimal** across compilers and optimization levels.

#### 🧪 Running Benchmarks

- 🚀 Run test binaries directly to view timing output (no need for `ctest`).
- 📚 Module documentation includes brief benchmark context and insights.

> ❗ No separate `benchmark/` directory or `tests/benchmark.cpp` exists; new performance testing will be added only when necessary.

---


### 🛠 Build Behavior

- 🔧 Benchmarks, examples, and debugging tools are **disabled in Release builds**.
- 🚫 Install is **disabled in Debug builds** to prevent accidental system changes.
- 🔁 Randomized test rounds are capped (e.g. 128 iterations) for long-verified modules — ensuring reasonable CI duration without sacrificing coverage.

---

## 📚 JH Toolkit Modules Overview

### 📦 Template-Based Modules

Modern C++20 header-only components focused on **zero-cost abstraction**, **type safety**, and **performance-conscious design**.

#### 🧰 Core Utility Modules

- [`pod`](docs/pod.md) — A lightweight system of POD-like value types (`pod::pair`, `array`, `optional`, `bitflags`, etc.), optimized for serialization and placement-new.
- [`sequence`](docs/sequence.md) — Minimal C++20 concept for forward-iterable sequences; foundation for `view` operations.
- [`iterator`](docs/iterator.md) — Iterator detection, `iterator_t<>` alias, and validation concepts (`input_iterator`, etc.).
- [`views`](docs/views.md) — Lazy, allocation-free range adapters: `enumerate`, `zip`, and more; compatible with all `sequence`s.

#### 🔁 Functional Utilities

- [`generator`](docs/generator.md) — Coroutine-based generator/stream-style interfaces.
- [`pool`](docs/pool.md) / [`sim_pool`](docs/sim_pool.md) — Shared pointer-based object pools with automatic deduplication.
- [`immutable_str`](docs/immutable_str.md) — ABI-stable, thread-safe immutable strings with `std::string_view` interop.

---

### 🧱 Value Types

Finalized C++ types intended for **immutable**, **compact**, and **semantically clear** ownership.

- [`runtime_arr`](docs/runtime_arr.md) — Fixed-size, move-only runtime buffer; STL-compatible, allocator-aware, with `reset_all()` for reuse.

---

### 🔗 Quick Links to Module Docs

- [`generator`](docs/generator.md)
- [`immutable_str`](docs/immutable_str.md)
- [`iterator`](docs/iterator.md)
- [`pod`](docs/pod.md)
- [`pool`](docs/pool.md)
- [`runtime_arr`](docs/runtime_arr.md)
- [`sequence`](docs/sequence.md)
- [`sim_pool`](docs/sim_pool.md)
- [`views`](docs/views.md)

---

## **🐍 Pythonic Aesthetics and Philosophy in JH Toolkit**

While **JH Toolkit** is a modern C++20 library, its **design philosophy** embraces several key **Pythonic principles**, making C++ development **more expressive, efficient, and intuitive**. These principles shape how `jh-toolkit` is structured, promoting **readability, maintainability, and performance**.

### **1️⃣ Duck Typing: Behavior Over Declaration**
> _"If it walks like a duck and quacks like a duck, it must be a duck."_

- **How JH Toolkit Implements This**:
    - **`jh::pool<T>` automatically detects compatible types** (requires `T::hash()` and `operator==`).
    - **`jh::sequence` validates immutable iteration at compile time**, without requiring explicit inheritance.
    - **`jh::iterator<>` concepts ensure iterators conform to expected behavior**, rather than relying on inheritance.

### **2️⃣ Lazy Evaluation: Compute Only When Necessary**
> _"Better to compute only when needed than to compute everything upfront."_

- **How JH Toolkit Implements This**:
    - **`jh::generator<T, U>` defers computation** using coroutines.
    - **`jh::immutable_str` delays hash computation** until the first `hash()` call, avoiding unnecessary processing.
    - **`jh::pool<T>` manages objects dynamically**, avoiding upfront allocation.

### **3️⃣ Automatic Memory Management: Smart and Efficient Resource Handling**
> _"Memory management should be handled by the system, not by the user."_

- **How JH Toolkit Implements This**:
    - **`jh::sim_pool<T, Hash, Eq>` uses `std::weak_ptr<T>`** to automatically expire objects.
    - **`jh::pool<T>` ensures memory deduplication** without requiring manual object tracking.
    - **Automatic cleanup** is built into pooling mechanisms, similar to **Python’s garbage collection**.

### **4️⃣ Explicit is Better Than Implicit: Preventing Unintended Behavior**
> _"Code should be clear in its intentions and prevent implicit conversions that cause ambiguity."_

- **How JH Toolkit Implements This**:
    - **`jh::immutable_str` prohibits implicit conversions** that could lead to data races.
    - **`jh::generator<T, U>` is explicitly move-only**, preventing unintended copies.
    - **Concept-based design** enforces correctness at compile time.

### **5️⃣ Readability and Maintainability: Expressive and Clean Code**
> _"Code is read more often than it is written."_

- **How JH Toolkit Implements This**:
    - **Encapsulates complexity into reusable, well-documented modules**.
    - **Uses concepts to provide clear type constraints** rather than relying on SFINAE.
    - **Promotes modularity and interoperability** with standard C++20 features.

### **🔹 Why This Matters**
By following these Pythonic principles, **JH Toolkit makes modern C++ more ergonomic**, helping developers write **concise, expressive, and high-performance** code while reducing **boilerplate and maintenance costs**.

---

## **🔹 Why `snake_case`? Aligning with `std` for a Seamless Toolkit Experience**

JH Toolkit follows **`snake_case` naming conventions**, aligning with **C++ standard library (`std`)** rather than the more common **`camelCase` or `CapitalCase`** styles used by other third-party libraries. This choice is intentional and deeply rooted in **our goal of providing a seamless, toolkit-like experience** for developers.

### **📌 1. Consistency with the Standard Library**
> _"A toolkit should feel like a natural extension of the language."_

- The C++ **Standard Library (`std`) uses `snake_case`** for its core utilities, including:
    - **Type traits** (`std::is_same_v`, `std::decay_t`)
    - **Containers** (`std::vector`, `std::unordered_map`)
    - **Iterators & Algorithms** (`std::next`, `std::find_if`)
    - **Concepts in C++20** (`std::same_as`, `std::convertible_to`)

- By **mirroring `std` naming conventions**, **JH Toolkit blends naturally** into the developer's codebase, reducing **cognitive overhead**.


### **📌 2. JH Toolkit is Primarily a Data Structure & Template Library**
> _"JH Toolkit is not just a utility collection; it's a core toolkit."_

- Many third-party C++ libraries, especially frameworks, use **`CamelCase`** (`Boost`, `Qt`) or **`CapitalCase`** (`Eigen`, `OpenCV`).
- However, **JH Toolkit is a toolkit for generic data structures and utilities**, not a high-level framework.
- **Using `snake_case` ensures users interact with JH Toolkit just like they do with `std`**, making it feel **native to modern C++ development**.


### **📌 3. Improved Readability & Seamless Integration**
> _"The best tools are the ones that feel like they were always there."_

- `snake_case` improves **readability** when dealing with **template-heavy, generic code**.
- Developers using **JH Toolkit alongside `std` will experience a uniform interface**, making it easier to:
    - **Mix and match `std` and `jh` components.**
    - **Reduce friction when integrating `jh-toolkit` into existing projects.**
    - **Lower the learning curve for new users already familiar with `std`.**

### **📌 4. Future-Proofing for C++ Evolution**
> _"Following the direction of the standard makes long-term maintenance easier."_

- C++ is increasingly adopting **concept-based design** and modern **generic programming paradigms**.
- JH Toolkit **embraces these trends** by ensuring **consistent naming conventions** for **concepts, iterators, and utilities**.
- By aligning with `std`, JH Toolkit remains **future-proof** and **less likely to require adaptation** when integrating with **new standard features**.

---


## 👤 Author

Developed by **JeongHan-Bae**  
📧 [mastropseudo@gmail.com](mailto:mastropseudo@gmail.com)  
🔗 [GitHub Profile](https://github.com/JeongHan-Bae)

## 📜 License

This project is licensed under the **Apache 2.0 License**. See the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Feel free to open issues and pull requests to enhance the library.

---

🚀 **Enjoy coding with `jh-toolkit`!** 🚀
