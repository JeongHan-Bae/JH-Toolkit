# 🏆 JH Toolkit: `POD` System API Documentation

📌 **Version:** 1.3.x   
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
  - [`pod::bytes_view`](#-jhpodbytes_view)
  - [`pod::span`](#-jhpodspant)
  - [`pod::string_view`](#-jhpodstring_view)
  - [`pod::optional`](#-jhpodoptionalt)
- [📚 POD Serialization & Stringify](#-pod-serialization--stringify)
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

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A POD-compatible replacement for `std::pair`, designed for raw memory use and binary-safe operations.  
It integrates with the `jh::pod::stringify` utilities for easy serialization.

---

#### ⚙️ Requirements
- `T1`, `T2` must satisfy `pod_like<T>`

---

#### 🧩 Members
```c++
T1 first;
T2 second;

using first_type  = T1;  // Compatible with std::pair<T1, T2>
using second_type = T2;  // Compatible with std::pair<T1, T2>

constexpr bool operator==(const pair&) const = default;
````

---

#### 💡 Example

```c++
jh::pod::pair<int, float> p{42, 3.14f};
std::cout << p;  // ✅ Output: {42, 3.14}
```

---

#### 🧠 Design Notes

* POD layout guarantees: trivially copyable, standard layout
* `first_type` and `second_type` are provided to allow **template-based usage identical to `std::pair`**
* Safe for use in:

  * `memcpy` / binary serialization
  * `mmap` memory mapping
  * custom allocators and raw buffers
* For semantically meaningful types → prefer named structs via `JH_POD_STRUCT(...)`

</div>
</details>
```

---
### ⚠️ `jh::pod::tuple<Ts...>` (transitional)

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **POD-safe, fixed-field tuple replacement**, meant **only as a migration tool** from `std::tuple` to proper `POD` structs.  
Supports up to 8 fields, but is **deprecated** in favor of `JH_POD_STRUCT(...)`.

---

#### 🧭 Intended Purpose
- ✅ Quick bridge from `std::tuple<Ts...>`
- ✅ Provides trivial layout and binary safety
- ✅ Supports simple positional unpacking via `.get<N>()`
- ❌ No structured binding (`auto [a, b] = t;`)
- ❌ No variadic template expansion or advanced metaprogramming
- ❌ Not semantically meaningful — fields are `_0, _1, ...`

> ⚠️ **In v1.3.2** we improved template and stringify behavior for other `pod` types (`pair`, `array`, etc.),  
> but `tuple` is **explicitly excluded** — it will never gain those features.  
> `std::tuple`’s real value lies in complex template machinery, but its nested-inheritance design **breaks POD guarantees**.  
> `jh::pod::tuple` is kept minimal: it offers only **fixed layout** and **basic unpacking**.

---

#### ⚙️ Requirements
- `T1`, `T2` must be `pod_like`
- Optional fields `T3`–`T8` may be `jh::typed::monostate` or `pod_like`

---

#### 🧩 Members
```c++
T1 _0; T2 _1; T3 _2; ...; T8 _7;

template<uint8_t V> Tn& get();       // Indexed access: get<0>(), get<1>(), ...
constexpr bool operator==(const tuple&) const = default;
````

---

#### 💡 Example

```c++
// Transitional usage only
jh::pod::tuple<int, float, char> t{1, 3.0f, 'x'};
t.template get<1>() += 1.5f; // Access second element
```

---

#### 🚫 Long-Term Guidance

* Replace with `JH_POD_STRUCT(...)` for semantic clarity and named fields.
* Treat `jh::pod::tuple` as a **temporary shim**, not a core feature.

</div>
</details>

---

### 🔹 `jh::pod::array<T, N>`

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **POD-compatible fixed-size array**, designed as a lightweight alternative to `std::array`,  
but with stricter guarantees: fully inline, trivially copyable, and limited to **≤ 16 KB**.

---

#### ⚙️ Requirements
- `T` must satisfy `pod_like`
- `sizeof(T) * N <= 16 * 1024` (max 16 KB, compile-time checked)

---

#### 🧩 Members
```c++
T data[N];  // Inline storage

// STL-compatible aliases
using value_type      = T;
using size_type       = std::uint16_t;
using difference_type = std::ptrdiff_t;
using reference       = value_type&;
using const_reference = const value_type&;
using pointer         = value_type*;
using const_pointer   = const value_type*;

// Element access
constexpr reference operator[](std::size_t i) noexcept;
constexpr const_reference operator[](std::size_t i) const noexcept;

// Iterators
constexpr pointer begin() noexcept;
constexpr const_pointer begin() const noexcept;
constexpr pointer end() noexcept;
constexpr const_pointer end() const noexcept;

// Capacity
[[nodiscard]] static constexpr size_type size() noexcept;

// Comparison
constexpr bool operator==(const array&) const = default;
````

---

#### 💡 Examples

```c++
jh::pod::array<int, 3> nums = {{1, 2, 3}};
std::cout << nums; 
// Output: [1, 2, 3]
```

```c++
jh::pod::array<char, 8> str = {{'H','e','l','l','o','\n','!','\0'}};
std::cout << str; 
// Output: "Hello\n!"
```

---

#### 🖨️ Stream Output Behavior

* **Generic `array<T, N>` (non-char)**

  * Serialized as JSON-like array:

    ```
    [elem0, elem1, elem2, ...]
    ```

* **Specialized `array<char, N>`**

  * Serialized as **escaped JSON string**:

    * Printable ASCII → unchanged
    * Control characters (`\n`, `\t`, etc.) → escaped
    * Non-ASCII → encoded as `\uXXXX`

Example:

```c++
jh::pod::array<char, 5> data = {{'A', '"', '\\', '\n', '\0'}};
std::cout << data;
// Output: "A\"\\\n"
```

---

#### 🚫 Limitations Compared to `std::array`

* No `.at()`, `.fill()`, `.swap()`
* No bounds checking
* Hard 16 KB limit → not for large or heap-like buffers

---

</div>
</details>

---

### 🔹 `jh::pod::bitflags<N>`

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **POD-compatible fixed-size bitfield**, designed as a low-level replacement for `std::bitset`,  
but with deterministic memory layout, no heap, and constexpr-friendly semantics.

---

#### ⚙️ Requirements
- `N` must be divisible by **8**
- `N <= 32,768` (≤ 4 KB total)

---

#### 📚 Variants
- **Native backend** (`N = 8, 16, 32, 64`)
  - Stored in `uint{N}_t bits;`
  - Follows **platform endianness** in memory

- **Byte-array backend** (`N` other multiples of 8)
  - Stored in `uint8_t data[N / 8];`
  - Stored in **explicit little-endian order** (byte-by-byte)

> ⚠️ Do **not** reinterpret-cast `.bits` or `.data[]`.  
> Always use `to_bytes()` / `from_bytes()` for snapshots.

---

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
````

---

#### 🧩 Serialization

```c++
jh::pod::array<uint8_t, N / 8> to_bytes(bitflags<N>);   // snapshot (always LE)
bitflags<N> from_bytes(jh::pod::array<uint8_t, N / 8>); // restore
```

* `to_bytes` always produces **little-endian snapshots**, even for native backends.
* `from_bytes` reconstructs from a snapshot buffer.

---

#### 🖨️ Stream Output (`operator<<`)

* Output format depends on **ostream basefield**:

  * `std::hex` → hexadecimal string with `0x'..'` wrapper
  * (default / `std::dec`) → binary string with `0b'..'` wrapper

```c++
jh::pod::bitflags<16> f{};
f.set(3);
std::cout << f;           // 0b'0000000000001000'
std::cout << std::hex << f; // 0x'0008'
```

---

#### 💡 Example

```c++
jh::pod::bitflags<16> f{};
f.set(3);

if (f.has(3)) {
    f.flip(3);
}

auto snapshot = jh::pod::to_bytes(f);
auto restored = jh::pod::from_bytes<16>(snapshot);
```

---

#### 📌 Endianness Summary

* `.bits` (native) → platform endianness
* `.data[]` (byte array) → always little-endian
* Snapshots (`to_bytes` / `from_bytes`) → **always little-endian**, safe for serialization and mmap

</div>
</details>

---

### 🔹 `jh::pod::bytes_view`

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **low-level, read-only, POD-safe wrapper** over raw memory regions.  
Designed for **binary parsing**, **protocol headers**, and **typed deserialization** of mapped memory, while avoiding unsafe `reinterpret_cast` or manual `memcpy`.

---

#### ⚙️ Requirements
- Memory must remain valid externally (non-owning view).
- Type `T` for interpretation must satisfy:
  - `trivial_bytes<T>` for `.from`, `.at`, `.fetch`
  - `pod_like<T>` for `.clone`

---

#### 🧩 Members
```c++
const std::byte* data;   // pointer to raw memory
uint64_t len;            // length in bytes
````

---

#### 🧩 Core Functions

```c++
template<trivial_bytes T>
static bytes_view from(const T& obj);              // view of a single object

template<trivial_bytes T>
static bytes_view from(const T* arr, uint64_t n);  // view of an array (size in elements)

template<trivial_bytes T>
const T& at(uint64_t offset = 0) const;            // reinterpret (unsafe, unchecked)

template<trivial_bytes T>
const T* fetch(uint64_t offset = 0) const;         // safe fetch, nullptr on OOB

template<pod_like T>
T clone() const;                                   // safe POD copy (default on size mismatch)

[[nodiscard]] constexpr uint64_t
hash(jh::utils::hash_fn::c_hash = fnv1a64) const noexcept;
                                                   // raw byte hash
bool operator==(const bytes_view&) const noexcept; // content equality
```

---

#### 🖨️ Stream Output (`operator<<`)

* Printed as **base64-encoded** string with `base64'..'` wrapper
* Ensures raw binary memory can be logged / serialized safely

```c++
Packet pkt{123, 0b0101};
auto view = jh::pod::bytes_view::from(pkt);
std::cout << view;  
// → base64'ewAAAAUAAAA='
```

---

#### 💡 Example

```c++
struct Packet { uint32_t id; uint8_t flags; };
JH_ASSERT_POD_LIKE(Packet);

Packet pkt{123, 0b0101};
auto view = jh::pod::bytes_view::from(pkt);

// reinterpreting
auto& id_ref = view.at<uint32_t>();   // reference to id
auto* flags_p = view.fetch<uint8_t>(4); // safe, may be nullptr

// cloning
Packet copy = view.clone<Packet>();   // deep POD copy

// hashing
auto h = view.hash(); // fnv1a64 by default
```

---

#### 🧠 Design Intent

* ✅ Provides a **zero-cost abstraction** over raw memory (`const std::byte* + size`).
* ✅ Encapsulates `reinterpret_cast` + `std::launder` in a **safe template interface**.
* ✅ Makes **binary parsing and cloning trivial** without `memcpy` boilerplate.

Instead of:

```c++
auto* hdr = reinterpret_cast<const Packet*>(ptr);
Packet copy;
std::memcpy(&copy, ptr, sizeof(Packet));
```

You can simply write:

```c++
auto hdr = view.at<Packet>();
auto copy = view.clone<Packet>();
```

---

#### ⚠️ Safety Notes

* **No bounds checking** in `.at<T>()` — use `.fetch<T>()` for safety.
* **No endian correction** — assumes platform-local byte order.
* Caller must ensure:

  * Memory lifetime ≥ view lifetime
  * Proper alignment for `T`
  * Correct `len` matching `sizeof(T)` for `.clone<T>()`


</div>
</details>

---

### 🔹 `jh::pod::span<T>`

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **POD-safe replacement for `std::span<T>`**, designed for zero-overhead views over contiguous memory.
Unlike `std::span`, this version is **strictly POD** (`T* + uint64_t`), making it safe for raw memory containers, MMAP’d buffers, and serialization.

---

#### ⚙️ Requirements

* `T` must be `pod_like`
* Must only reference **contiguous memory blocks** (arrays, `.data()`, raw buffers)
* Not intended for linked or non-contiguous containers

---

#### 🧩 Members

```c++
T* data;
uint64_t len;
```

---

#### 🧩 Type Aliases (std-compatible)

`jh::pod::span<T>` provides the same type aliases as `std::span<T>`:

```c++
using element_type     = T;
using value_type       = std::remove_cv_t<T>;
using size_type        = std::uint64_t;
using difference_type  = std::ptrdiff_t;
using reference        = value_type&;
using const_reference  = const value_type&;
using pointer          = value_type*;
using const_pointer    = const value_type*;
```

> ✅ This allows generic code written for `std::span` to also recognize `jh::pod::span`.

---

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

bool operator==(const span&) const = default; // compared by address + length
```

---

#### 🛠️ Helpers

```c++
/// Create span from raw array (T[N])
template<typename T, std::size_t N>
span<T> to_span(T (&arr)[N]);

/// Create span from container with .data() + .size()
template<detail::LinearContainer C>
span<T> to_span(C& container); // works for vector, string, etc.
```

---

#### 🖨️ Printing

```c++
template<streamable T>
std::ostream& operator<<(std::ostream& os, const span<T>& sp);
```

**Output format**:

```
span<int>[1, 2, 3, 4]
```

---

#### 💡 Example

```c++
int buffer[4] = {1, 2, 3, 4};
auto s = jh::pod::to_span(buffer);

std::cout << s; 
// Output: span<int>[1, 2, 3, 4]

auto tail = s.last(2);
// → span to {3, 4}
```

---

#### 📌 Notes

* Use `bytes_view` for **packed or heterogeneous binary blobs**
* Use `span` for **typed, contiguous arrays**
* Equality is **address-based**, not content-based (unlike `string_view` or `bytes_view`)

</div>
</details>

---

### 🔹 `jh::pod::string_view`

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **POD-safe, immutable string slice** — raw `const char* + size` pair.
Unlike `std::string_view`, it is **binary-safe** and never assumes null-termination.
Designed for **arena/mmap-based storage** and **immutable string tables**.

---

#### ⚙️ Requirements

* Backing memory must outlive the view
* Data is **not null-terminated**
* Intended for immutable string storage (e.g., `immutable_str`, `arena`, `mmap`)

---

#### 🧩 Members

```c++
const char* data;
uint64_t len;
```

---

#### 🧩 Type Aliases (std-compatible)

```c++
using value_type       = char;
using size_type        = std::uint64_t;
using difference_type  = std::ptrdiff_t;
using reference        = value_type&;
using const_reference  = const value_type&;
using pointer          = value_type*;
using const_pointer    = const value_type*;
```

> ✅ These aliases mimic `std::string_view` so that generic code can interoperate.

---

#### 🧩 Core Methods

```c++
char operator[](uint64_t i) const noexcept;

const char* begin() const noexcept;
const char* end() const noexcept;

uint64_t size() const noexcept;
bool empty() const noexcept;

bool operator==(const string_view&) const noexcept; // compared by content
int compare(const string_view&) const noexcept;

bool starts_with(const string_view& prefix) const noexcept;
bool ends_with(const string_view& suffix) const noexcept;
uint64_t find(char ch) const noexcept;

string_view sub(uint64_t offset, uint64_t length = 0) const noexcept;

void copy_to(char* buffer, uint64_t max_len) const noexcept;

[[nodiscard]] constexpr std::uint64_t
hash(jh::utils::hash_fn::c_hash hash_method = jh::utils::hash_fn::c_hash::fnv1a64) const noexcept;
```

---

#### 🖨️ Printing

```c++
std::ostream& operator<<(std::ostream& os, const string_view& sv);
```

**Output format**:

```
string_view"hello"
```

---

#### 💡 Example

```c++
jh::pod::string_view name{"hello", 5};

if (name.starts_with({"he", 2})) {
    std::cout << name;  
    // Output: string_view"hello"
}

char buf[16];
name.copy_to(buf, sizeof(buf));
// buf = "hello"
```

---

#### 🧠 Design Intent

* Fully POD: `const char* + uint64_t`
* **Binary-safe**: length is explicit, not inferred from `'\0'`
* Supports equality, substring, prefix/suffix checks
* `.hash()` provides stable, fast 64-bit hashing of contents
* `copy_to()` is included for **interop/debug**, not recommended for core POD logic

---

#### ⚠️ Notes

* Use `jh::pod::string_view` for **immutable slices**
* Use `jh::pod::bytes_view` for **arbitrary binary blobs**
* Equality is **content-based**, unlike `span` which uses **address-based equality**

</div>
</details>

---

### 🔹 `jh::pod::optional<T>`

<details>
<summary>📖 Click to expand</summary>
<div markdown="1">

A **minimal, POD-safe replacement for `std::optional<T>`** — designed for raw memory and binary containers.
Unlike `std::optional<T>`, it **does not construct or destruct** values; instead, values are stored with `memcpy`.

---

#### ⚙️ Requirements

* `T` must satisfy `pod_like`
* No constructors or destructors will ever be invoked

---

#### 🧩 Members

```c++
std::byte storage[sizeof(T)];
bool has_value;
```

---

#### 🧩 Type Aliases (std-compatible)

```c++
using value_type = T;
```

---

#### 🧩 Core Methods

```c++
void store(const T& value) noexcept; // memcpy into storage
void clear() noexcept;               // mark empty

T* get() noexcept;
const T* get() const noexcept;

T& ref() noexcept;                   // undefined if empty
const T& ref() const noexcept;

bool has() const noexcept;           // true if value present
bool empty() const noexcept;         // true if no value

T value_or(T fallback) const noexcept;
```

---

#### 🧩 Factory

```c++
jh::pod::make_optional<T>(value);
```

Creates a filled optional by calling `.store(value)` internally.

---

#### 🖨️ Printing

```c++
std::ostream& operator<<(std::ostream& os, const optional<T>& opt);
```

* If `opt.has() == true` → prints the contained value
* If empty → prints `"nullopt"`

---

#### 💡 Example

```c++
auto opt = jh::pod::make_optional<int>(123);

if (opt.has()) {
    std::cout << opt;       // prints: 123
    int v = opt.ref();      // safe access
}

opt.clear();
std::cout << opt;           // prints: nullopt
```

---

#### 📏 ABI

* ABI: sizeof(optional<T>) == sizeof(T) + 1 (rounded up to alignof(T))
* Layout is trivial, copyable with `memcpy`, and safe in raw buffers
* Suitable for **`pod_stack`**, **arena allocators**, or **MMAP**

---

#### ⚠️ Notes

* **No lifetime management**: optional does not run destructors
* **Always check `.has()` before calling `.ref()`**
* Intended for **POD-only codebases** (raw serialization, binary protocols, arenas)

</div>
</details>

---

# 📚 POD Serialization & Stringify

## 1. 🖨️ `stringify` — Human-readable Debug Printing

* Located in **`jh/pods/stringify.h`**
* Provides **`operator<<`** overloads for all core POD wrappers:

  * `array<T, N>` / `array<char, N>`
  * `pair<T1, T2>`
  * `optional<T>`
  * `bitflags<N>`
  * `bytes_view`
  * `span<T>`
  * `string_view`
  * `typed::monostate`

### ✅ Purpose

* Produce **readable representations** for logging / debugging.
* Output formats are **JSON-like or descriptive**, e.g.:

  * `array<int,3>{1,2,3}` → `[1, 2, 3]`
  * `array<char,8>{"hi"}` → `"hi"`
  * `optional<int>{42}` → `42`, empty → `nullopt`
  * `bitflags<16>` → `0b'00010000'` (binary) or `0x'0010'` (hex mode)
  * `bytes_view` → `base64'AAECAw=='`
  * `string_view{"abc"}` → `string_view"abc"`

### ⚠️ Limitations

* Not stable across versions — purely for **visualization**.
* Format may change depending on stream flags (`std::hex` vs default).
* Should **not** be used for persistence or cross-system interchange.

---

## 2. 📦 `bytes_view + base64` — Stable Serialization

* Located in **`jh/pods/bytes_view.h`** + **`jh/utils/base64.h`**

### ✅ Purpose

* Convert arbitrary POD memory into **binary snapshots** and **Base64 encoding** for:

  * Persistence (files, DB, KV stores)
  * Network transport (JSON / HTTP-safe text form)
  * Debug dumps that must be restorable

### 🧩 Workflow

1. Create a raw memory view:

   ```c++
   MyPacket pkt{123, 0x5};
   auto bv = jh::pod::bytes_view::from(pkt);
   ```

2. Serialize into base64:

   ```c++
   std::string encoded = jh::utils::base64::encode(
       reinterpret_cast<const uint8_t*>(bv.data), bv.len
   );
   // → "ewAAAAUAAAA="
   ```

3. Restore from base64:

   ```c++
   std::vector<uint8_t> buffer;
   auto restored_bv = jh::utils::base64::decode(encoded, buffer);
   auto clone = restored_bv.clone<MyPacket>();
   ```

### 🧠 Design Guarantees

* **`bytes_view`** ensures type-safe reinterpretation via `.at<T>()`, `.fetch<T>()`, `.clone<T>()`.
* **`base64`** ensures transport-safe serialization:

  * Always ASCII, portable
  * No padding ambiguity (`=` is explicit)
  * Endianness is preserved as raw bytes (caller responsible if cross-arch)

---

## 3. 🔑 Difference: Printing vs Serialization

| Feature              | `stringify` (`operator<<`)       | `bytes_view + base64`   |
|----------------------|----------------------------------|-------------------------|
| Output purpose       | Debugging / visualization        | Persistence / transport |
| Format               | Human-readable (JSON-like, hex…) | Base64 text (stable)    |
| Stability            | Not guaranteed                   | Deterministic           |
| Round-trip possible? | ❌ No (lossy, formatting only)    | ✅ Yes (via decode)      |
| Performance          | Streaming to `ostream`           | Explicit encode/decode  |

---

## 4. 💡 Example

```c++
struct Packet { int id; int flag; };
JH_ASSERT_POD_LIKE(Packet);

Packet pkt{123, 5};
auto bv = jh::pod::bytes_view::from(pkt);

// Debug (human-readable)
std::cout << bv;  
// → base64'ewAAAAUAAAA='

// Persistent (restorable)
std::string encoded = jh::utils::base64::encode(
    reinterpret_cast<const uint8_t*>(bv.data), bv.len);
// Save "ewAAAAUAAAA=" into DB

std::vector<uint8_t> buf;
auto restored = jh::utils::base64::decode(encoded, buf);
Packet pkt2 = restored.clone<Packet>(); // ✅ round-trip
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
