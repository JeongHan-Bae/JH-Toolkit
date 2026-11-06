# 🧱 **JH Toolkit — `jh::immutable_str` API Reference**

📁 **Header:** `<jh/core/immutable_str.h>`  
🔄 **Forwarding Header:** `<jh/immutable_str>`  
📦 **Namespace:** `jh`  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)

</div>

---

## ⚙️ Global Configuration Macro

```cpp
#ifndef JH_IMMUTABLE_STR_AUTO_TRIM
#define JH_IMMUTABLE_STR_AUTO_TRIM true
#endif
```

### `JH_IMMUTABLE_STR_AUTO_TRIM`

Supplies the default template argument for the immutable string API family. Explicit
`<true>` and `<false>` specializations select their own construction policy.

| Macro value        | Behavior                                                   |
|--------------------|------------------------------------------------------------|
| `true` *(default)* | Removes whitespace automatically for semantic consistency. |
| `false`            | Preserves exact input bytes.                               |

**Notes**

* This macro is evaluated **at compile time** — changing it later has no effect.  
* Configure an override once for the program, through a shared configuration header included before `<jh/immutable_str>` in every relevant translation unit, or through compiler definitions applied consistently to every target that includes it.
* A source-local `#define` is appropriate only when exactly one translation unit in the program includes this header. Different defaults across translation units can violate the one-definition rule even when explicit specializations are also used.
* Explicit `immutable_str<true>` and `immutable_str<false>` retain distinct type identities regardless of the configured default.
* Under the Trim policy, strings differing only by boundary ASCII whitespace store the same content.

---

## Policy specializations and migration

Every policy type uses `template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>`:

```cpp
using default_text = jh::immutable_str<>;
using raw_text = jh::immutable_str<false>;
using trim_text = jh::immutable_str<true>;

using raw_handle = jh::shared_str_ptr<false>;
using raw_observer = jh::weak_str_ptr<false>;
using raw_slot = jh::atomic_str_slot<false>;
using raw_weak_slot = jh::atomic_weak_str_slot<false>;
using raw_hash = jh::atomic_str_hash<false>;
using raw_eq = jh::atomic_str_eq<false>;
```

C++20 object declarations can omit `<>` for every default-policy component:

```cpp
jh::immutable_str text{"hello"};
jh::shared_str_ptr shared;
jh::weak_str_ptr weak;
jh::atomic_str_slot slot;
jh::atomic_weak_str_slot weak_slot;
jh::atomic_str_hash hash;
jh::atomic_str_eq eq;
```

In type contexts such as `using`, `std::unordered_set<...>`, and
`jh::observe_pool<...>`, use `jh::immutable_str<>`, `jh::shared_str_ptr<>`,
and the corresponding `<>` forms for slots, hash, and equality. The type and
static-library symbol changes require existing type-context declarations to be updated.

The factories accept an optional policy parameter:

```cpp
auto raw = jh::make_atomic<false>(" hello ");
auto slot = jh::make_atomic_slot<false>(" hello ");
std::mutex source_lock;
auto safe = jh::safe_from<false>(std::string_view{" hello "}, source_lock);
auto safe_slot = jh::safe_from_slot<false>(std::string_view{" hello "}, source_lock);
```

A hash and equality pair must use the same policy as its key type:

```cpp
std::unordered_set<jh::shared_str_ptr<false>,
                   jh::atomic_str_hash<false>,
                   jh::atomic_str_eq<false>> raw_set;
raw_set.insert(jh::make_atomic<false>(" hello "));
raw_set.contains(" hello ");
```

`jh::observe_pool<jh::immutable_str<true>>` and
`jh::observe_pool<jh::immutable_str<false>>` are distinct pools. In static mode,
both policy specializations are linked from the library; in header-only mode,
both are emitted from the header. The macro must be uniform across translation
units that include the header in one program.

---

## 🧭 Introduction

`jh::immutable_str` is an immutable string type for modern C++.
It owns its character data and exposes no post-construction mutation API.

It guarantees:

