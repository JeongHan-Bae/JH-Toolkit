# 🧊 **JH Toolkit — `jh::pod::bytes_view` API Reference**

📁 **Header:** `<jh/pods/bytes_view.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.5+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::bytes_view` is a **POD-safe memory observation proxy**,
representing a typed, read-only view over an arbitrary contiguous memory region.

It is composed of a **pointer and a byte length**, and provides
safe reinterpretation, binary inspection, and optional reconstruction
of trivially representable objects.

Unlike `std::ranges::range`, it is **not an iterable container** —  
`bytes_view` is a **flat memory descriptor**, allowing access by byte offset
instead of iteration.

```cpp
struct {
    const std::byte* data;
    std::size_t      len;
};
```

It never owns memory and performs no allocation.

---

## 🔹 Definition

```cpp
struct bytes_view final {
    const std::byte* data;
    std::size_t      len;
};
```

| Field  | Type               | Description                               |
|--------|--------------------|-------------------------------------------|
| `data` | `const std::byte*` | Pointer to start of the observed memory.  |
| `len`  | `std::size_t`      | Total byte length of the observed region. |

`bytes_view` is a trivially copyable POD type (`pointer + std::size_t`),
with no hidden metadata. Its size field follows the target ABI width.

---

## 🔹 Conceptual Model

All offset-based operations (`at()`, `fetch()`, etc.)
are relative to `data`,
interpreting the memory as a **flat byte sequence**.

```cpp
struct Header { uint16_t id; float value; };

Header h{1, 2.5f};
auto bv = jh::pod::bytes_view::from(h);

auto val = bv.at<float>(sizeof(uint16_t)); // reinterpret offset +2
```

Offsets are counted in **bytes**, not in source object structure.  
This design makes `bytes_view` predictable and portable
for binary or protocol inspection.

---

## 🧩 Type Concepts

### `jh::pod::trivial_bytes<T>`

A relaxed constraint for viewing trivially laid-out memory.

```cpp
template<typename T>
concept trivial_bytes =
    std::is_standard_layout_v<T> &&
    std::is_trivially_constructible_v<T>;
```

| Property                | Required | Note                     |
|-------------------------|----------|--------------------------|
| Standard layout         | ✅        | Predictable field order  |
| Trivially constructible | ✅        | Safe reinterpretation    |
| Trivially copyable      | ❌        | Optional for observation |
| Trivially destructible  | ❌        | Optional for observation |

> Allows inspection of memory that is layout-compatible
> but not strictly POD (e.g., has custom destructor).

---

### Example — Observing Non-POD Objects

```cpp
struct FileHandle {
    int fd;
    ~FileHandle() { puts("closed"); } // non-trivial destructor
};
FileHandle fh{3};

auto view = jh::pod::bytes_view::from(fh);
auto* ptr = view.fetch<int>(); // OK: safe byte-level observation
```

`FileHandle` cannot be cloned (not POD),
but it can be observed safely as raw bytes.

---

## 🧩 Safe Type Reinterpretation

`jh::pod::bytes_view` acts as a **safe reinterpretation proxy**,
internally using `std::addressof()` and `std::launder()`
to ensure all reinterpreted references are valid
under modern C++ object model semantics.

```cpp
struct Vec2 { float x, y; };
Vec2 v{1.0f, 2.0f};

auto bv = jh::pod::bytes_view::from(v);
auto& x = bv.at<float>();  // safe reinterpretation
auto  c = bv.clone<Vec2>(); // POD copy
```

> ✅ Source and target must satisfy `trivial_bytes`.
> ✅ `clone<T>()` additionally requires `T` to be `pod_like`
> and the size to match exactly.  
> ⚙️ All pointer laundering is automatic and constexpr-safe.

---

## 🧩 Behavior Overview

| Feature                       | Description                               |
|-------------------------------|-------------------------------------------|
| **Pointer + length model**    | Describes raw memory, not a container.    |
| **Non-owning**                | Never allocates or frees.                 |
| **Safe reinterpretation**     | via `from()`, `at()`, `fetch()`.          |
| **Clone support**             | For `cv_free_pod_like` types only.        |
| **Hashable**                  | Deterministic `std::size_t` content hash. |
| **Constexpr (not consteval)** | Optimizable, not compile-evaluable.       |
| **Endianness-stable**         | Operates on native raw bytes.             |

---

## 🔬 **API Breakdown — `jh::pod::bytes_view`**

### 🧩 Example Types

