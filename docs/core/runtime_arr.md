# 🧱 **JH Toolkit — `jh::runtime_arr` API Reference**

📁 **Header:** `<jh/core/runtime_arr.h>`  
🔄 **Forwarding Header:** `<jh/runtime_arr>`  
📦 **Namespace:** `jh`  
📅 **Version:** **1.4.1 (2026)**  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## ⚙️ Overview

`jh::runtime_arr<T>` is a **POD-accelerated dynamic array** type that supports
the *write once, build differently* principle of the Dual-Mode Header System.  
It provides predictable layout, zero allocator overhead, and safe range behavior
for both header-only and static-linked builds.

Unlike `std::vector`, `runtime_arr` enforces **non-copyable** and **non-movable** semantics,
ensuring deterministic memory ownership and simplified concurrency analysis.

It is a standard-conforming `std::ranges::range` with stable iterator semantics —
specifically designed for trivially copyable, plain-old-data (POD) elements.

---

## 🔹 Core Characteristics

| Aspect              | `jh::runtime_arr<T>`                    | `std::vector<T>`  | Note                                      |
|---------------------|-----------------------------------------|-------------------|-------------------------------------------|
| Mutability          | ✅ Mutable elements, immutable structure | ✅ Fully mutable   | Prevents unsafe reallocation              |
| Move / Copy         | ❌ Disabled                              | ✅ Supported       | Ensures stable memory                     |
| Thread Safety       | ✅ Safe for read-only access             | ⚠️ Not guaranteed |                                           |
| Memory Layout       | Compact POD buffer                      | Allocator-managed | Predictable layout                        |
| Range Compatibility | ✅ `std::ranges::range`                  | ✅                 | Some STL algos require movable containers |

---

## 🔹 Construction

`jh::runtime_arr` provides several construction paths, covering fixed-size, raw POD allocation, and range-based
initialization.  
It does **not** behave like `std::vector` — once created, the array size is fixed and cannot change.

```cpp
jh::runtime_arr<int> arr{5}; // single element [5]
jh::runtime_arr<int> arr(5); // 5 default-initialized elements [0,0,0,0,0]
jh::runtime_arr<int> arr(5, jh::runtime_arr<int>::uninitialized);
// 5 uninitialized elements (POD only) [?,?,?,?,?]

jh::runtime_arr<int> arr{1, 2, 3, 4, 5};
jh::runtime_arr<int> arr{std::move(vec)}; // vector with default allocator
jh::runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr(std::move(vec), alloc); 
// vector with any allocator
jh::runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr(std::move(pmr_vec)); 
// vector with same allocator

jh::runtime_arr<int> arr(v.begin(), v.end());
// range constructor with default allocator (Alloc = jh::typed::monostate)
jh::runtime_arr<int, std::allocator<int>> arr(v.begin(), v.end());
// range constructor with default-constructable allocator
jh::runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr(v.begin(), v.end(), alloc);
// range constructor with custom allocator
```

### Notes

From 1.4.0+, `runtime_arr` supports initialization from `std::initializer_list<T>`.

* `runtime_arr<T>(2)` → 2 default-initialized elements
* `runtime_arr<T>{2}` → single element with value 2

Same as `std::vector` behavior.

POD types support the **uninitialized allocation path**.  
Once constructed, the container **cannot be copied or resized**.  
Memory is **always contiguous**.

---

## 🔹 Range Compatibility Example

```cpp
#include <jh/runtime_arr>
#include <ranges>
#include <iostream>

jh::runtime_arr<int> arr{1,2,3,4,5};

auto even = arr.as_span()
           | std::views::filter([](int x){return x % 2 == 0;})
           | std::views::transform([](int x){return x * 10;});

for (int v : even)
    std::cout << v << ' ';
```

Output

```
20 40
```

---

## 🔹 Container Model

```cpp
using value_type        = T;
using size_type         = std::uint64_t;
using difference_type   = std::ptrdiff_t;
using reference         = value_type&;
using const_reference   = const value_type&;
using pointer           = value_type*;
using const_pointer     = const value_type*;
using iterator          = pointer;
using const_iterator    = const_pointer;
using allocator_type    = Alloc;
```

### Characteristics

* Entire container uses **raw pointer iteration**
* Satisfies `std::contiguous_iterator`
* No custom iterator layer
* Predictable layout

---

## 🔹 POD Acceleration

`runtime_arr` provides specialized fast paths when
`jh::pod::pod_like<T> == true`.

Features:

* raw allocation
* zero-cost reset
* tight contiguous layout
* no element destruction

---

## 🔹 Dual-Mode Header Integration

`runtime_arr` participates in the **Dual-Mode Header System**.

| Type                              | Header-only | Static Build | Description         |
|-----------------------------------|-------------|--------------|---------------------|
| runtime_arr<T>                    | ✅           | —            | Inline              |
| runtime_arr<bool>                 | ✅           | ⚙️           | Bit-packed          |
| runtime_arr<bool,bool_flat_alloc> | ✅           | ⚙️           | Flat bool           |
| runtime_arr<bool,CustomPolicy>    | ✅           | —            | User specialization |

---

## 🔹 Performance Summary

| Variant                           | Access Model | Storage    | POD Behavior  | Relative Speed | Space Efficiency | Notes       |
|-----------------------------------|--------------|------------|---------------|----------------|------------------|-------------|
| runtime_arr<T> POD                | direct       | contiguous | memset        | ⚡ 6–9× faster  | compact          | raw buffer  |
| runtime_arr<T> non-POD            | object       | contiguous | ctor loop     | ~1×            | compact          | vector-like |
| runtime_arr<bool>                 | proxy        | bit-packed | bit ops       | slower         | 8× smaller       | dense       |
| runtime_arr<bool,bool_flat_alloc> | direct       | byte       | direct access | faster         | 1×               | flat        |

