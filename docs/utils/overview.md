# 📦 **JH Toolkit — Utility Repository Overview**

📦 **Namespace:** `jh::utils`  
📍 **Location:** `jh/utils/`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## 🧭 Introduction

`jh::utils` is the **incubator repository** of the JH Toolkit —  
a lightweight collection of helper utilities used during internal development,
experimentation, and module prototyping.

Throughout the 1.3.x series, this repository functions as a **staging ground**
for unclassified yet practical features.  
Once a component matures, it is **migrated** to its corresponding specialized repository
(`jh::macro`, etc.).

Unlike most repositories, `jh::utils` intentionally provides **no aggregated header**.  
Each component must be explicitly included via its individual file.

---

## 🔹 Current Components

| Component                           | Header                                       | Status                       | Description                                                                                                                                                                                                                                                |
|-------------------------------------|----------------------------------------------|------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`hash_fn`](hash_fn.md)             | `<jh/utils/hash_fn.h>`                       | ✅ Stable                     | Compile-time–safe hashing utilities; includes constexpr implementations of FNV, DJB2, and SDBM for identifiers and lightweight indexing.                                                                                                                   |
| [`typed`](typed.md)                 | `<jh/utils/typed.h>`                         | ✅ Stable                     | Minimal POD-type utilities including `monostate`, a trivial "no value" placeholder compatible with POD containers.                                                                                                                                         |
| [`base64`](base64.md)               | `<jh/utils/base64.h>`                        | ✅ Stable / Planned extension | Standard Base64 serialization and deserialization utilities. The decoding table is generated at compile time, while the encoding/decoding operations run at runtime for maximum safety and performance. Base64URL support is planned for a future release. |
| [`platform`](../macros/platform.md) | `<jh/utils/platform.h>` *(forwarding alias)* | ⚠️ Transitional              | Alias for `<jh/macros/platform.h>` retained during the 1.3.x series; **deprecated and will be removed in 1.4.0.**                                                                                                                                          |

---

## 🧩 Repository Summary

The **Utility Repository** acts as a sandbox for emerging infrastructure components.  
It offers low-level building blocks that support the broader toolkit but
do not yet belong to a specialized subsystem.

* Components here are **self-contained** and **header-only**.  
* Once a utility stabilizes, it is migrated into a dedicated namespace (e.g., `jh::macro`).  
* No global header (`<jh/utils.h>`) exists — import only what you need.  
* This folder’s contents evolve frequently and may change location between minor versions.

---

## 🔹 Evolution and Migration

* Since **v1.3.0**, `platform.h` has been temporarily hosted here,
  but beginning with **v1.4.0**, it will reside solely in `jh/macros/`.  
* Other incubating utilities (e.g., data formatters, literal encoders)
  may follow a similar lifecycle.  
* All components in `jh::utils` are **safe to include**, but stability guarantees
  apply only to those explicitly marked as ✅ *Stable*.

---

## 🔹 Usage Notes

* `jh::utils` is not a formal module — it is a **development incubator**.  
* Use its headers directly, **not through aggregated includes**.  
* Code in this repository is fully constexpr-compatible in its constants and tables,
  but its algorithms execute at runtime for robustness and portability.  
* No external linkage symbols are emitted; everything is inline and header-only.

---

## 🧭 Navigation

|        Resource         |                                                                    Link                                                                    |
|:-----------------------:|:------------------------------------------------------------------------------------------------------------------------------------------:|
|  🏠 **Back to README**  |                [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)                |
| 📘 **Go to `hash_fn`**  | [![Go to Hash Function Reference](https://img.shields.io/badge/Go%20to%20Hash%20Function%20Reference-green?style=flat-square)](hash_fn.md) |
|  📗 **Go to `typed`**   |           [![Go to Typed Reference](https://img.shields.io/badge/Go%20to%20Typed%20Reference-green?style=flat-square)](typed.md)           |
|  📙 **Go to `base64`**  |         [![Go to Base64 Reference](https://img.shields.io/badge/Go%20to%20Base64%20Reference-green?style=flat-square)](base64.md)          |
| 📘 **Go to `platform`** |  [![Go to Platform Reference](https://img.shields.io/badge/Go%20to%20Platform%20Reference-gray?style=flat-square)](../macros/platform.md)  |
