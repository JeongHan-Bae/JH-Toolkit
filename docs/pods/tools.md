# 🧊 **JH Toolkit — `jh::pod::tools` Reference**

📁 **Header:** `<jh/pods/tools.h>`  
📦 **Namespace:** `jh::pod`  
📅 **Version:** 1.3.4 (code-frozen)  
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

`jh::pod::tools` is a **POD helper submodule** providing **macros and transitional types**
to simplify the creation and validation of POD-compatible data structures.  

Unlike `jh::pod::array`, `jh::pod::pair`, or `jh::pod::optional`,
this header does **not** define containers — it defines **meta-level helpers**
for enforcing POD compliance and bridging non-POD systems (like `std::tuple`)
into the plain-data ecosystem.

```cpp
#include <jh/pods/tools.h>

JH_POD_STRUCT(Point, int x; int y;);
JH_ASSERT_POD_LIKE(Point); // compile-time validation

jh::pod::tuple<int, int, jh::typed::monostate> t{{1, 2}};
```

---

## 🔹 Contents

| Symbol                     | Kind                | Description                                        |
|----------------------------|---------------------|----------------------------------------------------|
| `JH_POD_STRUCT(NAME, ...)` | Macro               | Declares a struct enforcing `pod_like` constraints |
| `JH_ASSERT_POD_LIKE(TYPE)` | Macro               | Static assertion that a type satisfies `pod_like`  |
| `jh::pod::tuple<Ts...>`    | Struct (deprecated) | Transitional POD tuple supporting up to 8 elements |

---

## 🔹 `JH_POD_STRUCT(NAME, ...)`

```cpp
JH_POD_STRUCT(MyPair,
    int x;
    int y;
);
```

**Purpose:**
Declares a struct as a **strict POD** with an automatically generated `operator==`
and compile-time validation via `jh::pod::pod_like`.  

**Guarantees:**

* Ensures standard layout, trivial copy/move/destruction.  
* Adds `constexpr bool operator==(...) const = default;`.  
* Triggers a compile-time error if any member is non-POD.  

**Example:**

```cpp
JH_POD_STRUCT(Vec2,
    float x;
    float y;
);
```

---

## 🔹 `JH_ASSERT_POD_LIKE(TYPE)`

```cpp
struct PacketHeader {
    uint16_t id;
    uint32_t length;
};
JH_ASSERT_POD_LIKE(PacketHeader);
```

**Purpose:**  
Performs a **static assertion** ensuring `TYPE` satisfies `jh::pod_like`.  

**Typical usage:**  
For manually declared structs not using `JH_POD_STRUCT`, but requiring
compatibility with `jh::pod` containers such as `pod_stack`, `bytes_view`, or `array`.

---

## 🔹 `jh::pod::tuple` (transitional)

```cpp
jh::pod::tuple<int, int, jh::typed::monostate> t{{1, 2}};
auto& x = t.get<0>();
auto& y = t.get<1>();
```

**Definition:**

```cpp
template<cv_free_pod_like T1, cv_free_pod_like T2,
         typename T3 = typed::monostate, ... up to T8>
struct [[deprecated]] tuple {
    T1 _0; T2 _1; T3 _2; ... T8 _7;
    template<std::uint8_t N> auto& get();
    constexpr bool operator==(const tuple&) const = default;
};
```

| Property                 | Description                            |
|--------------------------|----------------------------------------|
| **Maximum arity**        | 8 elements                             |
| **Element placeholders** | `jh::typed::monostate` for empty slots |
| **Access**               | `get<N>()`                             |
| **Layout**               | POD-stable (no inheritance, no vtable) |
| **Equality**             | Defaulted (`operator==`)               |
| **Construction**         | Aggregate initialization only          |
| **Deprecated**           | ✅ transitional only                    |

---

## ⚙️ Design and Transition Notes

`jh::pod::tuple` is a **temporary bridge** designed to replace uses of `std::tuple`
in memory-safe or serialization-bound systems, where strict POD layout is required.

It allows tuple-based code to migrate incrementally toward explicit structs.

| Characteristic      | Current `tuple`         | Planned Replacement (≥ 1.3.4+)                               |
|---------------------|-------------------------|--------------------------------------------------------------|
| Design goal         | Transitional POD bridge | True compositional POD type                                  |
| Max elements        | 8                       | Unlimited (variadic pack)                                    |
| Access              | `get<N>()`              | external `get<N>()`, `std::tuple_size`, `std::tuple_element` |
| Initialization      | Aggregate `{}`          | `make_tuple()` (trivial construction only)                   |
| Structured bindings | ❌                       | ✅ (planned, via ADL `get<>`)                                 |
| Location            | `tools.h`               | Will move to `tuple.h` (new design)                          |
| Status              | Code-frozen, stable     | Under active prototype research                              |

