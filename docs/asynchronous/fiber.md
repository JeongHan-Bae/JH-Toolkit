# 🌀  **JH Toolkit — `jh::async::fiber` API Reference**

📁 **Header:** `<jh/asynchronous/fiber.h>`  
📦 **Namespace:** `jh::async`  
📅 **Version:** 1.4.0 (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::async::fiber` is a **C++20 coroutine-based fiber** for
**explicit, manual suspension and resumption**.

A `fiber` represents a **single sequential execution flow** that is:

* resumed explicitly by user code,
* suspended only at well-defined points,
* executed **entirely on the calling thread**.

`fiber` **does not own a thread**, **does not schedule**, and **does not run concurrently**.

---

## 🔹 Namespace & Include Behavior

| Header                      | Behavior                                                |
|-----------------------------|---------------------------------------------------------|
| `<jh/asynchronous/fiber.h>` | Defines `jh::async::fiber` and `jh::async::resume_tag`. |
| `<jh/async>`                | **Public entry point** — exports `jh::async::fiber`.    |

### Public API Rule

Users must include:

```cpp
#include <jh/async>
```

`<jh/asynchronous/fiber.h>` is an **internal module header** and is not intended
for direct inclusion.

### Namespace Rule

The type is **always** named:

```cpp
jh::async::fiber
```

There is no alias and no forwarding into `jh::`.

---

## 🧭 Design Motivation

`fiber` is designed as a **state-machine coroutine**, not a task and not a generator.

Its purpose is to allow code to be written **linearly**, while execution is
**advanced step-by-step by the caller**.

Key properties:

* no data flow,
* no return value,
* no implicit continuation,
* no scheduler involvement.

This makes `fiber` suitable for:

* explicit state machines,
* protocol and handshake logic,
* simulation steps,
* deterministic test execution,
* manually driven control flow.

---

## ⚙️ Core Operations

| Function   | Description                                                     |
|------------|-----------------------------------------------------------------|
| `resume()` | Resume execution until the next suspension point or completion. |
| `done()`   | Returns `true` if the fiber has finished execution.             |

No other control operations exist.

---

## 🧵 Coroutine Semantics

### 🔸 Suspension

Suspension inside a `fiber` is introduced exclusively via:

```cpp
co_await jh::async::resume_tag;
```

This expression:

* always suspends,
* performs no scheduling,
* waits indefinitely,
* resumes **only** when `fiber::resume()` is invoked.

No other `co_await` expressions are supported or meaningful in a `fiber`.

---

### 🔹 Execution and Thread Affinity

A `fiber` **does not create or own a thread**.

Important rules:

* The coroutine executes on the **thread that calls `resume()`**.
* The first call to `resume()` establishes the **execution thread affinity**.
* **Resuming the same fiber from different threads is undefined behavior**.

> A `fiber` must be treated as **thread-affine**.
> It is the caller's responsibility to ensure that all `resume()` calls
> occur on the same thread.

`fiber` is therefore **not a synchronization primitive** and **not thread-safe**.

---

## ⚠️ Exception Handling

`jh::async::fiber` **does not propagate exceptions**.

Rule:

> **Any uncaught exception inside a fiber causes `std::terminate()`.**

This is intentional and mirrors the semantics of a thread entry function.

All potentially throwing logic **must be handled inside the coroutine body**.

---

## 🧱 Lifetime & Ownership

* A `fiber` owns exactly one coroutine instance.
* It is **non-copyable**.
* It is **movable**.
* Destruction destroys the coroutine frame.

A moved-from fiber becomes empty and inert.

---

## 🧩 Example — State-Driven Execution

```cpp
jh::async::fiber make_fiber() {
    initialize();
    co_await jh::async::resume_tag;

    process();
    co_await jh::async::resume_tag;

    finalize();
}
```

```cpp
int main() {
    auto f = make_fiber();

    f.resume(); // initialize
    f.resume(); // process
    f.resume(); // finalize
}
```

Each `resume()` advances the fiber by **exactly one state transition**.

---

## ⚠️ Platform Notes

### GCC 14 and Newer — **Prohibited Pattern**

On GCC 14+, **immediately-invoked coroutine lambdas are prohibited**.

The frontend frequently mis-analyzes temporary lifetimes, which can lead to
**coroutine frame leaks or premature destruction**.

❌ **Forbidden on GCC**:

```cpp
auto f = []() -> jh::async::fiber {
    co_await jh::async::resume_tag;
}();
```

✅ **Required safe pattern**:

```cpp
auto make = []() -> jh::async::fiber {
    co_await jh::async::resume_tag;
};
auto f = make();
```

If your code may ever be built with GCC, **the immediate-invocation form must not be used**.

Clang does not exhibit this issue, but the safe pattern is recommended universally.

---

## 🧠 Summary

| Aspect        | `jh::async::fiber`             |
|---------------|--------------------------------|
| Model         | Manual, state-driven coroutine |
| Suspension    | `co_await resume_tag`          |
| Advancement   | `resume()`                     |
| Threading     | Executes on caller thread      |
| Thread Safety | Not thread-safe, thread-affine |
| Values        | None                           |
| Exceptions    | Fatal (`std::terminate`)       |
| Scheduling    | None                           |
| Intended Use  | Explicit state machines        |
