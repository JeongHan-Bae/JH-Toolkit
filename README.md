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
    src="https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/JeongHan-Bae/JH-Toolkit/1.3.x-LTS/version_badge.json"
    alt="badge"
    width="196"
  >
</p>

<div align="center" style="margin-left: 8%; margin-right: 8%; font-size: medium;">

<strong>
A modern C++20 toolkit built on duck-typed concepts and fully static, template-driven design.
It offers observing-only object pooling, safe immutable strings, coroutine-powered async generators, 
a lightweight POD framework, expressive metaprogramming utilities, semantically rich range/view extensions, 
and stable serialization components — entirely RTTI-free and concurrency-friendly.
</strong>
</div>


<div align="center">
<p></p>

<!-- ✅ Core Info -->
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-violet.svg)](https://en.cppreference.com/w/cpp/20)
![Header-Only](https://img.shields.io/badge/header--only-supported-green)
![Static Build](https://img.shields.io/badge/static--build-supported-green)

<!-- ✅ CI / Contributors / Wiki -->
[![CI](https://github.com/JeongHan-Bae/JH-Toolkit/actions/workflows/ci.yml/badge.svg?branch=1.3.x-LTS)](https://github.com/JeongHan-Bae/JH-Toolkit/actions/workflows/ci.yml)
[![Contributors](https://img.shields.io/github/contributors/JeongHan-Bae/JH-Toolkit.svg)](https://github.com/JeongHan-Bae/JH-Toolkit/graphs/contributors)
[![Wiki](https://img.shields.io/badge/docs-wiki-blue)](https://github.com/JeongHan-Bae/JH-Toolkit/wiki)


<!-- ✅ Feature Highlights -->
![Object Pool](https://img.shields.io/badge/object--pooling-observing--only-brown)
![Immutable Strings](https://img.shields.io/badge/immutable--strings-safe-brown)
![Async](https://img.shields.io/badge/async%20%28coroutines%29-zero--boilerplate-brown)
![POD System](https://img.shields.io/badge/plain--old--data-primitive-brown)
![Duck Concepts](https://img.shields.io/badge/duck--concepts-powerful-brown)
![Meta Programming](https://img.shields.io/badge/metaprogramming-compile--time-brown)
![Ranges and Views](https://img.shields.io/badge/ranges%20%26%20views-semantically--rich-brown)
![Serialization](https://img.shields.io/badge/serialization-stable-brown)
</div>

<div align="center">
  <img
    src="https://raw.githubusercontent.com/bulgogi-framework/.github/main/res/img/Oree.svg"
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

## 🧭 Branch Line and Long-Term Support Policy

During the **1.3.x** era, the **`1.3.x-LTS`** branch was maintained by
**merging release tags from the `main` branch** —
that is, updates flowed **from `main → 1.3.x-LTS`**.

As the `main` branch now transitions into the **1.4.x** development line,
the release flow for **1.3.x** has been restructured to ensure long-term stability
and independent lifecycle management.

### 🔹 `1.3.x-support-dev` — Source Maintenance Branch

The **`1.3.x-support-dev`** branch is cloned directly from
**`main` at tag `JH-Toolkit-1.3.3`**,
serving as the **exclusive source-maintenance branch** for the 1.3.x family.

From this point onward:

* All **1.3.x** development is conducted on **`1.3.x-support-dev`**,
  focusing **solely on source code updates** — no documentation, CMake, or version edits.
* The source changes are periodically synchronized **upward** into
  **`main`** or **`1.4.0-dev`**, depending on which higher branch is currently active.
* When preparing a new 1.3.x release (e.g., **1.3.5**):

  1. Merge **`1.3.x-support-dev`** into **`1.3.5-dev`**.
  2. Update version numbers and metadata in **`1.3.5-dev`**.
  3. Merge changes into **`1.3.x-LTS`**.
  4. Update documentation and publish the new tag **`JH-Toolkit-1.3.5`**.

> **`1.3.5-dev`** is a long-standing branch, originally cloned from **`1.3.x-LTS`** during 
> version `1.3.3`, serving as a communication bridge between **`1.3.x-support-dev`** and **`1.3.x-LTS`**. 
> With each `1.3.{x}` version upgrade, it is renamed to `1.3.{x}-dev` because **`1.3.x-support-dev`**, 
> as the consensus version for both `1.3.x` and `1.4.x` series, cannot perform certain actions, 
> thus requiring delegation to `1.3.{x}-dev`.

This process ensures that:

* **`1.3.x-LTS`** becomes the canonical release line for all 1.3.x versions.
* The **`1.3.x-support-dev`** branch remains a conflict-free base
  that can merge cleanly into **LTS**, **main**, and future versions.
* Low-level module improvements stay synchronized across all generations of JH Toolkit.

---

## 🧰 Build & Platform Guide

The project builds with **GCC 13+ or Clang 15+**, with **GCC 14+ or LLVM 20** strongly recommended for optimal performance and language support.
**MSVC is not supported.**

For complete build instructions, supported toolchains, and Conan packaging notes, refer to the
[Build & Platform Guide](./docs/build.md).

> Covers: toolchains, CMake targets, Conan `.tar.gz` releases, and the dual-mode header design.

The project has **no runtime dependencies**.
Its **only build-time requirement** is a conforming **C++20 standard library**.
Testing uses **Catch2**.
For version details, see: [Dependencies](dependencies.toml).

---

## 📚 JH Toolkit Modules Overview

---

### 🧱 Core Modules — Practical, Direct, Familiar

The **core** subsystem provides a collection of containers and container-like types that follow a clear and intentionally minimal philosophy.
Much like the utilities directly under `std`, these components are meant to be **used immediately**, without framework overhead or hidden semantics.

Each type focuses on:

* **predictable behavior**
* **clear ownership rules**
* **unambiguous memory expectations**

They are **straightforward building blocks**, shaped to be stable, readable, and composable.


---

### ⚗️ Metaprogramming — Structure Behind Expression

The `jh::meta` family expresses a simple belief:

> *Compile-time code should read like a description, not an excavation site.*

It provides small, purposeful tools that make type-level reasoning feel structured, not chaotic.
The focus is not on power, but on **expressiveness shaped by intent**.

---

### 🧩 POD Framework — Data Without Pretension

The POD subsystem embraces the idea that **not all data wants to be a class**.

It models information as:

* transparent
* explicit
* layout-stable

This module captures the spirit of working with raw structure while keeping code readable and deliberate.

---

### 🔭 Ranges & Views — Composable Meaning

The ranges extension builds on modern C++ pipelines, leaning into a guiding philosophy:

> *Iteration should describe thought, not machinery.*

Views and algorithms here encourage pipelines that read like small stories—
composable, expressive, and semantically intentional.

---

### 🌀 Asynchronous — Structured Coroutines, Not Coroutine Chaos

The asynchronous subsystem aims to **reshape** C++20 coroutines into a small, well-defined set of models.
Instead of exposing the full, open-ended machinery of promises, awaiters, and ad-hoc control flow,
this module treats coroutines as **semantic constructs** with clear behavior rules.

By narrowing what a coroutine *can* do, each abstraction becomes easier to reason about and far more predictable.
`generator` is the first expression of this idea, and future components—such as fibers or signal-slot system —will follow the same principle:

> **less surface, more clarity;
> fewer mechanisms, stronger meaning.**

The goal is not to wrap coroutines, but to **give them shape**.

---

### 🛠️ Additional Modules — Tools With Personality

Other subsystems round out the toolkit with focused, principled utilities:
synchronization shaped around semantics, serialization shaped around stability,
and small helpers shaped around transparency.

Each piece exists only because it contributes to the overall language of the toolkit.

---

## 🔗 Quick Links to Module Docs


### 🧩 Project Structure
<details>
  <summary>
    Click to expand ▶️ / collapse 🔽
  </summary>

```
include/jh/
    ├── asynchronous/       # jh::async
    │   ├── ...             # future coroutine based components
    │   └── generator.h     # jh::async::generator (coroutine based pythonic generator)
    ├── conceptual/         # jh::concepts
    │   └── ...             # all concepts in jh::concepts
    ├── core/               # namespace jh as core
    │   ├── immutable_str.h # jh::immutable_str (truly immutable string)
    │   ├── pool.h          # jh::pool (jh::sim_pool with auto-detection of hash and eq)
    │   ├── runtime_arr.h   # jh::runtime_arr (reallocation-disabled alternative of std::vector)
    │   └── sim_pool.h      # jh::sim_pool (non-owning object pool)
    ├── macros/             # jh::macro
    │   ├── header_begin.h  # dual-mode header basic
    │   ├── header_end.h    # dual-mode header basic
    │   ├── platform.h      # macro defined platform detection
    │   └── type_name.h     # jh::macros::type_name
    ├── metax/              # jh::meta
    │   ├── adl_apply.h     # adl alternative of std::apply
    │   ├── char.h          # concept of any_char and related check
    │   ├── flatten_proxy.h # compile time mapping nested tuple to flattened tuple
    │   └── hash.h          # consteval deterministic hash
    ├── pods/               # jh::pod
    │   ├── array.h         # jh::pod::array
    │   ├── bits.h          # jh::pod::bitflags (pod alternative of std::bitset)
    │   ├── bytes_view.h    # jh::pod::bytes_view (safe wrapper for reinterpret_cast)
    │   ├── optional.h      # jh::pod::optional (default init as nullopt)
    │   ├── pair.h          # jh::pod::pair
    │   ├── pod_like.h      # jh::pod::pod_like concept
    │   ├── span.h          # jh::pod::span (observing continuous memory)
    │   ├── string_view.h   # jh::pod::string_view (lite alternative of string_view)
    │   ├── stringify.h     # enable printing jh::pod structs (debugging)
    │   ├── tools.h         # macro helpers
    │   └── tuple.h         # jh::pod::tuple and structured binding of jh::pod::pair and jh::pod::array
    ├── ranges/             # jh::ranges bases
    │   ├── views/          # jh::ranges::views
    │   │   └── ...         # view algorithms with direct and pipe form (jh::views::*)
    │   ├── ...             # ranges_ext algorithms with direct and pipe form (jh::ranges::*)
    │   └── ...             # view adaptors defined in jh::ranges
    ├── serialize_io/       # jh::serio
    │   ├── ...             # future jh::serio components
    │   └── base64.h        # runtime base64 encode decode
    ├── synchronous/        # jh::sync
    │   ├── ...             # future jh::sync components
    │   └── const_lock.h    # read-first lock, can fallback to write-lock, semantically const
    ├── typing/             # jh::typed
    │   ├── monostate.h     # jh::typed::monostate
    │   └── null_mutex.h    # jh::typed::null_mutex representing no lock
    ├── generator           # jh::generator (exposing jh::async::generator)
    ├── immutable_str       # jh::immutable_str
    ├── iterator            # legacy forwarder
    ├── meta                # jh::meta forward-aggregator header
    ├── pod                 # jh::pod forward-aggregator header
    ├── pool                # jh::pool
    ├── ranges_ext          # jh::ranges algorithms (only ranges_ext algos) forward-aggregator header
    ├── runtime_arr         # jh::runtime_arr
    ├── sequence            # legacy forwarder
    ├── serio               # jh::serio forward-aggregator header
    ├── sim_pool            # jh::sim_pool
    ├── typed               # jh::typed forward-aggregator header
    └── views               # jh::views (jh::ranges::views) forward-aggregator header
```

</details>

### Documentation Navigation
<details>
  <summary>
    Click to expand ▶️ / collapse 🔽
  </summary>
Docs related to public forwarding headers

 - [`<jh/generator>`](docs/asynchronous/generator.md)
 - [`<jh/immutable_str>`](docs/core/immutable_str.md)
 - [`<jh/iterator>`](docs/conceptual/iterator.md)
 - [`<jh/meta>`](docs/metax/overview.md)
 - [`<jh/pod>`](docs/pods/overview.md)
 - [`<jh/pool>`](docs/core/pool.md)
 - [`<jh/ranges_ext>`](docs/ranges/ranges_ext.md)
 - [`<jh/runtime_arr>`](docs/core/runtime_arr.md)
 - [`<jh/sequence>`](docs/conceptual/sequence.md)
 - [`<jh/serio>`](docs/serialize_io/overview.md)
 - [`<jh/sim_pool>`](docs/core/sim_pool.md)
 - [`<jh/typed>`](docs/typing/overview.md)
 - [`<jh/views>`](docs/ranges/views/overview.md)

</details>

---

<h2>
  <span>
    <img src="https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg"
       alt="Python Logo"
       width="28" valign="middle">
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

## 📜 License

This project is licensed under the **Apache 2.0 License**. See the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Feel free to open issues and pull requests to enhance the library.

---

🚀 **Enjoy coding with `jh-toolkit`!**  
