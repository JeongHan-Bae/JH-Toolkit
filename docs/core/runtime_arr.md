# 🧱 **JH Toolkit — `jh::runtime_arr` API Reference**

📁 **Header:** `<jh/core/runtime_arr.h>`  
🔄 **Forwarding Header:** `<jh/runtime_arr>`  
📦 **Namespace:** `jh`  
📅 **Version:** 1.3.5+ (2025)  
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
jh::runtime_arr<int> arr{5};  // Allocate 5 elements, value-initialized (zero for POD)
jh::runtime_arr<int> arr(5, jh::runtime_arr<int>::uninitialized); // POD only, raw uninitialized memory
jh::runtime_arr<int> arr{{1, 2, 3, 4, 5}};  // ✅ Deduced as std::vector<int>
jh::runtime_arr<int> arr{std::vector<int>{1, 2, 3, 4, 5}};  // ✅ Equivalent explicit form
jh::runtime_arr<int> arr(v.begin(), v.end());  // ✅ Constructed from iterator range
```

### Notes

* Both `{ {1,2,3,4,5} }` and `{ std::vector<int>{1,2,3,4,5} }` are **functionally equivalent** —
  the compiler automatically deduces `{1,2,3,4,5}` as a temporary `std::vector<int>`.
* `jh::runtime_arr` has **no constructor** for raw initializer lists like `{1,2,3,4,5}` (without double braces).
* For POD types, `uninitialized_t` allows you to allocate without initialization,
  providing semantics equivalent to `std::vector::reserve()` — useful when you intend to fill manually.
* Once constructed, `jh::runtime_arr` cannot be copied, moved, or resized.
* All constructors guarantee **contiguous, trivially destructible memory** suitable for low-level operations.

---

## 🔹 Range Compatibility Example

Although `jh::runtime_arr` is non-copyable and non-movable, it fully satisfies
the **`std::ranges::range`** concept and can interoperate with most standard range algorithms.

To support pipe-based functional composition, include `jh/conceptual/sequence.h`
to access `jh::to_range()`, which wraps a runtime array into a `std::ranges::subrange`.

```cpp
#include <jh/runtime_arr>
#include <jh/conceptual/sequence.h>
#include <ranges>
#include <iostream>

jh::runtime_arr<int> arr{{1, 2, 3, 4, 5}};  // ✅ Proper vector-deduced initialization

auto even = jh::to_range(arr)
           | std::views::filter([](int x) { return x % 2 == 0; })
           | std::views::transform([](int x) { return x * 10; });

for (int v : even)
    std::cout << v << ' ';  // 20 40
