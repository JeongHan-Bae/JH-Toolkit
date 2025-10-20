# 🧊 **JH Toolkit — `jh::pod::bitflags` API Reference**

📁 **Header:** `<jh/pods/bits.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.3+ → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::bitflags<N>` is a **POD-compatible fixed-size bit container**,  
representing `N` individual boolean flags in a **compact bitwise form**.

It is a compile-time–sized, trivially copyable type that stores
all bits inline — **no heap, no STL dependency, no virtual dispatch**.

Conceptually, it is a **POD version of a fixed-size `std::bitset`**,  
but with deterministic ABI, constexpr-friendly operations,
and full interoperability with binary I/O and memory mapping.

---

## 🔹 Definition

```cpp
template<std::uint16_t N>
requires (N % 8 == 0 && N <= 8 * 4096)
struct bitflags;
```

| Parameter | Description                                                       |
|-----------|-------------------------------------------------------------------|
| `N`       | Number of bits. Must be a multiple of 8 and ≤ 32,768 bits (4 KB). |

### Internal storage model

| Range of N           | Backing storage             | Implementation        |
|----------------------|-----------------------------|-----------------------|
| 8, 16, 32, 64        | Single `uint*_t`            | Native specialization |
| Other multiples of 8 | `uint8_t[N/8]` inline array | Byte-array fallback   |

> Although specializations differ internally,
> all behave identically and expose the same API.  
> Users should **not rely on internal layout** —
> access only through provided methods and serialization helpers.

---

## ⚙️ Member Summary

| Member                                        | Type / Description                                               |
|-----------------------------------------------|------------------------------------------------------------------|
| `.set(i)` / `.clear(i)` / `.flip(i)`          | Manipulate single bit (unchecked).                               |
| `.has(i)`                                     | Query bit value.                                                 |
| `.set_all()` / `.reset_all()` / `.flip_all()` | Bulk operations.                                                 |
| `.count()`                                    | Return number of bits set (popcount).                            |
| `.size()`                                     | Return total bit capacity `N`.                                   |
| Bitwise operators                             | <code>&#124;</code>, `&`, `^`, `~`, `==` supported.              |
| `.bits` / `.data`                             | Internal field (native integer or byte array). Avoid direct use. |

All operations are `constexpr`, `noexcept`, and have zero runtime overhead.
They operate purely on stack memory, never touching heap or globals.

---

## 🧩 Example — Basic Usage

```cpp
#include <jh/pods/bits.h>
#include <iostream>

int main() {
    jh::pod::bitflags<16> flags{};

    flags.set(0);
    flags.set(3);
    flags.flip(3);

    std::cout << "Bit 0: " << flags.has(0) << '\n';
    std::cout << "Count: " << flags.count() << '\n';
}
```

**Output:**

```
Bit 0: 1
Count: 1
```

---

## 🧩 Serialization Helpers

`bitflags` supports portable snapshot serialization
through its companion helpers `to_bytes()` and `from_bytes()`.

```cpp
auto bytes = jh::pod::to_bytes(flags);
auto copy  = jh::pod::from_bytes<16>(bytes);
```

* All snapshots are **little-endian**, independent of platform.
* The format is stable and safe for binary persistence.
* For native (8/16/32/64-bit) types, conversion is direct via `uint*_t`.
* For larger sizes, the byte array is copied as-is.

> Recommended for transferring flag states between processes,
> storing them in POD buffers, or embedding in binary headers.

---

## 🧩 Conceptual Model

`bitflags<N>` represents a **compile-time–bounded Boolean buffer**
that compresses bits into raw POD storage.
Unlike `std::bitset`, it guarantees:

* ✅ Trivial copy, assignment, and destruction
* ✅ No constructors, destructors, or hidden state
* ✅ Deterministic layout across compilers and architectures
* ✅ Compatibility with `memcpy`, `mmap`, and placement-new
* ✅ Full `constexpr` evaluation in C++20 and newer

Internally, `bitflags` behaves like a **pure data struct**.
Each bit corresponds to one boolean value, compacted in order.

---

## 🚀 Performance Notes

* Direct bitwise manipulation — no proxy references.
* In native specializations (`8–64` bits), operations compile
  to single machine instructions (`bts`, `bt`, `xor`, etc.).
* Larger arrays are optimized via unrolled loops or vectorized code.
* `popcount()` is implemented using compiler intrinsics
  (`__builtin_popcount*` or `std::popcount`), ensuring maximum speed.
