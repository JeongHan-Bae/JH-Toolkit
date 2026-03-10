<h1 align="center">
  <span>
    <img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" 
         alt="C++ Logo" 
         width="72" valign="middle">
  </span>
  <span style="font-size: x-large;">&nbsp;Contribution Guide for JH Toolkit</span>
</h1>

<p align="center">
  <img
    src="https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/JeongHan-Bae/JH-Toolkit/main/version_badge.json"
    alt="badge"
    width="196"
  >
</p>

---

## 💡 Introduction

Thank you for contributing to **JH Toolkit**.

JH Toolkit is a **modern, RTTI-free C++20 engineering toolkit** built around **behavioral compatibility and duck
typing**.
Types are identified by **what they can do**, not by inheritance hierarchies or runtime type information.

Starting from **1.3.3**, the toolkit guarantees full compatibility with `-fno-rtti`.  
From **1.4.0 onward**, the project significantly expands its scope and target use cases, prioritizing:

* **Real-world engineering applicability**
* **Predictable semantics**
* **Cross-platform toolchain compatibility**
* **Auditable, intention-revealing APIs**

This is **not** a playground for showcasing new C++20 features.
Every feature must justify its **practical value in production software**.

---

## 🧩 Versioning and Release Strategy

### 🔢 Semantic Versioning

JH Toolkit follows **Semantic Versioning**:

```
MAJOR.MINOR.PATCH
```

| Level     | Meaning                                               |
|-----------|-------------------------------------------------------|
| **PATCH** | Bug fixes or strictly compatible behavior refinements |
| **MINOR** | New features, modules, or expanded capabilities       |
| **MAJOR** | Breaking API or architectural changes                 |

Backward compatibility is guaranteed within each `Major.Minor.x` series unless explicitly documented.

---

### 🧱 CMake as the Single Source of Truth

The **only authoritative version source** is the top-level `CMakeLists.txt`:

```cmake
set(PROJECT_VERSION 1.4.0)
```

All other versioned artifacts (badges, releases, metadata, CI variables) are **derived automatically** via tooling.
Manual duplication of version numbers is prohibited.

---

## 🧪 Development Workflow

### 🧵 Branch Policy

| Branch          | Purpose                                |
|-----------------|----------------------------------------|
| `main`          | Documentation, metadata, release entry |
| `<version>-dev` | Active development branch              |
| `*-LTS`         | Supported maintenance branches         |

* Direct source changes to `main` or `*-LTS` are not allowed.
* All development must occur on a `*-dev` branch and be merged.

---

Below is a **simple English section** you can paste directly into the document.

---

## 🔗 GitHub Actions and CI Integration

JH Toolkit uses **GitHub Actions** as its CI and automation system.
All workflows and helper scripts are located in:

```
.github/workflows/
```

### 📁 Directory Layout

The project uses a **flat structure**.

Rules:

* No subdirectories
* All `.yml` and `.py` files are placed directly in this directory

---

### 📄 File Naming Rules

* **Workflow YAML files**

    * Use **kebab-case**
    * Example:

      ```
      ci.yml
      docs-check.yml
      update-dependencies.yml
      ```

* **Python helper scripts**

    * Use **snake_case**
    * Example:

      ```
      docs_hygiene_check.py
      generate_dependencies.py
      ```

---

### 🏷️ Workflow `name` Rules

Each workflow must define a `name` with the following rules:

* **Title Case**
* Words separated by **spaces**
* **Hyphenated words are kept**
* Each part of a hyphenated word is capitalized

Examples:

```yaml
name: "Cross-Platform CI"
name: "Documentation Integrity Check"
name: "Release Conan Packages"
```

`cross-platform` is treated as a single hyphenated word, so both parts are capitalized.

---

### 🐍 Using Python Scripts in Workflows

Python scripts in `.github/workflows/` are executed directly.

Required syntax:

```yaml
- name: Some Name
  run: python .github/workflows/script_name.py
```

Rules:

* Always use `python`, not `python3`
* Scripts must be referenced with the full path
* No inline downloads or generated scripts

---

### 🧪 Python Setup Requirement

Python must be installed using **GitHub’s official action**:

* `actions/setup-python@v4`
* or `actions/setup-python@v5`

Direct downloads or custom installers are **not allowed**.

---

### ⚠️ Cross-Platform CI (ci.yml)

`ci.yml` runs the **Cross-Platform CI** workflow.

Known limitation:

* The **Windows runner (MinGW GCC)** is unstable
* Even without stress tests or Debug builds, it may **freeze occasionally**

