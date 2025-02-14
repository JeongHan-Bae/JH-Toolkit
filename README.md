# JH Toolkit

### **version: 1.2.2**

**A Modern C++20 Utility Library with Coroutine-based Generators, Behavior-defined Concepts, Immutable Strings and Weak pointer-based Object Pooling.**

---

🚀 **JH Toolkit 1.2.2 - Cross-Platform Compatibility Improved, Now Tested on Windows (MinGW), macOS (Clang), and Ubuntu (GCC).**

✅ **CMake build requirement lowered to 3.20+ for broader system compatibility.**  
✅ **CMake usage requirement remains 3.14+.**

🆕 **New Feature:**
- **Unified Header Inclusion:** You can now include headers using either `#include <jh/header>` or `#include <jh/header.h>` interchangeably.

Future patches (`1.2.x`) will continue to enhance **cross-platform support**, refine build configurations, and potentially introduce more detailed test cases. MSVC support is not planned yet.

🟢 **Looking for the last stable release?**  
👉 Check out **[JH Toolkit 1.1.3 (Latest Stable Release)](https://github.com/JeongHan-Bae/JH-Toolkit/releases/latest)**

---

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

## 🚀 What's New in 1.2.0+ (Feature-Complete with Cross-Platform Support)

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

### 1.1.x Features

- **`jh::generator<T, U>` now supports iterators** (`jh::generator<T, U>::iterator` which is actually `jh::iterator<jh::generator<T, U>>`), allowing use in **range-based for loops** for types without `send()` (i.e., `U == std::monostate`).
- **Generators and immutable strings are now `final`**, preventing unintended inheritance.
- **Explicit copy prohibition** for `jh::generator<T, U>` – only move construction is allowed.

---

## Overview

_Current version improves memory efficiency with object pooling, enhances immutable strings for safer lifetime management, and refines iterator validation with a behavior-driven approach._

`jh-toolkit` is a modern, lightweight C++20 utility library that provides:
- **Coroutine-Based Generators** – A Pythonic `yield` mechanism in C++20 for efficient lazy evaluation.
- **Immutable Strings** – True immutability, memory efficiency, and thread safety, now with mutex-based safe construction.
- **Sequence Concept** – A C++20 `concept` for compile-time validation of immutable sequences.
- **Object Pooling System** – A `std::weak_ptr`-based **content-aware object pool** for deduplication and efficient resource reuse.
- **Iterator Concepts and Forward Declarations** – A **behavior-driven** approach to iterator validation that ensures **compatibility with both standard (`std::`) and custom (`jh::` or self-defined) iterators** without relying on explicit type declarations.

This library is designed for **high-performance applications**, ensuring **efficiency, type safety, and clean code**.

---

## 📌 Requirements

- **C++20** (mandatory)
- **CMake 3.14+**
- **A modern C++20 compiler** (GCC 10+, Clang 10+, MSVC 19.28+)
  > **GCC(MinGW)**, **Clang** are tested and supported in every release.

Ensure your project enables **C++20**, otherwise `jh-toolkit` will not compile:
```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## 📥 Installation

#### 🔸 **Option 1: Latest Development Version (1.2.x - Ongoing Updates)**
```sh
git clone https://github.com/JeongHan-Bae/jh-toolkit.git
```

#### 🔹 **Option 2: Last Stable Release (1.1.3)**
```sh
# Clone the last stable release (1.1.3)
git clone --branch JH-Toolkit-1.1.3 --depth=1 https://github.com/JeongHan-Bae/jh-toolkit.git
```
👉 Or download from: **[JH Toolkit 1.1.3 Release Page](https://github.com/JeongHan-Bae/JH-Toolkit/releases/tag/JH-Toolkit-1.1.3)**
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
---

### **🔧 Modules Overview**

### **🌀 Coroutine-Based Generators (`jh::generator<T, U>`)**
- Provides **lazy evaluation** via **iterative** and **interactive** coroutine-based generators.
- Supports `send()` for interactive communication within a generator.
- Enables **range-based for loops** via `jh::generator<T, U>::iterator`.
- Includes utilities for converting generators to **`std::vector`** and **`std::list`**.
- **Explicitly move-only** to prevent unintended copies.

### **🔒 Immutable Strings (`jh::immutable_str`)**
- Provides **true immutability** with **memory-level enforcement**.
- Ensures **thread safety** and prevents **accidental modification**.
- Supports **efficient hash-based storage** with **delayed hash computation** (lazy evaluation).
- Constructor: Accepts `const char*` as default single-parameter constructor (LLVM API compatible).
- **New constructor:** Accepts `std::string_view` with an associated `std::mutex` for safe external storage.
- Designed for **read-only data, global constants, and concurrent environments**.

### **📚 Sequence Concept (`jh::sequence`)**
- Defines `jh::sequence` as a **C++20 concept** for immutable iteration.
- Provides `sequence_value_type<T>` for extracting element types at compile time.
- Works seamlessly with **STL containers** and **custom iterables**.
- Ensures **compile-time validation** of sequence-like types for safer API design.


### **🔄 Object Pooling (`jh::pool<T>` & `jh::sim_pool<T, Hash, Eq>`)**

**Designing principles:**
- **Designed for memory management and object deduplication**, rather than extreme performance optimization.
- Uses **`std::weak_ptr` for automatic cleanup**, eliminating manual tracking overhead.
- **Not a raw memory pool**: `jh::pool<T>` does **not** allocate objects in a contiguous memory block like traditional memory pools.
- **Provides safe, high-level API design over manual memory management.**

**Realized features:**
- Implements **weak pointer-based** content-aware object pooling.
- **`jh::pool<T>`**: Automatically deduces **hash and equality functions** based on `T::hash()` and `operator==`.
- **`jh::sim_pool<T, Hash, Eq>`**: Supports **custom hash and equality functions** for specialized object pooling.
- **Thread-safe** with `std::shared_mutex`.
- **Automatic cleanup** of expired objects with **optional manual cleanup**.

### **⚙️ Iterator Concepts (`jh::iterator<>`)**
- Provides **forward declaration** of `jh::iterator<>` for use in generators and containers.
- Defines **C++20 concepts** (`input_iterator`, `output_iterator`, `forward_iterator`, etc.) to validate iterator behavior.
- Works with **both `std::` and costume iterators**.
- **Duck typing approach**: Concept validation is based on **behavior, not inheritance**.

---

## 🔬 Debug Mode (Optional)
To enable **tests and examples**, build in `Debug` mode:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```

---

## 📖 API Documentation

For detailed API documentation, please refer to the corresponding module headers:

- [Coroutine-Based Generators (`generator.h`)](docs/generator.md)
- [Immutable Strings (`immutable_str.h`)](docs/immutable_str.md)
- [Iterator Declaration & Concept (`iterator.h`)](docs/iterator.md)
- [Object Pooling (`pool.h`)](docs/pool.md)
- [Sequence Concept (`sequence.h`)](docs/sequence.md)
- [Object Pooling Base Structure (`sim_pool.h`)](docs/sim_pool.md)

---

### ⚡ **Performance & Design Philosophy**  
JH Toolkit is designed to bring **high-level abstractions to modern C++20**, focusing on **usability, safety, Pythonic design, and cross-platform stability**.

- **Performance is important, but usability and safety come first.**
- **Reasonable overhead is acceptable** if it improves **API clarity, automatic memory management (RAII), and maintainability**.
- **Standard C++ (`std`) is used as the foundation** to ensure **cross-platform stability** and **future compatibility** with C++ standards.
- **RAII, `move`, `std::shared_ptr`, and `std::unique_ptr`** are leveraged correctly to ensure **safe and efficient resource management**.
- **1.2.x focuses on feature completion**; optimizations may come later but are **not the primary goal** at this stage.

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

## 📊 **Why No Benchmarks Yet?** (Planned for 2+.x or higher)  
JH Toolkit **prioritizes feature completeness over micro-optimizations** at this stage.

- **Modern C++ optimizes well when used correctly.**
    - Proper use of **RAII, `move`, `std::shared_ptr` / `std::unique_ptr`** leads to **reasonable efficiency**.
- **Usability, safety, and Pythonic low-code abstractions** are the primary focus.
- **Performance tuning may be introduced in future releases**, but for now, the priority is **ensuring API stability and cross-platform compatibility**.

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