```

### Notes

* `jh::runtime_arr` provides all the standard container typedefs (`value_type`, `iterator`, `size_type`, etc.),
  so it integrates seamlessly with the `std::ranges` framework.
* For the general version (`T != bool`), iterators are plain pointers.  
  For the `bool` specialization, iterators implement a **proxy output iterator** model,
  which avoids bitset aliasing issues and provides more stable behavior across compilers.
* The container is designed for **POD acceleration** — tightly packed, trivially copyable, and
  optimized for predictable layout and cache efficiency.
* On some compilers, `std::vector<bool>` iterators fail to satisfy `input_iterator`/`output_iterator`;  
  `jh::runtime_arr<bool>` intentionally uses a simpler, safer proxy model to avoid this instability.

---

## 🔹 **Container Model**

`jh::runtime_arr` implements a **flat pointer–based container model**,
optimized for trivially copyable POD types.  
All range and iterator operations map directly to contiguous memory access —
no proxy indirection (except for `bool` specialization).

```cpp
using value_type        = T;                 ///< Element type.
using size_type         = std::uint64_t;     ///< 64-bit size type.
using difference_type   = std::ptrdiff_t;    ///< Signed offset type.
using reference         = value_type&;       ///< Reference type.
using const_reference   = const value_type&; ///< Const reference.
using pointer           = value_type*;       ///< Direct data pointer.
using const_pointer     = const value_type*; ///< Const data pointer.
using iterator          = pointer;           ///< Iterator = pointer.
using const_iterator    = const_pointer;     ///< Const iterator = const pointer.
```

### Characteristics

* The entire model is **pointer-based** — no custom iterator layer.  
  This design provides zero overhead and guarantees that all
  standard `std::ranges` algorithms can access elements directly.
* Fully satisfies `std::ranges::range` (iterable via begin/end).
* Deterministic memory layout; trivially copyable and POD-friendly.
* Iterators are *raw pointers* (`T*`), so `std::contiguous_iterator` is satisfied automatically.

### Specialization: `runtime_arr<bool>`

* For `bool`, iterators behave like those of `std::vector<bool>` —
  they model **`std::input_iterator`** via *common reference binding* rather than
  direct element reference.  
  This design provides stable range compatibility and avoids proxy category issues.

* `runtime_arr<bool>::iterator` and `std::vector<bool>::iterator` both rely on
  a proxy reference type (`bool_proxy`) whose common reference resolves to `bool`,
  ensuring full conformance with the C++20 `std::ranges` iterator requirements.

* In other words, `runtime_arr<bool>` preserves correct `input_iterator` semantics
  and interoperates seamlessly with all standard range algorithms.

---

## 🔹 POD Acceleration

`runtime_arr` provides a **specialized fast path for POD types**
(`jh::pod::pod_like<T> == true`).  
This optimization eliminates all constructor, destructor, and allocator overhead —
the array becomes a pure contiguous memory block managed by RAII.

* **Trivially copyable storage:** raw `operator new[]` / `delete[]`, no metadata.
* **Zero-cost reset:** implemented via `std::memset`, not element-wise clearing.
* **Cache locality:** tight linear memory layout, ideal for numeric or packed workloads.
* **No element move or destruction:** buffer reused safely across scopes.

This design offers high throughput for primitive and POD-structured data
(e.g. `int`, `float`, `char`, simple structs).

---

### `uninitialized_t` Constructor (POD-only)

For **POD types only**,
`runtime_arr` exposes an explicit constructor that skips all initialization:

```cpp
jh::runtime_arr<int> arr(1024, jh::runtime_arr<int>::uninitialized);
```

**Constraints**

This overload exists **only if**:

```cpp
requires jh::pod::pod_like<T> && typed::monostate_t<Alloc>
```

That means:

* `T` must be trivially copyable, standard-layout (POD-like).
* `Alloc` must be the default `monostate` allocator —
  custom allocators are not supported for uninitialized construction.
* The constructor **does not participate in overload resolution**
  for non-POD or allocator-bound types (no fallback, no substitution).

**Behavior**

* Allocates raw memory for `size` elements, **without zero-filling or constructing**.
* Contents are **indeterminate** until explicitly written.
* Semantics roughly mirror `std::vector::reserve()`,
  though no capacity bookkeeping or dynamic growth is present.
* For trivial data, initialization speed is effectively instantaneous
  (allocation only, no memory writes).

---

## 🔹 Dual-Mode Header Integration

`runtime_arr` participates in the Dual-Mode Header System,
allowing optimized static builds for specific types while keeping most variants header-only.

| Type                                                     | Header-only | Static Build | Description                            |
|----------------------------------------------------------|-------------|--------------|----------------------------------------|
| `runtime_arr<T>`                                         | ✅ Always    | —            | Inline-only                            |
| `runtime_arr<bool>`                                      | ✅           | ⚙️ Yes       | Proxy-based, statically optimized      |
| `runtime_arr<bool, runtime_arr_helper::bool_flat_alloc>` | ✅           | ⚙️ Yes       | Flat-layout version, faster than proxy |
| `runtime_arr<bool, CustomPolicy>`                        | ✅           | —            | Always header-only                     |

```cpp
if (jh::runtime_arr<bool>::is_static_built()) {
    std::puts("using static-optimized bit-array implementation");
}
```

💡 User-defined specializations (`runtime_arr<bool, CustomPolicy>`)
are **always header-only** and never linked as precompiled objects.

---

## 🔹 CMake Integration

```cmake
target_link_libraries(my_app PRIVATE jh::jh-toolkit)        # header-only
# or
target_link_libraries(my_app PRIVATE jh::jh-toolkit-static) # static bool optimization
```

Only `runtime_arr<bool>` and its flat-layout variant
link to precompiled symbols under static mode.  
All other types remain inline and header-only.

---

## 🔹 Performance Summary

| Variant                              | Access Model | Storage     | POD Behavior                  | Relative Speed                       | Space Efficiency | Notes                              |
|--------------------------------------|--------------|-------------|-------------------------------|--------------------------------------|------------------|------------------------------------|
| `runtime_arr<T>` (POD)               | Direct POD   | Contiguous  | Zero-fill or uninitialized    | ⚡ ~6–9× faster than `std::vector<T>` | ✅ Compact        | RAII-managed heap buffer           |
| `runtime_arr<T>` (non-POD)           | Object-based | Contiguous  | Default-construct per element | ⚙️ ~1.0× baseline (`std::vector<T>`) | ✅ Compact        | Full ctor/dtor semantics           |
| `runtime_arr<bool>`                  | Bit-proxy    | Bit-packed  | Proxy-based bit access        | 🐢 ~3–5× slower than flat bool       | 💾 8× smaller    | Proxy overhead balanced by density |
| `runtime_arr<bool, bool_flat_alloc>` | Flat bytes   | Plain array | 1 byte per bool               | ⚡ ~3–5× faster than proxy            | ⚠️ 1× baseline   | Static build variant               |
| `runtime_arr<bool, CustomPolicy>`    | Custom       | Depends     | Depends on allocator          | Varies                               | Depends          | Always inline header-only          |

---

### Observations

* For **POD types** (`int`, `float`, `char`, etc.):

    * Initialization compiles down to a single `memset` or `operator new[]`.
    * The `uninitialized` constructor (`runtime_arr<T>(n, uninitialized)`) simply skips zeroing;  
      both forms compile identically for trivial types.
    * `reset_all()` uses `std::memset()` for maximum throughput and deterministic clearing.
    * Benchmarks show **~6–9× faster** construction and reset than `std::vector<T>` under `-O2` to `-O3`.

* For **non-POD types** (e.g., `std::string`, custom structs):

    * Elements are constructed one by one with default constructors.
    * Runtime cost equals or slightly exceeds that of `std::vector<T>`,
      since both rely on equivalent constructor loops.
    * Still benefits from reduced indirection (no `allocator_traits` layer).

* For **bool**:

    * The **bit-proxy specialization** (`runtime_arr<bool>`) is optimized for density, not speed.
    * The **flat allocator variant** (`runtime_arr<bool, bool_flat_alloc>`) restores direct access,
      trading memory compression for up to **5× faster bit I/O**.
    * Both variants are dual-mode (header-only and static precompiled) for consistent behavior.

---

## 🔸 API Breakdown — `runtime_arr<T>`

| Category                   | Member                                                                    | Description                                                                                        |
|----------------------------|---------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------|
| **Construction**           | `runtime_arr(std::uint64_t n)`                                            | Allocates and zero-initializes `n` elements. POD types are memset to 0; others default-construct.  |
|                            | `runtime_arr(std::uint64_t n, uninitialized_t)`                           | POD-only. Allocates raw memory without initialization. Contents are indeterminate until written.   |
|                            | `runtime_arr(std::uint64_t n, Alloc&& alloc)`                             | Uses custom allocator (non-monostate). Captures allocator in deleter lambda.                       |
|                            | `runtime_arr(std::vector<T>&& vec)`                                       | Moves elements from an existing `std::vector<T>`. New contiguous buffer allocated.                 |
|                            | `runtime_arr(std::initializer_list<T> init)`                              | Constructs from initializer list. Equivalent to copying all elements of `init` into the new array. |
|                            | `runtime_arr(std::initializer_list<T> init, const allocator_type& alloc)` | Constructs from initializer list using the specified allocator.                                    |
|                            | `runtime_arr(ForwardIt first, ForwardIt last)`                            | Constructs from any forward iterator range. Copies or moves elements as needed.                    |
| **Element Access**         | `T& operator[](u64 i)` / `const T& operator[](u64 i) const`               | Unchecked element access. Undefined if out of range.                                               |
|                            | `T& at(u64 i)` / `const T& at(u64 i) const`                               | Checked access. Throws `std::out_of_range` if invalid.                                             |
|                            | `T* data()` / `const T* data() const`                                     | Returns direct pointer to contiguous storage.                                                      |
|                            | `std::span<T> as_span()`                                                  | Returns `std::span` view of the data.                                                              |
| **Iteration**              | `iterator begin()`, `iterator end()`                                      | Iteration over raw pointers (`T*`).                                                                |
|                            | `const_iterator begin() const`, `end() const`                             | Const versions. Fully STL/ranges compatible.                                                       |
| **Modifiers**              | `void set(u64 i, Args&&... args)`                                         | Assigns to element `i` via forwarding constructor args.                                            |
|                            | `void reset_all()`                                                        | Reinitializes all elements: `memset` for POD, placement-new for trivial, `T{}` otherwise.          |
| **Capacity**               | `u64 size() const`                                                        | Returns element count.                                                                             |
|                            | `bool empty() const`                                                      | True if `size == 0`.                                                                               |
| **Ownership / Conversion** | `operator std::vector<T>() &&`                                            | Moves out to `std::vector<T>`. Invalidates source.                                                 |
|                            | `is_static_built()`                                                       | Compile-time flag for dual-mode static linking.                                                    |
| **Semantics**              | Move-only                                                                 | Copy constructor/assignment deleted. Move ops `noexcept`.                                          |
|                            | Contiguous                                                                | Memory layout = plain array.                                                                       |
|                            | POD-optimized                                                             | Zeroing and construction paths reduced to raw `memset`/`new[]`.                                    |

---

## 🔸 API Breakdown — `runtime_arr<bool>`

| Category                   | Member                                                                       | Description                                                       |
|----------------------------|------------------------------------------------------------------------------|-------------------------------------------------------------------|
| **Construction**           | `runtime_arr(u64 size)`                                                      | Allocates ceil(size/64) words, all bits = 0.                      |
|                            | `runtime_arr(std::vector<bool>&& vec)`                                       | Copies bits from vector<bool>.                                    |
|                            | `runtime_arr(std::initializer_list<bool> init)`                              | Constructs from a sequence of booleans; bits are set accordingly. |
|                            | `runtime_arr(ForwardIt first, ForwardIt last)`                               | Builds from range of bools or convertible values.                 |
| **Bit Access**             | `bit_ref operator[](u64 i)`                                                  | Proxy reference to bit i (unchecked).                             |
|                            | `bool operator[](u64 i) const`                                               | Read-only bit value (unchecked).                                  |
|                            | `bit_ref at(u64 i)` / `bool at(u64 i) const`                                 | Checked access; throws `out_of_range`.                            |
| **Bit Operations**         | `void set(u64 i, bool val = true)`                                           | Sets or clears bit i.                                             |
|                            | `void unset(u64 i)`                                                          | Clears bit i.                                                     |
|                            | `bool test(u64 i) const`                                                     | Returns true if bit is set.                                       |
|                            | `void reset_all()`                                                           | Zeroes all bits (via memset).                                     |
| **Iterators**              | `bit_iterator begin()` / `end()`                                             | Proxy iterators over bits.                                        |
|                            | `const bit_iterator begin() const` / `end() const`                           | Const-compatible versions.                                        |
| **Raw Access**             | `uint64_t* raw_data()` / `const uint64_t* raw_data() const`                  | Access packed 64-bit word storage.                                |
|                            | `u64 raw_word_count() const`                                                 | Number of underlying 64-bit words.                                |
| **Capacity**               | `u64 size() const`                                                           | Number of logical bits.                                           |
|                            | `bool empty() const`                                                         | True if no bits allocated.                                        |
| **Ownership / Conversion** | `operator std::vector<bool>() &&`                                            | Converts to vector<bool>, consuming the source.                   |
|                            | `is_static_built()`                                                          | Returns compile-time linkage mode.                                |
| **Semantics**              | Move-only                                                                    | Copy disabled.                                                    |
|                            | Bit-packed                                                                   | 64 bits per word; not contiguous in bool*.                        |
|                            | Safe RAII                                                                    | Fully managed via `unique_ptr<uint64_t[]>`.                       |

---

## 🧩 Summary

`jh::runtime_arr` is a **lightweight, deterministic, and POD-accelerated** dynamic container
supporting the JH Toolkit's *write-once, build-differently* model.

It offers full range compatibility, predictable memory layout,
and static specialization where it truly improves performance.

---

> **Tip:**
> Include `<jh/runtime_arr>` (without `.h`) for modern import style.
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
