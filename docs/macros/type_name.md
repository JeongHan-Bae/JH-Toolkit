# 🔮 **JH Toolkit — `jh::macro::type_name` Reference**

📁 **Header:** `<jh/macros/type_name.h>`  
📦 **Namespace:** `jh::macro`  
📅 **Version:** 1.3.3  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::macro::type_name` provides **compile-time type name extraction**
using compiler-specific metadata such as `__PRETTY_FUNCTION__` (Clang/GCC).  
It enables **human-readable, unmangled type identification** without requiring RTTI.  

Unlike demangling or reflection utilities,
`type_name<T>()` evaluates entirely at compile time and returns
a `std::string_view` representing the template parameter name of `T`.  

This is primarily used for **debugging**, **logging**, and **diagnostics**,
not for serialization or ABI-level logic.

---

## 🔹 Interface

| Symbol                      | Type                         | Description                                                          |
|-----------------------------|------------------------------|----------------------------------------------------------------------|
| `jh::macro::type_name<T>()` | `constexpr std::string_view` | Returns a readable type name for `T`, or `"unknown"` if unsupported. |

---

## 🔹 Example Usage

```cpp
#include <jh/macros/type_name.h>
#include <iostream>

int main() {
    std::cout << jh::macro::type_name<int>() << '\n';
    // → "int"

    std::cout << jh::macro::type_name<jh::pod::array<int, 4>>() << '\n';
    // → "jh::pod::array<int, 4>"
}
```

---

## 🔹 Behavior and Guarantees

| Property                    | Description                                                |
|-----------------------------|------------------------------------------------------------|
| **Compile-time evaluation** | Fully constexpr; no runtime demangling.                    |
| **Compiler dependency**     | Works on GCC and Clang using `__PRETTY_FUNCTION__`.        |
| **Fallback**                | Returns `"unknown"` for unsupported compilers.             |
| **Safety**                  | Does not require RTTI (`-fno-rtti` safe).                  |
| **Format stability**        | Not guaranteed — output differs between compiler versions. |

---

## ⚠️ Limitations and Warnings

* ❌ Not suitable for serialization, hashing, or ABI-level decisions.
* ⚙️ Intended for **developer diagnostics only** (debug, logging, assertions).
* 🧪 Output format is **compiler- and version-specific**:

    * Clang → `std::string_view type_name() [T = int]`
    * GCC → `constexpr std::string_view type_name() [with T = int]`
* 🛑 Any change in compiler version may alter string layout.

---

## 🧩 Implementation Notes

`type_name()` relies on **compile-time string slicing** within `__PRETTY_FUNCTION__`:

```cpp
constexpr std::string_view func = __PRETTY_FUNCTION__;
constexpr std::string_view key  = "T = ";
const auto start = func.find(key);
const auto from  = start + key.size();
const auto end   = func.rfind(']');
return func.substr(from, end - from);
```

This approach guarantees:

* No runtime allocations
* No I/O or reflection
* Purely constexpr behavior
* 100% header-only, side-effect-free

---

## 🧠 Summary

| Aspect              | Description                             |
|---------------------|-----------------------------------------|
| Category            | Debug macro utility                     |
| Kind                | Compile-time diagnostic helper          |
| Namespace           | `jh::macro`                             |
| Supported Compilers | Clang, GCC                              |
| RTTI Dependency     | None                                    |
| ABI Impact          | None                                    |
| Typical Use         | Logging, static assertions, debug info  |
| Not for             | Serialization, reflection, or ABI logic |

---

> **Design Philosophy**  
> `jh::macro::type_name` is a **diagnostic-only macro helper** —  
> a way to gain visibility into type structures during compilation,
> without RTTI, demangling, or reflection overhead.  
>
> It is deliberately **unstable and compiler-specific**,
> but provides a safe and constexpr-compatible way to identify template types
> across Clang and GCC toolchains.
