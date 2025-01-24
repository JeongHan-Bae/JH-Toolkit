### **JH Toolkit: Sequence API Documentation**

📌 **Version:** 1.1  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

## **Overview**

The `jh::sequence` module provides a **lightweight C++20 concept** to enforce **immutable iteration**.  
It ensures that a type supports **`begin()` and `end()`** methods, allowing it to be **recognized as a sequence-like
container at compile time**.

### **Key Features**

- **Concept-based validation** (`jh::sequence<T>`) to enforce **immutable iterability**.
- **Compile-time checks** to determine whether a type qualifies as a sequence (`jh::is_sequence<T>`).
- **Value type extraction** (`jh::sequence_value_type<T>`) to retrieve the element type of a sequence.
- **Seamless compatibility** with **STL containers** (`std::vector`, `std::list`, `std::array`, etc.).
- **Lightweight and constexpr-friendly**, with no runtime overhead.

---

## **API Reference**

📌 **Detailed module description can be found in `sequence.h`**  
📌 **Function-specific documentation is embedded in the source code and can be viewed in modern IDEs.**

---

### **Concept: `jh::sequence<T>`**

📌 **Description:**  
A **C++20 concept** that checks whether a type `T` is a **sequence**, meaning it:

- Has a const **`begin()`** method returning an **input iterator**.
- Has a const **`end()`** method returning an **input iterator**.

```c++
template<typename T>
concept sequence = requires(const T t) {
    { t.begin() } -> jh::input_iterator;
    { t.end() } -> jh::input_iterator;
};
```

🔹 **Example Usage**

```cpp
static_assert(jh::sequence<std::vector<int>>, "std::vector<int> should be a sequence");
static_assert(jh::sequence<std::list<double>>, "std::list<double> should be a sequence");
```

---

### **Type Extraction: `jh::sequence_value_type<T>`**

📌 **Description:**  
Extracts the **value type** of a sequence **at compile time**.

```c++
template<typename T>
using sequence_value_type = typename sequence_value_type_impl<T>::type;
```

🔹 **Example Usage**

```c++
using ValueType = jh::sequence_value_type<std::vector<int>>;  // int
static_assert(std::is_same_v<ValueType, int>, "ValueType should be int");
```

---

### **Boolean Check: `jh::is_sequence<T>`**

📌 **Description:**  
A **compile-time boolean flag** to check whether a type satisfies the `jh::sequence<T>` concept.

```c++
template<typename T>
constexpr bool is_sequence = sequence<T>;
```

🔹 **Example Usage**

```c++
static_assert(jh::is_sequence<std::array<int, 5>>, "std::array<int, 5> is a sequence");
static_assert(!jh::is_sequence<int>, "int is not a sequence");
```

---

## **Use Cases**

- **Compile-time validation** of types that should support **immutable iteration**.
- **Generic programming** that needs **type-safe sequences** without requiring concrete container types.
- **Optimized functional programming** in C++20, where compile-time checks ensure **container compatibility**.

---

## **Conclusion**

The `jh::sequence` module provides a **lightweight**, **zero-runtime overhead** concept for **immutable sequences** in
C++20.  
It integrates seamlessly with STL containers and enables **safe, type-checked iteration** in generic code.

📌 **For detailed module information, refer to `sequence.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**