```cpp
// ✅ trivial_bytes: trivially laid out but not POD (has destructor)
struct TrivialBytes {
    double x;
    std::uint64_t y;
    ~TrivialBytes() { puts("~TrivialBytes"); }
};

// ✅ cv_free_pod_like: fully trivial and POD
struct PlainPod {
    std::uint64_t a;
    double b;
};
```

| Type           | Layout              | `trivial_bytes` | `pod_like` | `cv_free_pod_like` | Observable | Cloneable |
|----------------|---------------------|-----------------|------------|--------------------|------------|-----------|
| `TrivialBytes` | `double + uint64_t` | ✅               | ❌          | ❌                  | ✅          | ❌         |
| `PlainPod`     | `uint64_t + double` | ✅               | ✅          | ✅                  | ✅          | ✅         |

---

### 🔬 `from(const T& obj)`

```cpp
template<trivial_bytes T>
static constexpr bytes_view from(const T& obj) noexcept;
```

**Purpose:**  
Creates a read-only byte view of a trivially laid-out object.

| Aspect     | Description                                                    |
|------------|----------------------------------------------------------------|
| Constraint | `T` must satisfy `trivial_bytes`.                              |
| Behavior   | Produces `{ std::as_bytes(&obj), sizeof(T) }`.                 |
| Lifetime   | The view must not outlive the referenced object (no dangling). |

**Example:**

```cpp
TrivialBytes t{3.14, 42};
auto bv = jh::pod::bytes_view::from(t); // observation only
```

---

### 🔬 `from(const T* ptr, std::size_t count)`

```cpp
template<trivial_bytes T>
static constexpr bytes_view from(const T* ptr, std::size_t count) noexcept;
```

**Purpose:**
Creates a view over an array of trivially laid-out elements.

| Aspect     | Description                                                                |
|------------|----------------------------------------------------------------------------|
| Constraint | `T` must satisfy `trivial_bytes`.                                          |
| Behavior   | Produces `{ reinterpret_cast<const std::byte*>(ptr), sizeof(T) * count }`. |
| Safety     | `ptr` may be `nullptr` if `count == 0`.                                    |
| Lifetime   | Caller must ensure the underlying array remains valid.                     |

**Example:**

```cpp
std::uint32_t buf[4]{1, 2, 3, 4};
auto bv = jh::pod::bytes_view::from(buf, 4); // 16-byte observation
```

---

### 🔬 `at()`

```cpp
template<trivial_bytes T>
constexpr const T& at(std::size_t offset = 0) const noexcept;
```

**Purpose:**  
Reinterpret a subregion of the view as a `const T&`.

| Aspect     | Description                                                        |
|------------|--------------------------------------------------------------------|
| Constraint | `T` must satisfy `trivial_bytes`.                                  |
| Bounds     | **Unchecked** — caller must ensure `offset + sizeof(T) <= size()`. |
| UB         | Out-of-bounds access results in undefined behavior.                |

**Example:**

```cpp
auto& d = bv.at<double>();
auto& n = bv.at<std::uint64_t>(sizeof(double));
```

---

### 🔬 `fetch()`

```cpp
template<trivial_bytes T>
constexpr jh::meta::expected<const T*, bytes_view::error_code>
fetch(std::size_t offset = 0) const noexcept;
```

**Purpose:**
Bounds-checked variant of `at()`. Check the expected before reading `value`.

| Aspect     | Description                                         |
|------------|-----------------------------------------------------|
| Constraint | `T` must satisfy `trivial_bytes`.                   |
| Success    | `value` contains the pointer.                         |
| Failure    | `out_of_bounds` or `null_data`.                       |
| UB         | Never — always safe.                                |

**Example:**

```cpp
auto result = bv.fetch<std::uint64_t>(sizeof(double));
if (result)
    std::cout << *result.value();
```

---
Exactly — excellent point.
That detail belongs to the **semantic note** section of `clone()` because it clarifies an *advanced but safe* capability of the reinterpretation model.  
Here’s the updated, clean, final version of the `clone()` section (and its adjacent summary table) that correctly documents that feature.

---

### 🔬 `clone()`

```cpp
template<cv_free_pod_like T>
constexpr jh::meta::expected<T, bytes_view::error_code> clone() const noexcept;
```

**Purpose:**
Reconstruct a POD-compatible object by copying the entire byte range.

| Aspect     | Description                             |
|------------|-----------------------------------------|
| Constraint | `T` must satisfy `cv_free_pod_like`.    |
| Size Rule  | Only succeeds if `sizeof(T) == size()`. |
| Behavior   | Copies raw bytes into a new `T` value.  |
| Failure    | `size_mismatch` or `null_data`.          |

