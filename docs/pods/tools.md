# 🧊 **JH-Toolkit — `jh::pod::tools` Reference**

📁 **Header:** `<jh/pods/tools.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4+ (macro-only)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

> **Note:**
> `jh::pod::tools` is a **submodule**, not a class or type.  
> It provides **POD construction helpers and transitional bridge types**
> such as macros (`JH_POD_STRUCT`) and the temporary `jh::pod::tuple`.

---

## 🏷️ Overview

`jh::pod::tools` provides **macro-based POD helpers**
for defining and validating trivially copyable, standard-layout structures.

This header is part of the **POD foundation layer** — it defines
compile-time validation and boilerplate generation utilities,
not runtime containers or types.

From **v1.3.4** onward, `tools.h` contains **only macro helpers**.  
All tuple-related functionality has been migrated to
[`<jh/pods/tuple.h>`](tuple.md).

---

## 🔹 Contents

| Symbol                     | Kind  | Description                                          |
|----------------------------|-------|------------------------------------------------------|
| `JH_POD_STRUCT(NAME, ...)` | Macro | Defines and validates a POD-compatible struct        |
| `JH_ASSERT_POD_LIKE(TYPE)` | Macro | Asserts that an existing struct satisfies `pod_like` |

---

## 🔹 `JH_POD_STRUCT(NAME, ...)`

Defines a **strict POD struct** with automatic equality and compile-time validation.

```cpp
#include <jh/pods/tools.h>

JH_POD_STRUCT(Point,
    int x;
    int y;
);
```

### 🧩 Behavior

* Declares a plain struct with the given members.  
* Automatically generates
  `constexpr bool operator==(const NAME&) const = default;`
  (and thus `operator!=` via C++20).  
* Performs `static_assert(jh::pod::pod_like<NAME>)` immediately after definition.  
* Triggers a compile-time error if any member violates POD requirements
  (e.g., contains `std::string`, `unique_ptr`, or any non-trivial type).

### 💡 Use Case

For defining **pure data aggregates** that are trivially copyable, layout-stable,
and safely usable in POD-only containers (`pod_stack`, `array`, etc.)
without needing manual validation.

---

## 🔹 `JH_ASSERT_POD_LIKE(TYPE)`

Used to verify that an existing struct or alias is a valid POD type.

```cpp
struct Header {
    uint16_t id;
    uint32_t size;
};
JH_ASSERT_POD_LIKE(Header);
```

### 🧩 Behavior

* Performs `static_assert(jh::pod::pod_like<TYPE>)` at compile time.  
* Guarantees that the type is trivially constructible, copyable, and destructible,
  and has a standard layout.  
* Emits a compile-time diagnostic if the constraint fails.

> ⚠️ **Note:**
> `JH_ASSERT_POD_LIKE` is meant for **manually declared types**.  
> Do **not** use it immediately after `JH_POD_STRUCT`,
> as `JH_POD_STRUCT` already performs the same validation internally.

---

## 🧩 Migration Note — Tuple Relocation

The transitional `jh::pod::tuple` (available in 1.3.0–1.3.3)
was **removed from `tools.h`** starting in **v1.3.4**.  

All tuple functionality — including:

* variadic POD tuple definition,
* `make_tuple()` helper,
* ADL-based `get<I>` for structured bindings,
* and interoperability with `std::tuple_size` / `std::tuple_element` —
  has been moved to:

📦 **Header:** [`<jh/pods/tuple.h>`](tuple.md)  
🧾 **Documentation:** [tuple.md](tuple.md)  

---

## ⚙️ Example — Practical Usage

```cpp
#include <jh/pods/tools.h>
#include <cstdint>

JH_POD_STRUCT(Packet,
    uint16_t id;
    uint32_t length;
);
```

This defines a completely trivial `Packet` type, suitable for
binary I/O, memory mapping, or embedded storage.
No constructors, no RTTI, and no hidden ABI state.

---

## 🧠 Summary

| Aspect        | Description                                    |
|---------------|------------------------------------------------|
| Category      | POD helper macros                              |
| Tuple support | ❌ (moved to `<jh/pods/tuple.h>`)               |
| ABI           | Deterministic, pure POD                        |
| Validation    | Compile-time (`pod_like`, `cv_free_pod_like`)  |
| Safety        | Fully verified at compile time                 |
| Purpose       | Define or verify POD-compliant data structures |

---

> 📌 **Design Philosophy**
>
> `jh::pod::tools` provides the *lowest-level compile-time layer*
> of the JH POD ecosystem — ensuring that user-defined structures
> conform to triviality, layout, and POD safety constraints.
>
> `JH_POD_STRUCT` is the canonical way to declare new POD aggregates,
> automatically generating equality and validation.
>
> `JH_ASSERT_POD_LIKE` complements it for external or legacy structs.
>
> All compositional POD abstractions — such as
> [`tuple`](tuple.md), [`pair`](pair.md), and [`array`](array.md) —
> are implemented in their respective headers.
>
> Since **v1.3.4**, this header is **macro-only**,
> forming the foundation layer for all higher-level POD utilities.

---

✅ **Summary of Change (v1.3.4)**

| Feature                               | Before v1.3.4 | Since v1.3.4                  |
|---------------------------------------|---------------|-------------------------------|
| `JH_POD_STRUCT`, `JH_ASSERT_POD_LIKE` | ✅ Stable      | ✅ Unchanged                   |
| Transitional `tuple`                  | ⚠️ Present    | ❌ Removed                     |
| True POD `tuple`                      | —             | ✅ In `<jh/pods/tuple.h>`      |
| Structured binding support            | —             | ✅ via new `tuple`             |

---

> 🧠 **In short:**
> From **v1.3.4**, `jh::pod::tools` focuses purely on **macro-level POD enforcement**,
> while all tuple and binding capabilities now live in
> [`<jh/pods/tuple.h>`](tuple.md).  
