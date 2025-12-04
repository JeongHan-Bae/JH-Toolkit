# 🌗 **JH Toolkit — Ranges Extension Module Overview**

📁 **Module:** `<jh/ranges_ext>`  
📦 **Namespace:** `jh::ranges`  
📍 **Location:** `jh/ranges/`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

The **Ranges Extension Module** (`jh::ranges`) provides the **terminal and bridging layer**
of the JH Ranges ecosystem — a precise and expressive extension to the C++23 range framework.

It formalizes the lifecycle of range pipelines by introducing **three key adaptors**:

1. [`adapt`](./adapt.md) — *semantic promotion adaptor* for `jh::concepts::sequence`,
2. [`collect`](./collect.md) — *explicit eager materializer* for intermediate normalization,
3. [`to`](./to.md) — *deterministic structural constructor* for final container realization.

Together, they define a **clear and analyzable semantic boundary**
between *lazy evaluation* and *structural materialization*,
forming the connective layer that binds views, concepts, and containers
into a single coherent range architecture.

Unlike the standard library’s loosely defined `std::ranges::to` behavior,
the `jh::ranges_ext` module enforces explicitness, compositional clarity,
and compile-time verification for every transformation stage.

---

## 🔹 Core Components

| Component Name        | Header                  | Status   | Description                                                                            |
|-----------------------|-------------------------|----------|----------------------------------------------------------------------------------------|
| `jh::ranges::adapt`   | `<jh/ranges/adapt.h>`   | ✅ Stable | Promotes any [`sequence`](../conceptual/sequence.md) into a viewable, reentrant range. |
| `jh::ranges::collect` | `<jh/ranges/collect.h>` | ✅ Stable | Materializes and normalizes lazy or proxy-based ranges into concrete containers.       |
| `jh::ranges::to`      | `<jh/ranges/to.h>`      | ✅ Stable | Constructs target containers structurally from closable range–container pairs.         |

---

## 🔹 Design Semantics

The `ranges_ext` layer establishes **semantic continuity**
between abstract sequence types, lazy views, and concrete container realizations.  
It redefines the terminal stage of range pipelines in terms of **semantic intent** rather than syntactic convenience.

---

## 🔹 Typical Usage Flow

A canonical JH pipeline illustrates the full lifecycle:

```cpp
auto result =
    source_sequence
    | jh::ranges::adapt()
    | jh::ranges::views::transform(...)
    | jh::ranges::views::enumerate()
    | jh::ranges::collect<std::vector<Key, Val>>()
    | jh::ranges::to<std::pmr::unordered_map<Key, Val>>(
          0,
          std::hash<Key>{},
          std::equal_to<Key>{},
          alloc
      );
```

---

## 🧭 Navigation

|        Resource        |                                                             Link                                                             |
|:----------------------:|:----------------------------------------------------------------------------------------------------------------------------:|
| 🗂️ **Back to Ranges** |           [![Back to Ranges](https://img.shields.io/badge/Back%20to%20Ranges-blue?style=flat-square)](overview.md)           |
|  📘 **Go to `adapt`**  |    [![Go to Adapt Reference](https://img.shields.io/badge/Go%20to%20Adapt%20Reference-green?style=flat-square)](adapt.md)    |
| 📗 **Go to `collect`** | [![Go to Collect Reference](https://img.shields.io/badge/Go%20to%20Collect%20Reference-green?style=flat-square)](collect.md) |
|   📙 **Go to `to`**    |        [![Go to To Reference](https://img.shields.io/badge/Go%20to%20To%20Reference-green?style=flat-square)](to.md)         |
