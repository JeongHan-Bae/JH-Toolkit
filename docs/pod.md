# 🧩 JH Toolkit: `POD` System API Documentation

📌 **Version:** 1.3   
🕝 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## 🔍 Overview

The `jh::pod` module defines a complete system for working with **Plain Old Data (POD)-like types**, optimized for **raw
memory containers**, **placement-new**, and **low-overhead computation**.

> 🔀 Both `<jh/pod>` and `<jh/pod.h>` are supported. Prefer `<jh/pod>` for modern-style inclusion.

This module enforces memory-safe layout and guarantees:

- Zero-cost construction
- `memcpy`-safe semantics
- Standard layout
- High-performance compatibility with `jh::pod_stack`, `arena`, `mmap`, etc.

> 📌 **Target platform:** 64-bit only — `std::size_t` is avoided in favor of fixed-size types (`std::uint*_t`) for layout clarity and deterministic memory modeling.


---

## 🧠 POD Design Philosophy

- 🧱 **POD means full control over memory layout**
- 🧼 No constructors, destructors, virtual tables, hidden heap allocations
- 🔒 Enables safe serialization, memory mapping, bare-metal operation
- 🧠 All fields known at compile time → layout-stable, tooling-friendly

---

## Header Structure

| Header Path               | Contents                                                       | Typical Usage                               |
|---------------------------|----------------------------------------------------------------|---------------------------------------------|
| `<jh/pod>`                | Full utilities: `pod_like`, `JH_POD_STRUCT`, `pod::pair`, etc. | Default include for end users               |
| `<jh/pods/pod_like.h>`    | Only the `pod_like` concept                                    | For SFINAE / constraints                    |
| `<jh/pods/pair.h>`        | `pod::pair<T1, T2>` only                                       | Low-level generic usage                     |
| `<jh/pods/array.h>`       | `pod::array<T, N>` only                                        | Low-level generic usage                     |
| `<jh/pods/tools.h>`       | Macros & non-recommended types like `tuple`                    | Internal only — auto-included by `<jh/pod>` |
| `<jh/pods/string_view.h>` | `pod::string_view` only                                        | Low-level algorithmic usage                 |
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

> 🧠 A type that satisfies `pod_like<T>` can be stored in `jh::pod_stack`, copied with `memcpy`, and used safely in
`.data` segments.

---

### ⚙️ `JH_POD_STRUCT(NAME, ...)`

Define a POD struct with boilerplate-free layout checking:

```c++
JH_POD_STRUCT(MyVec2,
    float x;
    float y;
);
```

- Declares a struct with `==` operator
- Triggers a `static_assert` if not `pod_like`

---

### 📌 `JH_ASSERT_POD_LIKE(T)`

Use this macro to validate any existing struct:

```c++
struct Packet { int len; char data[8]; };
JH_ASSERT_POD_LIKE(Packet);
```

> Ensures `Packet` satisfies `pod_like`.

---

## 📦 Built-in POD Types

### 🔹 `jh::pod::pair<T1, T2>`

Minimal `std::pair` replacement:

```c++
pair<int, float> p = {1, 2.0f};
```

- Always `{ T1 first; T2 second; }`
- Fully layout-safe, `==` supported
- Drop-in for simple key-value/tuple use

---

### 🔹 `jh::pod::tuple<Ts...>` (2 to 8 elements)

Fixed-field, layout-stable tuple replacement with POD guarantees:

```c++
tuple<int, float, char> t = {1, 2.5f, 'A'};
t.get<1>() = 3.0f;
```

- Fields are named `_0` to `_7`
- Supports **2 to 8 fields only**
- Unused fields default to `std::monostate` and are optimized away
- POD-safe, layout-stable, `==` supported
- **Index-based access only** via `get<N>()`

#### 📌 Design rationale:

> 🧠 This is a **hardcoded, 8-field struct**, not a variadic pack.  
> This is **intentional**, by design — not a technical limitation.

- ✋ Unlike `std::tuple`, this type avoids recursive inheritance to **preserve `standard_layout` and POD properties**.
- ✅ We enforce **2–8 fields** — the typical range in real-world use.
- ❌ Single-field tuples are disallowed — they offer no real benefit and reduce clarity.
- ✅ The 8-field cap is deliberate — it’s sufficient for almost all algorithmic or migration use cases, and discourages
  abuse.
- 🚫 Supporting arbitrary element counts invites over-generic, unreadable "data soup" — **which this library explicitly
  prevents**.

> 🔒 This is a **deliberate design constraint** — to protect your codebase from overengineering and ensure memory layout
> safety.

---

#### ⚠️ When to use:

