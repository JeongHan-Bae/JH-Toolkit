# 🔭 **JH Toolkit — Views Module Overview**

📁 **Module:** `<jh/views>`  
📦 **Namespace:** `jh::views`  
📍 **Location:** `jh/ranges/views/`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)

</div>

---

## 🧭 Introduction

`jh::views` defines the **range adaptor module** of the JH Toolkit —
a header-only extension of the standard `std::views` namespace
that operates on both STL ranges and duck-typed sequences.  

All adaptors here accept any type satisfying `jh::concepts::sequence`,
and internally normalize them into valid `std::ranges::range` objects
via `jh::to_range()`.  
This ensures seamless interoperation across both standard and JH Toolkit pipelines.

---

## 🔹 Core Components

| Component                        | Header                          | Status   | Description                                                                     |
|----------------------------------|---------------------------------|----------|---------------------------------------------------------------------------------|
| [`zip(...)`](zip.md)             | `<jh/ranges/views/zip.h>`       | ✅ Stable | Lazy tuple-based adaptor combining multiple sequences or ranges.                |
| [`enumerate(...)`](enumerate.md) | `<jh/ranges/views/enumerate.h>` | ✅ Stable | Index-based adaptor equivalent to Python-style `enumerate`; built over `zip()`. |

---

## 🧩 Module Summary

`jh::views` serves as the **official range adaptor namespace**
for all view-like constructs in the JH Toolkit.  
It mirrors the naming conventions of `std::views`,
but broadens applicability to **any duck-typed sequence**.  
Every adaptor defined here returns a valid `std::ranges::range`
and composes freely with STL or JH Toolkit view pipelines.

---

## 🧩 Compatibility and Transition

| Version    | Behavior                                                                                                                                                          |
|------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **1.3.3+** | Provides both `<jh/views.h>` (**aggregated**) and `<jh/views>` (**forwarding**). The aggregated header re-exports `jh::ranges::views` for backward compatibility. |
| **1.4.0**  | `<jh/views.h>` will be **deprecated and removed**. The `<jh/views>` header becomes the canonical **aggregate-forwarding entry**.                                  |

In **1.3.3+** and following versions, the following namespace mapping is applied:

```cpp
namespace jh::views {
    using namespace jh::ranges::views;
}
```

---

## 🧭 Navigation

|           Resource            |                                                                Link                                                                |
|:-----------------------------:|:----------------------------------------------------------------------------------------------------------------------------------:|
|    🗂️ **Back to Ranges**     |            [![Back to Ranges](https://img.shields.io/badge/Back%20to%20Ranges-blue?style=flat-square)](../overview.md)             |
|    📘 **Go to `zip(...)`**    |          [![Go to Zip Reference](https://img.shields.io/badge/Go%20to%20Zip%20Reference-green?style=flat-square)](zip.md)          |
| 📗 **Go to `enumerate(...)`** | [![Go to Enumerate Reference](https://img.shields.io/badge/Go%20to%20Enumerate%20Reference-green?style=flat-square)](enumerate.md) |
