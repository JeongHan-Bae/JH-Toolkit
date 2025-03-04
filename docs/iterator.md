### **JH Toolkit: Iterator API Documentation**

📌 **Version:** 1.2
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

### **Extracting Iterator Type: `iterator_t<Container>`**

📌 **Description:**  
A **type alias** used to deduce the iterator type of a given container `Container`.  
It is particularly useful for **generic programming and concept-based validation**, ensuring that a container has a **well-defined iterator type**.

#### **Definition:**
```c++
template<typename Container>
using iterator_t = typename iterator<Container>::type;
```

#### **⚠ Important Requirement:**
- **`jh::iterator<Container>::type` MUST be explicitly defined.**
    - If `jh::iterator<Container>::type` is **not** defined, `iterator_t<Container>` **cannot be used**.
    - Containers **must** either define `iterator<Container>` explicitly or provide a valid specialization.
    - This ensures that **all containers using `iterator_t<Container>` have a well-formed iterator type**.

#### **Purpose:**
- Enables **generic iterator type deduction** without explicitly defining `Container::iterator`.
- Works for both **custom containers** (that define `jh::iterator<>`) and **linear structures** (that use `T*` as an iterator).
- Ensures **compatibility with STL-style containers** while allowing **custom iterator implementations**.

#### **Details:**
- The iterator type is **strictly determined** via `jh::iterator<Container>::type`.
    - **Example:** `iterator_t<jh::generator<T>>` resolves to `jh::iterator<jh::generator<T>>::type` (which is `jh::iterator<jh::generator<T>>`).
- `iterator_t<Container>` **may be** `jh::iterator<Container>`, such as for `jh::generator`.
- **For certain linear structures**, `iterator_t<Container>` **may resolve to `T*`**.
    - **Example:** `jh::runtime_arr<T>` (planned for release in v1.3+) uses `T*` as its iterator type.
- **Concept-friendly**, allowing seamless integration in **concept-based constraints**.

#### **Usage Scenarios:**
- **Generic programming**:
    - Helps obtain the iterator type **only if** `jh::iterator<Container>::type` is **explicitly defined**.
    - For continuous linear structures, `T*` can be used as `jh::iterator<Container<T>>::type`.
- **Concept-based programming**:
    - Useful in `concept` constraints for **SFINAE** and **template metaprogramming**.
- **Supports both STL-style containers and custom containers**, provided they are adapted to `jh::iterator<>`.

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