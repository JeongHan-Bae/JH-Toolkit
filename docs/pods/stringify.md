# 🧾 **JH Toolkit — `jh::pod::stringify` Reference**

📁 **Header:** `<jh/pods/stringify.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>


> **Note:**
> `jh::pod::stringify` is a **submodule**, not a class or type.  
> It provides the **debug stringification layer** for `jh::pod`,
> exposing `operator<<` overloads for human-readable visualization
> of all POD-like containers and utility types.

---

## 🏷️ Overview

`jh::pod::stringify` defines **inline stream adapters** (`operator<<`)
for all POD-compatible containers and utility types in the `jh::pod` module.  

It provides a consistent, readable, and **nestable textual form** for POD objects,
intended for **logging**, **debugging**, and **inspection** — not serialization.

```cpp
#include <jh/pod>

int main() {
    auto p = jh::pod::pair{1, 2};
    auto opt = jh::pod::make_optional(p);
    jh::pod::array arr{{opt, jh::pod::optional<decltype(p)>{}}};
    std::cout << arr; // [{1, 2}, nullopt]
    return 0;
}
```

---

## 🔹 Purpose and Scope

* **Human-readable formatting** for `jh::pod` structures
* **Non-owning and pure streaming** (no internal allocation, except temporary Base64 buffer)
* **Fully inline, safe across translation units**
* **Overridable at global scope** via `using` or custom overloads
* **Recursively nestable** through type concepts `streamable` and `streamable_pod`

---

## 🔹 Streamability Concepts

```cpp
template<typename T>
concept streamable = requires(std::ostream &os, const T &value) {
    { os << value } -> std::same_as<std::ostream &>;
};

template<typename T>
concept streamable_pod =
    jh::pod::pod_like<T> &&
    streamable<T> &&
    !std::is_fundamental_v<T> &&
    !std::is_enum_v<T> &&
    !std::is_pointer_v<T>;
```

**Meaning:**

* Any `pod_like` type with a valid `operator<<` is printable.
* Nested printing is automatic via **ADL** resolution.
* The recursive property means any printable POD structure inside another POD
  is automatically formatted using its own stream adapter.

---

## 🔹 Output Behavior Summary

| Type                   | Output Example                | Notes                               |
|------------------------|-------------------------------|-------------------------------------|
| `array<T, N>`          | `[1, 2, 3]`                   | Recursively prints all elements     |
| `array<char, N>`       | `"escaped\\nstring"`          | Escapes control characters          |
| `pair<T1, T2>`         | `{a, b}`                      | Requires both printable types       |
| `optional<T>`          | `value` / `nullopt`           | Prints held value recursively       |
| `bitflags<N>`          | `0x'ABCD'` or `0b'0101'`      | Adapts to current stream base       |
| `bytes_view`           | `base64'...encoded_bytes...'` | Uses `jh::utils::base64::encode()`  |
| `span<T>`              | `span<int>[1, 2, 3]`          | Includes element type name          |
| `string_view`          | `string_view"hello world"`    | No escaping — literal text only     |
| `tuple<Ts...>`         | `()` / `(1,)` / `(1, 2, 3)`   | Pythonic style                      |
| `jh::typed::monostate` | `null`                        | Used for empty or placeholder types |

---

## 🔹 Overriding and Extensibility

All `operator<<` overloads are declared **`inline`**, giving them **weak linkage semantics**.  
They can safely coexist across translation units and be shadowed or replaced globally.

### ✅ To override behavior:

Define a stronger overload **at global scope**, or selectively import your preferred definitions:

```cpp
using my_project::operator<<;  // inject custom overload
```

### ⚠️ Never redefine inside `namespace jh::pod`

Doing so violates the One Definition Rule (ODR) and causes UB.  
The `jh::pod` namespace must remain untouched to preserve ABI and inline consistency.

---

## 🔹 Nesting Semantics

Printing is recursive and compositional:  
any inner POD type with a valid `operator<<` will be formatted automatically.

```cpp
auto inner = jh::pod::pair{1, 2};
auto o1 = jh::pod::make_optional(inner);
jh::pod::optional<decltype(inner)> o2{}; // empty optional
jh::pod::array arr{{o1, o2}};

std::cout << arr;
// Output: [{1, 2}, nullopt]
```

The recursive structure is resolved entirely through concept matching (`streamable_pod`).

---

## 🔹 Escaping and Base64

* **Character arrays** are JSON-escaped.
* **Binary views** (`bytes_view`) are Base64-encoded as

  ```
  base64'...encoded_bytes...'
  ```

  for visual inspection only.

> ⚠️ These are **textual diagnostics**, not serialization formats.
> For stable, versioned encoding, use [`jh::utils::base64`](../utils/base64.md) directly.

---

## 🔹 Integration Notes

* Included automatically by:

  ```cpp
  #include <jh/pod>
  ```
* Makes `operator<<` visible for all `jh::pod` containers.
* Purely inline — zero ABI footprint, no global state.
* No STL containers are printed directly — only POD-like types.
* All printers use deterministic formats for repeatable inspection.

---

## 🧠 Summary

| Aspect         | Description                                    |
|----------------|------------------------------------------------|
| Category       | Debug visualization layer                      |
| Ownership      | Non-owning, pure streaming                     |
| Inline Model   | All overloads inline, safe across TUs          |
| Overridable    | Yes — via global `using` or stronger overloads |
| Namespace Rule | Never redefine inside `jh::pod` (ODR-safe)     |
| Escaping       | Only for `array<char, N>`                      |
| Base64 Output  | For `bytes_view` (via `jh::utils::base64`)     |
| Nesting        | Recursive via `streamable_pod`                 |
| Stability      | Debug-only, not a serialization format         |
| ABI Impact     | None — header-only adapters                    |

---

> 📌 **Design Philosophy**
>
> `jh::pod::stringify` provides a **deterministic, composable textual view**
> of all `jh::pod` structures.  
> Its inline adapters expose consistent, recursive printing behavior
> for any POD-like container, ensuring clarity and readability
> without violating ABI or ODR safety.
>
> The system is intentionally human-facing — for logs, debugging, and inspection —  
> and deliberately avoids persistence semantics.