Reason:

* Insufficient runner resources
* Interaction with UCRT runtime behavior

Current policy:

* This is **accepted**
* If the Windows job freezes, use **“Re-run jobs”** in GitHub Actions
* The second run usually succeeds

There is currently no better solution without increasing complexity.

---

All new or modified workflows **must follow these rules**.


---

## ⚙️ Platform and Toolchain Requirements (1.4.0+)

All new contributions **must satisfy the following guarantees**:

### ✅ Fully Supported Platforms

The feature **must compile and pass tests** on:

* **macOS** — LLVM Clang (Apple toolchain)
* **Ubuntu Linux** — GCC **13 or newer**

These platforms define the **reference behavior**.

### ⚠️ Minimum Windows Requirement

On **Windows (MinGW GCC)**:

* The feature must provide **API-level availability**
* **Semantic compatibility must be preserved at a minimum level**
* Full optimization or identical behavior is not required, but:

    * The API must exist
    * The documented contract must not be violated

> Windows support focuses on **engineering usability**, not parity at any cost.

---

## 🧾 Documentation and Doxygen Policy

### 📘 What Doxygen Should Describe

Doxygen documentation must focus on:

* **Behavior**
* **Intent**
* **Design philosophy**
* **Expected usage patterns**
* **Constraints and assumptions**

### 🚫 What Doxygen Must Avoid

* Over-describing implementation details
* Mirroring internal logic
* Explaining mechanisms that may change between versions

> Implementation details age quickly and provide little value to readers.
> Behavior and intent do not.

When a behavior relies on **user discipline** rather than enforceable constraints, this **must be explicitly
documented**.

---

## 🧱 Coding and API Design Guidelines (1.4.0+)

### 1. 🎯 Engineering-First Design Philosophy

JH Toolkit is:

* **Engineering-oriented**
* **Software-development-oriented**

It is **not**:

* A C++20 feature showcase
* A template metaprogramming experiment
* A "maximum genericity" library

Do **not** design APIs:

* Without a clear real-world use case
* "Just in case" they might be useful
* Solely to demonstrate abstraction capability

Practical applicability always overrides theoretical generality.

---

### 2. 🧠 STL-Aligned Naming and Mental Model

* Follow **STL naming conventions** wherever possible
* Prefer familiar aliases, types, and function names
* Minimize cognitive overhead for users

The closer an API feels to the STL, the **lower the learning and audit cost**.

---

### 3. 🧩 Compile-Time Strings and NTTP Usage

All compile-time strings **must use** `jh::meta::TStr`.

#### ✅ Correct Pattern

```cpp
template<jh::meta::TStr Name>
struct label {};

label<"demo"> l;
```

#### 🚫 Prohibited Patterns

Do **not** use:

* String literals directly as template parameters
* `std::string_view`
* Any runtime string wrapper

These **cannot provide stable identity in templates** and break compile-time guarantees.

---

### 4. 🧠 Constraints, Concepts, and Illegal States

* Prefer **concepts + SFINAE** to disable illegal operations at compile time
* If an invalid usage pattern **cannot be reliably detected**, then:

    * The API must document the assumption
    * Doxygen must clearly state the required user discipline

This is essential for:

* **Code review**
* **Security auditing**
* **Long-term maintainability**

Silent undefined behavior is unacceptable.

---

### 5. ✂️ Controlled Genericity and Compiler Assistance

* Avoid over-generalization
* Prefer **purpose-driven APIs**
* Allow **reasonable specialization**
* Proactively leverage **Clang's stronger deduction capabilities**

Selective constraints are encouraged if they:

* Improve diagnostics
* Reduce misuse
* Clarify intent

---

## 🔄 Version Evolution, Refactoring, and Lifecycle Policy

JH Toolkit explicitly supports **in-series evolution** and **controlled large-scale refactoring**.
Major internal changes are permitted, but **user-facing continuity is mandatory** within a version series.

This section defines how evolution, compatibility, and lifecycle transitions are handled.

---

### 🧩 In-Series Backward Compatibility Guarantee

When a **large-scale refactor** occurs within a version series
(for example, **`1.3.3`** being a major internal redesign):

* **All versions within the same `Major.Minor.x` series must remain backward-compatible**
* Code written for:

    * `1.3.0`
    * `1.3.1`
    * `1.3.2`

  **must continue to compile and function correctly on `1.3.3`**

This guarantee applies even if:

* Core implementations are replaced
* Internal architecture is reorganized
* Performance or memory models change

