# ⚗️ **JH Toolkit — `jh::meta::lookup_map` API Reference**

📁 **Header:** `<jh/metax/lookup_map.h>`  
📦 **Namespace:** `jh::meta`  
📅 **Version:** **1.4.x** (2025)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../README.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-green?style=flat-square)](overview.md)

</div>

---

## 🧭 Introduction

`jh::meta::lookup_map<K, V, N, Hash>` is a **fixed-capacity, hash-based flat map**
providing **switch-like dispatch semantics** with deterministic layout.

All entries are pre-hashed, sorted by hash, and stored in a flat array.
Lookup consists of a binary search on hashes followed by a short linear scan
within equal-hash ranges.

The container supports **both runtime and compile-time construction**, while
exposing a uniform lookup interface.

---

## 🌍 Structural Properties

| Property             | Description                                        |
|----------------------|----------------------------------------------------|
| Fixed capacity       | Entry count `N` is part of the type.               |
| Flat storage         | Sorted array of entries; no buckets, no chaining.  |
| No allocation        | Never performs dynamic allocation.                 |
| Deterministic layout | Storage order is fully defined after construction. |
| Default return       | Lookup failure returns `default_value`.            |
| constexpr-capable    | Construction and lookup may occur at compile time. |

---

## 🔹 Conceptual Model

`lookup_map` should be understood as:

* a **flat map**
* a **hash-dispatch table**
* a **safe generalization of `switch`**

It does **not** behave like an inserting map:

* `operator[]` never creates new entries
* missing keys are handled explicitly
* mutation is not part of the design

---

## 🔹 Construction Modes

### Runtime construction (ordinary initialization)

Direct construction builds the table at runtime:

```cpp
jh::meta::lookup_map table{
    std::array{
        std::pair{key1, value1},
        std::pair{key2, value2}
    },
    default_value
};
```

Characteristics:

* Hash computation and sorting occur at runtime
* Keys and values may be runtime-only types
* Lookup semantics are identical to compile-time tables

---

### Compile-time construction (`make_lookup_map`)

All `make_lookup_map(...)` overloads are declared `consteval`.

They **force compile-time construction**:

```cpp
constexpr auto table =
    jh::meta::make_lookup_map(
        init_array,
        default_value
    );
```

Properties:

* Hashing and sorting are evaluated during compilation
* Failure to satisfy constexpr requirements is a compile error
* Resulting tables may be placed in read-only memory

---

### 🔹 Hash Selection Strategy

When no hash is explicitly specified, `lookup_map` uses [`jh::hash<K>`](../conceptual/hashable.md).

The deduction order is:

1. `std::hash<K>` (if available)
2. ADL-discovered `hash(K)`
3. member function `K::hash()`

⚠️ **Important:**
Most standard-library `std::hash<T>` specializations are **not constexpr**.
If deduction selects such a hash, compile-time construction will fail.

For reliable compile-time tables, explicitly provide a constexpr hash.

---

### 🔹 `make_lookup_map` Overloads (Explicitly Documented)

Multiple `make_lookup_map` overloads exist to **reduce boilerplate while keeping
constexpr guarantees explicit**.

#### Explicit hash (most reliable)

```cpp
make_lookup_map<Hash>(init, default_value)
```

* Uses the user-provided hash functor
* Guaranteed constexpr if `Hash` is constexpr-capable
* Recommended for non-POD or custom key types

---

#### Deduced hash via `jh::hash<K>`

```cpp
make_lookup_map(init, default_value)
```

* Hash is deduced using `jh::hash<K>`
* Succeeds only if the selected hash is constexpr-capable
* Safe for types like `jh::pod::string_view`

---

#### `std::string_view` keys (compile-time convenience)

```cpp
make_lookup_map(
    std::array<std::pair<std::string_view, V>, N>{...},
    default_value
)
```

This overload exists **specifically for compile-time usage** and performs
an internal conversion.

---

### 🔹 Why `std::string_view` Is Converted at Compile Time

When `std::string_view` is used as a key in a compile-time table,
it is **converted into [`jh::pod::string_view`](../pods/string_view.md) internally**.

This serves **two independent purposes**:

#### Constexpr hashing

* `std::hash<std::string_view>` is not constexpr
* `jh::pod::string_view` provides a constexpr-capable hash
* Conversion enables compile-time hashing and sorting

#### POD optimization

* `jh::pod::string_view` is POD-like
* When **both key and value are POD**, the entire table qualifies for POD storage
* This enables:

    * placement in read-only memory
    * simpler object layout
    * zero runtime initialization cost

The conversion is performed explicitly via `key_traits<jh::pod::string_view>`.

---

## 🔹 POD Optimization Conditions

`lookup_map` selects its internal storage type based on:

* whether `entry` is POD-like
* whether total size is below `jh::pod::max_pod_array_bytes`

To benefit from POD optimization:

* key type `K` must be POD-like
* value type `V` must be POD-like

When these conditions are met, the table is stored as [`jh::pod::array`](../pods/array.md)
instead of `std::array`.

---

## 🔹 Lookup Semantics

Lookup syntax is uniform across runtime and compile-time usage:

```cpp
auto v = table[key];
```

Rules:

* `key` may be any type accepted by `key_traits<K>`
* missing keys return `default_value`
* no insertion ever occurs
* no exceptions are thrown

---

## 🔹 Transparent Lookup via Canonical Form

The term "transparent lookup" refers to **explicit canonicalization**.

Lookup proceeds as follows:

1. Input key is converted using
   `extension::key_traits<K>::to_canonical(input)`
2. Hashing and comparison use the canonical `K`
3. The canonical key is not stored or cached

This is **not** heterogeneous lookup by reference.
A canonical key object is always constructed.

---

## 🔧 Extension Point: `key_traits`

`key_traits<K>` is a deliberate **customization and registration point**.

By specializing it, users can:

* define accepted input key forms
* control canonicalization cost
* enforce lifetime and ownership rules

The provided specialization for `jh::pod::string_view` accepts:

* string literals
* `std::string_view`
* `std::string`
* `jh::meta::t_str<N>`

All are normalized into the same canonical POD view.

---

## 🧠 Summary

* `lookup_map` is a fixed-size, hash-sorted flat map
* It generalizes `switch` semantics to arbitrary key types
* Ordinary construction builds tables at runtime
* `make_lookup_map` overloads **force compile-time construction**
* Hash deduction uses `jh::hash<K>` but may fail constexpr
* `std::string_view` keys are converted to `jh::pod::string_view` at compile time

    * for constexpr hashing
    * for POD optimization
* Lookup never inserts; missing keys return `default_value`
* "Transparent lookup" is explicit canonicalization via `key_traits`
