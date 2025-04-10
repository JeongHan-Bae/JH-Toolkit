# 🔁 JH Toolkit: `generator` API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

## **Overview**

The `jh::generator<T, U>` class provides a **coroutine-based generator** inspired by Python's `yield` mechanism.  
It allows both **iterative** and **interactive** coroutine-based generators, enabling **lazy evaluation** and *
*on-the-fly computations** in C++20.

### **Why `final`?**

The `generator` class is marked as `final` because **inheritance is unnecessary and counterproductive** in this case:

1. **Coroutines are not designed for polymorphism** – Unlike traditional class hierarchies, coroutine-based objects rely
   on **co_await/co_yield mechanisms**, making virtual function overrides impractical.
2. **Generator semantics do not require extension** – The generator's behavior is **self-contained** and does not
   benefit from subclassing.
3. **Performance considerations** – Avoiding virtual dispatch ensures **zero-cost abstractions**, keeping coroutine
   execution efficient.

### **Key Features**

- **Coroutine-powered iteration** (`co_yield`) for **lazy sequences**.
- **Interactive sending** (`send()` and `send_ite()`) to modify execution dynamically.
- **Automatic conversion** to `std::vector` and `std::list`.
- **Exception-safe coroutine management**, preventing leaks and undefined behavior.
- **Explicit iterator support**, allowing both **manual iteration** and **range-based loops**. (`begin()` and `end()`
  only available when `U == std::monostate`)

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

```c++
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

### **Iterator Methods**

#### 📌 `iterator begin()`

**Description:**  
Returns an **iterator** to the beginning of the generator sequence, enabling **range-based for loops**.

**Requirements**

- Available **only when `U == std::monostate`** (i.e., no `send()` required).
- The generator must be **mutable** (`begin() const` is explicitly deleted).

🔹 **Returns**

- An iterator pointing to the first value in the generator.

**Usage**

- Enables iteration using `for (auto x : gen)`.
- Allows integration with STL algorithms that require iterators.

---

#### 📌 `iterator end()`

**Description:**  
Returns an **end iterator**, which serves as a **past-the-end** marker for iteration.

**Requirements**

- Available **only when `U == std::monostate`** (same as `begin()`).
- Safe to call multiple times without consuming values.

🔹 **Returns**

- An iterator representing the end of the sequence.

**Usage**

- Used as the termination condition in range-based for loops.
- Provides compatibility with STL algorithms requiring `begin()` and `end()`.

---

#### 📌 **Range-Based For Loop Support**

**Description:**  
Generators supporting range-based iteration can be used with the standard C++ for-each syntax.

**Requirements**

- Only available when `U == std::monostate`.
- The generator **must not be declared as `const`**, as iteration modifies its internal state.

**Usage**

- Iteration consumes elements one-by-one.
- The generator state is **advanced on each iteration step**.
- Declaring a generator as `const` would prevent iteration, as `begin()` is **not available** in that case.

---

## **Conversion Functions**

### 📌 `jh::generator<T> make_generator(const T& seq)`

**Description:**  
Converts a **sequence** into a generator.  
This function is only enabled when `T` is **not** a `std::ranges::range`.

🔹 **Template Parameters**
- `T` - The type of sequence. Must support range-based for loops (`begin()` & `end()`).
    - This function is **only available** when `T` does **not** satisfy `std::ranges::range`.
    - If `T` is a `std::ranges::range`, the overload `make_generator(R&& rng)` will be used instead.

🔹 **Parameters**
- `seq` - The sequence to convert.

🔹 **Returns**
- `jh::generator<sequence_value_type<T>>` - A generator yielding elements from `seq`.

🔹 **Usage Example**
```c++
struct MySequence {
    int data[3] = {1, 2, 3};
    int* begin() { return data; }
    int* end() { return data + 3; }
};

