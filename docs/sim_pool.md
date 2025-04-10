# 📦 JH Toolkit: `sim_pool` Simple Object Pool API Documentation

📌 **Version:** 1.2  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **1. Introduction to `jh::sim_pool<T, Hash, Eq>`**

The `jh::sim_pool<T, Hash, Eq>` class is a **generic weak pointer-based object pool** designed for **efficient, thread-safe, and content-aware** object pooling.  
It is particularly useful for scenarios where objects are frequently created and destroyed, ensuring **deduplication and automatic cleanup** of expired instances.

### **Key Features**
✅ **Automatic Object Pooling**
- Prevents redundant `std::shared_ptr<T>` allocations by **storing only one instance of semantically identical objects**.

✅ **Dynamic Memory Management**
- **Expands dynamically** when full and **shrinks automatically** when underutilized.
- Uses `std::atomic<std::uint64_t>` for **efficient, lock-free size tracking**.

✅ **Thread-Safe Implementation**
- Uses `std::shared_mutex` for **safe concurrent access**.
- `acquire()` and `cleanup()`(`cleanup_shrink()`) operations are **lock-protected**.

✅ **Custom Hashing and Equality Support**
- **Uses content-based hashing** (`Hash`) and equality (`Eq`) instead of pointer-based lookup.
- Allows **custom pool policies** for managing objects based on their content.

✅ **Automatic Expiration Handling**
- Once all shared references are gone, the object **expires and will be removed upon the next cleanup cycle**.

---

## **2. When Should You Use `sim_pool<T, Hash, Eq>`?**

Use `sim_pool` when you need:
- **A reusable object pool** that avoids unnecessary heap allocations.
- **Content-based object deduplication**, storing only **one instance** per unique value.
- **Automatic cleanup** of expired objects **without manual tracking**.

🚀 **Recommended Usage**
- **Use `sim_pool<T>` when there is significant object reuse**.
    - If objects are rarely reused, **the pool's internal management overhead may outweigh its benefits**.
- **Avoid excessive clearing (`clear()`)** unless switching workloads.
    - The pool **already performs automatic cleanup**. Frequent manual clearing may lead to unnecessary re-tracking.
- **If objects do not frequently expire**, the pool may retain outdated weak pointers—periodic `cleanup()` calls can optimize memory usage.

📌 **For automatic pooling of types with `hash()` and `operator==`, use `jh::pool<T>` instead.**  
📌 **See [`pool.md`](pool.md) for details on `jh::pool<T>`.**

---

## **3. API Reference**

📌 **For additional details, refer to `sim_pool.h`**.

### **Class: `jh::sim_pool<T, Hash, Eq>`**

```c++
template<typename T, typename Hash, typename Eq>
struct sim_pool;
```
**Note**: Hash and Eq should never use std::hash and std::equal_to<>, as these only compare the raw pointer, which means comparing the address instead of the content.

---

### **Constructor**

#### 📌 `explicit sim_pool(std::uint64_t reserve_size = 16)`

**Description:**  
Creates a `sim_pool` with an **initial reserved size** (default: `16`).

🔹 **Parameters**
- `reserve_size` → The initial number of elements the pool reserves.

🔹 **Example**
```c++
struct MyObj{
    int value;
    MyObj(int value) : value(value) {}
}; // Suppose MyObj has a constructor that takes an int and details are omitted

struct MyObjHash {
    std::size_t operator()(const std::weak_ptr<MyObj> &ptr) const noexcept;
    // Declaration not implemented in example, but should be content-based
    // Return type should be convertible to std::uint64_t
};

// 🎯 Custom Equality Function (Expired weak_ptrs should be considered different)
struct MyObjEq {
    bool operator()(const std::weak_ptr<MyObj> &lhs, const std::weak_ptr<MyObj> &rhs) const noexcept;
    // Declaration not implemented in example, but should be content-based
    // Return type should be convertible to bool
};
jh::sim_pool<MyObj, MyObjHash, MyObjEq> pool(32); // Start with 32 reserved slots
```

---

### **Object Retrieval & Pooling**

#### 📌 `std::shared_ptr<T> acquire(Args&&... args)`

**Description:**  
Retrieves an object from the pool, or **creates a new one** if none exists.

🔹 **Returns**
- `std::shared_ptr<T>` → A **shared** instance of `T`, either newly created or retrieved from the pool.