> 🧩 **Current Status (2025.10)**  
> The current `jh::pod::tuple` implementation is **code-frozen** and tested.  
> It will not be modified unless a compiler issue arises.  
> However, the design is **under research** for a **composition-based rewrite**
> (using member composition rather than inheritance).  
>
> If the new approach proves stable on both Clang and GCC,
> a full replacement will be introduced in **v1.3.4+**,
> including proper structured bindings and `make_tuple()` construction semantics.  
>
> ⚠️ Please **avoid long-term dependency** on the current `tuple`.  
> It exists to assist migration, not as a final API.

---

## 🧠 Summary

| Aspect           | Description                                                               |
|------------------|---------------------------------------------------------------------------|
| Category         | POD helper macros and transitional types                                  |
| Stability        | `JH_POD_STRUCT` / `ASSERT_POD_LIKE`: ✅ stable<br>`tuple`: ⚠️ transitional |
| ABI Model        | Pure POD (no hidden state)                                                |
| Layout           | Deterministic and trivial                                                 |
| Tuple Limit      | 8 fields maximum                                                          |
| Replacement Plan | Full redesign with structured binding and `make_tuple()`                  |
| Migration Advice | Avoid new code using `tuple`; prefer explicit POD structs                 |

---

## 🚧 Research Status — *Next-generation POD Tuple Prototype*

As of **2025.10**, the existing `jh::pod::tuple` (defined in `tools.h`)
remains **code-frozen and fully tested**, serving only as a **transitional bridge type**.  

A new **composition-based tuple model** is under **early internal experimentation**,
focused on enabling structured bindings while preserving strict POD guarantees.  
The prototype is currently **tested only on Clang**,
and its correctness or portability has **not yet been validated on GCC**.  

---

### 🧩 Development Branches and Roadmap

The prototype is being explored within the **1.3.x development line**,
which is the main branch for all POD-related enhancements.  

| Phase                      | Branch / Version | Status                   | Intent                                                                   |
|----------------------------|------------------|--------------------------|--------------------------------------------------------------------------|
| **1️⃣ Current stable**     | `main` → `1.3.3` | ✅ Released / frozen      | Stable release; current `tuple` implementation (transitional)            |
| **2️⃣ Active development** | `1.3.4-dev`      | 🚧 Experimental          | Continuing POD toolkit research and tuple redesign                       |
| **3️⃣ Forward merge path** | → `1.4.0-dev`    | 🔬 Planned (conditional) | If the new design stabilizes, it may merge directly before 1.4.0 release |

> 🧩 **Note:**  
> `1.3.4-dev` and `1.4.0-dev` are **parallel branches** —   
> all POD-related innovations originate in the `1.3.x` series
> and may be **fast-forward merged** into the upcoming major release if validated in time.

---

### 🔹 Research Focus

The ongoing prototype investigates a **composition-based implementation**
that replaces inheritance with field aggregation,
achieving `std::tuple`-like unpacking via `get<>`, `tuple_size`, and `tuple_element`,
while ensuring **trivial layout and full POD compliance**.  

Key goals include:

* **Structured binding support** (`auto& [a, b, c] = tuple;`)
* **Factory-style trivial initialization**
* **Deterministic field layout (composition, not inheritance)**
* **Strict `pod_like` and `cv_free_pod_like` compatibility**

All names, signatures, and internal structures are **unstable and subject to change**.  
No experimental tuple interface should be used outside controlled test builds.

---

### ⚠️ Developer Notice

* The current `jh::pod::tuple` is **stable but deprecated**.  
  It exists solely for migration and bridging — not for new designs.  
* The **prototype tuple** remains **experimental**,
  and **no API or symbol name is final**.  
* Once the design is validated on both **Clang** and **GCC**,
  the new model will replace the transitional version
  and move from `tools.h` to a dedicated header (`tuple.h`).

---

> 🧠 **Summary of Intent**  
>
> The tuple redesign aims to achieve full `std::tuple` interoperability
> (structured binding, element traits, and trivial construction)
> using **composition-based POD semantics**.  
> The effort is experimental, tracked within the **1.3.x development line**,
> and may be merged into **1.4.0** if confirmed stable across compilers.


---

> 📌 **Design Philosophy**
>
> `jh::pod::tools` exists to enforce **compile-time POD correctness**
> and provide safe transitional bridges away from meta-level abstractions like `std::tuple`.  
>
> The macros are permanent and stable — the `tuple` is **not**.  
> Once the composition-based model is validated across compilers,
> `tuple` will migrate to a dedicated header (`tuple.h`)
> and gain real `std::tuple`-like unpacking support with trivial construction guarantees.
