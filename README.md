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

<div align="center" style="margin-left: 8%; margin-right: 8%; font-size: medium;">

<strong>
A Modern, Duck-Typed C++20 Toolkit for Generic and Asynchronous Programming — Featuring
Concurrency-Safe Infrastructure, POD Subsystems, Concept-Bridged Type Compatibility, and Extended Range Utilities —
Fully Template-Driven and RTTI-Free.
</strong>
</div>


<div align="center">
<p></p>

<!-- ✅ Core Info -->
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-violet.svg)](https://en.cppreference.com/w/cpp/20)
[![Header-Only](https://img.shields.io/badge/header--only-supported-green)](#)
[![Static Build](https://img.shields.io/badge/static--build-supported-green)](#)

<!-- ✅ CI / Contributors / Wiki -->
[![CI](https://github.com/JeongHan-Bae/JH-Toolkit/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/JeongHan-Bae/JH-Toolkit/actions/workflows/ci.yml)
[![Contributors](https://img.shields.io/github/contributors/JeongHan-Bae/JH-Toolkit.svg)](https://github.com/JeongHan-Bae/JH-Toolkit/graphs/contributors)
[![Wiki](https://img.shields.io/badge/docs-wiki-blue)](https://github.com/JeongHan-Bae/JH-Toolkit/wiki)


<!-- ✅ Feature Highlights -->
[![Object Pool](https://img.shields.io/badge/object--pooling-optimized-brown)](docs/pool.md)
[![Immutable Strings](https://img.shields.io/badge/immutable--strings-pure-brown)](docs/immutable_str.md)
[![Generators](https://img.shields.io/badge/generators-coroutines-brown)](docs/generator.md)
[![POD System](https://img.shields.io/badge/pod--system-trivial-brown)](docs/pod.md)
[![Duck Concepts](https://img.shields.io/badge/duck--concepts-flexible-brown)](docs/concepts.md)

</div>

---

<div align="center" style="margin-top: 16px; margin-bottom: -8px">
  <img src="https://raw.githubusercontent.com/bulgogi-framework/.github/main/res/img/Oree.svg"
       alt="Oree mascot"
       style="width: 128px; height: auto;">
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

## 🧰 Build & Platform Guide

For detailed build instructions, supported compilers, and Conan packaging notes,  
please refer to the [Build & Platform Guide](./docs/build.md).

> Covers: toolchains, CMake targets, Conan `.tar.gz` releases, and dual-mode headers.

---

## 📚 JH Toolkit Modules Overview

---

### 🧩 Conceptual & Ranges-Views System — Duck Typing Philosophy

The **`jh::concepts`** & **`jh::views`** system embodies JH Toolkit's **duck typing philosophy** —
if a type behaves like a sequence, it is treated as one.
This system generalizes iteration and range utilities through behavior rather than inheritance,
bridging standard and non-standard containers seamlessly.

`jh::concepts` defines JH Toolkit's **duck-typing model for iteration**,
recognizing **behavioral compatibility** rather than formal inheritance.

Any type that supports range-for iteration — readable or writable —
and **preserves its state across iterations** is treated as a **duck sequence**.
`jh::concepts::iterator` and `jh::concepts::sequence` together enable
**`jh::to_range()`**, which transparently proxies such types into a standard-like range view.

`jh::views` provides **lazy, allocation-free adapters** that extend this model.
**`jh::views::zip`** and **`jh::views::enumerate`** operate on duck sequences through `jh::to_range()`,
built upon **`jh::ranges::zip_view`**, a C++20 implementation mirroring
C++23's `std::ranges::zip_view` when available.

> In all **1.3.x releases**, `jh::concepts` are **globally down-leveled to `jh::*`**
> for backward compatibility, while `jh::views::*` remains the canonical interface,
> following the same convention as the C++ standard library (`std::views::*`).

---

### 🌀 Async System — Cooperative Concurrency, True Parallelism

**`jh::async` — Unified Asynchronous Semantics**

`jh::async` defines the asynchronous behavior model of JH Toolkit.
It provides coroutine-based lazy evaluation through **`jh::async::generator`**,
a **deferred state machine** that models computation rather than storage.
Generators are **consumable**, representing one logical evaluation path per lifetime,
and are exposed under both `jh::generator` and `jh::async::generator` for convenience.

The upcoming **1.4.x series** extends `jh::async` with other concurrency primitives:
- **`occ_box`** for optimistic transactional control
- **`process_mutex`** for timed interprocess synchronization
- **`process_launcher`** for isolated, parallel process execution

> Concurrency in JH Toolkit is defined by **behavior**, not class hierarchy —
> every asynchronous primitive follows explicit lifetime and deterministic semantics.

---

### 🧊 POD System — Plain Old Data, Modern Discipline

`jh::pod` defines a **layout-stable, trivially copyable type system** for C++20.
It provides a suite of **fixed-layout value primitives** — like `pod::pair`, `pod::array`, `pod::optional`, and `pod::bitflags` —
built for **raw memory safety**, **placement-new**, and **binary serialization**.

All `pod` types are:

* 🧩 **`memcpy`-safe**, trivially copyable, and standard-layout
* 🧱 Designed for `mmap`, `arena`, and **zero-overhead serialization**
* ⚙️ Usable inside STL containers with no hidden heap or lifetime coupling

> `*_view` and `span<T>` types are **non-owning references** — they borrow external memory
> and must not outlive their backing buffers or mapped regions.
> They are designed for **inspection, slicing, and serialization**, not ownership or mutation.

`jh::pod` also introduces lightweight tools for structure definition and validation:

* **`JH_POD_STRUCT(...)`** — define POD structs with compile-time layout checks
* **`JH_ASSERT_POD_LIKE(T)`** — enforce POD compliance on existing types

Together they form a foundation for **safe binary data modeling** —
bridging high-level C++ templates with low-level, deterministic memory control.

> ✅ 64-bit only, fully constexpr, header-only, and STL-compatible.
> Designed for predictable layout, not legacy C-style "plain structs."

---

### 🧱 Core System — Immutable, Pooled, Runtime-Stable

The **Core System** (`immutable_str`, `runtime_arr`, `sim_pool`)
provides stable storage, safe sharing, and deterministic ownership across threads.

* **`jh::immutable_str`** — a **fully immutable string**, managed via smart pointers.
  Objects are never moved or modified; only their handles (`unique_ptr` / `shared_ptr`) are replaceable.

* **`jh::runtime_arr<T>`** — a **mutable buffer container**,
  uniquely owned or shared through smart pointers.
  Specialized precompiled variants (bit-packed and byte-based) ensure layout stability and performance.

* **`jh::sim_pool<T>`** — a **Smart Immutable-objects Managing Pool**,
  acting as a non-owning **observer and redistributor**.
  It tracks objects via **weak references**, allowing automatic reuse without owning lifetimes.
  The lightweight **`jh::pool<T>`** is its duck-type counterpart.

> 🧩 Identical API for both **header-only** and **static-build** modes —
> same code, different linkage (`jh::jh-toolkit` / `jh::jh-toolkit-static`).

> 📦 Located under `jh::*` and `<jh/*>` —
> STL-compatible, ABI-stable, and designed for concurrent environments.

---

## 🔗 Quick Links to Module Docs


### 🧩 Project Structure
<details>
  <summary>
    Click to expand ▶️ / collapse 🔽
  </summary>

```
include/jh/
    ├── asynchronous           # jh::async
    │   ├── generator.h        # jh::async::generator
    │   └── ...                # future 1.4.x jh::async submodules
    ├── conceptual             # jh::concepts
    │   ├── iterator.h         # jh::concepts::iterator
    │   └── sequence.h         # jh::concepts::sequence
    ├── macros                 # jh::macros
    │   ├── header_begin.h     # dual-mode header basic
    │   ├── header_end.h       # dual-mode header basic
    │   ├── platform.h         # macro defined platform detection
    │   └── type_name.h        # jh::macros::type_name
    ├── pods                   # jh::pod
    │   ├── ...                # array, bits, bytes_view, optional, pair, pod_like, span, string_view
    │   ├── stringify.h        # visualize jh::pod debugging output
    │   └── tools.h            # supporting macros for jh::pod
    ├── ranges                 # jh::ranges bases
    │   ├── views              # jh::views
    │   │   ├── enumerate.h    # jh::views::enumerate based on zip
    │   │   └── zip.h          # jh::views::zip based on zip_view
    │   ├── range_wrapper.h    # jh::to_range base
    │   └── zip_view.h         # jh::ranges::zip_view mimic C++23
    ├── utils                  # jh::utils
    │   ├── ...                # jh::utils::base64, hash_fn
    │   ├── platform.h         # 1.3.x temporary alias for jh/macros/platform.h
    │   └── typed.h            # jh::typed::monostate
    ├── generator              # forwarding header for jh::generator
    ├── immutable_str          # forwarding header for jh::immutable_str
    ├── immutable_str.h        # jh::immutable_str
    ├── iterator               # 1.3.x temporary forwarding header for jh::iterator
    ├── pod                    # forwarding header for jh::pod
    ├── pod.h                  # aggregate header for jh::pod
    ├── pool                   # forwarding header for jh::pool
    ├── pool.h                 # jh::pool, duck typed jh::sim_pool
    ├── runtime_arr            # forwarding header for jh::runtime_arr
    ├── runtime_arr.h          # jh::runtime_arr
    ├── sequence               # 1.3.x temporary forwarding header for jh::sequence
    ├── sim_pool               # forwarding header for jh::sim_pool
    ├── sim_pool.h             # jh::sim_pool
    ├── views                  # forwarding header for jh::views
    └── views.h                # 1.3.x temporary aggregate header for jh::views
```

</details>

### Documentation Navigation
<details>
  <summary>
    Click to expand ▶️ / collapse 🔽
  </summary>

- [asynchronous/](docs/asynchronous/overview.md) — `jh::async`
  - generator.h — `jh::async::generator`
  - ... — future 1.4.x `jh::async` submodules
- [conceptual/](docs/conceptual/overview.md) — `jh::concepts`
  - iterator.h — `jh::concepts::iterator`
  - sequence.h — `jh::concepts::sequence`
- [macros/](docs/macros/overview.md) — `jh::macros`
  - header_begin.h — dual-mode header basic
  - header_end.h — dual-mode header basic
  - platform.h — macro-defined platform detection
  - type_name.h — `jh::macros::type_name`
- [pods/](docs/pods/overview.md) — `jh::pod`
  - ... — `array`, `bits`, `bytes_view`, `optional`, `pair`, `pod_like`, `span`, `string_view`
  - stringify.h — visualize `jh::pod` debugging output
  - tools.h — supporting macros for `jh::pod`
- [ranges/](docs/ranges/overview.md) — `jh::ranges` bases
  - [views/](docs/ranges/views/overview.md) — `jh::views`
    - enumerate.h — `jh::views::enumerate`
    - zip.h — `jh::views::zip`
  - range_wrapper.h — `jh::to_range` base
  - zip_view.h — `jh::ranges::zip_view`
- [utils/](docs/utils/overview.md) — `jh::utils`
  - ... — `jh::utils::base64`, `jh::utils::hash_fn`
  - platform.h — alias for `jh/macros/platform.h`
  - typed.h — `jh::typed::monostate`
- [generator](docs/asynchronous/generator.md) — forwarding header for `jh::generator`
- [immutable_str](docs/immutable_str.md) — forwarding header for `jh::immutable_str`
- immutable_str.h — `jh::immutable_str`
- [iterator](docs/conceptual/iterator.md) — temporary forwarding header for `jh::iterator`
- [pod](docs/pods/overview.md) — forwarding header for `jh::pod`
- pod.h — aggregate header for `jh::pod`
- [pool](docs/pool.md) — forwarding header for `jh::pool`
- pool.h — `jh::pool`, duck-typed `jh::sim_pool`
- [runtime_arr](docs/runtime_arr.md) — forwarding header for `jh::runtime_arr`
- runtime_arr.h — `jh::runtime_arr`
- [sequence](docs/conceptual/sequence.md) — temporary forwarding header for `jh::sequence`
- [sim_pool](docs/sim_pool.md) — forwarding header for `jh::sim_pool`
- sim_pool.h — `jh::sim_pool`
- [views](docs/ranges/views/overview.md) — forwarding header for `jh::views`
- views.h — temporary aggregate header for `jh::views`

</details>

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

## 📜 License

This project is licensed under the **Apache 2.0 License**. See the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Feel free to open issues and pull requests to enhance the library.

---

🚀 **Enjoy coding with `jh-toolkit`!**  
