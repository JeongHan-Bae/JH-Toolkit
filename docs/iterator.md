# 🧩 JH Toolkit: `iterator` Concepts API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **Overview**

The `jh::iterator` module provides **concepts and forward declarations** for **iterator validation and customization**.  
It ensures **compatibility with both `std::` iterators and self-defined iterators**, allowing seamless integration with
STL algorithms and custom container types.

### **Key Features**

- ✅ **Concept-based iterator validation** (`input_iterator`, `output_iterator`, etc.)
- ✅ **Supports both `std::iterator_traits<T>` and custom-defined iterators**
- ✅ **Smart iterator type deduction** via `iterator_t<T>`:
  - Explicit `jh::iterator<T>::type` (preferred for faster compilation)
  - `.begin()` method fallback
  - Pointer types (`T*`) fallback
- ✅ **Forward declaration of `iterator<T>`** to support modular container interfaces
- ✅ **Compile-time validation** with zero runtime overhead

> 💡 The use of `jh::iterator_t<T>` and iterator concepts provides a **non-intrusive, duck-type-friendly mechanism**  
> for validating iterators — especially useful for custom types.

---

## ⚠️ Concept-Based Iterator Validation

The `jh::iterator` module emphasizes **compile-time safety** using modern C++20+ concepts:

- ⏱ **Zero runtime cost** — everything is resolved at compile time.
- 🧩 **STL-compatible**, while extendable to user-defined types.
- 🛡️ **Catches invalid iterator usage early**, during compilation.
- 🦆 **Duck typing friendly** — no need for explicit base classes or traits.

---

## 🔧 Forward Declaration: `iterator<T>`

📌 **Purpose:**  
Allows modular container design by declaring the iterator type without including the full definition.

```c++
template<typename T>
struct iterator;  // Forward declaration
```

Used in containers like `jh::generator<T>`, where `iterator<T>` is specialized elsewhere.

---

## 🧰 Deducing Iterator Type: `iterator_t<Container>`

📌 **Description:**  
A **type alias** that deduces the iterator type for a given container `Container`, using:

1. An explicit `jh::iterator<Container>::type` (**preferred** — for clarity and faster compilation)
2. A valid `.begin()` member function
3. Fallback: Pointer types (`T*` only)

### ✅ Deduction Logic

```c++
template<typename Container>
using iterator_t = typename detail::iterator_resolver<Container>::type;
```

### 💡 Deduction Priority

| Priority | Mechanism                       | Example                                    |
|----------|---------------------------------|--------------------------------------------|
| 1️⃣      | `jh::iterator<Container>::type` | `jh::generator<T>`, `jh::runtime_arr<T>`   |
| 2️⃣      | `.begin()` method (e.g. STL)    | `jh::pod::array<T, N>`, `std::array<T, N>` |
| 3️⃣      | Raw pointer types (`T*`) only   | `int*`, `float[]`, etc.                    |

### ❗ Clarification

> 🛑 **Do not confuse** containers like `jh::runtime_arr<T>` with raw pointer types:
> - `jh::runtime_arr<T>` **is not** a pointer, even though its iterator is `T*`
> - It explicitly defines `jh::iterator<Container>::type` and provides `.begin()`
> - So it falls under **priority 1**, not the pointer fallback case

### 📎 Notes

- **Explicit specialization of `jh::iterator<T>` is optional**, but helps **accelerate compilation**.
- Fallback deduction allows **plug-and-play compatibility** with STL and third-party containers.
- Ideal for use in **template metaprogramming** and **non-intrusive** validation systems.

---

## 🧠 Concepts for Iterator Validation

| Concept                     | Description                                               |
|-----------------------------|-----------------------------------------------------------|
| `input_iterator<I>`         | Can be read from: dereference, increment, comparison      |
| `output_iterator<I, T>`     | Can be written to: assignable, incrementable              |
| `forward_iterator<I>`       | Stable, re-readable: input + consistent references        |
| `bidirectional_iterator<I>` | Forward + supports `--it` and `it--`                      |
| `random_access_iterator<I>` | Bidirectional + pointer-like arithmetic (`+`, `-`, `[n]`) |
| `is_iterator<T>`            | General concept for validating iterator-like behavior     |

### ✅ Summary of Requirements

| Concept                     | Requirements                                                          |
|-----------------------------|-----------------------------------------------------------------------|
| `input_iterator<I>`         | `*it`, `++it`, `it++`, `==`, `!=`                                     |
| `output_iterator<I, T>`     | `*it = value`, `++it`, `it++`                                         |
| `forward_iterator<I>`       | All input requirements + stable deref (`*it`)                         |
| `bidirectional_iterator<I>` | All forward requirements + `--it`, `it--`                             |
| `random_access_iterator<I>` | All bidirectional + arithmetic ops (`+`, `-`, `[n]`, `<`, `<=`, etc.) |
| `is_iterator<T>`            | `*it`, `++it`, `it++`, valid `value_type` in `std::iterator_traits`   |

---

## 💡 Use Cases

- 📦 **Container libraries** requiring flexible iterator declaration
- 🧪 **Generic algorithms** ensuring safe iterator operations
- 🧬 **Concept-based SFINAE / constraint checking** in templates
- 🦾 Seamlessly supports both `std::` containers and user-defined types
- 🧠 Encourages **non-intrusive duck typing** — no need to inherit or specialize unnecessarily

---

## ✅ Conclusion

The `jh::iterator` module provides a powerful yet lightweight abstraction for **iterator type deduction and validation**, featuring:

- 🔍 Smart fallback resolution (`type` → `.begin()` → pointer)
- 📏 Rich concepts for compile-time correctness
- 🚀 Minimal coupling — works out-of-the-box with STL and custom types
- 🧩 Friendly to **duck-typed**, **non-intrusive**, and **modern C++** container designs

📄 See `iterator.h` for complete implementation details.  
🛠️ Ready to level up your template and iterator game with **zero runtime cost**.