#### Compatibility Mechanism

To satisfy this requirement, contributors **must**:

* Preserve existing APIs and semantics
* Introduce **compatibility layers / alias interfaces** when implementations migrate
* Avoid breaking observable behavior unless explicitly documented and deferred to a future release

> Compatibility layers are an **engineering tool**, not technical debt.
> They are expected to exist during transitional phases.

---

### 🧱 Post-Refactor LTS State Transition

Once a **major refactor version** (e.g. `1.3.3`) is released:

#### 1️⃣ Previous LTS Enters Maintenance Mode

* The previous LTS branch (e.g. `1.2.x-LTS`) transitions to:

  ```
  1.2.x-Maintenance
  ```
* This branch:

    * Receives **critical fixes only**
    * No longer receives feature updates
    * Is kept buildable and usable for existing users

---

#### 2️⃣ Maintenance → EOL on Next Minor Release

When the **next minor version** is released (e.g. `1.4.0`):

* The maintenance branch transitions to:

  ```
  1.2.x-EOL
  ```
* At this point:

    * No further updates are applied
    * The branch is considered **archived**

---

### 🗃️ Release Retention and Archival Policy

When a branch enters **EOL** status:

* **All GitHub Releases for that version and earlier versions are withdrawn**

    * Includes:

        * `1.2.x`
        * `1.1.x`
        * `1.0.x`
* Only **Git tags** are retained:

    * For historical reference
    * For evolution tracing
    * For source inspection

There will be:

* ❌ No release binaries
* ❌ No main-page download entries

However:

* ✅ The repository remains clonable
* ✅ Tags remain accessible for engineering reference

> Tags exist to document **design evolution**, not to serve as supported distributions.

---

### 🔁 Dual-LTS Parallel Phase

After the release of `1.4.0`, the project re-enters a **dual-LTS phase**:

* **Active LTS branches**:

    * `1.3.x-LTS`
    * `1.4.x-LTS`

Both branches are:

* Actively supported
* Synchronized where applicable
* Maintained until the **next major internal refactor**

This dual-track model continues **until another large-scale restructuring is required**, at which point the cycle
repeats.

---

### 🧠 Design Rationale

This evolution model ensures that:

* Users are never forced into abrupt migrations
* Large internal improvements remain feasible
* Maintenance cost stays bounded
* Active branches share a **unified structural baseline**

By deliberately retiring older baselines, JH Toolkit avoids long-term fragmentation while preserving **engineering
continuity**.

---

## 🚀 Contribution Workflow Summary

1. Branch from the latest `<version>-dev`
2. Implement a **justified, production-relevant** change
3. Follow commit and PR conventions
4. Ensure all required platforms compile successfully
5. Verify CI
6. Merge into `main` for release, then synchronize to LTS

### Commit message format

Please commit *ONLY in English* using the following format:

```
<behavior>(<domain>): <short description>

<detailed explanation: optional, multi-line, no-markdown-formatting, optional `*` for bullet points>
```

Example:

```
refactor(module1_name, module2_name, tests): do something and something else

This commit refactors module1 and module2 to improve performance and maintainability.

Module1:
* do something
* do something else

Module2:
* do something
* do something else

Tests:
* update some tests to reflect some semantic changes
```

```
docs(module_name): update documentation for some behavior
This commit updates the documentation for module_name to clarify the expected behavior of some function.

module_name::submodule_name:
* some_function: documented to reflect some behavior
* some_sub_class: documented to reflect some behavior

another_module_name:
* some_function: documented to reflect some behavior
```

Anti-patterns:

```
fix: fix some bug in some module
```
> Explain: should use `fix(module_name):` instead of a generic `fix:`.

```
* Module2:
  * do something
```
> Explain: no `*` for paragraphs, only for bullet points.
> no nested bullet points.

```
**Module2**:
 - do something
```
> Explain: use `*` for bullet points, not `-`.
> Markdown formatting is not allowed in the commit message body.
> Markdown `**` is not allowed in the commit message body.

### Pull Request format

Please create PRs *ONLY in English*, including the following sections:

* What problem does this PR solve / What is new in this PR?
* Is the PR backed by CI tests?
* Any breaking changes?

---

## 🙏 Acknowledgement

JH Toolkit exists to support **real engineering work**:

> **Behavior-driven APIs, RTTI-free design, predictable semantics, and cross-platform reliability — in disciplined
C++20.**

Your contributions help keep the toolkit **useful, auditable, and production-ready**.

— *The JH Toolkit Project*
