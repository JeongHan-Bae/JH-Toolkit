<h1 align="center">
  <span>
    <img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" 
         alt="C++ Logo" 
         width="72" valign="middle">
  </span>
  <span style="font-size: x-large;">&nbsp;JH Toolkit</span>
</h1>

<p align="center">
  <img
    src="https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/JeongHan-Bae/JH-Toolkit/main/version_badge.json"
    alt="badge"
    width="196"
  >
</p>

<p align="center">
  <a href="https://jeonghan-bae.github.io/JH-Toolkit/">
    <picture>
      <!-- Dark mode -->
      <source
        media="(prefers-color-scheme: dark)"
        srcset="https://img.shields.io/badge/API%20Documentation-e5e7eb?logo=github&logoColor=000000"
      />
      <!-- Light mode -->
      <img
        src="https://img.shields.io/badge/API%20Documentation-black?logo=github"
        alt="API Documentation"
        width="196"
      />
    </picture>
  </a>
</p>


<div align="center" style="margin-left: 8%; margin-right: 8%; font-size: medium;">

<strong>
A modern C++20 toolkit built on duck-typed concepts and fully static, template-driven design.
<br><br>
It offers object pooling, safe immutable strings, coroutine-powered async facilities with sync-style syntax,
a lightweight POD framework, expressive metaprogramming utilities, semantically rich range/view extensions, 
inter-process coordination primitives, new associative containers, 
and stable serialization components 
<br> — entirely RTTI-free and concurrency-friendly.
</strong>
</div>
<br>
<div align="center" style="margin-left: 8%; margin-right: 8%; font-size: small;">
In other words, we aim to help you reclaim your understanding of modern C++ from the perspective of a 
"software engineer" rather than a "system programmer/language enthusiast": a language for software development that offers richer semantics, superior
performance, and enhanced security.
</div>

<div align="center">
<p></p>