* No post-construction modification.
* Thread-safe concurrent reads.
* Content-based hash and equality semantics within a program.

Ideal for:

* Configuration or metadata caches
* Shared symbol tables
* Static registry and reflection systems

---

## 🔹 Overview

| Aspect                 | Description                                                   |
|------------------------|---------------------------------------------------------------|
| **Purpose**            | Represent string data that cannot be modified after creation. |
| **Thread Safety**      | Safe for concurrent reads.                                    |
| **Immutability Level** | Read-only public API after construction.                       |
| **Hashing**            | Cached, lazily computed once.                                 |
| **Build Mode**         | Header-only or static library via Dual-Mode system.           |
| **Integration**        | Works directly with `jh::observe_pool` for deduplication.     |

---

## 🔹 Core Characteristics

| Feature                   | `jh::immutable_str`    | `std::string`          |
|---------------------------|------------------------|------------------------|
| Public mutation API       | ❌ None                 | ✅ Available            |
| Concurrent reads          | ✅ Safe                 | ✅ Safe without mutation |
| Reallocation risk         | ❌ None                 | ✅ Possible             |
| Hash caching              | ✅ Cached               | ❌ Recomputed each call |
| Pool compatibility        | ✅ Automatic            | ⚠️ Manual              |
| Storage model             | Compact (`unique_ptr`) | Dynamic capacity model |

---

## 🔹 Construction and Ownership

### From C-String

```cpp
jh::immutable_str hello("Hello, world!");
```

Creates an immutable copy of a null-terminated C-string.  
The default specialization trims leading/trailing ASCII whitespace when
`JH_IMMUTABLE_STR_AUTO_TRIM` is `true`; an explicit policy uses its own value.
A `nullptr` input becomes an empty string.

---

### From String View (Safe Mode)

```cpp
std::string buffer = "Config Value";
std::mutex buffer_lock;

jh::immutable_str imm(std::string_view(buffer), buffer_lock);
```

`jh::immutable_str` provides a general-form safe constructor:

```cpp
template <jh::concepts::mutex_like M>
immutable_str(std::string_view sv, M &mtx);
```

This overload accepts **any synchronization type satisfying**
[`jh::concepts::mutex_like`](../conceptual/mutex_like.md) —
including `std::mutex`, `std::shared_mutex`, or the semantic placeholder `jh::typed::null_mutex`
of type [`jh::typed::null_mutex_t`](../typing/null_mutex.md).

It copies a **bounded string view** while holding a **scope-based guard**:

```cpp
jh::sync::const_lock<M> guard(mtx);  // Scope-based lock, auto-detects shared/exclusive
```

[`jh::sync::const_lock`](../synchronous/const_lock.md) automatically detects
whether the mutex supports shared locking and applies the appropriate lock mode.
The caller must also keep the source and the view valid while the guard is
being acquired. Creating a view of concurrently mutable storage before the
call is not made safe retroactively by this constructor's lock.

---

#### Example — Partial View Construction

```cpp
std::string text = "Hello world";
std::mutex text_lock;

auto space_pos = text.find(' ');
std::string_view partial(text.data(), space_pos);

jh::immutable_str imm(partial, text_lock);  // Copies only "Hello"
```

The constructor copies only the bytes within the provided `string_view`.
This allows *substring-based immutability* — a controlled copy of just the desired range
from a larger buffer, without additional allocation or slicing overhead.

---

#### Thread Safety and Lock Semantics

| Condition                | Behavior                                                                                                  |
|--------------------------|-----------------------------------------------------------------------------------------------------------|
| **Concurrent access**    | The mutex is locked for the full duration of the copy.                                                    |
| **Embedded null check**  | Uses `::strnlen()` to verify that the view contains no `'\0'` bytes. Throws `std::logic_error` otherwise. |
| **Lifetime requirement** | The source and view must remain valid before the guard is acquired; the mutex **must** guard the region referenced by `sv.data()` during the copy. |
| **Compatibility**        | Works with `std::mutex`, `std::shared_mutex`, and any `mutex_like` object.                                |

