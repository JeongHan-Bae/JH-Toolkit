# 🌀  **JH Toolkit — `jh::async::generator` API Reference**

📁 **Header:** `<jh/asynchronous/generator.h>`  
📦 **Namespace:** `jh::async`  
📅 **Version:** 1.3.x (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🏷️ Overview

`jh::async::generator<T, U>` is a **C++20 coroutine-based generator**, 
inspired by Python's `Generator[T, U, R]` type.  
It provides **type-safe, header-only**, and **constexpr-friendly** coroutine iteration, 
with full support for both **yielding** and **receiving** values.

---

### 🔹 Namespace & Include Behavior

| Header                          | Behavior                                                                                         |
|---------------------------------|--------------------------------------------------------------------------------------------------|
| `<jh/asynchronous/generator.h>` | Defines `jh::async::generator`, `jh::async::generator_range`, and adl functions.                 |
| `<jh/generator>`                | Convenience forwarding header — automatically imports all generator-related symbols into `jh::`. |

Including `<jh/generator>` provides:

```cpp
namespace jh {
    template<typename T, typename U = typed::monostate>
    using generator = async::generator<T, U>;

    template<typename T>
    using generator_range [[maybe_unused]] = async::generator_range<T>;

    using async::make_generator;
    using async::to_vector;
    using async::to_deque;
}
```

This means `<jh/generator>` behaves as a **generic container include**, exposing `jh::generator` as a first-class container type,  
while `<jh/asynchronous/generator.h>` (and later `<jh/async>`) exposes the coroutine-specialized `jh::async` namespace.

---

## 🧭 Design Motivation

Python defines:

```python
Generator[T, U, R]
```

| Symbol | Role                                                                 |
|--------|----------------------------------------------------------------------|
| `T`    | Values produced by `yield`.                                          |
| `U`    | Values sent in via `.send()`.                                        |
| `R`    | Final return value when generator completes (`StopIteration.value`). |

In C++, coroutine semantics already distinguish **yielded**, **awaited**, and **returned** values.
`R` is unnecessary — exceptions and `co_return` naturally express termination.

Therefore, the JH Toolkit simplifies this to:

```cpp
template<typename T, typename U = typed::monostate>
class generator; // Equivalent to Python's Generator[T, U, None]
```

---

## 🔧 Type Parameters

| Parameter | Meaning                                                                  | Default                       |
|-----------|--------------------------------------------------------------------------|-------------------------------|
| `T`       | Value type yielded by `co_yield`.                                        | —                             |
| `U`       | Value type awaited via `co_await`, received by `send()` or `send_ite()`. | `typed::monostate` (no input) |

---

## ⚙️ Core Operations

| Function      | Description                                                                              |
|---------------|------------------------------------------------------------------------------------------|
| `next()`      | Resumes coroutine until next `co_yield`. Produces one output.                            |
| `send(U)`     | Sends one input value into coroutine (`co_await`), resumes only until input is received. |
| `send_ite(U)` | Performs `next()` and `send()` together — single-step send+compute.                      |
| `value()`     | Returns `std::optional<T>` holding current yielded value.                                |
| `done()`      | Returns `true` if coroutine is finished.                                                 |
| `stop()`      | Destroys coroutine and clears handle.                                                    |

---

## 🧵 Coroutine Semantics

### 🔸 Two-Phase Execution Model

Each generator iteration alternates strictly between **input** and **output**:

| Phase  | Coroutine Expression | External API | Effect                 |
|--------|----------------------|--------------|------------------------|
| Input  | `co_await U{}`       | `send(U)`    | Receives input value   |
| Output | `co_yield T`         | `next()`     | Produces yielded value |

Only `next()` and `send_ite()` can advance the generator past a yield point.

---

### 🔹 Idempotent Control (Send/Next Order)

In JH's generator design, all control calls (`send()`, `next()`, `send_ite()`) are **idempotent in order**:
performing `send()` before `next()`, or `next()` before `send()`, yields the same semantic result —
as long as each iteration includes exactly **one send and one next** operation.

That is, the following two patterns are equivalent:

```cpp
// Pattern A — input first
gen.send(5);
gen.next();

// Pattern B — compute first
gen.next();
gen.send(5);
```

Both will deliver the same result to the coroutine body:

```cpp
int delta = co_await int{};
co_yield process(delta);
```

Internally, the generator ensures that pending inputs are safely stored (`last_sent_value`)
and consumed exactly once when the coroutine resumes through `next()` or `send_ite()`.

> 🧠 In short:
> **`send()` and `next()` are order-independent**,
> because input handoff (`co_await`) and output (`co_yield`) are synchronized at the same suspension boundaries.
>
> If you prefer simplicity, use `send_ite()` to combine both operations in one step.

---

### ⚠️ Input Discipline (Await Contract)

`jh::async::generator` enforces a **one-to-one relationship** between `send()` and `co_await`:

> Every `co_await` expression must correspond to **exactly one external `send()` or `send_ite()` call**.

**Never perform multiple consecutive `co_await`s** in the same iteration —
only the first one will receive valid input. Subsequent awaits will find `last_sent_value` empty.

#### ✅ Correct Pattern

```cpp
while (true) {
    auto input = co_await int{};   // One input
    co_yield process(input);       // One output
}
```

#### ❌ Incorrect Pattern

```cpp
co_await int{}; // receives input
co_await int{}; // no corresponding send() — undefined behavior
```

If multiple values must be received per iteration, **bundle them** into a single composite input:

```cpp
auto [x, y] = co_await std::tuple<int, double>{};
```

or use shared ownership:

```cpp
auto ptr = co_await std::shared_ptr<MyInput>{};
```

> 🧠 Rule of thumb: *One send per iteration.*
> Each `co_await` should have exactly one `send()` partner.

---

## 🪶 Exception Handling

### 🔹 Why No `R`

Python uses `StopIteration.value` to carry the generator's return result.
In C++, this is redundant: coroutine completion and exception propagation are first-class.

When a generator finishes:

* It simply `co_return`s or falls off the end.
* If an exception occurs, it is caught and stored as `std::exception_ptr` inside the promise.

Thus, `generator<T, U>` corresponds to `Generator[T, U, None]`.

---

### 🔸 Propagation Rules

Each coroutine stores an `std::exception_ptr` in its promise.
Exceptions are rethrown at the **next control boundary** (`next()` or `send_ite()`).

```cpp
try {
    while (gen.next()) {
        auto v = gen.value().value();
        consume(v);
    }
} catch (const std::exception& e) {
    std::cerr << "Generator failed: " << e.what() << '\n';
}
```

| Function      | Role                                | Throws?                                |
|---------------|-------------------------------------|----------------------------------------|
| `next()`      | Executes until next `co_yield`.     | ✅ May rethrow stored exception         |
| `send(U)`     | Delivers `U` to pending `co_await`. | 🚫 Normally no (unless already failed) |
| `send_ite(U)` | Combined send + compute step.       | ✅ May rethrow (includes compute phase) |

> **`send()` resumes coroutine**, but only long enough to fulfill the pending `co_await` expression — it does *not* trigger a new yield and normally does not throw.

---

### 🔸 Comparison to Python

| Concept          | Python `Generator[T, U, R]` | JH `generator<T, U>`               |
|------------------|-----------------------------|------------------------------------|
| Yield output     | `yield`                     | `co_yield`                         |
| Input value      | `.send(U)`                  | `.send(U)` / `co_await U{}`        |
| Return value `R` | `StopIteration.value`       | Normal return / external state     |
| Termination      | Raises `StopIteration`      | `done() == true`                   |
| Error            | Raises Python exception     | Re-throws via `std::exception_ptr` |

---

## 🔁 Ranged (for) Iteration

Generators with `U == typed::monostate` (no input) can be used directly in C++ range-for loops:

```cpp
jh::async::generator<int> seq = make_sequence();
for (auto v : seq) {
    std::cout << v << "\n";
}
```

* Each iteration resumes coroutine once until next `co_yield`.
* Generator is **single-pass** — once exhausted, cannot be reused.
* `begin()` is **non-const**; iteration inherently mutates coroutine state.

> For reusable sequences, wrap a generator *factory* using `jh::to_range()` to obtain `jh::async::generator_range<T>`.

---

## 🧩 Conversion & Utility Functions

These helper functions connect `generator` with STL containers, views, and factory functions.
They are divided into three conceptual categories:

| Category                             | Function(s)                 | Purpose                                                        | Notes                       |
|--------------------------------------|-----------------------------|----------------------------------------------------------------|-----------------------------|
| **Generator creation (Source)**      | `make_generator()`          | Wrap a range or iterable object into a coroutine generator.    | Lazy, may be infinite.      |
| **Generator consumption (Sink)**     | `to_vector()`, `to_deque()` | Consume the entire generator and collect all yields.           | Eager, **finite only**.     |
| **Range interoperability (Adapter)** | `to_range()`                | Wrap a generator factory into a reusable `generator_range<T>`. | Reusable, range-compatible. |

---

### 🔹 Generator Creation

#### 🏗️ `make_generator()`

Creates a **generator object** from a finite or infinite range, view, or iterable.

```cpp
auto gen = jh::async::make_generator(std::views::iota(0, 3));
for (auto v : gen)
    std::cout << v << " "; // → 0 1 2
```

It automatically yields each element of the given sequence:

```cpp
template<std::ranges::range R>
auto make_generator(R&& range) -> generator<std::ranges::range_value_t<R>>;
```

This utility is **lazy**, **type-preserving**, and safe for **finite** ranges.

> 🧠 If you pass an *infinite* view (e.g. `std::views::iota(0)`),
> the resulting generator will also be infinite — it will never terminate,
> so it must not be used with `to_vector()` or `to_deque()`.

---

### 🔹 Generator Consumption

The following functions **fully consume** a generator and collect all its yielded values.
They iterate until completion and store results in standard containers.

#### 📦 `to_vector()`

Collects all yields into a `std::vector<T>`:

```cpp
auto data = jh::async::to_vector(counter());
```

or with an input sequence:

```cpp
auto data = jh::async::to_vector(counter(), std::vector{1, 2, 3});
```

#### 📦 `to_deque()`

Same interface, returns a `std::deque<T>`:

```cpp
auto dq = jh::async::to_deque(counter(), std::vector{1, 2, 3});
```

Both functions repeatedly call `.next()` (or `.send_ite()` if input is provided)
until the coroutine is finished.
They **consume** the generator completely — it cannot be reused afterwards.

> ⚠ **Warning:**
> `to_vector()` and `to_deque()` must **not** be used on infinite generators.
> They are semantically equivalent to:
>
> ```cpp
> while (true) container.emplace_back(gen.value().value());
> ```
>
> and will never stop, eventually exhausting memory.

---

### 🔹 Range Interoperability

#### 🔁 `to_range()`

Converts any **function or lambda** that produces a
`generator<T>` (*i.e.*, `generator<T, typed::monostate>`)
into a **reusable STL-compatible range wrapper**.

The callable must take **no parameters** and must return a generator that **does not require input** (`U == typed::monostate`):

```cpp
jh::async::generator<int> make_seq() {
    for (int i = 1; i <= 3; ++i)
        co_yield i;
}

auto range = jh::to_range(make_seq);
for (auto x : range) std::cout << x; // 1 2 3
for (auto x : range) std::cout << x; // repeatable
```

You can also use a lambda factory:

```cpp
auto range = jh::to_range([]() -> jh::async::generator<int> {
    for (int i = 1; i <= 3; ++i)
        co_yield i;
});
```

Under the hood, `to_range()` simply wraps your factory into a `generator_range<T>`,
whose internal type is defined as:

```cpp
template<typename T>
class generator_range {
public:
    using generator_factory_t = std::function<generator<T>()>;
    ...
};
```

This means:

* `Factory` must be callable as `generator<T>()`.
* The generator must have `U == typed::monostate`.
* Each call to `begin()` constructs a **new generator instance** from the stored factory.
* The range is **repeatable**, but **each iterator** is single-pass internally.

> ⚙️ **Constraint Summary:**
>
> * Factory signature: `generator<T>()`
> * Generator type: `jh::async::generator<T, typed::monostate>`
> * Input parameter `U` must be `typed::monostate`.
> * Intended for output-only generators — those that yield values but never await input.

---

### 🔸 Utility Summary — Source vs Sink vs Adapter

| Category    | Function                    | Direction                      | Notes                                  |
|-------------|-----------------------------|--------------------------------|----------------------------------------|
| **Source**  | `make_generator()`          | → Creates generator from range | Lazy, may represent infinite sequences |
| **Sink**    | `to_vector()`, `to_deque()` | ← Consumes generator           | Finite only, eager collection          |
| **Adapter** | `to_range()`                | ⇄ Bridges to STL ranges        | Wraps factory for reuse                |

---

## 🧠 Summary

| Concept              | Python                                       | JH Equivalent                                             |
|----------------------|----------------------------------------------|-----------------------------------------------------------|
| `Generator[T, U, R]` | `co_yield`, `.send()`, `StopIteration.value` | `generator<T, U>` with normal return                      |
| Input mechanism      | `.send(U)` resumes generator                 | `.send(U)` fulfills `co_await`                            |
| Output mechanism     | `yield`                                      | `co_yield`                                                |
| Return mechanism     | `StopIteration.value`                        | coroutine completion                                      |
| Reusability          | single-shot                                  | single-shot (`generator`), reusable via `generator_range` |
| Input discipline     | 1 send → 1 await                             | same (strict contract)                                    |

---


## 🧩 Example — Interactive Counter

```cpp
jh::async::generator<int, int> counter() {
    int base = 0;
    while (true) {
        int delta = co_await int{};  // wait for next input
        base += delta;
        co_yield base;               // yield accumulated value
    }
}

int main() {
    auto gen = counter();

    // Combined send + compute (input 5, yield 5)
    gen.send_ite(5);
    std::cout << gen.value().value() << '\n';  // → 5

    // Separate send (input) then next (yield)
    gen.send(2);
    gen.next();
    std::cout << gen.value().value() << '\n';  // → 7
}
```

### 🔸 Behavior Summary

| Step          | Function           | Meaning                   | Output |
|---------------|--------------------|---------------------------|--------|
| `send_ite(5)` | Send + compute     | feed `5` and yield `5`    | ✅ `5`  |
| `send(2)`     | Set next input     | input only, no yield yet  | —      |
| `next()`      | Compute next yield | consumes last input (`2`) | ✅ `7`  |

---

### 🧠 Intuition

* `send_ite()` = "send **and** iterate" — the coroutine both receives and produces in one call.
* `send()` = "store next input, wait for explicit `next()`."
* `next()` = "compute the next yield."

> This pattern gives you precise control over coroutine pacing:
>
> * When you want **interactive step-by-step control**, use `send()` and `next()` separately.
> * When you want **simplified one-shot interaction**, use `send_ite()`.


> **One input → one yield.**
> Deterministic, structured, and type-safe coroutine iteration.
