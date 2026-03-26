# AGENTS.md — JH Toolkit Agent Specification

## 1. Positioning

This document defines the **canonical behavioral specification** for AI agents interacting with the JH Toolkit codebase.

It separates rules by **explicit scope**:

- **Global** → applies to all agents
- **User-Facing Agent** → default mode
- **Maintainer Agent** → only when explicitly requested

When conflicts arise:

> This document overrides any general AI behavior guidelines.

---

# 2. Project Identity

JH Toolkit is:

- A **C++20 engineering-oriented extension of STL**
- A **header-first library** with optional static linking
- Designed for **zero runtime dependencies**
- Built for **predictable, auditable, high-performance systems**

---

# 3. Platform & Compiler Constraints

## Scope: Global

- **Language baseline**: C++20
- **Primary compiler**: llvm-clang
- **Supported**:
    - Apple Clang
    - GCC

- **MUST NOT support**:
    - MSVC

Reason:

> Requires **standard Itanium ABI**

Agents **MUST NEVER suggest MSVC compatibility**

---

# 4. Role Definitions

## 4.1 User-Facing Agent (Default)

Responsibilities:

- Help users use the library correctly
- Provide API usage guidance
- Explain integration

Restrictions:

- MUST NOT read implementation
- MUST NOT infer undocumented behavior
- MUST rely on docs, Doxygen, and examples

---

## 4.2 Maintainer Agent (Explicit Only)

Responsibilities:

- Assist repository maintenance
- Support code changes and structure understanding

Restrictions:

- MUST respect documented behavior
- MUST NOT rely on undefined implementation behavior

---

# 5. Documentation Hierarchy

## Scope: Global

Agents MUST follow this priority:

1. `docs/` → navigation and entry-level understanding
2. **Doxygen** → authoritative semantics
3. `examples/` → best practices

---

## docs vs Doxygen

- `docs/`:
    - Usage-oriented
    - Entry-level guidance

- **Doxygen**:
    - Exact semantics
    - True API contract

Agents:

- MUST use docs for navigation
- MUST use Doxygen for correctness
- MUST NOT infer behavior from source code

---

## Mapping Rule

```text
docs/path/to/xxx.md
↔ include/jh/path/to/xxx.h
```

---

# 6. Header & API Rules

## Scope: Global

### Public API Access

Agents MUST use:

```cpp
#include <jh/xxx>
```

---

### Header Classification

* `include/jh/xxx` (no suffix)

    * Public API entry headers

* `.h` files

    * Implementation details

---

## Scope: User-Facing Agent

Agents MUST NOT:

* Include `.h` files
* Suggest `.h` usage
* Expose implementation headers

Reason:

> Implementation headers may change across patch versions

---

## Scope: Maintainer Agent

Agents MAY:

* Read `.h` files
* Modify `.h` files
* Use `.h` files internally

Agents MUST:

* Preserve public API boundaries
* Avoid exposing `.h` usage to users

---

# 7. Source Code Access Policy

## Scope: User-Facing Agent

MUST NOT:

* Read implementation source
* Infer behavior from internal logic

MUST:

* Use docs
* Use Doxygen
* Use examples

---

## Scope: Maintainer Agent

MAY:

* Inspect repository structure
* Work with implementation files

MUST NOT:

* Treat implementation as specification

---

# 8. Examples vs Tests

## Scope: Global

### Examples

* Located in `examples/`
* Represent best practices

Agents MUST:

* Use examples as reference

---

### Tests

* Framework: Catch2 (v3.5.0) for the entire 1.4.x life cycle

## Scope: User-Facing Agent

Agents MUST NOT:

* Read test files
* Use tests as examples

Reason:

> Tests contain edge cases, not intended usage

---

# 9. Namespace Rules

## Scope: Global

* Root: `jh::`

Usage:

* Core → `jh::`
* Non-core → `jh::xxx::`

Agents MUST respect namespace hierarchy

---

# 10. Project Structure Awareness

## Scope: Global

* `include/jh/` → Public API
* `docs/` → Documentation
* `examples/` → Best practices
* `src/` → Precompiled TUs (NOT implementation source)
* `.github/` → CI/CD workflows

---

## Scope: Maintainer Agent

Agents MUST NOT:

* Modify `.github/` workflows

---

# 11. Maintainer Agent Rules

## Scope: Maintainer Agent

### Build System

* MUST use CMake
* MUST respect existing configuration

---

### Contribution

* MUST follow `CONTRIBUTING.md`
* SHOULD use Conventional Commits
* MUST NOT modify CI workflows

---

### Code Changes

* MUST preserve API stability
* MUST maintain namespace integrity
* MUST NOT expose implementation headers

---

# 12. User Integration Guidance

## Scope: User-Facing Agent

This section describes how users integrate JH Toolkit.

It is NOT an agent constraint, but a usage reference.

---

## Integration Modes

### install + find_package

Targets:

* `jh::jh-toolkit`
* `jh::jh-toolkit-static`

---

### subdirectory / FetchContent

Targets:

* `jh-toolkit`
* `jh-toolkit-static`

---

## Compile Options (User-Side Only)

### `USE_JH_COMPILE_OPT`

* Default: ON
* Proxies compile options into consumer

If disabled:

* No compile option propagation

---

### `JH_FORCE_RTTI`

* Default: OFF
* Forces RTTI even if proxy disables it

Note:

* RTTI is typically disabled in Release when proxying is active

---

## Agent Responsibility

Agents SHOULD:

* Explain compile options when relevant
* Treat them as integration controls
* NOT treat them as internal constraints

---

## Build Instructions

* Uses CMake
* Refer to:

    * `docs/build.md`
    * `dependencies.toml`

These provide:

* Toolchain requirements
* Version information
* Build instructions

---

## Conan

* Provided for releases
* Optional usage

---

# 13. Validation Strategy

## Scope: User-Facing Agent

* MUST use examples
* MUST NOT use tests
* MUST NOT use implementation

---

## Scope: Maintainer Agent

* SHOULD perform minimal validation
* MUST ensure compiler compatibility

---

# 14. Anti-Patterns

## Scope: Global

Agents MUST NOT:

* Infer behavior from implementation
* Recommend non-CMake workflows
* Ignore namespace rules

---

## Scope: User-Facing Agent

Agents MUST NOT:

* Read implementation files
* Use test files as examples
* Include `.h` headers
* Suggest MSVC support

---

## Scope: Maintainer Agent

Agents MUST NOT:

* Break API boundaries
* Modify CI workflows
* Introduce undocumented behavior

---

# 15. Core Principle

> Documentation defines behavior. Implementation does not.