---

#### No-Lock Mode (Single-Thread / Static Data)

If your source buffer is **thread-local** or **permanently immutable across threads**,
locking introduces no benefit. In such cases, instead of declaring a fake mutex,
use the global [`jh::typed::null_mutex`](../typing/null_mutex.md):

```cpp
#include <jh/typed>

jh::immutable_str imm(std::string_view("Hello, world"), jh::typed::null_mutex);
```

`null_mutex` is a **duck-typed, zero-cost mutex substitute**:
it satisfies all `mutex_like` concepts but performs **no actual locking**.
Every lock operation is a compile-time no-op, fully optimized away.

This makes your intent explicit —
you are stating that the underlying data is either single-thread confined
or globally immutable and therefore safe for concurrent reads.

> ⚠️ **Important:**
> Always ensure that the mutex (real or null) semantically protects the memory region you pass.
> Passing an unrelated or unlocked mutex violates the safety contract.

---

### Shared Ownership Factories

#### `jh::make_atomic`

```cpp
auto shared = jh::make_atomic("JH Toolkit");
auto copy   = shared; // reference-counted, safe to share
```

Creates a `std::shared_ptr<jh::immutable_str<>>` (`shared_str_ptr<>`) from a null-terminated C-string.
The preferred method for constructing immutable strings intended for shared caches or registries.

---

#### `jh::safe_from`

```cpp
std::string config = "Host=localhost;Port=3306";
std::mutex config_lock;

auto key_view = std::string_view(config.data(), config.find('='));
auto key = jh::safe_from(key_view, config_lock);
```

`safe_from` mirrors the constructor above but returns a managed shared pointer:

```cpp
template <bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM, jh::concepts::mutex_like M>
shared_str_ptr<AutoTrim> safe_from(std::string_view sv, M &mtx);
```

It performs the same locked copy via
[`jh::sync::const_lock`](../synchronous/const_lock.md),
copying an immutable string from a protected memory region. The caller remains
responsible for the view's validity before the constructor acquires the lock.

**Key features**

* Accepts any `mutex_like` object — including [`jh::typed::null_mutex`](../typing/null_mutex.md) for no-lock semantics.
* Supports **substring construction** through arbitrary `std::string_view` bounds.
* Validates that no embedded null characters exist using `::strnlen()`.
* Ensures consistent locking semantics with the primary constructor.

---

#### Example — Safe Substring Creation

```cpp
std::string data = "Path:/usr/local/bin";
std::mutex m;

auto view = std::string_view(data.data() + 5, 13); // "/usr/local/bin"
auto path = jh::safe_from(view, m);
```

---

#### Comparison

| Feature                    | `make_atomic`       | `safe_from`                     |
|----------------------------|---------------------|---------------------------------|
| Input type                 | `const char*`       | `std::string_view + M`          |
| Locking                    | None                | Scoped (`jh::sync::const_lock`) |
| Supports substrings        | ❌ No                | ✅ Yes                           |
| Embedded null check        | Implicit (`strlen`) | Explicit (`::strnlen`)          |
| Thread-safe initialization | ❌ User-guaranteed   | Conditional on the caller's view lifetime and lock discipline |
| No-lock mode               | N/A                 | ✅ via `jh::typed::null_mutex`   |

---

#### Atomic slot factories

`make_atomic_slot()` and `safe_from_slot()` create atomic storage for the
handles returned by the corresponding ordinary factories. Like
`make_atomic()`, `make_atomic_slot()` rejects mutable C-string pointers and
user-defined conversions to `const char*`; pass a string literal or a
`const char*` whose data remains valid throughout construction. Load a snapshot
before accessing the string. See [Atomic pointer slots](../entry_points/atomic_ptr.md)
for the slot operations.

---

## 🔹 Thread-Safe Hashing

```cpp
std::size_t h = shared->hash();
```

* The hash is computed once on first access and cached.
* Uses `std::once_flag` to synchronize the one-time computation.
* Identical content → identical hash on the same target, guaranteed.

---

