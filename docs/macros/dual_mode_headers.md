# 🔮 **JH Toolkit — Dual-Mode Header System**

📁 **Headers:** `<jh/macros/header_begin.h>`, `<jh/macros/header_end.h>`  
📦 **Namespace:** `/` *(pure macro scope)*  
📅 **Version:** 1.3.3  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Repository](https://img.shields.io/badge/%20Back%20to%20Repository-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

The **Dual-Mode Header System** is a minimal macro framework that enables a single header
to serve both **header-only** and **compiled** (static/shared) use cases
without maintaining separate `.cpp` and `.h` files.  

It provides two cooperating headers:

```cpp
#include <jh/macros/header_begin.h>
...
#include <jh/macros/header_end.h>
```

Together they allow one `.h` file to safely express declarations and definitions
depending on the current **build context**.  

> This mechanism is used by [`jh::immutable_str`](../immutable_str.md)
> and `jh::runtime_arr` to realize JH Toolkit's
> *"write once, build differently"* principle.

---

## 🔹 Core Behavior

| Mode                  | Macro                  | Effect                               | Typical Use                         |
|-----------------------|------------------------|--------------------------------------|-------------------------------------|
| *(none)*              | *(none)*               | Emit `inline` definitions            | Default header-only include         |
| **Implementation TU** | `JH_HEADER_IMPL_BUILD` | Emit non-inline (strong) definitions | Used in `.cpp` during library build |
| **Interface-only**    | `JH_HEADER_NO_IMPL`    | Emit declarations only               | Used by prebuilt/static consumers   |

`header_begin.h` automatically sets the internal macro
`JH_INTERNAL_SHOULD_DEFINE` based on these conditions,
while `header_end.h` cleans up all temporary definitions.  
Multiple inclusions are intentional and safe — the first inclusion
locks the behavior per translation unit (TU).

---

## 🔹 Correct Usage Pattern

```cpp
#include <jh/macros/header_begin.h>

// === Declarations ===
int foo_sum(int a, int b);  // always visible

// === Implementations ===
#if JH_INTERNAL_SHOULD_DEFINE
JH_INLINE int foo_sum(int a, int b) { return a + b; }
#endif  // JH_INTERNAL_SHOULD_DEFINE

#include <jh/macros/header_end.h>
```

✅ **Rules:**

* All public declarations must appear **before** the conditional block.  
* Only definitions should be wrapped by `#if JH_INTERNAL_SHOULD_DEFINE`.  
* Functions depending on user-side compile-time parameters
  should remain visible outside conditional guards.

---

## 🔹 Automatic Mode Resolution

Unlike traditional header/source separation,
this system does not require users to define any macros manually.  

### In the Library Build (author side)

Example from `immutable_str.cpp`:

```cpp
#define JH_HEADER_IMPL_BUILD
#include "jh/immutable_str.h"
```

This single inclusion generates all strong definitions for that module.  
No parallel `.cpp` logic is needed — the header itself provides the body.  
All other internal or dependent headers included later automatically respect this mode.

### In Consumer Code (user side)

Users simply write:

```cpp
#include <jh/immutable_str.h>
```

They **never** define any macros.  
CMake automatically injects the correct configuration via exported targets.

```cmake
target_compile_definitions(jh-toolkit-static INTERFACE JH_HEADER_NO_IMPL)
```

This ensures:

* The prebuilt library exposes only declarations.  
* Header-only builds remain inline and self-contained.  
* Consumers do not need to know or care about internal build modes.  

---

## 🔹 Integration with CMake

Inside JH Toolkit, both modes are exported automatically:

| Target                  | Behavior                | Definition Source       |
|-------------------------|-------------------------|-------------------------|
| `jh::jh-toolkit`        | Header-only mode        | Inline everywhere       |
| `jh::jh-toolkit-static` | Prebuilt static library | Linked definitions only |

Users select behavior simply by linking a different target —
no preprocessor macros, no duplicated headers.

This structure guarantees consistent, ODR-safe linkage across all usage scenarios.

---

## 🔹 Why It Matters

For **library authors**,
the system eliminates parallel maintenance of `.h` / `.cpp` files
and prevents desynchronization between declarations and definitions.

For **users**,
it offers a clean, zero-macro interface — behavior is automatically inferred
by which build target is linked.  

This results in:  

* Deterministic compile-time mode resolution
* Seamless interoperation between inline and prebuilt variants
* Simplified release and packaging through standard CMake exports

---

## 🔹 Embedding in Your Own Project

You can safely copy and use this system independently — it is completely standalone.

**To integrate:**

1. Copy both headers:

   ```
   jh/macros/header_begin.h  
   jh/macros/header_end.h
   ```
2. Include them around your declarations and definitions as shown above.  
3. If your project also depends on JH Toolkit,
   rename the macros (e.g. `MYLIB_HEADER_IMPL_BUILD`)
   to avoid cross-pollution with `JH_` definitions.  
4. You may modify the macros freely,
   as long as you preserve the Apache 2.0 license header at the top.

---

## 🧠 Summary

| Aspect                  | Description                               |
|-------------------------|-------------------------------------------|
| **Category**            | Build-mode coordination macros            |
| **Kind**                | Dual-header system                        |
| **Headers**             | `header_begin.h`, `header_end.h`          |
| **Namespace**           | `/` (macro scope)                         |
| **Supported Compilers** | Clang, GCC                                |
| **Include Guards**      | None (multi-inclusion intended)           |
| **Automatic Mode**      | Inferred via build context / CMake target |
| **Used By**             | `jh::immutable_str`, `jh::runtime_arr`    |
| **Standalone Use**      | Safe to embed directly                    |
| **ODR Safety**          | Guaranteed per TU mode lock               |
| **User Macros**         | None required                             |

---

> **Design Philosophy**  
> The Dual-Mode Header System embodies JH Toolkit's core idea:  
> **one header, one truth.**  
> Library authors build once with `JH_HEADER_IMPL_BUILD`;  
> consumers link and include normally — no duplication, no hidden logic.  
> The system provides deterministic, reproducible behavior
> across inline, static, and shared contexts —  
> making cross-target builds *cleaner, safer, and simpler by design.*
