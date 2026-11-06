# `jh::conc::pointer_pool`

Header: `<jh/concurrent/pointer_pool.h>`

`pointer_pool<T, Hash, Eq, Mutex = std::shared_mutex>` interns objects by
content. Each object is constructed in its own heap node, so `T` may be
non-copyable and non-movable. `acquire()` returns `pointer_pool::ptr`, a
reference-counted handle. The pool adds no independent object reference.

The former weak-pointer design used `weak_ptr::lock()` and a `shared_ptr`
control block whose synchronization could not be selected through `Mutex`.
The current handle brings index and lifetime operations under the selected RW
mutex. On a Windows MinGW combination where the default `std::shared_mutex`
is unreliable, provide a mutex with verified cross-thread behavior. The
[pool platform note](overview.md#pool-platform-support) records the observed
gap and the remaining mutex requirement.

## Identity and lookup

`Hash` receives `const T&` and returns `std::size_t`. `Eq` compares two
`const T&` values. Equal objects must have equal hashes, and the fields used
by both callbacks must remain stable while an object is observed.

For each `acquire(args...)` call, the pool:

1. Constructs a candidate node from `args...`.
2. Computes its hash once before acquiring the pool mutex.
3. Finds the hash range in a sorted vector of slots.
4. Compares objects only when cached hashes match.
5. Returns the existing node's handle or adopts the candidate node.

Each occupied slot holds a cached hash and a `unique_ptr` to a stable node.
The last handle release destroys its node immediately and leaves a reusable
slot. A second sorted vector records every empty slot position. For a new hash,
insertion uses binary search to find the nearest empty slot and moves only the
records between that slot and the sorted insertion position. Lookup takes
`O(log N + C)` comparisons, where `C` is the number of entries with the same
hash. Insertion may move up to `O(N)` slot records when no nearby slot is free.

## Handles and lifetime

```cpp
using pool_type = jh::conc::pointer_pool<T, Hash, Eq, Mutex>;
pool_type pool;
pool_type::ptr handle = pool.acquire(constructor_args...);
```

A handle supports copy, move, `reset()`, `get()`, `operator*`, `operator->`,
`use_count()`, and comparison with another handle or `nullptr`. Copying and
releasing a handle update reference counts through the configured `Mutex`.
The last release destroys its node immediately and leaves a reusable index slot.

Handles remain valid after the pool is moved, cleared, or destroyed. `clear()`
starts a new index. Existing handles retain the previous index until their
objects are released. Concurrent access to one handle object requires external
synchronization.

## Capacity and maintenance

`size()` counts currently observed live objects. `capacity()` reports the
number of index slots. The default is 16; a full live index doubles on
insertion. Expansion and `cleanup()` distribute free slots through the sorted
index. `cleanup_shrink()` also halves a slot count above 16 when
live occupancy is at most one quarter, down to 16. `clear()` starts a new
16-slot index.

## Synchronization

`Mutex` must satisfy `jh::concepts::rw_mutex_like` and provide working
cross-thread synchronization on the target platform. The pool uses instances
of this type for index changes, equality comparison, handle reference counts,
and state changes such as `clear()`. Hashing a new candidate occurs before the
lock. Equality callbacks run under the lock and must not reenter the same pool.

The default is `std::shared_mutex`. The chosen implementation must supply
the required cross-thread synchronization on the target platform.

## Migration

The former API returned `std::shared_ptr<T>` and accepted Hash/Eq functors for
`std::weak_ptr<T>`. The current API returns `pointer_pool::ptr` and applies
Hash/Eq directly to `T`. Code with explicit handle types or custom functors
must be updated.
