### **JH Toolkit: Immutable String API Documentation**

📌 **Version:** 1.2  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

## **Overview**

The `jh::immutable_str` class provides a **truly immutable string** in C++, ensuring that **once created, it cannot be
modified**.  
It is specifically designed to address the limitations of `const std::string`, such as:

- **Unintended modifications** via `const_cast`.
- **Reallocation overhead** due to copy-on-write or implicit resizing.
- **Thread safety concerns** in shared string usage.

### **Why `final`?**

The `jh::immutable_str` class is marked as `final` because **inheritance is unnecessary and counterproductive** for an
immutable string. The reasons are:

1. **True Immutability Must Be Enforced**

- Allowing subclassing could introduce unintended behaviors that break immutability, such as overriding methods to
  expose internal modifications.
- Declaring the class `final` ensures that **once created, the string remains unchanged at both the API and memory
  levels**.

2. **Memory Safety and Performance**

- `immutable_str` is optimized for **minimal allocation and direct storage**.
- Inheritance could introduce **virtual dispatch overhead**, which is unnecessary for a **fixed, immutable structure**.

3. **No Need for Customization**

- Unlike `std::string`, `immutable_str` is **not meant to be extended**—its entire purpose is to remain **immutable and
  efficient**.
- Any desired modifications should be handled through **composition** rather than inheritance.

4. **Ensuring Consistent Hashing and Equality**

- Since `immutable_str` is designed for **efficient comparisons and hashing**, subclassing could lead to **unexpected
  behavior** when storing instances in hash-based containers.
- Making it `final` ensures **consistent, predictable hashing and comparison behavior**.

### **Key Features**

- ✅ **True immutability** at the memory level, preventing unintended modifications.
- ✅ **Memory Efficient**: Uses `std::unique_ptr<const char[]>` for compact storage.
- ✅ **Thread safety** by design, with no modification APIs.
- ✅ **Optimized hashing & comparison**, ideal for `std::unordered_map` and `std::unordered_set`.
- ✅ **Configurable whitespace trimming**, enabled by default (`immutable_str::auto_trim = true`).
- ✅ **Seamless C-string Compatibility**: Provides `c_str()`[const char*], `view()`[std::string_view()],
  and `str()`[std::string].
- ✅ **Pooling Support**: Compatible with `jh::pool` for efficient object pooling.

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

#### 📌 `static inline std::atomic<bool> auto_trim = true`

**Description:**  
A **global flag** that controls whether **leading and trailing whitespace** should be automatically removed **before storing the string**.  
**Trimming occurs only during initialization**—once an `immutable_str` is created, **its content never changes, regardless of `auto_trim`'s value**.

🔹 **Values**
- `true` (default) → **Trims** whitespace before storing.
- `false` → **Preserves** the original string **without modification**.

🔹 **Important Notes**
- **Trimming is performed only once, during initialization.**
- **Once an `immutable_str` instance is created, changing `auto_trim` has no effect on existing instances.**
- **⚠ Modifying this setting at runtime is strongly discouraged**, as it may cause **inconsistent behavior across different parts of the program**.
- **The recommended approach** is to **set `auto_trim` correctly before creating any `immutable_str` instances** to avoid unexpected behaviors.

🔹 **Example Usage**
```c++
jh::immutable_str::auto_trim = false;  // Define behavior globally before creating instances

jh::immutable_str str1("   padded   ");
std::cout << str1.view();  // Output: "   padded   " (no trimming)

jh::immutable_str::auto_trim = true;   // Change setting (NOT recommended at runtime)

jh::immutable_str str2("   trimmed   ");
std::cout << str2.view();  // Output: "trimmed"

std::cout << str1.view();  // Still "   padded   ", because existing instances remain unchanged
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

### **Shared String Creation with Mutex Protection**

#### 📌 `jh::atomic_str_ptr safe_from(std::string_view sv, std::mutex &mtx)`

**Description:**  
Creates a **shared pointer** to an `immutable_str` from a `std::string_view`, ensuring safe access.

🔹 **Parameters**
- `sv` → A `std::string_view` representing the string data.
- `mtx` → A reference to the `std::mutex` that protects the lifetime of `sv`.

🔹 **Throws**
- `std::logic_error` → If `sv` contains embedded null (`\0`) characters.

🔹 **Warning**
- Any **implicitly convertible type** to `std::string_view` (e.g., `std::string`, `char[]`) **is allowed**.
- The caller **must ensure** that `mtx` correctly protects the lifetime of the base string (e.g., `std::string`).
- If the base string **changes** during `immutable_str` initialization, **undefined behavior may occur**.

🔹 **Warning**
- The caller **must ensure** that `mtx` correctly protects `sv`. If an unrelated mutex is provided, **undefined behavior may occur**.

🔹 **Returns**
- `atomic_str_ptr` → A **shared** immutable string.

🔹 **Example**
```c++
std::mutex mtx;
std::string shared_data = "Thread-safe string"; // Implicitly convertible to string_view
// This is valid as long as `mtx` ensures `shared_data` does not change before immutable_str is created.
jh::atomic_str_ptr safe_str = jh::safe_from(shared_data, mtx);
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

### **Pooling Support**

📌 **`immutable_str` is compatible with `jh::pool` for efficient object pooling.**  
📌 **For detailed usage, refer to **[pool.md](pool.md)**.**

🔹 **Header Inclusion**
- **No need to include `<jh/pool.h>` separately** if you are only using `jh::pool<immutable_str>`,  
  since `immutable_str.h` already includes `pool.h`.

🔹 **Example Usage**
```c++
jh::pool<jh::immutable_str> string_pool;  // No need to include <jh/pool.h> separately

jh::atomic_str_ptr pooled_str = string_pool.acquire("Pooled String");
std::cout << pooled_str->view();  // Output: "Pooled String"
```

---

## **Use Cases**

- **Thread-safe immutable storage**, eliminating unintended modifications.
- **Efficient caching and deduplication**, reducing memory overhead.
- **Fast comparisons and lookups**, leveraging **optimized hashing**.
- **Seamless integration** with **C-strings, `std::string`, and `std::string_view`**.

---

## **Conclusion**

The `jh::immutable_str` class provides a **lightweight**, **thread-safe**, and **truly immutable** string for modern
C++20 applications.  
It eliminates the limitations of `std::string` while ensuring **safe, efficient, and flexible string storage**.

📌 **For detailed module information, refer to `immutable_str.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**