MySequence seq;
auto gen = jh::make_generator(seq);
for (auto x : gen) {
    std::cout << x << " ";  // Outputs: 1 2 3
}
```

### 📌 `jh::generator<T> make_generator(std::ranges::range R)`

**Description:**  
Converts a **range** into a generator.  
This function is available for all `std::ranges::range`, including STL containers (`std::vector`, `std::list`) and lazy ranges (`std::views::iota`, etc.).

🔹 **Template Parameters**
- `R` - The type of input range (`std::ranges::range`).

🔹 **Parameters**
- `rng` - The range to convert.

🔹 **Returns**
- `jh::generator<std::ranges::range_value_t<R>>` - A generator yielding elements from the range.

🔹 **Usage Example**
```c++
#include <ranges>
auto gen = jh::make_generator(std::views::iota(1, 5));  // Generates {1, 2, 3, 4}
for (auto x : gen) {
    std::cout << x << " ";
}
```

---

## **📌 Conversion to Containers**

### **1. `std::vector<T> to_vector(generator<T>& gen)`**
**Description:**  
Converts a generator **without `send()` support** into a `std::vector<T>`.  
The function consumes all values produced by the generator and returns them as a `std::vector`.

🔹 **Parameters**
- `gen` - A generator that does not require `send()`.

🔹 **Returns**
- `std::vector<T>` - A vector containing all generated values.

🔹 **Usage Example**
```c++
jh::generator<int> gen = some_generator();
std::vector<int> result = jh::to_vector(gen);
```

---

### **2. `std::vector<T> to_vector(generator<T, U>& gen, U input_value)`**
**Description:**  
Converts a generator **with `send()` support** into a `std::vector<T>`,  
where **the same value is sent to the generator at each step**.

🔹 **Parameters**
- `gen` - A generator that supports `send()`.
- `input_value` - The value sent to the generator at each step.

🔹 **Returns**
- `std::vector<T>` - A vector containing all generated values.

🔹 **Usage Example**
```c++
jh::generator<int, int> gen = some_generator();
std::vector<int> result = jh::to_vector(gen, 42);  // Always sends 42
```

---

### **3. `std::vector<T> to_vector(generator<T, std::ranges::range_value_t<R>>& gen, const R& inputs)`**
**Description:**  
Converts a generator **with `send()` support** into a `std::vector<T>`,  
where **a sequence of input values is sent in order**.

🔹 **Parameters**
- `gen` - A generator that supports `send()`.
- `inputs` - A `std::ranges::range` of values to send sequentially.

🔹 **Returns**
- `std::vector<T>` - A vector containing all generated values.

🔹 **Usage Example**
```c++
std::vector<int> inputs = {1, 2, 3};
jh::generator<int, int> gen = some_generator();
std::vector<int> result = jh::to_vector(gen, inputs);
```

---

### **4. `std::list<T> to_list(generator<T>& gen)`**
**Description:**  
Converts a generator **without `send()` support** into a `std::list<T>`.  
The function consumes all values produced by the generator and returns them as a `std::list`.

🔹 **Parameters**
- `gen` - A generator that does not require `send()`.

🔹 **Returns**
- `std::list<T>` - A list containing all generated values.

🔹 **Usage Example**
```c++
jh::generator<int> gen = some_generator();
std::list<int> result = jh::to_list(gen);
```

---

### **5. `std::list<T> to_list(generator<T, U>& gen, U input_value)`**
**Description:**  
Converts a generator **with `send()` support** into a `std::list<T>`,  
where **the same value is sent to the generator at each step**.

🔹 **Parameters**
- `gen` - A generator that supports `send()`.
- `input_value` - The value sent to the generator at each step.

🔹 **Returns**
- `std::list<T>` - A list containing all generated values.

🔹 **Usage Example**
```c++
jh::generator<int, int> gen = some_generator();
std::list<int> result = jh::to_list(gen, 42);  // Always sends 42
```

---

### **6. `std::list<T> to_list(generator<T, std::ranges::range_value_t<R>>& gen, const R& inputs)`**
**Description:**  
Converts a generator **with `send()` support** into a `std::list<T>`,  
where **a sequence of input values is sent in order**.

🔹 **Parameters**
- `gen` - A generator that supports `send()`.
- `inputs` - A `std::ranges::range` of values to send sequentially.

🔹 **Returns**
- `std::list<T>` - A list containing all generated values.

🔹 **Usage Example**
```c++
std::list<int> inputs = {1, 2, 3};
jh::generator<int, int> gen = some_generator();
std::list<int> result = jh::to_list(gen, inputs);
```


---

## **Use Cases**

- **Lazy Iteration**: Efficiently generate sequences without precomputing them.
- **Interactive Processing**: Send values dynamically to modify computation.
- **Stream Processing**: Work with large datasets without loading them into memory.

---

## **Iterator Overview**

### **Iterator for Coroutine-Based Generators**

The `jh::iterator<jh::generator<T, U>>` class is designed **exclusively** for **range-based iteration** over
`jh::generator<T, U>`. It provides standard **input iterator** behavior, allowing seamless integration with *
*range-based for-loops** and **STL algorithms**.

While **we strongly discourage manually instantiating this iterator**, it is not explicitly prohibited. The recommended
way to obtain an iterator is via `generator::begin()` and `generator::end()`.

Additionally, the generator provides a **type alias** for convenience:

```c++
using iterator = iterator<generator>;
```

This allows the iterator to be accessed directly as:

```c++
jh::generator<T, U>::iterator
```

Thus, the iterator can be referenced through the generator itself, making it more intuitive to use in generic code.

---

### **Key Properties**

- **Restricted to Generators Without Input (`U == std::monostate`)**
    - If `send()` is required, range-based iteration is **not possible** since input values must be explicitly provided.

- **Single-Pass, Consumable Iteration**
    - Advancing (`++`) **consumes** elements, meaning values cannot be revisited.
    - Works like Python iterators: each step modifies the generator's internal state.

- **Safe Expiration Handling**
    - If the generator is **destroyed** or **exhausted**, the iterator **automatically resets itself** to prevent
      invalid memory access.

- **No Shared Ownership**
    - Instead of `std::shared_ptr<G>`, this iterator holds a **non-owning** `std::optional<std::reference_wrapper<G>>`.
    - This avoids **unintended ownership extension**, ensuring the generator is managed **only by its coroutine**.

---

### **Usage Notes**

- If a generator supports `send()`, it **cannot** be iterated using `for(auto x : gen)`.
- If the generator is destroyed before iteration completes, the iterator will behave like `end()`.
- Iterators should only be used **while the generator is valid**—if the generator is manually stopped (`stop()`),
  iterators become invalid.

---

### **Iterator Behavior**

| Operation         | Description                                                                                                       |
|-------------------|-------------------------------------------------------------------------------------------------------------------|
| `operator++()`    | Advances to the next value, consuming the current one.                                                            |
| `operator++(int)` | Post-increment, but since iteration is single-pass, behaves the same as pre-increment.                            |
| `operator*()`     | Retrieves the current value. Throws if the iterator is exhausted.                                                 |
| `operator->()`    | Provides pointer access to the current value.                                                                     |
| `operator==()`    | Two iterators are equal if they reference the same generator **and** hold the same value, or if both are `end()`. |
| `operator!=()`    | Opposite of `operator==()`.                                                                                       |

---

### **Final Considerations**

- **Designed for safe, idiomatic C++ iteration.**
- **Optimized for performance with coroutine-based generators.**
- **Ensures safety against invalid accesses when generators expire.**
- **Avoids unnecessary complexity by enforcing a strict usage pattern.**

This iterator is an essential part of `jh::generator<T, U>`, enabling seamless iteration while maintaining the 
**efficiency, safety, and integrity** of coroutine-based execution.

## **Conclusion**

The `jh::generator<T, U>` class provides an **efficient**, **type-safe**, and **flexible** coroutine-based generator for
**C++20**, bringing **Python-style iteration and interaction** to C++.

📌 **For detailed module information, refer to `generator.h`.**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**