<!-- ✅ Core Info -->
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](https://github.com/JeongHan-Bae/JH-Toolkit/tree/main?tab=Apache-2.0-1-ov-file#readme)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-violet.svg)](https://en.cppreference.com/w/cpp/20)
![Header-Only](https://img.shields.io/badge/header--only-supported-green)
![Static Build](https://img.shields.io/badge/static--build-supported-green)

<!-- ✅ CI / Contributors / Wiki -->
[![CI](https://github.com/JeongHan-Bae/JH-Toolkit/actions/workflows/ci.yml/badge.svg?branch=1.3.x-LTS)](https://github.com/JeongHan-Bae/JH-Toolkit/actions/workflows/ci.yml)
[![Contributors](https://img.shields.io/github/contributors/JeongHan-Bae/JH-Toolkit.svg)](https://github.com/JeongHan-Bae/JH-Toolkit/graphs/contributors)
[![Wiki](https://img.shields.io/badge/docs-wiki-blue)](https://github.com/JeongHan-Bae/JH-Toolkit/wiki)

<br>

<!-- ✅ Feature Highlights -->
![Pooling](https://img.shields.io/badge/pooling-powerful-brown)
![Immutable Strings](https://img.shields.io/badge/immutable--strings-safe-brown)
![Async](https://img.shields.io/badge/async%20%28coroutines%29-zero--boilerplate-brown)
![POD System](https://img.shields.io/badge/plain--old--data-primitive-brown)
![Duck Concepts](https://img.shields.io/badge/duck--concepts-smart-brown)
![Meta Programming](https://img.shields.io/badge/metaprogramming-compile--time-brown)
![Ranges and Views](https://img.shields.io/badge/ranges%20%26%20views-semantically--rich-brown)
![IPC](https://img.shields.io/badge/inter--process%20coordination-simple-brown)
![Serialization](https://img.shields.io/badge/serialization-stable-brown)
</div>

<div align="center">
  <img
    src="https://raw.githubusercontent.com/JeongHan-Bae/JH-Toolkit/main/docs/img/Oree.svg"
    alt="Oree mascot"
    width="128"
  >
</div>

<p align="center" style="font-size: large;">
  <strong>Let's meet our mascot — <em>Oree</em></strong>
</p>

<p align="center" style="font-size: medium;">
  <em>Oree</em>, the green-headed duck whose name comes from the Korean word "오리",<br>
  embodies the philosophy of the <strong>JH Toolkit</strong> — a world where<br>
  <strong>if it behaves like a duck, it is a duck.</strong>
</p>

<p align="center" style="font-size: medium;">
  Instead of enforcing inheritance, the toolkit follows <strong>behavioral compatibility</strong>:<br>
  every type belongs by what it <em>can do</em>, not by what it <em>inherits</em>.<br>
  <br>
  Oree represents freedom, adaptability, and elegance —<br>
  just like modern C++ itself.
</p>

---

## Why C++20 Is Used (Brief)

JH Toolkit uses **C++20** as a *baseline*, not as a showcase.

The choice is driven by the need for **foundational language guarantees and tooling support**, including:

* C++17+ guaranteed RVO/NRVO for predictable value semantics
* C++17 facilities such as `std::string_view` and `std::variant`
* C++20 coroutines for zero-boilerplate async composition
* C++20 NTTPs for compile-time identity and structural binding

More importantly, C++20 enables **SFINAE + concepts–based constraints**, which are used to:

* ensure behavior is **locally and globally predictable**
* enforce semantics that match the documented intent
* prevent invalid or ambiguous usage at compile time
* help compilers—**especially LLVM-Clang**—perform deeper analysis, pruning, and optimization

This improves both **safety** and **performance**, often by eliminating entire classes of misuse before code generation.

C++20 is therefore adopted as an **engineering tool**, not as a language feature showcase.
The goal is not to demonstrate what "modern C++" can express, but to use the language to **constrain behavior, reduce
ambiguity, and support reliable system design**.


> **Modern C++ is already a strong fit for software development workloads.**  
> Even when it is not used as the exclusive implementation language, it can reliably take responsibility for *critical
> parts* of a system—such as high-performance paths, security-sensitive components, or logic that requires strong
> semantic guarantees.  
> Modern C++ integrates cleanly with long-lived, stable interfaces such as **gRPC**, **HTTP-based services**, and
> **Cython bindings**, allowing it to coexist naturally in polyglot systems. In such architectures, it is preferable to
> rely on **typed, structured interfaces** rather than raw C ABIs, which tend to erase semantics and weaken safety and
> auditability.  
> Importantly, modern C++ enables **auditable and highly predictable systems**. Its type system, constexpr facilities,
> and concepts allow illegal or ambiguous behavior to be intercepted at compile time, rather than deferred to runtime.
> This makes it possible to enforce design intent mechanically, not socially.  
> Modern C++ also supports **safe semantic encapsulation of system-level primitives**. Low-level APIs (for example,
> POSIX IPCs) can be wrapped into higher-level, intention-revealing abstractions with explicit guarantees—such as
> `jh::ipc`, which provides a semantic, constrained interface over POSIX IPC mechanisms.  
> From a performance standpoint, modern C++ is inherently fast—*significantly faster than all dynamic languages*,
> including those using ahead-of-time compilation. This performance comes without requiring heavyweight runtimes or
> hidden dynamic metadata.  
> Finally, modern C++ operates naturally in **no-RTTI environments**, does not require additional dynamic type data, and
> avoids bloated runtime infrastructures. When combined with specialized, allocation-aware data structures (such as
> those provided by JH Toolkit), it enables low fragmentation, compact memory layouts, and predictable resource
> usage—properties
> that are essential for long-running, production-grade software systems.

---

## 🧰 Build & Platform Guide

The project builds with **GCC 13+ or Clang 15+**, with **GCC 14+ or LLVM 20** strongly recommended for optimal
performance and language support.
**MSVC is not supported.**

For complete build instructions, supported toolchains, and Conan packaging notes, refer to the
[Build & Platform Guide](./docs/build.md).

> Covers: toolchains, CMake targets, Conan `.tar.gz` releases, and the dual-mode header design.

The project has **no runtime dependencies**.  
Its **only build-time requirement** is a conforming **C++20 standard library**.  
Testing uses **Catch2**.  
For version details, see: [Dependencies](dependencies.toml).

---

## API Documentation References

Below is the complete list of **user-facing aggregate forwarding headers** provided by **JH-Toolkit**.
These headers are the **recommended entry points** for reading the API documentation.

All listed headers are **suffix-free** (for example, `<jh/async>`).
Headers ending with `.h` are **implementation headers** and are **not intended** as primary reading targets.

Each link below points to an **overview document** for the corresponding forwarding header.
From there, you can **navigate downward** into each concrete submodule and its detailed API documentation.

> In short:
> **Start from the forwarding header overview → then drill down into specific APIs as needed.**

---

### User-Facing API Entry Points

The list below is ordered **alphabetically by forwarding header name**.  
This ordering is purely for readability and navigation convenience and **does not reflect internal structure,
module hierarchy, or dependency relationships**.

Each entry links to a corresponding documentation page under the `docs/` directory.  
These pages are **built-in, quick-start–level Markdown documents** that introduce each module from a
*usage-oriented and conceptual* perspective.
They are intended to provide orientation, high-level guidance, and typical usage patterns.

The documents under `docs/` are **not exhaustive semantic specifications**.
They deliberately focus on *what to use* and *when to use it*, rather than enumerating every rule and constraint.

For detailed and complete semantics—including:

- precise class and function behavior
- initialization requirements (copy, move, aggregate, and other forms)
- template constraints and substitution rules
- documented warnings, assumptions, and corner cases

please refer to the **Doxygen documentation**, which is the authoritative source.

The Doxygen documentation can be read either:

- directly inside an IDE (with **classic CLion** as the reference environment), or
- via the official [pre-rendered web pages](https://jeonghan-bae.github.io/JH-Toolkit/).

Due to the **flat layout of the public headers** under `include/`, forwarding headers without file suffixes
(for example, `<jh/async>`) and their implementation directories (for example, `jh/asynchronous/`)
**must use different names**.  
This is a constraint imposed by the file system: a directory and a suffix-free file cannot share the same name
within a single directory.

As a result:

- **Suffix-free headers** (such as `<jh/async>`) are **aggregate or forwarding headers**
  and do **not** have a one-to-one "file-matching" documentation page.  
  Their documentation is provided explicitly in this table.
- **Concrete APIs and implementations** follow a direct mapping:  
  `include/jh/{path}/{name}.h` ↔ `docs/{path}/{name}.md`

All documentation pages are cross-linked and intentionally guide readers between
forwarding headers, concrete modules, and detailed API references.

Please rely on the **actual documentation** when understanding the library.  
Do **not** infer behavior or structure from file names alone, and avoid relying on
AI-generated guesses that are not grounded in the documented contracts.

<table>
  <tbody>
    <tr>
      <td><code>&lt;jh/async&gt;</code></td>
      <td align="center">🌀</td>
      <td><a href="docs/asynchronous/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/concepts&gt;</code></td>
      <td align="center">🧩</td>
      <td><a href="docs/conceptual/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/concurrency&gt;</code></td>
      <td align="center">🎍</td>
      <td><a href="docs/concurrent/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/flat_multimap&gt;</code></td>
      <td align="center">🧱</td>
      <td><a href="docs/core/flat_multimap.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/generator&gt;</code></td>
      <td align="center">🌀</td>
      <td><a href="docs/asynchronous/generator.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/immutable_str&gt;</code></td>
      <td align="center">🧱</td>
      <td><a href="docs/core/immutable_str.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/ipc&gt;</code> (InterProcess Coordination)</td>
      <td align="center">🛰️</td>
      <td><a href="docs/synchronous/ipc.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/jindallae&gt;</code> (Easter Egg Alias)</td>
      <td align="center">⚗️</td>
      <td><a href="docs/metax/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/meta&gt;</code></td>
      <td align="center">⚗️</td>
      <td><a href="docs/metax/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/ordered_map&gt;</code></td>
      <td align="center">🧱</td>
      <td><a href="docs/core/ordered_map.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/pod&gt;</code></td>
      <td align="center">🧊</td>
      <td><a href="docs/pods/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/pool&gt;</code></td>
      <td align="center">🎍</td>
      <td><a href="docs/concurrent/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/ranges_ext&gt;</code></td>
      <td align="center">🌗</td>
      <td><a href="docs/ranges/range_ext.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/runtime_arr&gt;</code></td>
      <td align="center">🧱</td>
      <td><a href="docs/core/runtime_arr.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/serio&gt;</code></td>
      <td align="center">🍯</td>
      <td><a href="docs/serialize_io/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/sync&gt;</code></td>
      <td align="center">⏱️</td>
      <td><a href="docs/synchronous/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/typed&gt;</code></td>
      <td align="center">🧬</td>
      <td><a href="docs/typing/overview.md">API References</a></td>
    </tr>
    <tr>
      <td><code>&lt;jh/views&gt;</code></td>
      <td align="center">🔭</td>
      <td><a href="docs/ranges/views/overview.md">API References</a></td>
    </tr>
  </tbody>
</table>

---

### Reading Guidance

These forwarding headers represent the **entire public surface area** of JH-Toolkit.
They are designed to **aggregate stable user-facing APIs**, independent of internal layout or implementation details.

From each overview page, you may continue navigating into:

* individual components
* submodules
* concrete type and function references

This structure intentionally **replaces long conceptual introductions** with a **clear API map**, allowing users to
quickly locate relevant functionality and then explore it at the desired depth.

---

## How to Read the JH-Toolkit Project

The **primary source of truth** for understanding JH Toolkit is its **Doxygen documentation**.

All semantic guarantees, design intent, usage constraints, and documented limitations are expressed through
Doxygen comments embedded directly in the public headers under `include/`.
These comments define the authoritative API contract.
Implementations exist to realize that contract and may change over time.

### IDE-First Documentation Design

The documentation is **authored with IDE-based reading as the primary target**.

In particular, **classic CLion** is used as the reference environment during documentation design.
When viewed in classic CLion, standard Doxygen (HTML-style) comments are rendered inline with the source code,
providing:

- Inline, fully rendered documentation while reading headers
- Rich hover tooltips with complete semantic descriptions
- Seamless navigation between types, functions, and their documentation

This creates a reading experience where **code and documentation interleave naturally**,
similar to notebook-style workflows in other ecosystems.

### Official Web Documentation Support

While the documentation is IDE-first by design, the **web-based Doxygen documentation is fully and officially
supported**.

The complete API reference is available as pre-rendered Doxygen HTML pages at:

👉 **[GitHub Pages](https://jeonghan-bae.github.io/JH-Toolkit/)**

The web documentation is generated from the same Doxygen sources as the IDE views and reflects the same
authoritative API contracts, semantics, and design intent.
It is intended for reference, review, external access, and situations where an IDE-based workflow is not available.

If you encounter any visual or rendering issues in the web documentation, please report them via the issue tracker.

### Source Layout and Reading Strategy

When reading the project, keep the following structure in mind:

- `include/`  
  Contains all real definitions, public APIs, and documented semantics.
  This is the authoritative source of truth.

- `src/`  
  Contains precompiled instantiation translation units (TUs) used to support static builds and improve compile times.
  These files are **not** the semantic specification of the library.

Under modern C++ semantics, directly inspecting implementations is often misleading.
The interface and its documentation already define the behavior.
Reading implementation details without the documented context can lead to incorrect assumptions.

The recommended reading approach is therefore:

1. Read **Doxygen comments** embedded in public headers
2. Use the **IDE-integrated documentation view** (preferred) or the **official web documentation**
3. Treat implementations as **replaceable details**, not specifications

### Documentation over Implementation

Doxygen documentation is where:

- Semantic guarantees are stated
- Design philosophy is explained
- Intended usage patterns are defined
- Assumptions and limitations are made explicit

Implementation details are documented only when necessary.
The focus is on **how the library is intended to be used correctly and safely**, not on how it happens to be implemented
at a particular point in time.

In short:

**Doxygen explains the design and intent; implementations merely realize it.**

---

## Recommended IDE for Best Source Code + Doxygen Experience

For the best experience when browsing the source code together with Doxygen documentation, **CLion** is strongly
recommended.

This recommendation is based on the fact that **classic CLion** can correctly render **standard Doxygen comments
(HTML-tags style)** and seamlessly integrate them into source code navigation.  
Specifically:

- Doxygen comments written in standard HTML-style tags are properly rendered inline in the editor.
- Hover tooltips display **fully rendered, highly readable Doxygen documentation**, rather than plain text.
- This greatly improves readability when inspecting type definitions, APIs, and implementation philosophies.

All Doxygen comments in our codebase are written using **standard Doxygen (HTML-tags style)**, as this format is
significantly more expressive and helpful for understanding definitions and semantics.

### Supported CLion Versions

- A **non-commercial license** is sufficient.
- CLion has been verified to work correctly **up to version 2025.3.2**.
- Please use **version 2025.3.2 or earlier**.
- Newer versions are **not guaranteed** to work as expected.

### Do NOT Use CLion Nova

Please **do not use CLion Nova**.

In recent CLion releases, **CLion Nova has been merged into the main CLion IDE** and is **enabled by default**. This can
negatively affect source code indexing and Doxygen navigation.

### Why CLion Nova Is Not Recommended

CLion Nova is not recommended because it only supports a **JetBrains-specific documentation rendering format**, which:

- Uses a Markdown-like / JavaDoc-style syntax instead of standard Doxygen HTML tags
- Does **not fully support native Doxygen tags**
- Cannot correctly render parts of our existing Doxygen documentation

As a result, using Nova leads to degraded documentation rendering and a poorer code-reading experience.

### How to Disable CLion Nova

To disable the Nova engine, follow these steps:

1. Open **Settings**
2. Go to **Advanced Settings**
3. Navigate to **CLion**
4. Locate the option:
    - [ ] Use ReSharper C++ language engine (CLion Nova)
5. **Uncheck** this option
6. **Restart the IDE**

After restarting, CLion will use the classic C++ engine, which provides a more stable and accurate code + Doxygen
reading experience.

## Engineering-Oriented Scope and Intent

JH Toolkit is **not designed solely for final production code**.

A portion of its components are intentionally built to **empower early-stage prototyping**, lowering cognitive load
while guiding users toward correct and scalable designs.

For example, some utilities (such as `jh::conc::occ_box`) are explicitly aimed at **prototype-driven development**.
They preserve generality while exposing **complete compare-and-swap (CAS) reasoning models**, encouraging users to write
logically sound concurrency code from the start.
The goal is not to lock users into these abstractions, but to make it easier to later migrate toward project-specific,
optimized implementations with minimal redesign cost.

In contrast, other components—such as `jh::ordered_map` and `jh::ipc`—are shaped by **engineering constraints rather
than maximal generality**.
Compared to STL or Boost equivalents, these modules intentionally reduce language-level freedom ("I want to do this, I
want to do that") in favor of **operational stability** ("this is what we repeatedly do in real systems, and it must be
reliable").

The `jh::ipc` module follows a similar philosophy:
it treats **POSIX semantics as the final deployment target**, while still allowing **Windows-based prototyping and early
modeling**.
This enables cross-platform experimentation without obscuring the actual production execution model.

The `jh::concepts` module—especially the `sequence` concept and iterator-related duck-typing—focuses on **adapting
third-party or non-standard containers**.
Its purpose is to elevate such types to valid `std::ranges::range` participants, unlocking modern range-based algorithms
without forcing invasive rewrites.

The `jh::pod` module supports **early modeling and legacy code migration** toward C++20 by enforcing a strict separation
between:

* *semantic objects* (behavior, invariants, logic)
* *semantic data* (plain, transportable, inspectable state)

This separation is crucial for long-term maintainability and safe system evolution.

Across modules such as `jh::async`, `jh::meta`, and `jh::ipc`, the toolkit also provides **philosophical engineering
guidance**.
APIs are shaped to encourage patterns that scale better in real systems, nudging users toward more disciplined and
production-oriented practices.

In summary, **JH Toolkit is an engineering-first toolkit**.
It is oriented toward practice rather than language experimentation, with the explicit goal of helping C++ developers:

* internalize engineering-oriented thinking,
* adopt modern C++20 idioms naturally,
* and reduce the friction of moving from prototypes to robust systems.

The library is not about showing what modern C++ *can* do—it is about helping engineers do what they *need* to do,
correctly and sustainably.

---

<h2>
  <span>
    <img src="https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg"
       alt="Python Logo"
       height="28" valign="middle">
  </span>
  <span style="font-size: large;">Pythonic Aesthetics and Philosophy in JH Toolkit</span>
</h2>

<p style="font-size: medium;"><strong>JH Toolkit</strong> brings Pythonic expressiveness and clarity into modern <strong>C++20</strong> design.</p>

<ol style="font-size: medium;">
  <li><strong>Duck Typing:</strong> Behavior matters more than declarations.</li>
  <li><strong>Lazy Evaluation:</strong> Compute only when necessary.</li>
  <li><strong>Automatic Memory Management:</strong> Resources manage themselves.</li>
  <li><strong>Explicit is Better Than Implicit:</strong> Clarity prevents unintended behavior.</li>
  <li><strong>Readability and Maintainability:</strong> Readable code lasts longer.</li>
</ol>

<h3 style="font-size: medium;"><strong>Why <code>snake_case</code>?</strong></h2>

<p style="font-size: medium;">Aligns with <code>std</code> to provide a seamless and natural C++ toolkit experience.</p>

---

## 👤 Author

Developed by **JeongHan-Bae**  
📧 [mastropseudo@gmail.com](mailto:mastropseudo@gmail.com)  
🔗 [GitHub Profile](https://github.com/JeongHan-Bae)

## 📜 [License](https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme)

This project is licensed under the **Apache 2.0 License**.  
See the license text at the link above for full details.

## 🤝 [Contributing](https://github.com/JeongHan-Bae/JH-Toolkit?tab=contributing-ov-file#readme)

Contributions are welcome!  
Please refer to the contribution guidelines linked above before opening issues or pull requests.

---

🚀 **Enjoy coding with `jh-toolkit`!**  
