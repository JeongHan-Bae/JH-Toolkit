# 📦 JH Toolkit: Module & Namespace Conventions

🧠 **Design Goal:** Semantic clarity, header hygiene, and compatibility with C++ include models.

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../../README.md)


This document outlines the folder ↔ namespace ↔ header naming conventions used throughout the JH Toolkit.

---

## 📐 Summary Table

| Module Name | Namespace   | Folder      | Public Header  | Notes                              |
|-------------|-------------|-------------|----------------|------------------------------------|
| `pod`       | `jh::pod`   | `jh/pods/`  | `<jh/pod>`     | Low-level layout-safe types        |
| `view`      | `jh::views` | `jh/views/` | `<jh/view>`    | High-level range/view adaptors     |
| `utility`   | `jh::utils` | `jh/utils/` | `<jh/utility>` | Helpers and wrappers (e.g., pairs) |


> 📌 **Note :** `jh::utility` is not ready for using, no umbrela header is provided temporarily.

## 📚 Philosophy

- **Modular, low-overhead building blocks**
- **Zero-magic: all headers are visible and include what they use**
- **Stable namespaces mapped to stable folder paths**
- **Support both `<jh/x.h>` and `<jh/x>` for ergonomic imports**

---