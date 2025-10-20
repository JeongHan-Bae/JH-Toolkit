# 🧩 **JH Toolkit — Conceptual Module Overview**

📁 **Module:** `<jh/concepts>`  
📦 **Namespace** `jh::concepts`  
📍 **Location:** `jh/conceptual/`  
📅 **Version:** 1.3.x → 1.4.0-dev (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

<div align="center" style="margin-top: -32px; margin-bottom: -32px">
  <img src="https://raw.githubusercontent.com/bulgogi-framework/.github/main/res/img/Oree_smooth.svg"
       alt="Oree mascot"
       style="width: 96px; height: auto;">
</div>

## 🧭 Introduction

`jh::concepts` defines the **behavioral foundation layer** of the JH Toolkit — 
a centralized system for **duck-typed semantic recognition**.

Unlike functional modules such as `jh::async`, which extend features outward,
`jh::concepts` evolves *inward* — continuously absorbing and unifying
behavioral models across the entire toolkit.

Its design follows JH Toolkit's **duck typing philosophy**:

> 🦆 *"If a type behaves like one, it is treated as one."*

At present, `jh::concepts` provides the fundamental **iteration and sequence detection layer**
used throughout the `jh::ranges` and `jh::views` systems.
Any type that can be traversed via `range-for` and preserves state stability
qualifies as a **duck sequence**.

---

## 🔹 Core Components

| Component                 | Header                       | Status   | Description                                                                                                           |
|---------------------------|------------------------------|----------|-----------------------------------------------------------------------------------------------------------------------|
| [`iterator`](iterator.md) | `<jh/conceptual/iterator.h>` | ✅ Stable | Defines behavioral iterator concepts (`input_iterator`, `forward_iterator`, etc.) through pure expression validation. |
| [`sequence`](sequence.md) | `<jh/conceptual/sequence.h>` | ✅ Stable | Defines duck-typed sequence recognition and `jh::to_range()` bridging to `std::ranges::range`.                        |

---

## 🧩 Evolution Path

| Characteristic       | Description                                                                                                                              |
|----------------------|------------------------------------------------------------------------------------------------------------------------------------------|
| **Expansion model**  | Continuous absorption — new duck-type predicates are gradually unified here.                                                             |
| **Predictability**   | Unlike `jh::async`, `jh::concepts` does not predeclare future submodules; it grows organically as new behaviors are standardized.        |
| **Long-term vision** | Become the single, toolkit-wide "behavioral definition layer" — unifying all semantic checks (iteration, streaming, state access, etc.). |
| **Compatibility**    | Backward-compatible with all 1.3.x releases; future modules will reference `<jh/concepts>` as their canonical detection base.            |

---

## 🧩 Module Summary

`jh::concepts` defines the JH Toolkit's *behavioral semantics layer* — the unified, ever-growing foundation that
recognizes types by how they *behave*, not how they are *declared*.

---

## 🧭 Navigation

|        Resource         |                                                              Link                                                               |
|:-----------------------:|:-------------------------------------------------------------------------------------------------------------------------------:|
|  🏠 **Back to README**  |          [![Back to README](https://img.shields.io/badge/Back%20to%20README-blue?style=flat-square)](../../README.md)           |
| 📘 **Go to `iterator`** | [![Go to Iterator Reference](https://img.shields.io/badge/Go%20to%20Iterator%20Reference-green?style=flat-square)](iterator.md) |
| 📗 **Go to `sequence`** | [![Go to Sequence Reference](https://img.shields.io/badge/Go%20to%20Sequence%20Reference-green?style=flat-square)](sequence.md) |
