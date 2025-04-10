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

✅ **Updated `generator` for enhanced coroutine support.**  
✅ **`data_sink` completed and benchmarked (with low-memory support).**  
✅ **`radix_sort` architecture-specific optimizations done for ARM & AMD.**  
❌ **`sequence` expansion planned but not yet implemented.**  
🔄 **`runtime_arr` completed and benchmarked – ideal for small, fast, templated arrays (Clang) – GCC support pending.**

---

## 📖 **Library Philosophy**
JH Toolkit's **original template structures (`data_sink` and `runtime_arr`)** are designed for **smaller,  
faster, and more specialized template structures**, unlike the **generic STL**.

- **`data_sink` and `runtime_arr` are custom structures** optimized for speed and memory efficiency.
- **Benchmarking is enabled for these unique structures** to fine-tune performance in `Debug` builds.
- **General-purpose functionality is prioritized over benchmark optimizations at this stage.**

Other structures (mostly inspired by other languages, such as `jh::generator`)
are **primarily implemented for functionality**.  
They will **not be optimized for performance until all functions are fully complete and robust**.

🔹 **Benchmarks for these general-purpose structures are planned in JH Toolkit 2+.x.**

---

This update ensures better flexibility in development while **keeping benchmarks optional** and **clearly separating Debug Mode builds**. 🚀


## 🚀 1.2.x features

### 🔥 **JH Toolkit is Now Feature-Complete!**

This version includes major improvements to **immutable strings** and **object pooling**, with extensive cross-platform testing.

#### 🚀 **New Features & Enhancements**
1. **Enhanced `jh::immutable_str`**
    - **New constructor:** Now supports construction from any type **implicitly convertible to `std::string_view`**, protected by an **explicitly provided `std::mutex`**.
    - **Safe shared construction:** Added `safe_from(std::string_view, std::mutex&)` to ensure **lifetime safety** when using views from mutable sources.
    - **Maintains full compatibility with LLVM `extern "C"` APIs** through the `const char*` constructor.

2. **New Object Pooling System: `jh::pool<T>`**
    - **Weak pointer-based content-aware pooling** for deduplicating objects, especially useful for **immutable types**.
    - **Automatic cleanup** of expired objects—no need for manual tracking.
    - **Custom hash & equality support** for optimized storage of unique instances.
    - **Seamlessly integrates with `jh::immutable_str`**, reducing redundant allocations.

---

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

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

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


### ⚙️ Debug Build Setup

```sh
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

- 🧪 Tests and benchmarks are built **alongside** regular targets.
- ⚠️ Do **not** use `ctest` for benchmarks — they are **intentionally slow**.
- `ENABLE_BENCHMARK` is **off by default** in debug builds.
- ✅ Run benchmarks with:

```sh
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_BENCHMARK=ON
cmake --build build-debug
./build-debug/tests/test_benchmark
```

---

### 💾 Low-Memory Debug Mode

For VMs or CI runners with tight memory:

```sh
cmake -B build-debug-lowmem \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_BENCHMARK=ON \
      -DLOW_MEMORY_BENCHMARK=ON

cmake --build build-debug-lowmem
./build-debug-lowmem/tests/test_benchmark
```

🔹 Disables memory-heavy benchmarks (like `std::list`) to prevent OOM.  
🔹 Ideal for environments with **<2GB RAM** or **shared runners**.

---

### ✅ Running Tests (Debug Mode)

```sh
ctest --test-dir build-debug
```
> Run Benchmarks separately with `./build-debug/tests/test_benchmark` or only if you understand the implications.


---

### 📌 Notes

- Benchmarks focus on **performance-oriented containers** like `data_sink`, `runtime_arr`, and `pod_stack`.
- Functional modules like `generator`, `pool`, and `immutable_str` are tested for **behavioral correctness** — not micro-optimized (yet).
- Benchmarking, Debugging and Examples are off in release builds.
- Install is forbidden in Debug Mode to prevent accidental system changes.

---

## 📚 JH Toolkit Modules Overview

### 📦 Template-Based Modules

Modern C++ templates designed for **performance**, **type safety**, and **zero-cost abstraction**.

#### 🚀 Performance-Oriented Templates
Optimized for **tight loops**, **memory locality**, and **zero overhead reset/fill**.

- [`runtime_arr<T>`](docs/runtime_arr.md) — Fixed-size flat buffer; bit-packed `bool` specialization; efficient for PODs and inner-loop algorithms.
- [`pod_stack<T>`](docs/pod_stack.md) — Append/rollback stack for PODs; perfect for non-heap scoped memory reuse.
- [`data_sink<T>`](docs/data_sink.md) — Append-only FIFO for low-latency pipelines; excellent for algorithms or cases where input size is not known in advance.

#### 🛠️ Functional Utilities & Object Sharing
Lightweight templates for **lifetime control**, **memory deduplication**, and **pool-based sharing**.

- [`generator<T, U = std::monostate>`](docs/generator.md) — Coroutine-based lazy generator with `send()` support; usable with `range-for` and interactive iteration.
- [`sim_pool<T, Hash, Eq>`](docs/sim_pool.md) — Type-stable content-aware object pool using `weak_ptr` + `shared_mutex`; supports custom hash/equality functions.
- [`pool<T>`](docs/pool.md) — Auto-specialized wrapper over `sim_pool`; detects `.hash()` + `operator==()` and deduplicates immutable types by content.

> 💡 Philosophy: Use `shared_ptr` + weak-pool strategy for **thread-safe**, **immutable** object sharing. We recommend `.hash()`-based caching on immutable types over repetitive `std::hash<T>`. Pool entries expire automatically when no longer referenced.

> All JH template structures are **behavior-safe**, not thread-safe by default.
> - Type constraints (like `T: pod_like`) ensure correctness by concept.
> - Thread safety should be **explicitly handled** via `shared_ptr`-based pooling or external locking.
> - We strongly recommend immutability for all pooled/shared objects.

---

### 🧱 Non-Template Value Types

Robust C++ objects designed to be **immutable**, **final**, and **safe by design**.

- [`immutable_str`](docs/immutable_str.md) — Truly immutable string with `std::string_view`, C-string, and hash-friendly access; thread-safe and `final`.
    - `immutable_str` disallows copy/move; instances are allocated once and never modified.
    - For shared access, use `jh::atomic_str_ptr` (`std::shared_ptr<immutable_str>`).

---

###  🧩 Concept & Trait Definitions

Modern **C++20 concepts** for compile-time validation and expressive template constraints.

- [`iterator<T>`](docs/iterator.md) — Extracts `iterator_t<T>` type trait; validates STL/custom iterators via concepts like `input_iterator`, `random_access_iterator`, etc.
- [`sequence`](docs/sequence.md) — A concept for validating range-compatible containers; works with STL and custom iterable types.
- [`pod_like<T>`](docs/pod.md) — Ensures a type is trivially constructible, copyable, destructible, and standard layout.
    - Includes macros: `JH_POD_STRUCT(...)` and `JH_ASSERT_POD_LIKE(...)` for enforcing and validating PODs.

---

### 🔗 Quick Links to Module Docs

- [`data_sink`](docs/data_sink.md)
- [`generator`](docs/generator.md)
- [`immutable_str`](docs/immutable_str.md)
- [`iterator`](docs/iterator.md)
- [`pod`](docs/pod.md)
- [`pod_stack`](docs/pod_stack.md)
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
