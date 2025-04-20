# 🏆 JH Toolkit: `POD` System API Documentation

📌 **Version:** 1.3   
🕝 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## 📚 Table of Contents

- [🌍 Overview](#-overview)
- [🧠 POD Design Philosophy](#-pod-design-philosophy)
- [Header Structure](#header-structure)
- [🔧 Key Concepts & Macros](#-key-concepts--macros)
- [📦 Built-in POD Types](#-built-in-pod-type-apis)
  - [`pod::pair`](#-jhpodpairt1-t2)
  - [`pod::tuple`](#-jhpodtuplets-transitional)
  - [`pod::array`](#-jhpodarrayt-n)
  - [`pod::bitflags`](#-jhpodbitflagsn)
  - [`pod::string_view`](#-jhpodstring_view)
  - [`pod::optional`](#-jhpodoptionalt)
- [📊 STL Optimization with pod_like Types](#-stl-optimization-with-pod_like-types)
- [✅ Best Practices](#-best-practices)
- [↔️ Byte Order Disclaimer](#-byte-order-disclaimer)
- [📊 Alignment Disclaimer](#-alignment-disclaimer)
- [🧬 Compiler Optimization Behavior](#-pod_like-and-compiler-optimizations)

---

## 🌍 Overview

The `jh::pod` module defines a complete system for working with **Plain Old Data (POD)-like types**, optimized for **raw memory containers**, **placement-new**, and **low-overhead computation**.

> 🔀 Both `<jh/pod>` and `<jh/pod.h>` are supported. Prefer `<jh/pod>` for modern-style inclusion.

This module enforces memory-safe layout and guarantees:

- Zero-cost construction
- `memcpy`-safe semantics
- Standard layout
- High-performance compatibility with `std::stack`, `arena`, `mmap`, etc.

> 📌 **Target platform:** 64-bit only — `std::size_t` is avoided in favor of fixed-size types (`std::uint*_t`) for layout clarity and deterministic memory modeling.  
> 💡 `jh::pod` types use fixed-size integers like `uint32_t` / `uint64_t` in place of `size_t`  
> This ensures consistent layout across all platforms and reflects our 64-bit-only design.  
> All buffer sizes are explicitly bounded — oversize instantiations will fail at compile time via `concept` checks.

---

## 🧠 POD Design Philosophy

- 🧱 **POD means full control over memory layout**
- 🧼 No constructors, destructors, virtual tables, hidden heap allocations
- 🔒 Enables safe serialization, memory mapping, bare-metal operation
- 🧠 All fields known at compile time → layout-stable, tooling-friendly

📦 `pod` is a lightweight, template-based subsystem that offers value-semantic types such as `pod::pair`, `pod::optional`, and `pod::bitflags`, designed specifically for **C++20** and **64-bit desktop/server-class platforms**.

🔹 While header-only and minimal in implementation, `pod` leverages advanced language features like:
- C++20 concepts and constraints
- `constexpr` evaluations
- Structured bindings and template deduction
- Flattened STL containers and `std::is_trivially_copyable` checks

❗ As such, **`pod` is not designed for embedded, 32-bit, or C++14/11 platforms**.

---

## Header Structure

| Header Path               | Contents                                                       | Typical Usage                               |
|---------------------------|----------------------------------------------------------------|---------------------------------------------|
| `<jh/pod>`                | Full utilities: `pod_like`, `JH_POD_STRUCT`, `pod::pair`, etc. | Default include for end users               |
| `<jh/pods/pod_like.h>`    | Only the `pod_like` concept                                    | For SFINAE / constraints                    |
| `<jh/pods/pair.h>`        | `pod::pair<T1, T2>` only                                       | Low-level generic usage                     |
| `<jh/pods/array.h>`       | `pod::array<T, N>` only                                        | Low-level generic usage                     |
| `<jh/pods/bits.h>`        | `pod::bitflags<N>` only                                        | Low-level algorithmic usage                 |
| `<jh/pods/bytes_view.h>`  | `pod::bytes_view` only                                         | Low-level serialization usage               |
| `<jh/pods/span.h>`        | `pod::span<T>` only                                            | Pod Compatible span view                    |
| `<jh/pods/string_view.h>` | `pod::string_view` only                                        | Low-level algorithmic usage                 |
| `<jh/pods/tools.h>`       | Macros & non-recommended types like `tuple`                    | Internal only — auto-included by `<jh/pod>` |
| `<jh/pods/optional.h>`    | `pod::optional<T>` only                                        | Pod Compatible optional type                |

---

## 🔧 Key Concepts & Macros

### ✅ `pod_like<T>` concept

Compile-time constraint for POD-compatible types.

```c++
template<typename T>
concept pod_like = std::is_trivially_copyable_v<T> &&
                   std::is_trivially_constructible_v<T> &&
                   std::is_trivially_destructible_v<T> &&
                   std::is_standard_layout_v<T>;
```

### ⚙️ `JH_POD_STRUCT(NAME, ...)`

Define a POD struct with layout validation.

```c++
// Example: Memory-mapped network packet header
JH_POD_STRUCT(PacketHeader,
    uint16_t id;
    uint8_t flags;
    uint8_t length;
);
```

- Declares a struct with `==` operator
- Triggers a `static_assert` if not `pod_like`

### 📌 `JH_ASSERT_POD_LIKE(T)`

Validate existing struct:

```c++
struct Packet { int len; char data[8]; };
JH_ASSERT_POD_LIKE(Packet);
```

---


## 📦 Built-in POD Type APIs

### 🔹 `jh::pod::pair<T1, T2>`

A POD-compatible replacement for `std::pair`, designed for raw memory use and binary-safe operations.

#### ⚙️ Requirements
- `T1`, `T2` must satisfy `pod_like<T>`

#### 🧩 Members
```c++
T1 first;
T2 second;

constexpr bool operator==(const pair&) const = default;
```

#### 💡 Example
```c++
jh::pod::pair<int, float> p{42, 3.14f};
```

---

### ⚠️ `jh::pod::tuple<Ts...>` (transitional)

A **POD-safe, fixed-field tuple replacement**, primarily designed to ease migration from `std::tuple` to `pod_like` types. It supports up to 8 fields, but is explicitly marked **deprecated** to encourage eventual migration to **custom POD structs with real field names**.

> ⚠️ **`pod::tuple` is a temporary migration tool**  
> Use it to quickly port from `std::tuple`, but replace with `JH_POD_STRUCT(...)` ASAP for better field naming and safety.

#### 🧭 Intended Purpose
> ✅ Use it as a bridge from `std::tuple<Ts...>`  
> ❌ Not meant for long-term or high-level use  
> ✅ Encourages compile-time layout and serialization stability  
> ❌ No structured bindings, no `std::get`, no runtime flexibility

> **Final goal:** define proper `JH_POD_STRUCT(...)` types with explicit field names (not `get<0>()`, etc.)

#### ⚙️ Requirements
- `T1`, `T2` must be `pod_like`
- Optional fields `T3`–`T8` can be `jh::typed::monostate` or `pod_like`
- `jh::typed::monostate` is a placeholder type to express "Nothing" without reliance to `std::variant` (`std::monostate`).

#### 🧩 Members
```c++
T1 _0; T2 _1; T3 _2; T4 _3; T5 _4; T6 _5; T7 _6; T8 _7;

template<uint8_t V> Tn& get();       // Indexed field access (get<0>(), get<1>(), ...)
constexpr bool operator==(const tuple&) const = default;
```

#### 🛑 Limitations
- No structured binding (`auto [a, b] = t;` is **not** supported)
- No variadic unpacking
- All field names are positional (`_0`, `_1`, etc.)
- Not semantically meaningful → harder to read/maintain

#### 💡 Example
```c++
// Temporary tuple form
jh::pod::tuple<int, float, char> t{1, 3.0f, 'x'};
t.template get<1>() += 1.5f;
```

---

### 🔹 `jh::pod::array<T, N>`

A fixed-size inline buffer — raw, no-allocator, no bounds-checking. Designed for stack or `.data` memory.

#### ⚙️ Requirements
- `T` must be `pod_like`
- `sizeof(T) * N <= 16 * 1024` (max 16 KB)

#### 🧩 Members
```c++
T data[N];

T& operator[](std::size_t i) noexcept;
const T& operator[](std::size_t i) const noexcept;

T* begin() noexcept;               // for range-based loops
const T* begin() const noexcept;
T* end() noexcept;
const T* end() const noexcept;

static constexpr std::size_t size();
constexpr bool operator==(const array&) const = default;
```

#### 💡 Example
```c++
jh::pod::array<int, 3> a = {{1, 2, 3}};
```

---

### 🔹 `jh::pod::bitflags<N>`

Low-overhead fixed-size bitfield, for embedded flags or mapped memory buffers. Works like `std::bitset`, but without heap or safety checks.

#### ⚙️ Requirements
- `N` must be divisible by 8
- `N <= 32,768` (max 4 KB)

#### 📚 Variants
- Native backend (`N` = 8, 16, 32, 64): stored in `uint{N}_t` (`.bits`)
- Byte-array backend (`N` = others): stored in `uint8_t[]` (`.data`)

> Although Realizations are placed in `namespace jh::pod::detail`, you can always manipulate underlying data
> by interacting with `.bits` or `.data`.  
> Do NOT call `reinterpret_cast` to `&.bits` as this disrupts the library-defined little-endian. 

#### 🧩 Core Methods
```c++
void set(uint16_t bit) noexcept;
void clear(uint16_t bit) noexcept;
void flip(uint16_t bit) noexcept;
bool has(uint16_t bit) const noexcept;

void set_all() noexcept;
void reset_all() noexcept;
void flip_all() noexcept;

uint16_t count() const noexcept;

bitflags<N> operator|, &, ^, ~;
bool operator==(const bitflags<N>&) const = default;
```

#### 🧩 Serialization
```c++
jh::pod::array<uint8_t, N / 8> to_bytes(bitflags<N>);   // get a snapshot
bitflags<N> from_bytes(jh::pod::array<uint8_t, N / 8>); // from snapshot
```

#### 💡 Example
```c++
jh::pod::bitflags<16> f{};
f.set(3);
if (f.has(3)) { ... }
```

#### 📌 Endianness Design

`bitflags<N>` uses two internal layouts depending on `N`:

- For `N = 8, 16, 32, 64`:  
  → Internally stored as `.bits` (a native unsigned integer)  
  → Follows the **platform's byte order**
- For all other values:  
  → Internally stored as `.data[N / 8]` (a byte array)  
  → Stores bits in **explicit little-endian order**, one byte at a time

---

#### 📤 Snapshot Format

The functions:

```c++
pod::array<uint8_t, N / 8> to_bytes(bitflags<N>);
bitflags<N> from_bytes(pod::array<uint8_t, N / 8>);
```

create or consume **snapshots** of the bitflags state.

- The snapshot is a **copy of the internal content**, but in a separate buffer
- Snapshots always use **little-endian byte order**, regardless of platform or internal layout
- Safe for serialization, MMAP, and cross-platform transfer

---

#### Notes

- You can always use the public API (`set()`, `has()`, `clear()`, etc.) — it works uniformly for all `N`
- If you need to **manually access the raw bits**:
  - Use `.bits` when `N = 8, 16, 32, 64`  
    → Fast, native access, but **platform endianness**
  - Use `.data[]` when `N` is any other value  
    → Each byte is stored in **defined little-endian order**

> **Do not reinterpret-cast** `.bits` or `.data[]` — this may lead to undefined behavior and is never portable

---

### 🔹 `jh::pod::bytes_view`

A low-level, read-only, POD-safe wrapper over raw memory regions. Designed for binary parsing, protocol headers, and typed deserialization of mapped memory.

#### ⚙️ Requirements
- The memory being pointed to must remain valid externally.
- Type `T` for interpretation must satisfy:
  - `trivial_bytes<T>` (for `.from`, `.at`, `.fetch`)
  - `pod_like<T>` (for `.clone`)

#### 🧩 Members
```c++
const std::byte* data;
uint64_t len;
```

#### 🧩 Core Functions

```c++
template<trivial_bytes T>
static bytes_view from(const T& obj);

template<trivial_bytes T>
static bytes_view from(const T* arr, uint64_t size); // size in elements, not bytes

template<trivial_bytes T>
const T& at(uint64_t offset = 0) const;    // unsafe, will NOT check bounding

template<trivial_bytes T>
const T* fetch(uint64_t offset = 0) const; // if extending the size, will return nullptr

template<pod_like T>
T clone() const;                           // Only pod-like can be cloned

bool operator==(const bytes_view&) const noexcept;   // compared by content
```

#### 💡 Example
```c++
struct Packet { uint32_t id; uint8_t flags; };
JH_ASSERT_POD_LIKE(Packet);

Packet pkt{123, 0b0101};
auto view = jh::pod::bytes_view::from(pkt);
Packet copy = view.clone<Packet>();
```

#### 🧠 Design Intent

- `.from(...)` creates a byte-level view over object or array memory.
- `.at<T>()` and `.fetch<T>()` reinterpret contents as typed structs (with optional offset).
- `.clone<T>()` **copies** content into a stack-allocated object (`T must be pod_like`).

#### ⚠️ Safety Note

- This system assumes **byte-for-byte memory equivalence** — it does not handle endian correction.
- You are responsible for ensuring **alignment**, **lifetime**, and **byte order** are valid.

#### ⚠️ Why `bytes_view` Does Not Provide `copy_to(...)`

Unlike `string_view`, which offers `copy_to(char*)` as a convenience method for short strings:

- `bytes_view` is **not null-terminated**, and its memory **may alias structured objects**.
- The `.clone<T>()` method supports **safe object copying only if `T` is `pod_like`**, ensuring correct semantics.
- A general-purpose `copy_to` would invite unsafe use in the presence of:
  - Misaligned structures
  - Dangling backing memory
  - Incorrect size assumptions

> If you need to copy raw bytes manually, the internal layout is transparent:
> ```c++
> std::memcpy(dst, view.data, view.len);
> ```

This gives you full control without exposing risky default behavior.

---

### 🔹 `jh::pod::span<T>`

A POD-safe `std::span<T>` replacement — zero-overhead, raw-pointer-based view over typed memory.

#### ⚙️ Requirements

- `T` must be `pod_like`
- Used only for **contiguous memory blocks** (like `T[]`, `.data()`, or raw pointer)
- Not designed for non-contiguous iterators or virtual ranges

#### 🧩 Members
```c++
T* data;
uint64_t len;
```

#### 🧩 Core Methods
```c++
T& operator[](uint64_t i) const noexcept;
T* begin() const noexcept;
T* end() const noexcept;
uint64_t size() const noexcept;
bool empty() const noexcept;

span sub(uint64_t offset, uint64_t count = 0) const noexcept;
span first(uint64_t count) const noexcept;
span last(uint64_t count) const noexcept;
bool operator==(const span&) const = default; // compared by address
```

#### 🛠️ Helpers
```c++
template<typename T, std::size_t N>
span<T> to_span(T (&arr)[N]);

template<detail::LinearContainer C>
span<T> to_span(C& container);              // requires: .data() → T*, .size() → uint64_t
```

#### 💡 Example
```c++
int buffer[4] = {1, 2, 3, 4};
auto s = jh::pod::to_span(buffer);
auto tail = s.last(2); // → span to {3, 4}
```

#### 📌 Notes
- For packed or heterogeneous binary data, prefer `bytes_view`.
- This span is for **array-like memory only**.
- You cannot use this with `std::list`, `std::set`, or custom containers without `.data()`/`.size()` methods.

---

### 🔹 `jh::pod::string_view`

A non-owning, null-unsafe, raw `const char* + size` string view. Fully POD and binary-safe.

#### 🧩 Members
```c++
const char* data;
uint64_t len;

char operator[](uint64_t i) const noexcept;
const char* begin() const noexcept;
const char* end() const noexcept;
uint64_t size() const noexcept;
bool empty() const noexcept;

bool operator==(const string_view&) const noexcept; // compared by content
bool starts_with(const string_view& prefix) const noexcept;
bool ends_with(const string_view& suffix) const noexcept;
uint64_t find(char ch) const noexcept;

string_view sub(uint64_t offset, uint64_t len = 0) const noexcept;
uint64_t hash() const noexcept;
void copy_to(char* buffer, uint64_t max_len) const noexcept;
```

#### 💡 Example
```c++
jh::pod::string_view name{"abc", 3};
if (name.starts_with("a")) ...
```

---

### 🔹 `jh::pod::optional<T>`

Minimal and binary-safe `optional<T>` for POD values — doesn't construct/destruct `T`.

#### ⚙️ Requirements
- `T` must be `pod_like`

#### 🧩 Members
```c++
std::byte storage[sizeof(T)];
bool has_value;

void store(const T&) noexcept;
void clear() noexcept;

T* get() noexcept;
const T* get() const noexcept;

T& ref() noexcept;
const T& ref() const noexcept;

bool has() const noexcept;
bool empty() const noexcept;
```

#### 🧩 Factory
```c++
jh::pod::make_optional<T>(value);
```

#### 💡 Example
```c++
auto opt = jh::pod::make_optional<int>(123);
if (opt.has()) {
    int v = opt.ref();
}
```


---

## 📊 STL Optimization with `pod_like` Types

Modern compilers (Clang, GCC, MSVC) aggressively optimize STL containers like `std::vector<T>` when `T` satisfies `pod_like<T>`. At `-O2` and above:

- Construction, assignment, and destruction use `memcpy`
- No runtime type dispatch or heap penalties
- STL containers achieve equivalent performance to handwritten buffers

### ✅ Recommendation:

> **Use `jh::pod` types to define layout. Use STL containers to manage structure.**

| Use Case                   | Recommended Tool              |
|----------------------------|-------------------------------|
| Low-level layout control   | ✅ `JH_POD_STRUCT(...)`        |
| Fixed field validation     | ✅ `JH_ASSERT_POD_LIKE(...)`   |
| General container usage    | ✅ `std::vector`, `std::array` |
| Compile-time small buffers | ⚠️ `pod::array<T, N>`         |
| Runtime growable buffers   | ❌ Don't hand-roll, use STL    |

### ⚠️ Clarification:

> Types like `pod::array`, `pod::bitflags`, `pod::string_view` are not containers.  
> They are **data buffers** — layout primitives used to embed data, not manage it.

---

## ✅ Best Practices

| Task                             | Use this                |
|----------------------------------|-------------------------|
| POD struct definition            | `JH_POD_STRUCT(...)`    |
| Validate existing type           | `JH_ASSERT_POD_LIKE(T)` |
| Raw key-value structure          | `jh::pod::pair<T1, T2>` |
| Migrated tuple-style data        | `jh::pod::tuple<Ts...>` |
| Raw, fixed array buffer (≤ 16KB) | `jh::pod::array<T, N>`  |
| Dynamic data / heap use          | ❌ Not supported here    |

**On Equality Semantics and Copy Behavior :**

> `bytes_view` and `string_view` use **content-based equality**, while `span` uses **address-based equality**.

This distinction is intentional and reflects their **typical usage patterns** and **performance expectations**:

- 🔹 `bytes_view` and `string_view` are designed for **small, read-only, copy-safe memory slices**, making content comparison practical and semantically meaningful.
- 🔹 `span`, on the other hand, often points to **large or heap-allocated memory regions**, where byte-by-byte equality is costly and often meaningless.  
  For such cases, **pointer equality** (i.e., `data` and `len`) reflects intended semantics more accurately.

> ✅ If your `span<T>` is known to be small and copy-safe, you can always manually compare `data` and `len`, or the contents directly.


---

## ↔️ Byte Order Disclaimer

> The `jh::pod` system assumes **platform-local byte order** throughout.
> You must apply endian transforms manually for cross-platform/network use.

---

## 📊 Alignment Disclaimer

- `pod::array<T, N>` uses `alignas(T)`
- `pod::tuple` does **not** guarantee alignment
- Apply `alignas(...)` manually on wrappers if needed

---

## 🧬 `pod_like` and Compiler Optimizations

For `pod_like<T>` types, the compiler enables:

| Feature                     | Enabled |
|-----------------------------|---------|
| RVO / NRVO                  | ✅       |
| `constexpr` construction    | ✅       |
| Layout-compatible `memcpy`  | ✅       |
| Assignment = raw copy       | ✅       |
| Constructors/destructors    | ❌ None  |

> For POD-safe types, **`std::move()` is unnecessary** — the compiler will optimize copies via `memcpy` or in-place construction.

---

> 🚀 Build fast. Stay safe. Go POD.

📌 **For full implementation details, see [`pod.h`](../include/jh/pod.h)**  
📌 **IDE IntelliSense and tooling support most functions inline**

🚀 **Enjoy coding with JH Toolkit!**
