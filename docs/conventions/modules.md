# 📦 JH Toolkit: Module & Namespace Conventions

🧠 **Design Goal:** Semantic clarity, header hygiene, and compatibility with C++ include models.

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../../README.md)

This document outlines the mapping between **folder layout**, **C++ namespaces**, and **public headers** in the JH Toolkit.  
These mappings are **manually curated** — they follow naming conventions where possible, but favor **semantic clarity over strict patterns**.

---

## 📐 Summary Table

| Module    | Namespace   | Folder      | Public Header | Notes                                            |
|-----------|-------------|-------------|---------------|--------------------------------------------------|
| `pod`     | `jh::pod`   | `jh/pods/`  | `<jh/pod>`    | Low-level, layout-safe value types               |
| `views`   | `jh::views` | `jh/views/` | `<jh/view>`   | Lazy, allocation-free range adaptors             |
| `utility` | `jh::utils` | `jh/utils/` | _(N/A)_       | Helpers like `pair`, `span`, not yet modularized |

> 📌 The namespace-folder relationship is defined **explicitly in this table**.  
> Do not assume automatic rules like `namespace = jh::<folder>`.  
> Some modules (e.g., `pod`) deliberately break that pattern for clarity.

---

## 📚 Design Principles

- 📦 **Modular layout:** One folder = one namespace = one logical module.
- 🧼 **No transitive includes:** All headers must explicitly include what they use.
- 🧭 **Stable and predictable public headers:** Always use `<jh/module>` or `<jh/module.h>`.
- 🛠 **No header-only monoliths:** Even utility modules are structured and split by purpose.

---

## 🔒 Guidelines for Contributors

- ❌ Avoid `using namespace` in public headers.
- ❌ Do not expose internal headers (`detail/`, `impl/`) in umbrella includes.
- ✅ New modules **must be registered in this table** before being published.
- ✅ When uncertain, mirror the design of existing modules. Prefer semantic clarity over rigid naming uniformity