🔹 **Example**
```c++
jh::sim_pool<MyObj, MyObjHash, MyObjEq> pool;
auto obj1 = pool.acquire(10);
auto obj2 = pool.acquire(10); // Retrieves the same object
auto obj3 = pool.acquire(20); // Different object

std::cout << obj1 == obj2; // ✅ Output: 1 (true) - Same object
std::cout << obj1 == obj3; // ✅ Output: 0 (false) - Different object
std::cout << pool.size();  // ✅ Output: 2 - Two objects in the pool
```

---

### **Automatic Expiration Handling**

#### 📌 `void cleanup()` & `void cleanup_shrink()`

**Description:**  
Removes all expired `weak_ptr` references from the pool.

🔹 **Example**
```c++
jh::sim_pool<MyObj, MyObjHash, MyObjEq> pool;
{
    auto obj = pool.acquire(100);
} // obj goes out of scope

pool.cleanup();  // Removes expired objects (or using pool.cleanup_shrink() to shrink the pool if necessary)
std::cout << pool.size();  // ✅ Output: 0 - Object has been removed
```

**Note** : `cleanup()` is **automatically called** by `expand_and_cleanup()` when the pool reaches capacity.

---

### **Pool Size and Reserved Capacity**

#### 📌 `std::uint64_t size() const`

**Description:**  
Returns the **current number of stored weak_ptrs**, **including expired ones**.
- If accurate size is needed, **call `cleanup()` first**.

Examples are already shown in the previous sections.

---

#### 📌 `std::uint64_t reserved_size() const`

**Description:**  
Returns the **current reserved size** before expansion or contraction.

🔹 **Example**
```c++
std::cout << pool.reserved_size();  // Check current pool capacity
```

---

### **Manual Clearing of Pool**

#### 📌 `void clear()`

**Description:**  
Removes **all tracked objects** from the pool, but **does not destroy objects that are still referenced elsewhere**.
- Objects that **still have active `std::shared_ptr` references** will **remain alive** until their last reference disappears.
- This operation **only removes weak pointers from the pool's internal tracking**, allowing them to be **re-inserted if acquired again**.

🔹 **Example**
```c++
auto obj1 = pool.acquire(10);
auto obj2 = pool.acquire(20);

pool.clear();  // Pool no longer tracks obj1 and obj2

REQUIRE(pool.size() == 0); // ✅ Pool is empty but objects exist if referenced elsewhere
// obj1 and obj2 are still alive as they are in the scope and not reset
```

---

## **4. Pool Expansion & Cleanup Mechanism**

### **How `expand_and_cleanup()` Works**
- If the **pool reaches capacity**, it **doubles** (`*2`) the reserved size.
- If usage **drops below 25%**, it **shrinks** (`/2`) but **never below `16`**.
- **Expired objects are always removed** during expansion/shrink cycles.

🔹 **Example**
```c++
jh::sim_pool<MyObj, MyObjHash, MyObjEq> pool(16); // Start with 16 reserved slots
for (int i = 0; i < 20; ++i) {
    pool.acquire(i); // Acquired objects are not stored so they will expire immediately
}
std::cout << pool.reserved_size() <= 16; 
// ✅ Output: 1 (true) - When the size reaches 16, the next acquire will triger `expand_and_cleanup()` 
// so the expired objects will be removed so the reserved size will NOT be doubled

pool.clear(); // Clear the pool

std::vector<std::shared_ptr<MyObj>> objs;
for (int i = 0; i < 20; ++i) {
    objs.push_back(pool.acquire(i)); // Acquired objects are stored so they will not expire immediately
}
std::cout << pool.reserved_size() > 16; // ✅ Output: 1 (true) - The pool will expand to store all the objects

std::cout << pool.size(); // ✅ Output: 20 - All objects are stored in the pool

objs.clear(); // All objects are recycled so the pool-instances will expire
pool.cleanup(); // Cleanup the pool
std::cout << pool.size(); // ✅ Output: 0 - All objects are removed
```

---

## **5. Modules in JH Toolkit That Use `sim_pool<T, Hash, Eq>`**

Currently, `sim_pool<T>` is **used by the following modules**:

| Module        | Description                                                 | Documentation           |
|---------------|-------------------------------------------------------------|-------------------------|
| `jh::pool<T>` | A specialized version of `sim_pool` with automatic hashing. | 📄 [`pool.md`](pool.md) |

---

## **6. Conclusion**

The `jh::sim_pool<T, Hash, Eq>` class provides:
- **Efficient, thread-safe content-aware object pooling**
- **Automatic expiration cleanup** with minimal overhead
- **Dynamic expansion and shrinking** based on usage
- **Full customization** via user-defined hash and equality functions

📌 **For standard automatic pooling, see [`pool.md`](pool.md).**

🚀 **Enjoy efficient object management with JH Toolkit!**