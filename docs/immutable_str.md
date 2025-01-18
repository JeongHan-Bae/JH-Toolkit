### **JH Toolkit: Immutable String API Documentation**
📌 **Version:** 1.0  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

## **Overview**
The `jh::immutable_str` class provides a **truly immutable string** in C++, ensuring that **once created, it cannot be modified**.  
It is specifically designed to address the limitations of `const std::string`, such as:
- **Unintended modifications** via `const_cast`.
- **Reallocation overhead** due to copy-on-write or implicit resizing.
- **Thread safety concerns** in shared string usage.

### **Key Features**
- ✅ **True immutability** at the memory level, preventing unintended modifications.
- ✅ **Efficient storage** using `std::unique_ptr<char[]>`, avoiding unnecessary memory reallocation.
- ✅ **Thread safety** by design, with no modification APIs.
- ✅ **Optimized hashing & comparison**, ideal for `std::unordered_map` and `std::unordered_set`.
- ✅ **Configurable whitespace trimming**, enabled by default (`immutable_str::auto_trim = true`).
- ✅ **C-string, `std::string`, and `std::string_view` compatibility**.

---

## **⚠ Important Reminder: Immutable & No Implicit Copying**
`jh::immutable_str` is designed to be **immutable** and **non-copyable**:
- ❌ **No copy constructor / copy assignment**.
- ❌ **No move constructor / move assignment**.
- ❌ **No modification APIs**.

If **multiple instances** of the same string are needed, use **shared storage** via `jh::atomic_str_ptr`.  
📌 **For efficient shared use, prefer `std::shared_ptr<immutable_str>` over multiple allocations.**

---

## **API Reference**

📌 **Detailed module description can be found in `immutable_str.h`**  
📌 **Function-specific documentation is embedded in the source code and can be viewed in modern IDEs.**

---

### **Class: `jh::immutable_str`**
📌 **Description:**  
A **lightweight immutable string** designed for **safe and efficient storage**.

```c++
struct immutable_str;
```

---

### **Constructor**
#### 📌 `explicit immutable_str(const char *str)`
**Description:**  
Creates an **immutable string** from a **null-terminated C-string**.

🔹 **Parameters**
- `str` → A null-terminated C-string (ownership transferred).
    - If `immutable_str::auto_trim` is `true`, leading and trailing whitespace is **automatically removed**.

🔹 **Example**
```c++
jh::immutable_str imm_str("  Hello World  ");  // Auto-trim enabled
std::cout << imm_str.view();  // Output: "Hello World"
```

---

### **Deleted Copy & Move Operations**
To **enforce immutability**, `immutable_str` **cannot** be copied or moved:

```c++
immutable_str(const immutable_str &) = delete;
immutable_str &operator=(const immutable_str &) = delete;
immutable_str(immutable_str &&) = delete;
immutable_str &operator=(immutable_str &&) = delete;
```

📌 **Use `jh::atomic_str_ptr` (`std::shared_ptr<immutable_str>`) for safe shared usage.**

---

### **Access Methods**
#### 📌 `const char *c_str() const noexcept`
**Description:**  
Returns a **C-string pointer** to the immutable data.

🔹 **Returns**
- `const char *` → The immutable string buffer.

🔹 **Example**
```c++
const char *ptr = imm_str.c_str();
std::cout << ptr;  // Output: "Hello World"
```

---

#### 📌 `std::string str() const`
**Description:**  
Converts the immutable string into a **`std::string`**.

🔹 **Returns**
- `std::string` → A copy of the immutable string.

🔹 **Example**
```c++
std::string standard_str = imm_str.str();
```

---

#### 📌 `std::string_view view() const noexcept`
**Description:**  
Returns a **`std::string_view`** for efficient read-only access.

🔹 **Returns**
- `std::string_view` → A lightweight view of the immutable string.

🔹 **Example**
```c++
std::string_view sv = imm_str.view();
```

---

#### 📌 `uint64_t size() const noexcept`
**Description:**  
Returns the **length** of the immutable string.

🔹 **Returns**
- `uint64_t` → The number of characters in the string.

🔹 **Example**
```c++
uint64_t len = imm_str.size();
std::cout << "String length: " << len;
```

---

### **Comparison & Hashing**
#### 📌 `bool operator==(const immutable_str &other) const noexcept`
**Description:**  
Checks if **two immutable strings are identical**.

🔹 **Parameters**
- `other` → Another `immutable_str` to compare.

🔹 **Returns**
- `true` → If the strings are **identical**.
- `false` → Otherwise.

🔹 **Example**
```c++
jh::immutable_str a("hello");
jh::immutable_str b("hello");
std::cout << (a == b);  // Output: 1 (true)
```

---

### **Global Configuration**
#### 📌 `static inline bool auto_trim = true`
**Description:**  
Controls whether **leading and trailing whitespace** is automatically removed.

🔹 **Values**
- `true` (default) → **Trims whitespace** before storage.
- `false` → **Preserves original string**.

🔹 **Example**
```c++
jh::immutable_str::auto_trim = false;  // Disable trimming
jh::immutable_str imm("  padded  ");
std::cout << imm.view();  // Output: "  padded  "
```

---

## **Shared Storage: `jh::atomic_str_ptr`**
📌 **For efficient shared usage, prefer `jh::atomic_str_ptr` (`std::shared_ptr<immutable_str>`)**  
This prevents unnecessary allocations when multiple copies are needed.

📌 The reason it is called "atomic" is that the pointer itself is mutable, but its assignment is atomic, 
making it suitable for multithreaded shared distribution.

```c++
using atomic_str_ptr = std::shared_ptr<immutable_str>;
```

---

### **Shared String Creation**
#### 📌 `jh::atomic_str_ptr make_atomic(const char *str)`
**Description:**  
Creates a **shared pointer** to an `immutable_str`.

🔹 **Parameters**
- `str` → A null-terminated C-string.

🔹 **Returns**
- `atomic_str_ptr` → A **shared** immutable string.

🔹 **Example**
```c++
jh::atomic_str_ptr shared_str = jh::make_atomic("Shared Example");
```

---

### **Custom Hashing & Equality**
To ensure correct behavior in hash containers:

#### 📌 `struct atomic_str_hash`
Custom **hash function** for `atomic_str_ptr`.

```c++
struct atomic_str_hash {
    std::uint64_t operator()(const atomic_str_ptr &ptr) const noexcept;
};
```

#### 📌 `struct atomic_str_eq`
Custom **equality function** for `atomic_str_ptr`.

```c++
struct atomic_str_eq {
    bool operator()(const atomic_str_ptr &lhs, const atomic_str_ptr &rhs) const noexcept;
};
```

🔹 **Example**
```c++
std::unordered_set<jh::atomic_str_ptr, jh::atomic_str_hash, jh::atomic_str_eq> str_set;
str_set.insert(jh::make_atomic("cached"));
```

---

## **Use Cases**
- **Thread-safe immutable storage**, eliminating unintended modifications.
- **Efficient caching and deduplication**, reducing memory overhead.
- **Fast comparisons and lookups**, leveraging **optimized hashing**.
- **Seamless integration** with **C-strings, `std::string`, and `std::string_view`**.

---

## **Conclusion**
The `jh::immutable_str` class provides a **lightweight**, **thread-safe**, and **truly immutable** string for modern C++20 applications.  
It eliminates the limitations of `std::string` while ensuring **safe, efficient, and flexible string storage**.

📌 **For detailed module information, refer to `immutable_str.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**