* Memory usage is deterministic — exactly `N / 8` bytes.

Because of its POD nature, `bitflags<N>` can be embedded
inside other POD structures, unions, or buffers safely.

---

## ⚠️ Safety and Constraints

| Condition                        | Enforcement                            |
|----------------------------------|----------------------------------------|
| `N % 8 != 0`                     | Compile-time error (constraint fails). |
| `N > 32 768`                     | Compile-time error (`requires` fails). |
| Out-of-range bit access          | Undefined behavior.                    |
| Reinterpretation as non-POD type | Undefined behavior.                    |

All errors are diagnosed at compile time where possible.  
`clangd` and other Clang-based IDEs will immediately highlight invalid instantiations.

---

## 🧾 Debug Stringification

When `<jh/pods/stringify.h>` is included — or when you include the umbrella header `<jh/pod>` —
`jh::pod::bitflags<N>` automatically supports stream output (`operator<<`).

### Printing modes

| Mode                         | Example output | Description                                                      |
|------------------------------|----------------|------------------------------------------------------------------|
| **Binary (default)**         | `0b'10001010'` | Prints all bits in big-endian visual order (highest byte first). |
| **Hexadecimal (`std::hex`)** | `0x'8A'`       | Prints bytes in compact hexadecimal form, highest byte first.    |

* The output always uses single-quoted payloads (`'...'`) for clarity.  
* Output endianness follows the **internal little-endian storage order**.  
* Printing is intended **only for debugging** — not a stable serialization format.

Example:

```cpp
jh::pod::bitflags<8> f{};
f.set(1);
f.set(7);

std::cout << f << '\n';        // → 0b'10000010'
std::cout << std::hex << f;    // → 0x'82'
```

For persistence or binary interchange, prefer `to_bytes()` / `from_bytes()`.

---

## 💡 Known Issue (v1.3.3 and earlier)

In **v1.3.3 and earlier**, bitwise operators
(`|`, `&`, `^`, `~`) on **native-backed** bitflags (`8, 16, 32, 64 bits`)
return the internal `detail::bitflags_uint` type instead of the outer `bitflags<N>`.

This means expressions like:

```cpp
jh::pod::bitflags<16> a{}, b{};
auto c = a | b;  // type mismatch in v1.3.3
```

produce a valid result but with an **unexpected type**,
causing confusion or implicit conversion warnings in strict builds. (non-printable)

**Workaround (safe for all versions):**

```cpp
auto c = jh::pod::bitflags<16>(a | b);  // explicit rewrap
a &= b;                                 // direct ops are fine
std::cout << c << std::endl;            // prints normally
```

This will be corrected in **v1.3.4**, and the fix will persist in **v1.4.x+**.
The issue affects **only the operator return type**, not runtime correctness.

---

## ⚠️ Integration Notes (update)

* Designed as a **compressed POD boolean buffer**, not an STL container.
* Independent from other modules; does not rely on `array` or `pair`.
* Suitable for static flag storage, file headers, or protocol bitfields.
* Directly satisfies `pod_like`, usable in reinterpret-safe contexts.
* Compatible with `bytes_view` for safe binary reinterpretation.

> ⚠️ **Known limitation (to be fixed in 1.3.4):**  
> For native bit widths (`8, 16, 32, 64`), bitwise operators
> currently return the internal implementation type (`detail::bitflags_uint`).  
> This affects type deduction, not correctness, and will be resolved
> to always return `jh::pod::bitflags<N>` in all future versions.

---

## 🧠 Summary

| Aspect        | Description                         |
|---------------|-------------------------------------|
| Category      | POD bit container                   |
| Storage       | Inline bit buffer (no heap)         |
| ABI           | Deterministic, little-endian stable |
| Behavior      | Compile-time fixed-size flag array  |
| Range         | 8 ≤ N ≤ 32 768 (multiple of 8)      |
| Serialization | `to_bytes()`, `from_bytes()`        |
| Comparison    | `==`, bitwise ops                   |
| Overhead      | None (trivial POD)                  |

---

> 📌 **Design Philosophy**
>
> `jh::pod::bitflags` is not a reimplementation of `std::bitset`,
> but a **compile-time–bounded, POD-based bitfield abstraction**.
> It compresses Boolean sequences into fixed binary form
> while maintaining strict layout predictability and full constexpr semantics.
>
> Use it when you need a bit container that the compiler
> can reason about *as data* — not as an object.
