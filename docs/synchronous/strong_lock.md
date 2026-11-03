# `jh::sync::strong_lock`

**Header:** `<jh/synchronous/strong_lock.h>`

**Namespace:** `jh::sync`

`strong_unique_lock<Mutex>` and `strong_shared_lock<Mutex>` are RAII lock
adapters. They add a `std::atomic_thread_fence(std::memory_order_seq_cst)`
after successful acquisition and before release. The exclusive adapter requires
`lock()` and `unlock()`; the shared adapter requires `lock_shared()` and
`unlock_shared()`.

| Adapter | Construction | Destruction |
|---|---|---|
| `strong_unique_lock` | `lock()`, then fence | Fence, then `unlock()` |
| `strong_shared_lock` | `lock_shared()`, then fence | Fence, then `unlock_shared()` |

The `posix_smtx_unique_lock` and `posix_smtx_shared_lock` aliases select these
adapters on Windows. On other platforms, they select `std::unique_lock` and
`std::shared_lock`. Both strong adapters are available by their own names on
all platforms.

```cpp
std::shared_mutex mutex;
{
    jh::sync::posix_smtx_unique_lock lock(mutex);
    // Exclusive access.
}
{
    jh::sync::posix_smtx_shared_lock lock(mutex);
    // Shared access.
}
```

## Synchronization contract

The adapters retain the synchronization and ownership behavior of their
underlying mutex. Their extra fences participate in C++ fence ordering and
have a runtime cost. A lock together with these fences does not supply the
required synchronization for every independent atomic operation. The
[pool platform support note](../concurrent/overview.md#pool-platform-support)
is the source for the standard mutex contract and the measured Windows
standard-library/runtime implementation gap that motivated the Windows alias
path.