**Example:**

```cpp
PlainPod p{1, 2.71};
auto bv = jh::pod::bytes_view::from(p);
auto result = bv.clone<PlainPod>();
if (result) {
    auto copy = result.value(); // full POD reconstruction
}
```

**Advanced Semantics:**
`clone()` can also safely reinterpret *nested POD aggregates*
as long as both source and target are `cv_free_pod_like`
and the total byte size matches.

For example:

```cpp
// Source: nested pairs
jh::pod::array<jh::pod::pair<int, int>, 4> a{};

// Clone as a flat array of 8 ints (same total size)
auto flat = jh::pod::bytes_view::from(a)
               .clone<jh::pod::array<int, 8>>();
```

This works because `jh::pod::pair` and `jh::pod::array` are both trivially representable,
so their internal memory layout is byte-compatible and deterministic.

**Safety:**

* Only valid when both layouts are trivially representable.  
* No pointer, reference, or padding-sensitive members.  
* Still subject to `sizeof(T) == size()`.

---

### 🔬 `hash(hash_method = fnv1a64)`

```cpp
constexpr jh::meta::expected<std::size_t, bytes_view::error_code>
hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64) const noexcept;
```

**Purpose:**
Computes a deterministic hash of the observed byte region and returns it as `std::size_t`, using
the compile-time algorithms provided by [`jh::meta::hash`](../metax/hash.md).

| Aspect     | Description                                                          |
|------------|----------------------------------------------------------------------|
| Input      | Raw bytes (`data`, `len`) only — no type semantics.                  |
| Return     | Expected hash; `null_data` if the backing pointer is null.             |
| Algorithm  | Selected via `jh::meta::c_hash`.                                     |
| Default    | `fnv1a64`.                                                           |
| Evaluation | `constexpr`-enabled, but meaningful only at runtime for live memory. |

**Example:**

```cpp
#include <jh/pods/bytes_view.h>
#include <jh/metax/hash.h>

auto bv = jh::pod::bytes_view::from(buffer, 64);

// Default: FNV-1a 64-bit
auto h1 = bv.hash();
if (h1) { /* use h1.value() */ }

// Explicit: xxHash64 variant
auto h2 = bv.hash(jh::meta::c_hash::xxhash64);
```

**Design Notes:**

* Uses the same deterministic, constexpr-safe algorithms
  described in [`jh::meta::hash`](../metax/hash.md).
* Operates purely on raw memory — does not consider endianness or type identity.
* Always defined as a runtime operation semantically,
  even though the implementation is `constexpr` for optimization.
* Ideal for hashing POD data blocks, network packets, or binary protocol fields.

