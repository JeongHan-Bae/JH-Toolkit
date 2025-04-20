# 📦 JH Toolkit: `pool` Auto Pooling API Documentation

📌 **Version:** 1.3  
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



## 🛡️ Design Philosophy

> **“Construct pessimistically, reuse optimistically.”**

`jh::pool<T>` embraces a design tailored for **immutable, fast-constructible objects** in modern concurrent C++ applications:

### ✅ Thread-Safe by Design

- All internal operations are guarded by `std::shared_mutex`
- **Shared reads** (e.g., `.size()`) use `shared_lock`
- **Exclusive writes** (e.g., `.acquire()`, `.cleanup()`) use `unique_lock`
- Safe for concurrent insertions and lookups across threads

📌 Locking is scoped and minimal — read paths are lightweight, and pool mutation is infrequent for typical usage.

---

### ✅ Friendly to Fast Construction and Pessimistic Pooling

Unlike traditional object caches, `jh::pool<T>` does **not pre-check** for object existence before construction.

This strategy:
- Assumes that **constructing a new object is cheap**
- Optimizes for **low cache latency**
- Eliminates **complex deduction** in multi-arg or templated constructors

> 🧠 **Ideal for**:
> - `immutable_str`
> - Small value objects
> - Config tokens / protocol fields
> - Objects where **reuse is likely, but not guaranteed**


---

## 🔄 Pooling of Non-Fully-Immutable Objects

While `jh::pool<T>` is designed for **content-based deduplication**, it also supports **reusable objects with internal mutable state**, as long as:

- The **equality logic (`operator==`) and `hash()`** are based only on immutable identity fields (e.g., `id`, `pid`, etc.)
- The internal state (e.g., mutexes, queues, init flags) is **used for behavior**, not for identifying equality
- The object’s lifetime is entirely governed by `std::shared_ptr` and the pool **never replaces equivalent entries**
- The pool will **not** replace the object in the pool if it is already present, even if the internal state changes.
### ✅ Atomic Shared Access

When using `pool.acquire(...)`:

- If a logically equivalent object already exists (by `==` and `hash()`), it will be **reused as-is**
- Otherwise, a new instance is **constructed and inserted**
- The returned `std::shared_ptr<T>` guarantees **atomic replaceability** in user-facing code
- Destruction occurs **automatically** when the last `shared_ptr` is released

> ✅ Acquiring a pooled object guarantees atomic shared access — if the same identity exists, it will always return the existing instance. No replacement, no duplication.
> 
> This allows **dynamic reuse** of behavior-capable objects with deterministic identity.

---

### 🌱 Example: `LazyTask`

A task structure with:

- Fixed identity (`id`)
- Cached hash for efficiency
- Deferred `run()` logic
- Internal mutex for synchronization

### 🧩 Template for Pooled Semi-Mutable Objects

The following example illustrates a typical use case where...

```c++
struct LazyTask {
    int id;
    std::atomic_bool constructed = false;
    std::mutex mtx;

    mutable std::optional<std::uint64_t> cached_hash;

    LazyTask(int id_) : id(id_) {}

    std::uint64_t hash() const noexcept {
        if (!cached_hash.has_value())
            cached_hash = std::hash<int>{}(id);
        return *cached_hash;
    }

    bool operator==(const LazyTask& other) const noexcept {
        return id == other.id;
    }

    void ensure_initialized() {
        std::scoped_lock lock(mtx);
        if (!constructed) { // Or use std::init_once if heavy thread-safe init
            heavy_init();  // Costly setup
            constructed = true;
        }
    }

    void run() {
        ensure_initialized();
        // ... perform logic with mtx held
    }

private:
    void heavy_init();
};
```

---

### 🧠 Why This Works Well

- `LazyTask` instances are **pooled by identity (`id`)** — content equality is fast and deterministic.
- Behavioral logic (`run()`) is separated from construction, keeping pool access fast.
- Cached `hash()` avoids recomputation on high-frequency access paths.
- The shared ownership model ensures **safe reuse and proper finalization**, even in concurrent environments.

---

> ✅ **Conclusion**: `jh::pool<T>` is ideal for pooling semi-mutable objects like `LazyTask`, where identity is stable but behavior is dynamic.
>
> This lets you **reuse heavyweight structures** without giving up lifecycle safety or deduplication efficiency.

---


## 🧠 Pooling Philosophy: *Latency Beats Hit Rate*

When using a **thread-safe pool guarded by internal locks**, **speed stability is everything**.

> ❗ A slow miss is worse than a fast failure.

### 🔧 Why `insert()` > `find()`?

- If you **fail to hit**, you **must construct anyway** — meaning the cost is already paid.
- A `find-then-insert` path introduces:
    - **Double memory access**
    - **Branch prediction cost** (`likely` / `unlikely` isn’t free)
    - **Lock duration variance** — more logic inside critical section
- Even when hit rate is high, **checking first then inserting on miss** adds noise.
- In high-throughput systems, this makes **“quick miss → quick insert”** the best default.

### ✅ Summary

- `cache locality > lazy construction`

   → Keeping the access path hot is more important than reducing allocations.  
   CPU caches reward predictable writes more than rare reads.

- **Insert-first pooling** ensures:
    - Fast-path construction
    - Stable lock behavior
    - Avoidance of branch misprediction stalls

> 💡 In JH Toolkit, we value **predictable latency** over speculative reuse.

---

## 🚀 Stabilizing Performance via Immediate Insert + Deferred Init

For high-throughput systems, stable latency is more important than cache hit rate.  
That’s why `jh::pool<T>` always prefers **direct insertion** over speculative lookup.

### 🔧 Why we avoid `find-then-insert`:

- Doubles memory access: hash map lookup *and* insertion
- Creates unpredictable branch behavior
- Increases lock hold time
- Penalizes both fast misses and slow hits

Instead, we **always insert directly**, and let the internal pool hash/deduplication resolve duplication transparently.

---

### 🌱 Best Practice: Deferred Initialization

If your object has **expensive initialization**, follow this pattern:

1. Constructor does *identity-only* work (e.g., assign `id`)
2. Insert immediately into pool with `acquire(...)`
3. Perform expensive logic on first call (e.g., inside `.run()`)

This ensures:

- ✅ Fast, predictable insert path
- ✅ Low lock contention
- ✅ Cache-friendly memory flow
- ✅ Construction cost paid only *when needed*

> 🧠 Deferred construction turns rare misses into cheap hits.

---

### ✅ Summary

| Goal                        | Strategy                            |
|-----------------------------|-------------------------------------|
| Keep insert path fast       | Avoid pre-checks, always insert     |
| Reduce lock variability     | Minimize logic inside critical path |
| Control heavy init behavior | Use lazy `init()` / `run`           |

## ✅ Duck-Typed Template Deduction

Thanks to C++20 concepts, `jh::pool<T>` will only instantiate if `T` satisfies:

- `T::hash()` → returns `std::uint64_t`
- `T::operator==()` → content comparison

You don’t need to write a custom trait — **if it quacks like a duck**, it pools like one.

```c++
template<typename T>
    requires requires(const T& obj) {
        { obj.hash() } -> std::convertible_to<std::uint64_t>;
        { obj == obj } -> std::convertible_to<bool>;
    }
```

This enables seamless integration for user-defined types that behave like “value types” without extra boilerplate.


📌 **For detailed module information, refer to `pool.h`.**  
📌 **For additional pool functionality, see [`sim_pool.md`](sim_pool.md).**

🚀 **Enjoy efficient object management with JH Toolkit!**