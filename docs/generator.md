### **JH Toolkit: Generator API Documentation**
📌 **Version:** 1.0  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

## **Overview**
The `jh::generator<T, U>` class provides a **coroutine-based generator** inspired by Python's `yield` mechanism.  
It allows both **iterative** and **interactive** coroutine-based generators, enabling **lazy evaluation** and **on-the-fly computations** in C++20.

### **Key Features**
- **Coroutine-powered iteration** (`co_yield`) for **lazy sequences**.
- **Interactive sending** (`send()` and `send_ite()`) to modify execution dynamically.
- **Automatic conversion** to `std::vector` and `std::list`.
- **Exception-safe coroutine management**.

---

## **⚠ Important Reminder: No Serialization, Comparison, or Internal Mutex**
`jh::generator<T, U>` is designed to be a **lightweight**, **performance-oriented** coroutine object.  
It **does not** support:
- **Serialization or deserialization**: Generators depend on **coroutine state**, which is not serializable.
- **Comparison operators (`operator==`, etc.)**: Comparing coroutine execution states is **meaningless**.
- **Built-in mutex mechanisms**: If you need **thread safety**, you **must manage your own mutex externally**.

**Why no built-in mutex?**
- Generators are **designed for single-threaded lazy evaluation**.
- Adding an internal mutex would introduce **unnecessary performance overhead**.
- Just like most STL containers, if cross-thread usage is needed, **external synchronization is required**.

📌 **If you require a thread-safe coroutine structure, you should build a wrapper that explicitly manages concurrency.**

---

## **API Reference**

📌 **Detailed module description can be found in `generator.h`**  
📌 **Function-specific documentation is embedded in the source code and can be viewed in modern IDEs.**

---

### **Class: `jh::generator<T, U>`**
📌 **Description:**  
A coroutine-based generator that supports both **yielding values** and **receiving inputs**.

```cpp
template<typename T, typename U = std::monostate>
struct generator;
```

#### **Template Parameters**
- **`T`** - The type of values the generator produces.
- **`U`** *(default: `std::monostate`)* - The type of values that can be **sent** into the generator.

---

### **Constructor**
#### 📌 `generator(std::coroutine_handle<promise_type> h)`
**Description:**  
Creates a `generator` object from an existing **coroutine handle**.

🔹 **Parameters**
- `h` - A coroutine handle that represents the execution context.

---

### **Destructor**
#### 📌 `~generator()`
**Description:**  
Destroys the coroutine handle if it exists.

🔹 **Behavior**
- Ensures that any active coroutine is properly destroyed.
- If the generator is exhausted, calling this destructor is a **no-op**.

---

### **Iteration Methods**
#### 📌 `bool next()`
**Description:**  
Advances the generator **to the next value**.

🔹 **Returns**
- `true` → If a new value is available.
- `false` → If the generator is **exhausted**.

---

### **Interactive Methods**
#### 📌 `bool send(U value)`
**Description:**  
Sends a value into the generator and **resumes execution**.

🔹 **Parameters**
- `value` → The input value sent to the generator.

🔹 **Returns**
- `true` → If the coroutine is still active.
- `false` → If the generator has **finished execution**.

---

#### 📌 `bool send_ite(U value)`
**Description:**  
Combines `next()` and `send()`, advancing the generator **and sending a value in one step**.

🔹 **Parameters**
- `value` → The input value sent to the generator.

🔹 **Returns**
- `true` → If the generator successfully advanced and accepted the value.
- `false` → If the generator has **finished execution**.

---

### **Value Retrieval**
#### 📌 `std::optional<T> value()`
**Description:**  
Retrieves the **current value** yielded by the generator.

🔹 **Returns**
- `std::optional<T>` → The latest yielded value (or `nullopt` if none).

---

#### 📌 `std::optional<U> last_sent_value()`
**Description:**  
Retrieves the **last sent value** (if any).

🔹 **Returns**
- `std::optional<U>` → The most recent value sent into the generator.

---

### **Control Methods**
#### 📌 `void stop()`
**Description:**  
Stops execution and **destroys the coroutine**.

---

## **Conversion Functions**
### 📌 `jh::generator<T> make_generator(const T& seq)`
**Description:**  
Converts a sequence into a generator.

---

### **Conversion to Containers**
#### 📌 `std::vector<T> to_vector(generator<T>& gen)`
**Description:**  
Converts the generator into a `std::vector<T>`.

---

#### 📌 `std::list<T> to_list(generator<T>& gen)`
**Description:**  
Converts the generator into a `std::list<T>`.

---

## **Use Cases**
- **Lazy Iteration**: Efficiently generate sequences without precomputing them.
- **Interactive Processing**: Send values dynamically to modify computation.
- **Stream Processing**: Work with large datasets without loading them into memory.

---

## **Conclusion**
The `jh::generator<T, U>` class provides an **efficient**, **type-safe**, and **flexible** coroutine-based generator for **C++20**, bringing **Python-style iteration and interaction** to C++.

📌 **For detailed module information, refer to `generator.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**