**Supported Algorithms:** see
👉 [`jh::meta::hash` — Core Components](../metax/hash.md#-core-components)

---

### 🔬 Memory Boundary Summary

| Function  | Bounds Checked | UB Possible | Return on Failure |
|-----------|----------------|-------------|-------------------|
| `from()`  | —              | ❌           | —                 |
| `at()`    | ❌              | ✅           | UB                |
| `fetch()` | ✅              | ❌           | `error_code`      |
| `clone()` | ✅              | ❌           | `error_code`      |

---

## 🧩 Construction and Access

```cpp
auto bv1 = jh::pod::bytes_view::from(my_pod);
auto bv2 = jh::pod::bytes_view::from(buffer, count);

auto& hdr = bv1.at<MyHeader>(0);   // unchecked
auto ptr = bv2.fetch<MyHeader>(); // check before reading ptr.value()
auto obj = bv1.clone<MyStruct>(); // check before reading obj.value()
```

---

## 🧠 Conceptual Rules

| Function           | Concept            | Purpose                                 |
|--------------------|--------------------|-----------------------------------------|
| `from()`           | `trivial_bytes`    | Build view from trivially laid-out data |
| `at()` / `fetch()` | `trivial_bytes`    | Safe reinterpretation by offset         |
| `clone()`          | `cv_free_pod_like` | Reconstruct POD object from bytes       |

Observation and reconstruction are **separated**:  
`bytes_view` describes layout, not ownership or lifetime.

---

## ⚠️ Safety and Limitations

| Constraint              | Enforcement                       |
|-------------------------|-----------------------------------|
| Memory lifetime         | Caller-managed                    |
| consteval               | ❌ not allowed                     |
| Clone size mismatch     | Returns `size_mismatch`            |
| Out-of-bounds `at()`    | Undefined behavior                |
| Out-of-bounds `fetch()` | Returns `out_of_bounds`            |
| Endianness              | Native byte order                 |
| Ownership               | Non-owning, no destructor         |

All methods are `constexpr` for optimization,
but none may be used in `consteval` evaluation
due to internal use of `reinterpret_cast` and `launder()`.

---

## 🧩 Example — Binary Inspection

```cpp
#include <jh/pods/bytes_view.h>
#include <iostream>

struct Packet { uint16_t id; uint32_t payload; };

int main() {
    Packet p{0x42, 0xAABBCCDD};
    auto bv = jh::pod::bytes_view::from(p);

    std::cout << "Bytes: ";
    for (std::size_t i = 0; i < bv.size(); ++i)
        std::cout << std::hex << static_cast<int>(bv.data[i]) << ' ';
    std::cout << '\n';
}
```

---

## 🧩 Integration Notes

* Serves as a **reinterpretation-safe, read-only memory descriptor**.  
* Ideal for low-level binary protocols, serialization, and inspection.  
* Can interoperate with `bitflags`, `array`, or `optional`.  
* Has no STL dependency or hidden state.  
* Provides debug-friendly printing and deterministic hashing.
* `constexpr` members cannot execute in `consteval` contexts —
  this is a **safety boundary**, not a limitation.  
  Functions such as `from()`, `at()`, and `clone()`
  are declared `constexpr` purely for optimization and inlining benefits,
  not for compile-time evaluation.

> 🧩 **Static Analysis Note**  
> Attempting to use these reinterpretation functions inside `consteval` contexts
> (e.g., constant expressions or compile-time object construction)
> will be **flagged by clangd’s static analysis**.
>
> Editors such as **CLion**, **VSCode**, or any IDE powered by clangd
> can detect and highlight such misuse instantly — before compilation.  
> This behavior is **intentional and correct**,
> as it defines the safe boundary between compile-time and runtime reinterpretation.
>
> Do not treat these diagnostics as errors of design:  
> they represent **clangd’s enforcement of semantic safety**,
> ensuring `bytes_view` stays within the valid, defined subset of POD reinterpretation.

---

## 🧾 Debug Stringification

The `jh::pod::bytes_view` type supports **debug-friendly printing**
when streaming helpers from

```cpp
#include <jh/pods/stringify.h>
```

are available.

When streamed to an `std::ostream`,
a `bytes_view` is rendered as a **Base64-encoded literal**:

```
base64'...encoded_bytes...'
```

For example:

```cpp
auto bv = jh::pod::bytes_view::from(obj);
std::cout << bv << '\n';
```

This output provides a **safe textual representation**
of arbitrary binary memory, suitable for inspection and logging.

If you include only:

```cpp
#include <jh/pods/bytes_view.h>
```

then the `operator<<` overload is **not defined** —
printing support requires the `stringify` header.

> ✅ **Recommended include for users:**
>
> ```cpp
> #include <jh/pod>
> ```
>
> This umbrella header automatically includes
> `<jh/pods/stringify.h>` and related helpers,
> ensuring consistent, portable, and convenient printing.

> ⚠️ **Debug-use only:**
> The Base64 output pattern (`base64'...'`) is **not a stable format**
> and may change in future releases.
> For guaranteed Base64 encoding, use
> [`jh::serio::base64::encode`](../serialize_io/base64.md) directly.

---


## 🧠 Summary

| Aspect      | Description                             |
|-------------|-----------------------------------------|
| Category    | POD memory proxy                        |
| Ownership   | Non-owning                              |
| Model       | Pointer + byte length                   |
| Observation | `trivial_bytes`                         |
| Clone       | `cv_free_pod_like` only                 |
| Hashing     | Runtime-only (constexpr, not consteval) |
| Evaluation  | `constexpr` (for optimization only)     |
| ABI         | Deterministic 2-field POD layout        |
| Printing    | Base64 literal (debug only)             |

---

> 📌 **Design Philosophy**
>
> `jh::pod::bytes_view` provides a **safe, constexpr-optimized runtime reinterpretation layer**
> for arbitrary binary data.  
> It intentionally avoids compile-time semantics such as `consteval` evaluation or symbolic hashing,
> ensuring strict runtime boundaries and predictable ABI behavior.  
>
> Its Base64 printing, hashing, and reinterpretation tools
> make it the foundation for deterministic, POD-stable memory inspection and serialization.
