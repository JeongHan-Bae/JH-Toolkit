# 🔭 **JH Toolkit — `jh::ranges::views::flatten` API Reference**

📁 **Header:** `<jh/ranges/views/flatten.h>`  
📦 **Namespace:** `jh::ranges::views`  
📅 **Version:** 1.3.5+ (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Ranges](https://img.shields.io/badge/%20Back%20to%20Ranges-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

## 📝 **Note**

> ⚠️ **Important clarification**  
> `jh::ranges::views::flatten` is **not** equivalent to the `flatten` or `flatMap` constructs in other programming
> languages.  
> Those operations work on **nested ranges or iterables**, not on **structured elements**.

| Language   | Common construct                     | Operates on             |
|------------|--------------------------------------|-------------------------|
| **Java**   | `stream.flatMap(x -x.stream())`      | `Stream<Stream<T>>`     |
| **Python** | `itertools.chain.from_iterable(...)` | `Iterable[Iterable[T]]` |
| **C++**    | `std::ranges::views::join`           | `range<range<T>>`       |

These correspond to **range-level flattening**, which merges multiple inner sequences into a single view.

In contrast, `jh::ranges::views::flatten` performs **element-level structural flattening**.  
It operates on a `range` whose elements are *nested tuples* (`range<nested_tuple>`), expanding tuple-like structures
rather than joining subranges.

This adaptor exists because:

1. It provides **compile-time structural mapping**, avoiding the need for explicit `transform` code.
2. It naturally handles the deeply nested tuples produced by adaptors such as `zip` and `enumerate`.

Since the C++ standard library defines no facility for flattening tuple-like elements,
`jh::ranges::views::flatten` introduces this capability as part of the JH Ranges framework.

---

## 🧭 Introduction

`jh::ranges::views::flatten` is a **lazy view adaptor** that inspects each element of a range
and replaces any that model [`jh::concepts::tuple_like`](../../conceptual/tuple_like.md)
with a [`jh::meta::flatten_proxy`](../../metax/flatten_proxy.md),
while forwarding all non–tuple-like elements unchanged.

This adaptor is implemented using [`jh::ranges::views::transform`](transform.md),
and provides a lazy, observational projection: creating the view does not copy
or mutate elements or eagerly expand them. Explicit conversion of a proxy to
`std::tuple` materializes the elements and may copy or move them as described
below.
It can be used in both **direct** and **pipe** forms.

---

## 🔹 Definition

```cpp
namespace jh::ranges::views {

inline constexpr detail::flatten_fn flatten{};

}
```

---

## 🔹 Interface

1. **Direct call form**

   ```cpp
   template <std::ranges::viewable_range R>
   constexpr auto flatten(R&& range);
   ```

2. **Pipe form**

   ```cpp
   constexpr auto flatten();
   ```

Equivalent usage:

```cpp
auto v1 = jh::ranges::views::flatten(r);
auto v2 = r | jh::ranges::views::flatten();
```

---

## 🔹 Description

`flatten()` applies a lazy projection to every element in the input range:

* If the element satisfies [`jh::concepts::tuple_like`](../../conceptual/tuple_like.md),
  it is wrapped into a [`jh::meta::flatten_proxy`](../../metax/flatten_proxy.md),
  exposing its structured contents as a single flattened view.
* Otherwise, the element is passed through unchanged.

Because the projection function used by `flatten` is **purely observational**,
the underlying consumption model of the range is preserved automatically by
[`jh::ranges::views::transform`](transform.md):

* If the underlying range is **non-consuming** (reentrant),
  the result is also non-consuming.
* If the underlying range is **consuming** (single-pass),
  the result remains consuming.

For example, when flattening the output of
[`jh::ranges::views::zip`](zip.md) or [`jh::ranges::views::enumerate`](enumerate.md),
the resulting view is **guaranteed non-consuming**,
since both adaptors already enforce reentrancy constraints.

---

## 🔹 Behavior

| Aspect                     | Description                                                     |
|----------------------------|-----------------------------------------------------------------|
| **Transformation model**   | Delegates to `jh::ranges::views::transform`.                    |
| **Tuple-like detection**   | Determined by `jh::concepts::tuple_like`.                       |
| **Proxy type**             | Uses `jh::meta::flatten_proxy` for structured tuple expansion.  |
| **Observational purity**   | View creation does not copy or mutate elements; materialization can. |
| **Consumption semantics**  | Preserved according to transform's dispatch.                    |
| **Pipeline compatibility** | Fully composable with lazy adaptors (`zip`, `enumerate`, etc.). |

---

## 🔹 Example

```cpp
#include <jh/ranges/views/flatten.h>
#include <jh/pod/tuple.h>
#include <vector>
#include <iostream>

int main() {
    using jh::pod::tuple;

    std::vector<tuple<int, tuple<int, int>>> nested = {
        {1, {2, 3}},
        {4, {5, 6}}
    };

    for (auto&& e : nested | jh::ranges::views::flatten()) {
        auto&& [a, b, c] = e;
        std::cout << a << " " << b << " " << c << "\n";
    }
}
```

**Output:**

```
1 2 3
4 5 6
```

---

## 🔹 Recognized Tuple-Like Types

The following are recognized as part of the framework's tuple-like set:

* `std::pair`, `std::tuple`, `std::array`
* `jh::pod::pair`, `jh::pod::tuple`, `jh::pod::array`
* `jh::ranges::zip_reference_proxy` (elements of `zip_view`)

User-defined aggregates are flattened only if they
explicitly declare structured-binding support (`std::tuple_size`, `std::tuple_element`).

Declaring these specializations is treated as **explicit permission**
for recursive deconstruction, in accordance with
[`jh::concepts::tuple_like`](../../conceptual/tuple_like.md).

---

## 🔹 Return Object and Conversion

Each element returned by `flatten()` is a [`jh::meta::flatten_proxy`](../../metax/flatten_proxy.md) —
a lightweight proxy that behaves as a structured tuple view.

`flatten_proxy` supports **implicit conversion** to a `std::tuple`,
forwarding flattened elements according to the proxy's value category. This
conversion is the materialization boundary: the resulting tuple contains values
and/or references, never `std::reference_wrapper` elements.
Writing `auto result = proxy` only deduces and stores the proxy type; it does
not request tuple materialization. Declare a `std::tuple<...>` target to test or
request that conversion.

Fully value-owned nested input also supports constant-evaluated materialization:

```cpp
constexpr std::tuple<int, int, int> flat =
    jh::meta::flatten_proxy{
        std::tuple{std::tuple{1, std::tuple{2}}, std::tuple{3}}
    };
static_assert(flat == std::tuple{1, 2, 3});
```

| Proxy category | Flattened element category |
|----------------|----------------------------|
| lvalue         | lvalue                     |
| rvalue         | rvalue                     |

An lvalue input range is held by reference, and an rvalue input range is owned
by the proxy. Source elements may be `std::reference_wrapper`; the materialized
tuple cannot retain wrapper elements. The destination tuple constructs directly
from the forwarded elements, avoiding an intermediate value tuple:

* Materializing an lvalue or const lvalue proxy into value elements copies from
  the source, whether the source element is owned, a reference, or a wrapper.
* Materializing a mutable rvalue proxy moves its owned values into the result.
  If an element is a reference or wrapper, its referent is copied first and that
  copy is moved into the result; the original referent is not moved from.
* A const rvalue proxy follows normal const rvalue construction rules; value
  elements are copied when they cannot be moved from a const source.
* Reference elements in the result continue to alias their source, which must
  outlive the materialized tuple. A proxy cannot return references into its own
  storage from an rvalue conversion, since those would dangle.

After materialization, later operations on the range or tuple act on those
resulting values and references. Copies and moves apply to the materialized
values; reference elements still access their original source objects.

Additionally, the resulting range produced by `flatten`
is recognized by [`jh::ranges::to`](../to.md)
as **directly constructible** into a container of tuples —
that is, `Container<std::tuple<...>>` —
because `flatten_proxy` is tuple-compatible and reentrant-safe.

---

## 🔹 Integration Notes

* `flatten()` is an *observational adaptor* — it reinterprets structure, not data.
* Delegates all dispatch and consumption rules to [`transform`](transform.md).
* Preserves the original range's reentrancy semantics.
* Included by `<jh/views>` or via `<jh/ranges/views/flatten.h>`.

---

## 🧩 Summary

`jh::ranges::views::flatten` provides a **structure-aware, lazy flattening adaptor**
that integrates tuple decomposition seamlessly into C++20 ranges.  
It preserves all underlying consumption and traversal properties,
and composes naturally with `zip`, `enumerate`, and other JH view adaptors
to deliver readable, zero-overhead structured pipelines.
