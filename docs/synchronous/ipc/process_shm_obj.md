# 🛰️ **JH Toolkit — `jh::sync::ipc::process_shm_obj` API Reference**

_`IPC` stands for **InterProcess Coordination**_

📁 **Header:** `<jh/synchronous/ipc/process_shm_obj.h>`  
📦 **Namespace:** `jh::sync::ipc`  
📅 **Version:** 1.4.x (2026)  
👤 **Author:** JeongHan-Bae `<mastropseudo@gmail.com>`

<div align="right">

[![Back to README](https://img.shields.io/badge/%20Back%20to%20README-blue?style=flat-square)](../../../README.md)
[![Back to Sync](https://img.shields.io/badge/%20Back%20to%20Sync-green?style=flat-square)](../overview.md)
[![Back to Module](https://img.shields.io/badge/%20Back%20to%20Module-orange?style=flat-square)](overview.md)

</div>

---

## 🧭 Overview

`jh::sync::ipc::process_shm_obj<"name", T, HighPriv>` is a **cross-process shared-memory container**
that stores **one instance of a POD-like data structure `T`** in an OS-named shared-memory region.

This is the **only IPC primitive in the system that actually shares user data**.

All other IPC components (`process_mutex`, `process_counter`, `process_cond_var`) exist solely to
**coordinate access**, not to hold application state.

* All participating processes map the **same physical memory**.
* Synchronization is **explicit and mandatory**.
* No constructors, destructors, or object lifetime semantics are involved.

---

## 🎯 Standard Library Correspondence

`process_shm_obj<T>` is the **InterProcess Coordination analogue** of:

> **`std::atomic<T>` for trivially copyable types**

### Conceptual mapping

| Aspect     | `std::atomic<T>`               | `process_shm_obj<T>`           |
|------------|--------------------------------|--------------------------------|
| Scope      | Single process                 | Multiple processes             |
| Storage    | Process-local memory           | OS-named shared memory         |
| Atomicity  | Implicit (lock-free or locked) | Explicit (inter-process mutex) |
| Size limit | Often ≤ 64 bits                | Arbitrary POD size             |
| Lifetime   | Object lifetime                | OS-managed                     |
| Identity   | Per-instance                   | Per-name (singleton)           |

Important clarification:

> **For `sizeof(T) > 64 bits`, most STL implementations already use locks internally.**
> `process_shm_obj` makes this explicit, portable, and cross-process.



---

## Philosophy — Engineering-Oriented Data Modeling

This system follows a **strict engineering model** rather than an object-sharing or messaging model.

The core assumption is that **shared state is finite, enumerable, and explicitly named**.
Under this assumption, the problem is not “communication”, but **coordination of state visibility**.

---

### Entity, DTO, and Data Separation

The design preserves a well-established interaction pattern commonly used in database-backed systems:

```
Entity  ↔  DataDTO  ↔  Data
```

Each layer has a distinct responsibility:

* **Entity**
  Represents business logic and domain behavior.
  Entities never interact with shared memory or storage directly.

* **DataDTO (Data Transfer Object)**
  A process-local representation constructed from raw data.
  DTOs may enforce invariants, perform validation, hold STL containers, and express meaning.

* **Data**
  A pure data representation with no behavior, ownership, or invariants.
  It is fully copyable and describes state only.

This separation is intentional and fundamental.

---

### Data Lives Where It Is Managed

In traditional architectures, the **Data layer is managed by a database**.
In this system, the Data layer is managed directly by the **operating system**.

Shared memory replaces the database **only as a storage substrate**, not as a semantic system:

* No object lifetimes
* No ownership graphs
* No queries
* No implicit transactions

The operating system provides naming, mapping, and visibility — nothing more.

---

### Coordination, Not Communication

The system does not model message exchange or streaming data.

Instead, it models **state publication and observation**:

* Writers publish complete state updates
* Readers observe consistent snapshots
* Synchronization is explicit
* Meaning is reconstructed locally

This mirrors how database records are read into DTOs and then interpreted by entities —
with shared memory acting as an **OS-level data store**.

---

### Engineering Consequences

This model yields predictable properties:

* Minimal shared state
* Short lock duration
* Clear failure modes
* No cross-process object semantics
* Stable evolution of data layouts

Most importantly, it avoids treating shared memory as an object system.

---

### Guiding Principle

> **Shared memory holds data, not meaning.
> Meaning belongs to the process that reads it.**

This principle governs the entire design.

---

## 🧱 Type Requirements (Hard Constraint)

The stored type `T` must satisfy:

```cpp
jh::pod::cv_free_pod_like
```

This means:

* Trivially copyable
* Trivially constructible and destructible
* Standard layout
* **No `const` or `volatile` anywhere in the object**
* No hidden invariants, RTTI, or ownership

If any field is `const` or `volatile`, **the type is not a valid POD**
and **`process_shm_obj` will not instantiate**.

This is a **compile-time enforced rule**, not a guideline.

---

## ⚠️ Semantic Restrictions (Not POD Rules)

The following restrictions are **not POD violations**.
They are **semantic constraints imposed by cross-process sharing**.

### What is *allowed* by POD rules, but **must not be used here**

The following are **legal POD constructs** in general C++ usage:

* raw pointers
* process-local handles
* view types (spans, string views, etc.)

However:

> **They must not appear inside `process_shm_obj<T>`**

### Why

* A pointer value is only meaningful in the address space of the process that wrote it.
* View types describe memory that exists **outside** the shared region.
* Handles and descriptors are process-local resources.

This is **not enforced by the type system**.
It is a **discipline enforced by design**.

> `process_shm_obj` stores **pure data**, not references to reality.

---

## 🧊 Relationship with `jh::pod`

Most **non-view** types in `jh::pod` are valid building blocks:

* `array`
* `pair`
* `tuple`
* `optional`
* `bitflags`
* user-defined POD structs

View types (`span`, `string_view`, `bytes_view`) are POD,
but **should only exist outside the shared region**.

**See:** [`<jh/pod>`](../../pods/overview.md)

---

## 🧱 Naming and Compile-Time Constraints

The template parameter `S` is the **logical name** of the shared object:

```cpp
process_shm_obj<"shared_state", MyPod>
```

Internally, two mutexes are created:

* `process_mutex<S>` — initialization guard
* `process_mutex<S + ".loc">` — access lock

To reserve space for `".loc"`:

```cpp
limits::valid_object_name<S, limits::max_name_length - 4>()
```

Users **must not define** mutexes with these names manually.

**See:** [`valid_object_name`](ipc_limits.md)

---

## 🔧 Access and Synchronization

### Accessors (no synchronization)

| Member                    | Description                |
|---------------------------|----------------------------|
| `ptr()`                   | Pointer to shared object   |
| `ref()`                   | Reference to shared object |
| `operator->`, `operator*` | Convenience                |

These **do not synchronize**.

---

### Explicit locking (required for writes)

```cpp
auto& shm = process_shm_obj<"state", T>::instance();

{
    std::lock_guard guard(shm.lock());
    shm.ref().field = new_value;
    process_shm_obj<"state", T>::flush_release();
} // lock released here
```

**Key rule**:

> `flush_release()` must occur **before the lock is released**.

---

### Read-side visibility

Before reading shared data written by other processes:

```cpp
process_shm_obj<"state", T>::flush_acquire();
auto snapshot = shm.ref();
```

or

```cpp
T snapshot;
{
    std::lock_guard guard(shm.lock());
    snapshot = shm.ref();
}
```

---

## 🧠 Usage Philosophy — Data, DTO, Object Separation

This is the **central design philosophy** of `process_shm_obj`.

### 1️⃣ Data (Shared Memory)

* Stored in `process_shm_obj<T>`
* Pure POD
* No behavior
* No ownership
* No invariants

This is **raw shared state**.

---

### 2️⃣ DTO (Process-local Interpretation)

Each process may define its own **Data Transfer Object**:

```cpp
struct DataDTO {
    explicit DataDTO(const SharedData& d);
    void transform();
};
```

* Constructed from shared data
* Lives in process-local memory
* May use STL, pointers, views, invariants

---

### 3️⃣ Object / Logic Layer

Application logic operates on DTOs or objects:

* Locks shared data
* Copies into DTO
* Releases lock
* Computes locally
* Locks again (if needed)
* Writes back POD

This separation ensures:

* Minimal lock duration
* Clear ownership
* Deterministic behavior
* No cross-process object semantics

---

### Design rule

> **Shared memory holds bytes, not meaning.**
> Meaning is reconstructed locally, per process.

---

## ⚙️ Platform Notes

### POSIX (Linux, BSD, Darwin)

* `shm_open` + `mmap`
* No special privileges
* Deterministic semantics

---

### Windows / MSYS2

* `CreateFileMapping` + `MapViewOfFile`
* Uses `Global\\` namespace
* Requires **Administrator privilege**

---

## ⚠️ Windows Usage Policy

* **MinGW / MSYS2 + GCC**
  Prototype development only.
* **Production on Windows**
  Use **WSL2 + GCC**.

---

## 🔐 Privilege and Unlink Semantics

### `HighPriv`

```cpp
process_shm_obj<"name", T, true>
```

* Enables `unlink()` on POSIX
* Deleted at compile time otherwise

---

### Unlink behavior

* POSIX: `shm_unlink` + internal mutex cleanup
* Windows: automatic cleanup
* Idempotent

---

## 🧬 Lifetime Semantics

* Shared memory created on first `instance()`
* Initialization guarded by `process_mutex<S>`
* Destruction only unmaps local view
* OS performs final reclamation

---

## Usage Philosophy — Locking vs. Visibility

__Fundamental principle__: **All locking should be inside an RAII guard and alive only in a local scope.**

### Write Side (Mandatory Locking)

All **writes must be synchronized**.

A writer **must acquire the process mutex**, perform a **complete update**, issue
`flush_release()`, and only then release the lock.

Writes are expected to be **logically atomic** at the POD level:

* Read current state via `ref()` / `ptr()`
* Modify a temporary POD value
* Assign the full value back to shared memory

This guarantees that each write is **published as a whole**.

---

### Read Side (Optional Locking)

Readers **will never observe a “half-written” state** as long as:

* Writers follow the rule above
* Reads occur **after** a completed `flush_release()`

If the system is **loosely coupled** and readers can tolerate **stale data**:

* Readers may skip locking
* Readers should call `flush_acquire()` before reading

This provides **maximum throughput** with well-defined semantics:
a reader observes **some complete version**, not necessarily the latest one.

---

### When Readers Should Lock

Readers **should acquire the lock** if:

* Reads and writes are both frequent **and**
* Readers are few **or**
* A consistent, up-to-date snapshot is required

Locking readers trades throughput for **stronger temporal guarantees**.

---

### Custom Protocols

Advanced users may implement custom schemes, such as:

* Versioned reads
* Seqlock-style protocols
* CAS-like update patterns

`process_shm_obj` provides **explicit visibility primitives**;
correctness is achieved by **protocol design, not hidden magic**.

---

### Design Principle

> Locks define *who may modify*.
> Flush defines *what others may observe*.

This separation is intentional and fundamental.

---

## Summary

* `process_shm_obj<T>` is the **authoritative shared-data primitive**.
* Corresponds to `std::atomic<T>` for POD-like types.
* Explicit locking is **required and intentional**.
* POD rules are **compile-time enforced**.
* Cross-process semantics require **self-discipline**.
* Data, DTO, and Object layers are **explicitly separated**.

> **Shared memory is not an object system.**
> It is a byte substrate on which disciplined systems are built.
