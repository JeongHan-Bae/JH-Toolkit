# Contribution Guide for JH-Toolkit

## Introduction
Thank you for considering contributing to **JH-Toolkit**, a **Modern C++20 Utility Library** featuring **Coroutine Generators, Immutable Strings, Sequence Concepts, and more**. Your contributions help improve functionality, maintain cross-platform compatibility, and ensure high code quality.

---

## Versioning Scheme

JH-Toolkit follows a **semantic versioning** format:

```
MAJOR.MINOR.PATCH
```

- **PATCH (x.y.Z)** – Incremented for **cross-platform adaptations, standard conformance fixes, or bug fixes**.
  - Example: `1.0.0 → 1.0.1`
- **MINOR (x.Y.0)** – Incremented when **adding features to an existing module**.
  - Example: `1.0.{} → 1.1.0`
- **MAJOR (X.0.0)** – Incremented when **introducing a new independent module**.
  - Example: `1.{}.{} → 2.0.0`

### PR Naming and Documentation
- Every **new PR** must **include the version number** in its **title**.
  - Example:
    ```
    [1.1.0] Add new ... utilities for ... module
    [1.0.2] Fix ... Platform compatibility for ... module
    ```
- The **README version number must be updated** at the beginning of the file:
  ```md
  # JH Toolkit
  
  ### **version: x.y.z**
  ```
- The **CMake version number must be updated** in:
  ```cmake
  # Manually written CMake configuration file (for find_package)
  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
          "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
          VERSION x.y.z
          COMPATIBILITY SameMajorVersion
  )
  ```

---

## Coding Guidelines

### 1. **Consistency with STL Style**
- Naming convention: Use **snake_case** for function and variable names.
- **Structs should be used as data containers**, with **private** data members.
- Functions should **mimic stdlib naming style** for an intuitive API.

### 2. **Cross-Platform Type Consistency**
To ensure **cross-platform compatibility**, explicitly specify non-template output types:
- **Use `int32_t` instead of `int`** → Avoids platform-dependent int sizes.
- **Use `uint64_t` instead of `size_t`** → `size_t` may be 32-bit on some platforms, while `uint64_t` ensures a fixed width.

### 3. **Character and Integer Types**
- **Use `char` for generic character representation**
  - `char` is mapped to the native character representation of the platform, whether signed or unsigned.
  - This ensures compatibility when working with **human-readable text** without encoding-specific concerns.
- **Use `unsigned char` or `uint8_t` for small integer storage, not character representation**
  - When dealing with **binary data or short integer values**, use `unsigned char` or `uint8_t` instead of `char` to avoid sign-related issues.

### 4. **Immutable String Implementation Example**
```c++
struct immutable_str {
    private:
        uint64_t size_ = 0; ///< Length of the string
        std::unique_ptr<char[]> data_; ///< Immutable string data

    public:
        [[nodiscard]] uint64_t size() const noexcept { return size_; }
};
```

---

## Contribution Process

### 1. **Issue Requirement for Pull Requests**
- Any **non-test, non-example PR** **must** have a corresponding **GitHub Issue**.
- The Issue should describe:
  - Purpose of the change
  - Impact on existing functionality
  - Any potential compatibility concerns

### 2. **Stable Version Releases**
- **New features can only be introduced if the previous version has a stable release.**
- The **project owner** will **release a stable version** based on the most recent **minor version** once all necessary bug fixes and compatibility patches have been completed.
- **Long-Term Support (LTS) Versions** will be maintained for stability and will only receive bug fixes and security updates.

---

## Thank You!
Your contributions help make **JH-Toolkit** a robust and maintainable Modern C++ library. We appreciate your effort in following these guidelines to keep the project high-quality and developer-friendly.

For any questions, please open a **GitHub Discussion** or **Issue**.

Happy coding! 🚀