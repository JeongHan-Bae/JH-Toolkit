@mainpage JH-Toolkit

# JH-Toolkit

**JH-Toolkit** is an **engineering-oriented C++20 toolkit** built on:

- duck-typed concepts
- fully static, template-driven design
- zero-RTTI, header-only architecture
- concurrency-aware and allocation-conscious components

The project is designed to help developers approach **modern C++ as a software engineering tool**,  
rather than as a language-exploration or system-engineering exercise.

Its goal is to provide **strong semantics, predictable behavior, and production-oriented constraints**
while retaining the performance and control C++ is known for.

---

## What This Documentation Is

This site contains the **official API and design documentation** for JH-Toolkit.

You can start reading the documentation in two primary ways:

- **Top navigation bar** — for conceptual and high-level entry points  
- **Left navigation tree** — for hierarchical exploration of namespaces, modules, and headers

\note **Recommended approach:**  
\note Start from the forwarding header overviews (for example `<jh/async>`, `<jh/concepts>`, `<jh/ipc>`),  
\note then drill down into specific components as needed.

---

## Engineering Philosophy

JH-Toolkit follows several guiding principles:

- **Behavior over inheritance**  
  If a type behaves like a duck, it is treated as a duck.

- **Semantics before freedom**  
  APIs intentionally restrict certain forms of usage to prevent ambiguous or unsafe designs.

- **Compile-time enforcement**  
  Concepts, SFINAE, and C++20 language guarantees are used to reject invalid usage early.

- **Static by default**  
  No RTTI, no hidden runtime metadata, and no implicit dynamic behavior.

This approach allows the library to scale from **early-stage prototyping** to **long-lived production systems**
without requiring large redesigns.

---

## Why C++20

C++20 is used as a **baseline**, not as a showcase.

It enables:

- reliable value semantics (guaranteed RVO/NRVO)
- coroutine-based async composition with minimal boilerplate
- non-type template parameters (NTTPs) for compile-time identity
- expressive concepts for semantic constraints
- deeper static analysis and optimization by modern compilers (especially Clang/LLVM)

C++20 is treated here as an **engineering instrument** that constrains behavior,
reduces ambiguity, and improves system auditability.

---

## How to Read the Project

The **source of truth** for JH-Toolkit lives in:

- `include/` — all real definitions, semantics, and guarantees
- `docs/` — conceptual explanations and API overviews

Files under `src/` exist mainly as **instantiation translation units** for static builds
and should not be treated as specifications.

\note **Important:**  
\note Reading implementation alone is discouraged.  
\note The documented interface defines the behavior.

---

## IDE-Centered Documentation Design

The Doxygen documentation in this project is **primarily written for IDE-based reading**.

We strongly recommend **classic CLion** for the best experience.

### Why classic CLion is recommended

- Fully renders **standard Doxygen HTML-style comments**
- Displays documentation inline with source code
- Provides rich hover tooltips and navigable semantic views

This creates a reading experience similar to **Jupyter Notebooks**,
where documentation and code interleave naturally.

### Notes on Embedded Rendering

The documentation is intentionally authored to support embedded IDE views:


We primarily support embedded documentation views in IDEs (classic CLion).
To ensure compatibility with embedded rendering, some example code in this
documentation intentionally contains HTML entities.
In certain cases, Doxygen does not unescape these entities correctly.
This is a known limitation and currently has no workaround.


This limitation affects **HTML generation only** and does not impact IDE rendering.

---

## Scope and Intent

JH-Toolkit is **not only a production library**.

Some components are explicitly designed to:

- support early modeling and prototyping
- expose correct reasoning models (especially for concurrency)
- guide users toward scalable designs without locking them in

Other components intentionally trade generality for **operational stability**
based on real-world system constraints.

Overall, the toolkit aims to help developers:

- internalize engineering-oriented C++ thinking
- adopt modern C++20 idioms naturally
- move from prototypes to robust systems with minimal friction

---

## Author

Developed by **JeongHan-Bae**

- GitHub: https://github.com/JeongHan-Bae
- License: [Apache 2.0](https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme)

---

**Use the navigation tree on the left or the top menu to begin exploring the API.**
