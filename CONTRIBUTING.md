# 📘 Contribution Guide for JH-Toolkit

## Introduction

Thank you for considering contributing to **JH-Toolkit**, a **Modern C++20 Utility Library** featuring **Coroutine Generators**, **Immutable Strings**, **Sequence Concepts**, and more.

Your contributions help improve functionality, ensure platform compatibility, and maintain high code quality through clear, readable, and explicitly designed C++.

---

## 📌 Versioning Scheme

JH-Toolkit follows **Semantic Versioning**:

```
MAJOR.MINOR.PATCH
```

- **PATCH (x.y.Z)** – Bug fixes, platform-specific improvements, or small enhancements to existing modules.  
  _Example: `1.3.0 → 1.3.1`_

- **MINOR (x.Y.0)** – Introduces **multiple new features** grouped into a stable minor release.  
  _Example: `1.2.0 → 1.3.0`_

- **MAJOR (X.0.0)** – Reserved for large-scale reworks, breaking API changes, or architectural milestones.  
  _Example: `1.5.0 → 2.0.0`_

> ⚠️ Major versions no longer correspond to benchmark inclusion.  
> Micro-benchmarks for optimized modules will be embedded inside test suites when needed.

---

## 📦 Version Declaration

JH-Toolkit uses a **top-level version declaration** in CMake for simplicity and reusability:

```cmake
# Top of CMakeLists.txt
set(PROJECT_VERSION 1.3.0)
```

This version is propagated automatically to packaging tools, `find_package`, and `write_basic_package_version_file`.

### 🔁 GitHub Release CI Version (`pack_version`)

The **GitHub Actions Release Workflow** (`.github/workflows/release.yml`) uses a top-level environment variable to define the version for Conan packaging:

```yaml
env:
  pack_version: 1.3.1
```

### ✅ Version Bump Checklist

When publishing a new release, ensure all version declarations are synchronized:

- [ ] `CMakeLists.txt` — `set(PROJECT_VERSION X.Y.Z)`
- [ ] `README.md` — `### version: X.Y.Z`
- [ ] `.github/workflows/release.yml` — `pack_version: X.Y.Z`

---

## 🧪 Testing, CI, and Workflow Rules

### 🧵 Branch Policy

| Branch Pattern | Purpose                                 | Permissions                        |
|----------------|-----------------------------------------|------------------------------------|
| `main`         | 📘 Documentation, metadata only         | ✅ Docs only, no source or CI logic |
| `dev*`, `*dev` | 🧪 Source code, workflow, testing logic | ✅ All feature and code development |

> ⚠️ **All source code or CI changes must be made on a `dev*` or `*dev` branch.**

---

### 🔁 Dependency Auto-Update Rules

- On `main`: **`dependencies.json` is updated on every commit**
- On `dev*` / `*dev`: It is **only updated when the commit message includes `-UpdateDependencies`**
- ✅ Dependency auto-update runs **only within JeongHan Bae's GitHub repo** to avoid permission errors on forks

---

### ✅ CI Trigger Matrix

| Event                    | CI Triggered | Notes                              |
|--------------------------|--------------|------------------------------------|
| Push to `dev*` or `*dev` | ✅ Yes        | Always runs test matrix            |
| PR to any branch         | ✅ Yes        | CI must pass before merge          |
| Push to `main`           | ❌ No         | Only allowed for docs and metadata |

> ✅ All CI checks must **pass before any merge** is allowed.

---

## 🔬 Experimental Modules

The `experimental/` directory holds **non-public prototype features**.

### Behavior:

- ❌ No public API exposure
- ✅ If proven useful with clear performance/semantic advantages, they may be:
  - Promoted to `dev*` branches for review
- ❌ If rejected or obsolete:
  - They may be removed **without deprecation or announcement**

---

## 👨‍💻 Coding Guidelines

### 1. 🧱 Consistency with STL Style

- Use **`snake_case`** for all names (`function`, `variable`, `type`, etc.)
- Match STL naming and idioms (`size()`, `begin()`, `value_type`, etc.)
- Prefer template-based design to macro "magic"

### 2. 💡 Avoid Ambiguous Types

- Use **fixed-width types** like `uint64_t`, `int32_t` instead of `int` or `size_t`
- This ensures ABI safety and consistent layout on all platforms

### 3. 🔤 Character and Byte Clarity

- Use `char` for readable text
- Use `unsigned char` / `uint8_t` for binary or integer-like data
- Never mix `char` with numeric semantics

---

### ✅ Example: Immutable String

```cpp
struct immutable_str {
  private:
    uint64_t size_ = 0;
    std::unique_ptr<const char[]> data_;

  public:
    [[nodiscard]] uint64_t size() const noexcept { return size_; }
};
```

---

## 🚀 Contribution Process

### 1. PR Title and Versioning

Every PR must begin with a version prefix:

```text
[1.3.1] Fix bug in pod::array indexing
[1.4.0] Add support for runtime_arr<T>
```

You must also update:

- `README.md`:
  ```md
  ### version: 1.3.1
  ```

- `CMakeLists.txt`:
  ```cmake
  set(PROJECT_VERSION 1.3.1)
  ```

---

### 2. Issues and PR Links

- Non-trivial PRs **must be backed by a GitHub Issue**
- The Issue should describe:
  - What is being changed
  - Why it matters
  - Impact on existing modules

---

## 📦 Stable and LTS Releases

- A new **LTS branch** is forked at each stable release: `1.3.x-LTS`, `1.4.x-LTS`, ...
- LTS branches:
  - ✅ Receive bug and security fixes
  - ❌ Do not accept new features
- Bug fixes in LTS are **merged forward** into later active versions

---

## 🧼 No "Hacks", No "Magic"

JH-Toolkit avoids black-box tricks and "hacking" behaviors common in other meta-programming libraries.

> We prefer **clean, explicit, STL-style implementations**, where every template and constraint is designed to be understood — not reverse-engineered.

---

## 🙏 Thank You

We appreciate your time and effort in helping keep **JH-Toolkit** fast, modern, and clean.

If you have any questions, feel free to open an **Issue** or start a **GitHub Discussion**.

Happy coding! 🚀