## 🔹 Pool Integration

`immutable_str` is natively compatible with [`jh::observe_pool<T>`](../concurrent/observe_pool.md) for **automatic deduplication**:

```cpp
jh::observe_pool<jh::immutable_str<>> pool;
auto x = pool.acquire("JH Toolkit");
auto y = pool.acquire("JH Toolkit");

if (x.get() == y.get()) {
    std::puts("deduplicated successfully");
}
```

`jh::observe_pool` compares objects by `hash()` and `operator==`,
so semantically identical strings share a single immutable instance.  
`pool.acquire()` returns the pool's own `ptr` handle. It manages references
through the pool's selected mutex and remains valid after the pool is
destroyed. It is a different type from `jh::shared_str_ptr<>`, which is an
ordinary `std::shared_ptr` used by the standalone factories.

---

## 🔹 Interoperability

### Shared handles and atomic slots

```cpp
using shared_type = jh::shared_str_ptr<>; // std::shared_ptr<jh::immutable_str<>>
using weak_type = jh::weak_str_ptr<>;
using atomic_type = jh::atomic_str_slot<>;
using atomic_weak_type = jh::atomic_weak_str_slot<>;
```

The first two aliases are ordinary handles. Use the atomic slot aliases when
several threads access the same stored pointer.

The former `atomic_str_ptr` alias has been removed. Use `shared_str_ptr`
for an ordinary handle or `atomic_str_slot` for atomic storage.

---

### Transparent Hashing and Equality

| Functor               | Purpose                                                      |
|-----------------------|--------------------------------------------------------------|
| `jh::atomic_str_hash<>` | Hashes stored content; normalizes C-string lookups under its policy. |
| `jh::atomic_str_eq<>`   | Compares stored content; normalizes C-string lookups under its policy. |

Example:

```cpp
std::unordered_set<
    jh::shared_str_ptr<>,
    jh::atomic_str_hash<>,
    jh::atomic_str_eq<>
> registry;

registry.insert(jh::make_atomic("admin"));
if (registry.contains("admin")) {
    std::puts("found!");
}
```

---

## 🔹 Dual-Mode Header Integration

| Mode                                     | Description                                    |
|------------------------------------------|------------------------------------------------|
| **Header-only (`jh-toolkit`)**           | Template definitions remain visible; non-template helpers are inline. |
| **Static library (`jh-toolkit-static`)** | Template definitions remain visible; non-template helpers and both class specializations are compiled in the library. |

You can detect the current linkage mode at runtime:

```cpp
if (jh::immutable_str<>::is_static_built()) {
    std::puts("running static build");
}
```

The static function
`bool jh::immutable_str<>::is_static_built();`
returns `true` when you are using the **static library** (`jh::jh-toolkit-static`),
and `false` when using the **header-only** (`jh::jh-toolkit`) mode.

---

## 🔹 Performance Notes

* Immutable buffer → zero reallocation.
* The cached hash is constant-time after its first computation.
* Content comparison may scan the full string.
* Read-dominant concurrency optimized.
* Each instance stores a pointer, size, one-time flag, and cached-hash state; the character buffer is allocated separately.
* Trimming scans boundary ASCII whitespace during construction; cost depends on input length.

---

## 🧩 Summary

`jh::immutable_str` is the **core immutable text primitive** of the JH Toolkit.  
It exposes read-only string data for safe concurrent reads and integrates
with the toolkit's pool and atomic systems.

---

> **Tip:**
> You can include either
>
> ```cpp
> #include <jh/immutable_str>
> // or
> #include <jh/core/immutable_str.h>
> ```
>
> Both forms are functionally identical.
> In **CMake**, link against one of the official targets:
>
> ```cmake
> target_link_libraries(your_target PRIVATE jh::jh-toolkit)        # header-only mode  
> target_link_libraries(your_target PRIVATE jh::jh-toolkit-static) # static library mode
> ```
>
> Select one target for the build mode you want. You can verify the selected
> mode at runtime via `jh::immutable_str<>::is_static_built()`.
