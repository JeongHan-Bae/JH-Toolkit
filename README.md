# JH Toolkit

### **version: 1.1.2**

**A Modern C++20 Utility Library with Coroutine-based Generators, Immutable Strings, and Sequence Concepts**

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

## What's New (1.1.0)

- **`jh::generator<T, U>` now supports iterators** (`jh::generator<T, U>::iterator` which is actually `jh::iterator<jh::generator<T, U>>`), allowing use in **range-based for loops** for types without `send()` (i.e., `U == std::monostate`).
- **Generators and immutable strings are now `final`**, preventing unintended inheritance.
- **Explicit copy prohibition** for `jh::generator<T, U>` – only move construction is allowed.

## Overview

`jh-toolkit` is a modern, lightweight C++20 utility library that provides:
- **Coroutine-Based Generators** – A Pythonic `yield` mechanism in C++20 for efficient lazy evaluation.
- **Immutable Strings** – True immutability, memory efficiency, and thread safety.
- **Sequence Concept** – A C++20 `concept` for compile-time validation of immutable sequences.

This library is designed for high-performance applications, ensuring efficiency, type safety, and clean code.

## 📌 Requirements

- **C++20** (mandatory)
- **CMake 3.14+**
- **A modern C++20 compiler** (GCC 10+, Clang 10+, MSVC 19.28+)

Ensure your project enables **C++20**, otherwise `jh-toolkit` will not compile:
```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## 📥 Installation

### 1️⃣ Build and Install

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

### 2️⃣ Verify Installation

If installation is successful, you should be able to include and use `jh-toolkit` in a minimal program:

```c++
#include <jh/immutable_str.h>
#include <iostream>

int main() {
    const jh::immutable_str str("Hello, JH Toolkit!");
    std::cout << str.view() << std::endl;
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

## 🔧 Modules Overview

### 🌀 Coroutine-Based Generators (`jh::generator<T, U>`)
- Implements **lazy evaluation** with **iterative** and **interactive** coroutine-based generators.
- Supports `send()` to interactively send values into the generator.
- **Now supports `iterator` (`jh::generator<T, U>::iterator`)** for range-based loops when `U == std::monostate`.
- Provides utilities for converting generators to **`std::vector`** and **`std::list`**.
- Designed to simplify the transition from Python’s `yield` mechanism to C++20.
- **Explicitly prohibits copying** to prevent unintended behavior.

**Example Use Case**:
- **Efficient data streaming** without allocating large memory buffers.
- **Range-based sequences** without precomputing all values.
- **Interactive computations** where values can be sent dynamically.

### 🔒 Immutable Strings (`jh::immutable_str`)
- Provides **true immutability** at the memory level.
- Eliminates unintended modifications, **ensuring thread safety**.
- Uses **fixed-size allocation** to prevent unnecessary reallocations.
- Designed for **efficient storage**, **hash-based containers**, and **read-only string processing**.

**Key Advantages over `std::string`**:

| Feature | `jh::immutable_str`                    | `const std::string` |
|---------|----------------------------------------|----------------------|
| **True Immutability** | ✅ Enforced at memory level             | ❌ Can be modified via `const_cast` |
| **Thread Safety** | ✅ No modifications possible            | ❌ Not inherently thread-safe |
| **Memory Efficiency** | ✅ Fixed-size allocation                | ❌ May trigger reallocation |
| **Optimized Hashing** | ✅ Custom hash function for shared_ptrs | ❌ Uses default `std::hash` |

**Use Cases**:
- **Global or shared string storage** where modification should be prevented.
- **Thread-safe immutable data** for multithreaded applications.
- **Efficient use in hash-based containers** (`std::unordered_map`, `std::unordered_set`).

### 📚 Sequence Concept (`jh::sequence`)
- Defines `jh::sequence` as a **C++20 concept** ensuring a type supports **immutable iteration**.
- Provides `sequence_value_type<T>` for extracting element types at compile time.
- Works seamlessly with **STL containers** (`std::vector`, `std::list`, `std::array`) and **custom iterables**.

**Use Cases**:
- **Compile-time validation** of sequence-like types.
- **Generic programming** without relying on template metaprogramming.
- **Safer APIs** that require immutable iterables.

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
- [Sequence Concept (`sequence.h`)](docs/sequence.md)

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
