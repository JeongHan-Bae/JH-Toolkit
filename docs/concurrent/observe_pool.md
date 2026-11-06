# `jh::observe_pool`

Header: `<jh/concurrent/observe_pool.h>`; also available through `<jh/pool>`
and `<jh/concurrency>`.

`observe_pool<T, Mutex = std::shared_mutex>` is the content-based form of
[`pointer_pool`](pointer_pool.md). It selects `jh::hash<T>` and
`std::equal_to<T>` for object identity:

```cpp
template<typename T, typename Mutex = std::shared_mutex>
using observe_pool = jh::conc::pointer_pool<
    T, jh::hash<T>, std::equal_to<T>, Mutex>;
```

It uses the same pool-specific handle and selected RW mutex as `pointer_pool`.
For the Windows MinGW synchronization context, see the
[pool platform note](overview.md#pool-platform-support).

`T` must support the project's hash deduction and content equality. Equal
objects must have equal hashes, and their identity fields must remain stable.
Objects may be non-copyable and non-movable.

```cpp
jh::observe_pool<jh::immutable_str<>> pool;
auto first = pool.acquire("name");
auto second = pool.acquire("name");
// first and second are handles to one canonical object.
```

`acquire()` returns `observe_pool<T, Mutex>::ptr`. This handle uses
the selected `Mutex` for copying and release. The index stores cached hashes
and owns its nodes while handles exist. The last handle release destroys the
node and leaves a reusable slot.
Handles remain valid after the pool is
cleared or destroyed.

A caller may supply a mutex with verified cross-thread behavior:

```cpp
jh::observe_pool<MyType, MyRwMutex> pool;
```

The default is `std::shared_mutex`. The mutex must satisfy
`jh::concepts::rw_mutex_like`. Its runtime synchronization behavior remains
part of the caller's platform choice. See the
[`pointer_pool` reference](pointer_pool.md) for lookup, capacity, and handle
operations.

The former weak-pointer Hash/Eq selection and `std::shared_ptr<T>` return type
are breaking API changes. Code that declares a handle type explicitly should
use `observe_pool<T, Mutex>::ptr`.
