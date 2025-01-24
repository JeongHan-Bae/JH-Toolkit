### **JH Toolkit: Iterator API Documentation**

📌 **Version:** 1.1
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **Overview**

The `jh::iterator` module provides **concepts and forward declarations** for **iterator validation and customization**.  
It ensures **compatibility with both `std::` iterators and self-defined iterators**, allowing seamless integration with
STL algorithms and custom container types.

### **Key Features**

- **Concept-based iterator validation** (`input_iterator`, `output_iterator`, etc.).
- **Supports both standard (`std::iterator_traits<T>`) and self-defined iterators.**
- **Forward declaration of `iterator<T>`** to allow its use in generator-based containers.
- **Ensures compile-time validation of iterators** for safer generic programming.

---

## **⚠ Important Reminder: Concept-Based Iterator Validation**

The `jh::iterator` module is designed for **concept-based validation**, which ensures:

- **Compile-time safety**, preventing invalid iterators from being used.
- **Zero runtime overhead**, as all checks happen at compile time.
- **Seamless STL compatibility**, supporting standard iterators.

---

## **API Reference**

📌 **Detailed module description can be found in `iterator.h`**  
📌 **Function-specific documentation is embedded in the source code and can be viewed in modern IDEs.**

---

### **Forward Declaration: `iterator<T>`**

📌 **Description:**  
A **forward declaration** of the `iterator<>` template for use in `generator<>` and other containers.

#### **Purpose:**

- Allows `generator<>` and container types to **reference `iterator<>`** without including the full definition.
- The actual implementation of `iterator<>` is **specialized elsewhere** for different types.

---

### **Concepts for Iterator Validation**

| **Concept**                 | **Description**                                                                         |
|-----------------------------|-----------------------------------------------------------------------------------------|
| `input_iterator<I>`         | Checks if a type `I` satisfies **input iterator** requirements.                         |
| `output_iterator<I, T>`     | Checks if a type `I` satisfies **output iterator** requirements for values of type `T`. |
| `forward_iterator<I>`       | Checks if a type `I` satisfies **forward iterator** requirements.                       |
| `bidirectional_iterator<I>` | Checks if a type `I` satisfies **bidirectional iterator** requirements.                 |
| `random_access_iterator<I>` | Checks if a type `I` satisfies **random-access iterator** requirements.                 |
| `is_iterator<T>`            | Checks if a type `T` behaves like an **iterator**.                                      |

---

### **Iterator Concept Requirements**

| **Concept**                     | **Requirements**                                                                                                                                                                    |
|---------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **`input_iterator<I>`**         | ✅ Supports dereferencing (`*iter`).<br> ✅ Supports increment (`++iter`, `iter++`).<br> ✅ Supports comparison (`==`, `!=`).                                                          |
| **`output_iterator<I, T>`**     | ✅ Supports assignment (`*iter = value`).<br> ✅ Supports increment (`++iter`, `iter++`).                                                                                             |
| **`forward_iterator<I>`**       | ✅ Meets all **input iterator** requirements.<br> ✅ Supports post-increment (`it++`).<br> ✅ Provides stable references (`*it`).                                                      |
| **`bidirectional_iterator<I>`** | ✅ Meets all **forward iterator** requirements.<br> ✅ Supports decrement (`--it`, `it--`).                                                                                           |
| **`random_access_iterator<I>`** | ✅ Meets all **bidirectional iterator** requirements.<br> ✅ Supports arithmetic (`it + n`, `it - n`).<br> ✅ Supports indexing (`it[n]`).<br> ✅ Supports comparison (`<, >, <=, >=`). |
| **`is_iterator<T>`**            | ✅ Has a valid `value_type` in `std::iterator_traits<T>`.<br> ✅ Supports increment (`++it`, `it++`).<br> ✅ Supports dereferencing (`*it`).                                           |

---

## **Use Cases**

- **Compile-time validation of iterators** to ensure **safe and optimized iteration**.
- **Custom iterator development**, enforcing compliance with STL-style iterator traits.
- **Generic programming**, allowing templates to enforce **correct iterator behavior**.
- **Zero-cost abstraction**, ensuring **no runtime overhead** for iterator checks.

---

## **Conclusion**

The `jh::iterator` module provides **concept-based iterator validation**, enabling **safe and efficient iteration** in *
*generic algorithms and containers**.  
It ensures compatibility with both **standard (`std::`) iterators and self-defined iterators**, preventing **runtime
errors** by catching **invalid iterator usage at compile time**.

📌 **For detailed module information, refer to `iterator.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**