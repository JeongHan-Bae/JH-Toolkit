# `jh::atomic::weak_ptr<T>`

## Include

```cpp
#include <jh/atomic/weak_ptr.h>
```

The class stores a `std::weak_ptr<T>` for concurrent access. Construct it from a weak or shared handle, use `store()` to publish a weak handle, `load()` to obtain a weak snapshot, and `exchange()` to replace one. Call `lock()` on the loaded snapshot to acquire shared ownership. It also provides value assignment and conversion, `value_type`, `is_lock_free()`, and `is_always_lock_free`. The atomic slot itself is non-copyable and non-movable.

When the active compiler and standard library expose `__cpp_lib_atomic_shared_ptr >= 201711L`, the operations forward to a private `std::atomic<std::weak_ptr<T>>` with the corresponding native behavior. Otherwise the slot uses one mutex to synchronize access to its stored weak handle. The compatibility path reports `false` from both lock-free queries. Use valid standard load and store memory orders; mutex synchronization may be stronger than a requested relaxed order.

See [`<jh/atomic_ptr>`](../entry_points/atomic_ptr.md) for the aggregate header and [shared pointer slots](shared_ptr.md) for owning storage.

Concurrency tests should coordinate a weak snapshot with removal of the last
strong owner, then check expiration and safe locking. High iteration counts
alone do not establish these lifetime guarantees.