| Use case                                 | Recommended |
|------------------------------------------|-------------|
| Replacing `std::tuple` in hot code       | ✅ Yes       |
| Algorithmic bridging (e.g. sorting keys) | ⚠️ Maybe    |
| General-purpose data modeling            | ❌ No        |
| Anything with more than 8 fields         | ❌ Refactor  |

---

#### ✅ Best practice:

> Use `jh::pod::tuple<Ts...>` only as a **transitional or algorithmic tool**.  
> For long-term structures, prefer clear, named fields with `JH_POD_STRUCT(...)`.  
> ❌ No structured bindings (`auto [a, b] = t;` is not supported), as tuples are actually of size 8.  
> ⚠️ This is intentional: structured binding would destructure all 8 fields including unused ones. Use `get<N>()` instead.

---

#### **🧾 Deprecation Notice**

> ⚠️ `pod::tuple<Ts...>` is **not deprecated because it's broken**, but because it's a **temporary, migration-oriented utility**.

Its `[[deprecated]]` attribute is intentional — not to block you, but to **gently push you** toward explicit, long-term-safe structures.

#### ❗ Important Clarification:

- `pod::tuple` is **not a JH Toolkit legacy API** — it's a supported part of the toolkit.
- But it **is** a type designed specifically for **short-term bridging** and **algorithmic composition**.
- You are **not "done"** if you've migrated from `std::tuple` to `pod::tuple`.
- You are only "done" when you've replaced it with a clear, fixed-layout `JH_POD_STRUCT`.

> ✅ `pod::tuple` helps you survive.  
> ✅ A real POD struct helps your compiler **optimize**.

---

#### 🧠 **Why it's marked `[[deprecated]]`**

- To discourage **complacency** — it’s not a permanent model.
- To give **structured build warnings** — every use reminds you to migrate.
- To support **toolchain scans** — enabling CI and static analysis to track migration progress.
- To prevent **“POD-soup” syndrome** — where unbounded genericity leads to unreadable, unsafe data blobs.

> 🧱 True SIMD and alignment-aware optimization **requires field names and layout certainty**.  
> That means real structs, not pseudo-generic tuples.

---

#### 🔧 Best Migration Strategy

| Migration Step                   | Goal                         |
|----------------------------------|------------------------------|
| Replace `std::tuple`             | ✅ Move to `pod::tuple`       |
| Replace generic algo glue        | ⚠️ Minimize tuple use        |
| Finalize long-term struct layout | ✅ Write `JH_POD_STRUCT(...)` |
| Kill tuple                       | ✅ Zero use in production     |

---

> 🧠 Don’t stop at being better than `std::tuple`.  
> **Go all the way** to layout-defined, inspectable, cache-predictable data.

---

### 🔹 `jh::pod::array<T, N>`

POD-safe array — `std::array` layout, but:

- No constructor
- No allocator
- No `fill()`, `at()`

> "POD-safe" means satisfying `pod_like<T>`, guaranteeing trivial layout.

```c++
jh::pod::array<int, 128> a;
a[0] = 1;
```

#### ✅ Features

| Property              | Supported |
|-----------------------|-----------|
| `operator[]`          | ✅         |
| `begin()`/`end()`     | ✅         |
| POD-like              | ✅         |
| Max size (16KB total) | ✅         |
| Bounds checking       | ❌         |

#### 🔐 Compile-Time Guard

```c++
inline constexpr std::uint16_t max_pod_array_bytes = 16 * 1024;

template<typename T, std::uint16_t N>
requires pod_like<T> && (sizeof(T) * N <= max_pod_array_bytes)
```

> 💡 **Tip:** When using small, static `pod::array<T, N>`, prefer `constexpr` instead of `const`.
> ```c++
> constexpr jh::pod::array<int, 3> v = {1, 2, 3}; // Compile-time guaranteed
> ```
> This allows full compile-time layout resolution and `.rodata` optimization.


---

### 🔹 `jh::pod::string_view`

> A lightweight, non-owning, POD-safe string reference type — ideal for syntax parsing, token analysis, and zero-copy view access.

`jh::pod::string_view` is a **POD-compliant**, **non-owning string view**, representing a `(char*, length)` pair.  
It is designed for high-performance use in scenarios where **string copying is too expensive**, and memory layout must remain **flat, compact, and trivially copyable**.

Unlike `std::string` or even `std::string_view`, this view is designed to be:

- 🧱 **Plain Old Data (POD)** — trivially copyable, standard layout, safe in `.data` or `mmap`
- 💨 **Zero-overhead** — no dynamic checks, no destructor, no memory allocation
- 🚀 **Ideal for use with `pod_stack<T>`**, parsers, and DSL engines
- 🧠 **Interoperable** with `jh::immutable_str`, `std::string`, and raw `char*`

