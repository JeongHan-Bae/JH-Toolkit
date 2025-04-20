# 🔁 JH Toolkit: `generator` API Documentation

📌 **Version:** 1.3  
📅 **Date:** 2025  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`  

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=for-the-badge)](../README.md)

---

## **Overview**

The `jh::generator<T, U>` class provides a **coroutine-based generator** inspired by Python's `yield` mechanism. It allows both **iterative** and **interactive** coroutine-based generators, enabling **lazy evaluation** and **on-the-fly computations** in modern C++.

### 🔐 Why `final`?

- Coroutines should not support inheritance semantically.
- Subclassing `generator` is unnecessary.
- Marking `final` avoids accidental vtable pollution, ensuring optimal performance.

---

## ✨ Key Features

- Coroutine-powered lazy sequences (`co_yield`).
- Supports interactive generators (`send()` & `send_ite()`).
- **Single-pass iteration** with optional `begin()/end()`.
- Conversion to `std::vector` and `std::deque`.
- Seamless interop with STL-style ranges and pipelines via `jh::to_range()`.
- Exception-safe and RAII-compliant.
- Move-only semantics — copy constructor and copy assignment are explicitly deleted.

---

## ⚠ Design Notes & Limitations

- 🚫 **No built-in serialization** — coroutine state is not serializable.
- 🚫 **No internal mutex** — generators are **not thread-safe** by design.
- 🚫 **No comparison operators** — comparing generator state is meaningless.
- 🚫 **Copy forbidden** — `generator` instances are non-copyable to avoid coroutine ownership ambiguity. Only move semantics are supported.
- 🚫 **No `begin() const`** — Range iteration is destructive. Constant generators are intentionally disallowed to prevent accidental consumption.
- 🚫 Generator instances must not be `const`.  

> ❗ Only generators with `U == jh::typed::monostate` can support range-based `for` iteration.  
> For generators with `send()` input, such iteration is semantically invalid.  
> Since iteration mutates internal state, assigning a generator to a `const` variable is semantically invalid.  
> This is enforced via `begin() const = delete`, which will result in compile-time error when misused.  

---

## 🧩 Type Parameters

```c++
template<typename T, typename U = jh::typed::monostate>
struct generator;
```

- `T` — The type of yielded values.
- `U` — The type of sent values (optional; default is `monostate`).

---

## 🔁 Iteration & Interaction

```c++
bool next();
bool send(U value);
bool send_ite(U value);
std::optional<T> value();
void stop(); // Immediately destroys coroutine and releases resources
```

- `next()` advances the coroutine.
- `send()` injects a value.
- `send_ite()` combines both.
- `value()` returns the last `co_yield`ed value.

> Use `stop()` to terminate the coroutine early. After calling, all subsequent calls become no-op.

> 🛡️ **Exception Safety**  
> All coroutine exceptions (e.g., thrown during `co_yield` / `co_await`) are automatically captured via `std::exception_ptr` and re-thrown during `next()` or `send()` calls.  
> This ensures consistent propagation and RAII-safe termination:
> ```c++
> try {
>     while (gen.next()) {
>         auto val = gen.value(); // safe
>     }
> } catch (const std::exception& ex) {
>     std::cerr << "Coroutine failed: " << ex.what() << "\n";
> }
> ```

---

## 🌀 Range Loop Support

Generators with `U == jh::typed::monostate` support `begin()` and `end()`:

```c++
jh::generator<int> gen = my_generator();
for (auto x : gen) std::cout << x << " ";
```

> Note: Iteration **consumes** the generator. Don't use `const` generator in ranged-for.  
> 🧠 **Why `begin()` is non-const:**  
> Since `generator` iteration **consumes** internal state, `begin()` is only available for mutable instances.  
> This design enforces clarity — range loops mutate the generator by design.

---

## 📦 Conversion Utilities

### ✅ To Vector

```c++
std::vector<T> to_vector(generator<T>& gen);
std::vector<T> to_vector(generator<T, U>& gen, U input);
std::vector<T> to_vector(generator<T, U>& gen, const Range& inputs);
```

### ✅ To Deque (Efficient Replacement for `std::list`)

> ⛔ Support for `std::list` is dropped. Use `std::deque` instead.

```c++
std::deque<T> to_deque(generator<T>& gen);
std::deque<T> to_deque(generator<T, U>& gen, U input);
std::deque<T> to_deque(generator<T, U>& gen, const Range& inputs);
```

> ✅ `deque` has better memory density, less pointer overhead, better large-page locality.

---

## 🌐 Generator Factory: `to_range`

Convert a generator factory into a **repeatable range**:

```c++
// ✅ Correct usage: capture view explicitly or use static
constexpr auto view = std::views::iota(0, 3);
const auto my_range = jh::to_range([view] {
    return jh::make_generator(view);
});

for (int x : my_range) std::cout << x << " "; // 0 1 2
for (int x : my_range) std::cout << x << " "; // 0 1 2 again
```

❗ **Invalid usage:**
```c++
// ❌ Temporary view leads to dangling reference
const auto my_range = jh::to_range([] {
    return jh::make_generator(std::views::iota(0, 3)); // dangling
});
```

### ⚠️ Safety Notes

- If your range (`std::views::iota`, etc.) is temporary, **capture it first** or store as `static`.
- You may also use `constexpr` or `static` variables in lambdas to persist views safely.
- ❗ Do **not** name the result `range` — use `range_`, `my_range`, etc., to avoid conflict with `range` module.

---

## 🧵 Integration Compatibility

The return value of `jh::to_range(...)` is compatible with:

- ✅ `jh::views::zip(...)`
- ✅ `jh::views::enumerate(...)`
- ✅ STL algorithms (`std::ranges::for_each`, `transform`, etc.)

> 🔍 Both `zip` and `enumerate` require arguments to be either:
> - A `std::ranges::range`, or
> - A valid `jh::sequence` (i.e., supports `.begin()` / `.end()` in const context)

### ❌ Not Allowed:
```c++
auto gen = jh::make_generator(std::views::iota(0, 10));
jh::views::zip(gen, some_other_sequence); // ❌ ILLEGAL: generator<T> is not a range
```

### ✅ Correct:
```c++
auto zipped = jh::views::zip(
    jh::to_range([] {
        static auto view = std::views::iota(0, 5);
        return jh::make_generator(view);
    }),
    some_other_sequence
);
```
See [jh::views](views.md) for views details.

---

## 💡 Use Cases

- Lazy numeric sequences
- On-demand token streams
- Streaming input processing
- Bidirectional reactive pipelines

---

## 📚 See Also

- `jh::views::enumerate` — Adds index to any compatible range.
- `jh::views::zip` — Zips multiple sequences together.
- `jh::typed::monostate` — Lightweight signal type for void input.

---

📌 **For full reference, see `generator.h`.**  
📌 **Detailed API is embedded inline for IDE documentation support.**

🚀 Enjoy coroutine-powered iteration with **JH Toolkit**!