---

## 🔸 API Breakdown — `runtime_arr<T>`

| Category       | Member                                                             | Description                               |
|----------------|--------------------------------------------------------------------|-------------------------------------------|
| Construction   | `runtime_arr(u64 n)`                                               | Allocates `n` elements                    |
|                | `runtime_arr(u64 n, uninitialized_t)`                              | POD-only raw allocation                   |
|                | `runtime_arr(u64 n, const Alloc& alloc)`                           | Uses custom allocator                     |
|                | `runtime_arr(std::vector<T>&& vec)`                                | Move-construct from vector                |
|                | `runtime_arr(std::vector<T, VecAlloc>&& vec, const Alloc& alloc)`  | Move from vector using provided allocator |
|                | `runtime_arr(std::vector<T, Alloc>&& vec)`                         | Move from vector using its own allocator  |
|                | `runtime_arr(std::initializer_list<T>)`                            | Initialize from list                      |
|                | `runtime_arr(std::initializer_list<T>, const Alloc&)`              | Init list with allocator                  |
|                | `runtime_arr(ForwardIt first, ForwardIt last)`                     | Construct from iterator range             |
|                | `runtime_arr(ForwardIt first, ForwardIt last, const Alloc& alloc)` | Range constructor using allocator         |
| Element Access | `operator[](u64)`                                                  | Unchecked access                          |
|                | `at(u64)`                                                          | Checked access                            |
|                | `data()`                                                           | Raw pointer                               |
|                | `as_span()`                                                        | Mutable span view                         |
|                | `as_span() const`                                                  | Const span view                           |
| Iteration      | `begin()`                                                          | Begin iterator                            |
|                | `end()`                                                            | End iterator                              |
|                | `begin() const`                                                    | Const begin                               |
|                | `end() const`                                                      | Const end                                 |
|                | `cbegin()`                                                         | Const begin iterator                      |
|                | `cend()`                                                           | Const end iterator                        |
| Modifiers      | `set(u64, Args...)`                                                | Assign element                            |
|                | `reset_all()`                                                      | Reset entire array                        |
| Capacity       | `size()`                                                           | Element count                             |
|                | `empty()`                                                          | Whether empty                             |
| Ownership      | `operator std::vector<T>() &&`                                     | Convert to vector                         |
|                | `is_static_built()`                                                | Dual-mode flag                            |
| Semantics      | Move-only                                                          | Copy disabled                             |
|                | Contiguous                                                         | Raw array storage                         |
|                | POD-optimized                                                      | memset fast path                          |

---

## 🔸 API Breakdown — `runtime_arr<bool>`

| Category       | Member                                     | Description           |
|----------------|--------------------------------------------|-----------------------|
| Construction   | `runtime_arr(u64 size)`                    | Allocates bit storage |
|                | `runtime_arr(std::vector<bool>&&)`         | Build from vector     |
|                | `runtime_arr(std::initializer_list<bool>)` | Init from list        |
|                | `runtime_arr(ForwardIt, ForwardIt)`        | Construct from range  |
| Bit Access     | `operator[](u64)`                          | Proxy reference       |
|                | `operator[](u64) const`                    | Read bit              |
|                | `at(u64)`                                  | Checked access        |
|                | `at(u64) const`                            | Checked read          |
| Bit Operations | `set(u64,bool)`                            | Set bit               |
|                | `unset(u64)`                               | Clear bit             |
|                | `test(u64)`                                | Read bit              |
|                | `reset_all()`                              | Clear all bits        |
| Iterators      | `begin()`                                  | Bit iterator          |
|                | `end()`                                    | Bit iterator end      |
|                | `begin() const`                            | Const begin           |
|                | `end() const`                              | Const end             |
|                | `cbegin()`                                 | Const begin iterator  |
|                | `cend()`                                   | Const end iterator    |
| Raw Access     | `raw_data()`                               | Pointer to words      |
|                | `raw_data() const`                         | Const pointer         |
|                | `raw_word_count()`                         | Word count            |
| Capacity       | `size()`                                   | Bit count             |
|                | `empty()`                                  | Empty check           |
| Ownership      | `operator std::vector<bool>() &&`          | Convert to vector     |
|                | `is_static_built()`                        | Static build flag     |
| Semantics      | Move-only                                  | Copy disabled         |
|                | Bit-packed                                 | 64 bits per word      |
|                | RAII                                       | unique_ptr storage    |

---

## ⚠️ Note on `runtime_arr<bool>` Usage

`runtime_arr<bool>` is a **space-optimized container**, not a general range.

Characteristics:

* proxy access
* bit-masking reads/writes
* no contiguous <code>bool&#42;</code>
* no span support

Recommended:

```cpp
for (auto& bit : arr) {
    // proxy usage
}
```

If you need a range-friendly or performance-oriented boolean sequence, prefer:

* `runtime_arr<bool, bool_flat_alloc>` (flat byte storage), or
* `runtime_arr<uint8_t>`

depending on semantics.

---

## 🧩 Summary

`jh::runtime_arr` is a **lightweight, deterministic, and POD-accelerated** dynamic container
supporting the JH Toolkit's *write-once, build-differently* model.

It offers full range compatibility, predictable memory layout,
and static specialization where it truly improves performance.

---

> **Tip:**
> Include `<jh/runtime_arr>`
>
> Choose your CMake linkage:
>
> ```cmake
> target_link_libraries(app PRIVATE jh::jh-toolkit)
> # or
> target_link_libraries(app PRIVATE jh::jh-toolkit-static)
> ```
>
> `is_static_built()` allows runtime detection of static-optimized builds.
>
> User-defined `bool` variants remain header-only and link-free.  