#### ✅ Fields

```c++
const char* data;
std::uint64_t len;
```

> ⚠️ `pod::string_view` does **not** manage or check the lifetime of `data`.  
> You must ensure the backing memory remains valid for the lifetime of the view.

---

#### 🔍 Core Methods

| Method                                | Description                                        |
|---------------------------------------|----------------------------------------------------|
| `operator[](i)`                       | Returns character at index `i` (⚠️ unchecked)      |
| `size()` / `empty()`                  | Length and zero-check                              |
| `begin()` / `end()`                   | Raw pointer range for iteration                    |
| `sub(offset, length = 0)`             | Creates a subview; `length=0` means till end       |
| `compare(rhs)`                        | Lexicographical compare (ASCII `memcmp` style)     |
| `starts_with(rhs)` / `ends_with(rhs)` | Prefix/suffix match, ASCII                         |
| `find(ch)`                            | Index of first occurrence, or `-1`                 |
| `hash()`                              | FNV-1a 64-bit hash (compile-time optimized)        |
| `copy_to(buf, max_len)`               | Null-terminated debug copy into buffer (⚠️ unsafe) |

---

#### 📌 Lifetime & Safety Rules

⚠️ This is a **non-owning, unsafe-by-default** view.  
Use only when the **underlying buffer is guaranteed alive** — such as:
- Static literals (`static const char*`)
- `immutable_str`
- `std::string` with known scope
- `arena`, `pool`, or other stable backing

`pod::string_view` **does not own memory**.  
It behaves like a C pointer — fast, but unsafe if misused.

| Source                     | Safe? | Notes                                                 |
|----------------------------|-------|-------------------------------------------------------|
| `std::string`              | ✅     | Only if string outlives the view                      |
| `const char*` (manual)     | ✅     | Must be stable and null-free                          |
| `std::string_view`         | ✅     | If tied to long-lived memory                          |
| **Returned from function** | ❌     | ⚠️ May dangle — only return views if source is static |
| `"literal"` (inline)       | ⚠️    | Technically safe (static), but not recommended        |
| `jh::immutable_str`        | ✅     | Safest — use `.pod_view()` to extract                 |

---

#### ✅ Example: Safe Usage

```c++
std::string buffer = "hello world";
pod::string_view view = { buffer.c_str(), buffer.size() }; // ✅ safe if buffer lives
```

Or:

```c++
auto imm = jh::make_atomic("hello");
pod::string_view view = imm->pod_view(); // ✅ guaranteed non-dangling
```

---

#### ⚠️ Example: Unsafe (Dangling)

```c++
pod::string_view view = {"inline", 6}; // ❌ literal — technically safe, but misleading
return {"tmp", 3};                     // ❌ dangling view from temporary
```

---

#### ✅ Best Use Cases

- ✅ SQL / DSL parsing
- ✅ Syntax token view in interpreters
- ✅ Cache keys with memory-pool backing
- ✅ Line-by-line log parsing
- ✅ High-frequency `emplace_back()` into `jh::pod_stack<pod::string_view>`

---

### 🧠 Tip: Use With `immutable_str` for Safety

If you need a safe, non-dangling source of immutable string data:

```c++
auto imm = jh::make_atomic("SELECT * FROM table;");
auto tok = imm->pod_view().sub(7, 1); // ✅ extracts "F"
```

---

#### 📐 Size & ABI

- Always **16 bytes** on 64-bit platforms
- Perfectly aligned for use in:

  - `pod_stack<string_view>`
  - `data_sink<string_view>`
  - Embedded POD containers

> 💡 Designed for serialization, placement-new, and safe copying via `memcpy`.

---

#### 📛 Reminder

> **Do not** return `pod::string_view` unless you're certain the data outlives the view.  
> It's a view, not a value.

---

### 🔹 `jh::pod::optional<T>`

> A layout-stable, trivially copyable alternative to `std::optional<T>`  
> for use in POD-only memory systems and compile-time optimized containers.

```c++
pod::optional<int> x;
x.store(42);
if (x.has()) return x.ref();
```

- ✅ Fully POD: no constructor, destructor, or heap
- ✅ Memory-safe: `memcpy`/`zero_init` compatible
- ✅ Ideal for use with `pod_stack`, `runtime_arr`, or serialization buffers
- 🚫 Does **not** support default value construction — you must `.store()` manually
- ⚠️ Accessors (`ref()` / `get()`) are unchecked — always check `.has()`

