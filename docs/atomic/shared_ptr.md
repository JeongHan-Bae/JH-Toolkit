# `jh::atomic::shared_ptr<T>`

## Include

```cpp
#include <jh/atomic/shared_ptr.h>
```

The class stores a `std::shared_ptr<T>` for concurrent access. Construct it from a shared handle, use `store()` to publish a handle, `load()` to obtain an owning snapshot, `exchange()` to replace one, and `compare_exchange_weak()` or `compare_exchange_strong()` to commit a conditional replacement. It also provides value assignment and conversion, `value_type`, `is_lock_free()`, and `is_always_lock_free`. The atomic slot itself is non-copyable and non-movable.

When the active compiler and standard library expose `__cpp_lib_atomic_shared_ptr >= 201711L`, the operations forward to a private `std::atomic<std::shared_ptr<T>>` with the corresponding native behavior. This path compiles independently of the legacy atomic shared-pointer free functions, including on newer libraries that have removed them. Otherwise the operations provide member syntax over those free functions. A build can set `JH_ATOMIC_SMART_PTR_FORCE_FALLBACK` to select the compatibility path when its native specialization cannot be used; tests also exercise this switch. All translation units sharing this class must select the same path because the layouts can differ.

Use valid standard memory orders. `load` excludes release and acq_rel; `store` excludes acquire, consume, and acq_rel. CAS failure order excludes release and acq_rel and must satisfy the target standard's success/failure constraint. Single-order CAS derives the standard failure order. Use `is_lock_free()` to query the selected implementation. Refcount decrements and destruction can occur after the atomic update.

See [`<jh/atomic_ptr>`](../entry_points/atomic_ptr.md) for the aggregate header and [weak pointer slots](weak_ptr.md) for non-owning storage.

Concurrency tests should hold a loaded snapshot across a controlled store or
exchange, and should start CAS contenders from the same expected value using a
barrier. This directly checks lifetime and failed-CAS updates; high iteration
counts and ThreadSanitizer are supplementary checks.
