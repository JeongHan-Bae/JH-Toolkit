### **JH Toolkit: Object Pooling API Documentation**

📌 **Version:** 1.2  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **1. Introduction to `jh::pool<T>`**

The `jh::pool<T>` class is an **automatically deduced weak pointer-based object pool** that enables efficient **content-based pooling** of immutable objects.  
It is particularly useful for **deduplicating objects that have the same semantic value**, reducing redundant allocations, and improving cache efficiency.

### **Key Features**
✅ **Automatic Hash and Equality Deduction**
- Uses `T::hash()` instead of `std::hash<T>`, improving efficiency if `T` is immutable.
- Uses `operator==` for **content-based** equality comparison, preventing duplicate storage of equivalent objects.

✅ **Efficient Object Pooling**
- Stores **only one instance of semantically identical objects**, minimizing memory overhead.
- **Automatic expiration management**: Once all shared references are gone, the object expires and will be removed upon the next cleanup cycle.

✅ **Seamless Integration with `sim_pool<T>`**
- `jh::pool<T>` automatically selects **`weak_ptr_hash<T>`** and **`weak_ptr_eq<T>`**, removing the need for manual template specialization.

✅ **Supports Argument-Based Object Construction**
- `T` **must** be constructible with arguments passed to `acquire()`.
- Enables **on-demand instance creation** and pooling via `pool<T>::acquire(Args&&...)`.

---

## **2. When is `jh::pool<T>` Instantiated?**

`jh::pool<T>` is only **instantiated** when `T` meets the following conditions:

1. `T` provides a **`hash()`** function returning `std::uint64_t`.
2. `T` implements **`operator==`** for equality comparison.

**If `T` does not meet these conditions, `jh::pool<T>` will not compile.**

🔹 **Example: Valid Type for Pooling**
```c++
struct ValidObj {
    int value;

    [[nodiscard]] std::uint64_t hash() const noexcept {
        return std::hash<int>{}(value);
    }
    // Return type should be convertible to std::uint64_t

    bool operator==(const ValidObj& other) const {
        return value == other.value;
    }
};

// This will work because ValidObj satisfies hash() and operator==
jh::pool<ValidObj> object_pool;
```

🔹 **Example: Invalid Type (No `hash()` Function)**
```c++
struct InvalidObj {
    int value;

    // No hash function provided
    bool operator==(const InvalidObj& other) const {
        return value == other.value;
    }
};

// ❌ jh::pool<InvalidObj> invalid_pool;
// Compilation Error: T must provide a hash() function
```

---

## **3. Pool Functionality (Inherited from `sim_pool<T>`)**

Since `jh::pool<T>` is a **specialization** of `sim_pool<T>`, all its functionalities come from `sim_pool`.  
📌 **For detailed pool operations, refer to [sim_pool.md](sim_pool.md).**

### **Automatic Expiration and Cleanup**
Unlike traditional object pools, `jh::pool<T>`:
- **Does not require manual object release.**
- **Does not increase reference count** on stored objects.
- Once all **`std::shared_ptr<T>` references are gone**, the weak pointer **automatically expires**.
- The pool will clean up expired objects **automatically when needed** or via a manual `cleanup()`.

🔹 **Example Usage**
```c++
jh::pool<ValidObj> object_pool;

{
    // Creates or retrieves a pooled object
    auto obj1 = object_pool.acquire(42);
    auto obj2 = object_pool.acquire(42);  // Retrieves the same object
    
    std::cout << (obj1 == obj2);  // Output: 1 (true) → Same instance reused
}
// When obj1 and obj2 go out of scope, the object expires
object_pool.cleanup();
std::cout << object_pool.size();  // Output: 0 → Pool is empty
```

For a complete list of features, see [`sim_pool.md`](sim_pool.md).

---

## **4. Modules in JH Toolkit That Use `jh::pool<T>`**

Currently, `jh::pool<T>` is **used by the following modules in JH Toolkit**:

| Module              | Description                                                      | Documentation                             |
|---------------------|------------------------------------------------------------------|-------------------------------------------|
| `jh::immutable_str` | Immutable, thread-safe strings with optional automatic trimming. | 📄 [`immutable_str.md`](immutable_str.md) |

### **Usage in `jh::immutable_str`**
`jh::pool<jh::immutable_str>` is used to **efficiently manage immutable string instances**, preventing redundant allocations.

🔹 **Example: Immutable String Pooling**
```c++
jh::pool<jh::immutable_str> string_pool;

// Automatically pools semantically identical immutable strings
jh::atomic_str_ptr str1 = string_pool.acquire("Pooled String");
jh::atomic_str_ptr str2 = string_pool.acquire("Pooled String");

std::cout << (str1 == str2);  // Output: 1 (true) → Same instance reused
```

---

## **5. Conclusion**

The `jh::pool<T>` class provides a **high-performance, content-aware object pooling system** that:
- Ensures **automatic deduplication** based on content.
- Reduces **memory overhead** by pooling equivalent objects.
- **Requires only `hash()` and `operator==`**, making it **easy to use**.

📌 **For detailed module information, refer to `pool.h`.**  
📌 **For additional pool functionality, see [`sim_pool.md`](sim_pool.md).**

🚀 **Enjoy efficient object management with JH Toolkit!**