#### ✅ Fields

```c++
std::byte storage[sizeof(T)];
bool has_value;
```

- `storage` holds raw value bits
- `has_value` indicates presence of value
- No constructors, no lifetime management — your code **must** handle state

#### ✅ Methods

| Method                | Description                                      |
|-----------------------|--------------------------------------------------|
| `.store(const T&)`    | Copies a value into internal storage             |
| `.clear()`            | Marks the value as empty (no destructor called)  |
| `.has()` / `.empty()` | Checks if value is set or not                    |
| `.get()`              | Returns raw pointer (unchecked!) to stored value |
| `.ref()`              | Returns reference to stored value (unchecked)    |

#### ⚠️ Layout & ABI

| Property               | Value                          |
|------------------------|--------------------------------|
| `sizeof(optional<T>)`  | `sizeof(T) + 1` (plus padding) |
| `alignof(optional<T>)` | `alignof(T)`                   |

Example:

```c++
static_assert(sizeof(pod::optional<std::uint32_t>) == 8);
static_assert(alignof(pod::optional<std::uint32_t>) == 4);
```

---

#### ✅ Best Use Cases

- ✅ Low-level systems where `std::optional` introduces unwanted constructors
- ✅ POD containers (`pod_stack<T>`, `runtime_arr<T>`)
- ✅ SIMD-friendly or cache-sensitive systems that demand fixed layout
- ✅ High-performance token streams or sparse table entries

---

#### 🧠 Constructor-Free Semantics

Unlike `std::optional`, `pod::optional` is **zero-initialized POD** — no construction occurs at runtime:

```c++
pod::optional<MyT> x;        // contains nothing
x.store(value);              // manually stored
x.ref();                     // returns T& (unsafe if empty)
```

If you want to construct inline:

```c++
auto o = pod::make_optional(42);
```

---

#### 🔐 Safety Contract

- 🧠 **Must** call `.has()` before accessing `.ref()` or `.get()`
- 🚫 Does **not** call `T` constructor — memory is raw
- 🚫 No structured bindings / monadic access (`?`, `.value()`, etc.)
- ✅ Optimized for zero-overhead memory placement and copying

---

#### ✅ Factory Function

```c++
auto o = jh::pod::make_optional(value);
```

Creates a filled optional without triggering default constructor. Internally:

```c++
optional<T> o;
o.store(value);
return o;
```

> 🔒 Use `pod::optional<T>` only when you need **full layout control and compile-time guarantees**.  
> If you're modeling nullable logic in normal business logic — use `std::optional`.

---

## 🧪 Validation Examples

```c++
JH_POD_STRUCT(Vec3,
    float x;
    float y;
    float z;
);

static_assert(jh::pod::pod_like<Vec3>);
static_assert(jh::pod::pod_like<jh::pod::pair<int, int>>);
static_assert(jh::pod::pod_like<jh::pod::array<std::uint8_t, 32>>);
```

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

---

### 📛 Byte Order Disclaimer

> 📢 **Endian Disclaimer:**

The `jh::pod` system assumes **platform-local byte order** throughout.

- No automatic endian conversion is provided
- No annotations for field-level endianness
- No runtime checks for host/target mismatch

> ⚠️ **If you're building cross-platform serialization or network protocols, you are responsible for defining and
applying the appropriate byte order transformations manually.**

---

### 📐 Alignment Disclaimer

The `jh::pod` module assumes **platform-default alignment rules** unless explicitly overridden.

- `pod::array<T, N>` uses `alignas(T)` — safe for raw memory operations
- `pod::string_view` is always 16 bytes on 64-bit systems — naturally aligned
- `pod::pair<T1, T2>` alignment depends on member types — use `alignas(...)` on the wrapper struct if needed
- `pod::tuple<Ts...>` does **not** provide alignment guarantees — due to monostate stripping and hardcoded layout

> 🧠 **If your application depends on explicit alignment (e.g. SIMD, memory mapping, cache line control), use `alignas(...)` on your own wrapper structs.**

Example:

```c++
struct alignas(64) AlignedVec {
    jh::pod::array<float, 16> data;
};
```

> 🚫 The `jh::pod` system does **not enforce alignment** — this is by design, to avoid hidden layout surprises.

---

> 🚀 Build fast. Stay safe. Go POD.

📌 **For detailed module information, refer to [`pod.h`](../include/jh/pod.h).**  
📌 **For documentation of jh::pod_stack, see [`pod_stack.md`](pod_stack.md).**  
📌 **Function-specific documentation is available directly in modern IDEs.**

🚀 **Enjoy coding with JH Toolkit!**
