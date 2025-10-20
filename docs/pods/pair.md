# 🧊 **JH Toolkit — `jh::pod::pair` API Reference**

📁 **Header:** `<jh/pods/pair.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.3+ → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::pod::pair<T1, T2>` is a **POD-compliant two-field struct**,
representing `{T1 first; T2 second;}` with deterministic layout and no runtime overhead.  
It has no tuple interface or constructors — only plain data semantics.

---

## 🔹 Definition

```cpp
template<cv_free_pod_like T1, cv_free_pod_like T2>
struct pair final {
    T1 first;
    T2 second;

    using first_type  = T1;  ///< Alias for first element type.
    using second_type = T2;  ///< Alias for second element type.

    constexpr bool operator==(const pair&) const = default;
};
```

| Parameter | Description                                                         |
|-----------|---------------------------------------------------------------------|
| `T1`      | Must be **POD-compatible** (`pod_like`) and **not const/volatile**. |
| `T2`      | Must be **POD-compatible** (`pod_like`) and **not const/volatile**. |

> Even if `T` itself satisfies `pod_like`,
> adding `const` or `volatile` does **not** invalidate `T`,  
> but it breaks **the trivial assignability of `pair<T1, T2>` itself**,
> causing the pair to lose its POD properties.
>
> Therefore, since v1.3.3, both template parameters must be
> **plain, cv-free `pod_like` types**, ensuring that the pair as a whole remains trivially copyable and layout-stable.

---

## ⚙️ Member Summary

| Member        | Type      | Description                       |
|---------------|-----------|-----------------------------------|
| `first`       | `T1`      | First element.                    |
| `second`      | `T2`      | Second element.                   |
| `first_type`  | alias     | Type alias for `T1`.              |
| `second_type` | alias     | Type alias for `T2`.              |
| `operator==`  | constexpr | Element-wise equality comparison. |

All members are public aggregates.  
Supports structured binding, aggregate initialization, and `memcpy`-safe usage.

---

## 🧩 Initialization

`jh::pod::pair` is an aggregate type:

```cpp
jh::pod::pair<int, double> p = {42, 3.14};
auto [a, b] = p;  // structured binding
```

Nested structures are initialized naturally:

```cpp
jh::pod::pair<int, jh::pod::pair<int, int>> q = {1, {2, 3}};
```

No constructors or helpers are needed.

---

## 📦 Example

```cpp
#include <jh/pods/pair.h>
#include <iostream>

int main() {
    jh::pod::pair<int, const char*> entry = {1, "alpha"};
    auto [id, name] = entry;
    std::cout << id << " -> " << name << '\n';
}
```

**Output:**

```
1 -> alpha
```

---

## 🧾 Debug Stringification

Printing is available if `<jh/pods/stringify.h>` or `<jh/pod>` is included:

```cpp
#include <jh/pod>
#include <iostream>

int main() {
    jh::pod::pair<int, double> p = {1, 2.5};
    std::cout << p << '\n';
}
```

**Output:**

```
{1, 2.5}
```

> Debug output only.  
> To override, define a global non-inline `operator<<`,
> or bring your overload into scope with `using my_namespace::operator<<;`.  
> ADL will automatically select the visible overload.

---

## 🚀 Performance Notes

* Layout identical to `{T1 first; T2 second;}`.  
* Fully trivially copyable and trivially destructible.  
* No hidden constructors, no RTTI, no heap use.  
* Predictable memory layout — safe for binary reinterpretation.  

When stored in arrays, compilers fully unroll and prefetch because layout is contiguous.

---

## ⚠️ Safety and Constraints

| Condition                         | Enforcement                                    |
|-----------------------------------|------------------------------------------------|
| Non-POD element type              | Compile-time error (`pod_like` fails).         |
| `const` / `volatile` element type | Compile-time error (`cv_free_pod_like` fails). |
| Reinterpretation to non-POD type  | Undefined behavior.                            |

All checks happen at compile time and are visible in Clangd diagnostics.

---

## 🧩 Integration Notes

* A **lightweight data holder**, not a container.  
* Behaves like a C struct but with template-level type checking.  
* Suitable for binary serialization or inline data mapping.  
* Commonly used to express simple relationships:  
  `{x, y}`, `{offset, size}`, `{id, value}`, etc.
* Does not depend on STL or `std::tuple`.

---

## 🧠 Summary

| Aspect         | Description                        |
|----------------|------------------------------------|
| Category       | POD structure                      |
| Purpose        | Simple two-field value association |
| ABI            | Deterministic, stable              |
| Typedefs       | `first_type`, `second_type`        |
| Initialization | Aggregate (`{...}`)                |
| Printing       | via `<jh/pods/stringify.h>`        |
| Comparison     | `operator==` only                  |
| Overhead       | None (plain struct)                |

---

> 📌 **Design Philosophy**
>
> `jh::pod::pair` is not a lightweight `std::pair` clone —  
> it is a *transparent two-value aggregate* that guarantees
> POD layout, constexpr safety, and ABI determinism  
> for low-level or binary-oriented code.
