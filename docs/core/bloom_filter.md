# JH Toolkit — `jh::bloom_filter` API Reference

**Header:** `<jh/core/bloom_filter.h>`  
**Forwarding header:** `<jh/bloom_filter>`  
**Namespace:** `jh`

Fixed-layout persistence is available through the same forwarding header;
see [`jh::bloom_persistence`](bloom_persistence.md).

## Purpose

`bloom_filter` maps user-supplied hash results to a fixed set of bits. It does
not choose a hash algorithm or derive additional hashes. Each hasher is a
compile-time function object that accepts `const T&` and returns a tuple-like
group whose elements convert to `std::size_t`.
Each element must also be materializable by value through
`flatten_proxy::get<I>()`. An element exposed as an lvalue reference therefore
needs a copy constructor.

For a server-side key lookup, `std::string_view` lets the filter check a key
before a disk or database query. A `false` result lets the caller skip that
query. A `true` result still requires checking the authoritative data source.
The view's characters must remain valid throughout each filter call.
Hasher results may also borrow storage that remains valid for that call; the
filter retains only bit state.

```cpp
template<typename T, std::size_t Modulo,
         typename Allocator = jh::typed::monostate, auto... Hashers>
class bloom_filter;
```

At least one hasher is required. `Modulo` is both the hash modulus and the
fixed bit capacity. Inline storage is limited to 16 KiB of bit words
(`Modulo <= 131072`) by `jh::pod::array`. Allocator storage can be larger;
its word count must fit the platform pointer-difference range and the
allocator's `max_size()` and available resources.
After conversion to `std::size_t`, each hash value selects `hash % Modulo`.
Repeated values or values that map to
the same bit are merged during insertion. A hasher may return an empty group;
such a group contributes no bits or checks.

## Example

```cpp
#include <jh/bloom_filter>
#include <jh/pod>

#include <cstddef>
#include <string_view>
#include <tuple>

struct prefix_hashes {
    auto operator()(const std::string_view& key) const {
        const auto first = key.empty() ? std::size_t{0} :
            static_cast<std::size_t>(static_cast<unsigned char>(key.front()));
        return std::tuple{key.size(), first};
    }
};

struct suffix_hashes {
    auto operator()(const std::string_view& key) const {
        const auto first = key.empty() ? std::size_t{0} :
            static_cast<std::size_t>(static_cast<unsigned char>(key.front()));
        const auto last = key.empty() ? std::size_t{0} :
            static_cast<std::size_t>(static_cast<unsigned char>(key.back()));
        return jh::pod::make_tuple(last + 7, key.size() * 13 + first);
    }
};

int main() {
    jh::bloom_filter<std::string_view, 1024, jh::typed::monostate,
                     prefix_hashes{}, suffix_hashes{}> filter;
    filter.insert("customer:42");
    const std::string_view requested = "customer:84";
    if (filter.possibly_contains(requested)) {
        // Check the authoritative disk or database index.
    }
}
```

These hashers illustrate the tuple-like interface. Select hashers with an
appropriate distribution for the expected key set.

## Operations and guarantees

| Operation | Behavior |
|---|---|
| `insert(value)` | Calls each hasher once, combines its tuple-like results with `flatten_proxy`, plans and merges word masks using storage proportional to the hash count, then commits them under one exclusive lock. If hashing or conversion throws, no bits change. |
| `possibly_contains(value)` | Checks each result within the current hasher in order and stops at the first missing bit. Later hashers are not called after a failure. Returns `false` for definite absence and `true` for possible presence. |
| `clear()` | Resets all bits under one exclusive lock. |
| `visit_words(visitor)` | Calls `visitor(index, word)` with each logical word value under one shared lock. A callback exception releases the lock. |
| `replace_words(words)` | Validates the word count and unused high bits, then replaces all words under one exclusive lock. Invalid input leaves the filter unchanged. |

## Concurrent word-state transitions

Under the C++ `std::shared_mutex` contract, `visit_words()` holds one shared
lock from before the first word callback until after the last.
`replace_words()` validates its input first, then holds one
exclusive lock while copying every word. `possibly_contains()` holds a shared
lock through all hasher calls and bit checks. Since the exclusive lock cannot
overlap either shared lock, a custom save reads one complete word state and a
query sees the state before or after a complete replacement. This guarantee
follows from the lock scopes; it does not depend on a stress-test schedule.
The same mutex also protects the word commits in `insert()` and `clear()`.

This argument covers the filter's own word state. Callers keep the filter and
replacement buffer alive during each operation, use Hashers that are safe for
concurrent calls, and coordinate any stream object shared by multiple threads.
The visitor and Hashers finish without waiting for another operation on the
same filter.

`replace_words()` is a whole-state replacement. An insertion completed before
that replacement may be overwritten by the restored state. A service should
finish startup restoration before accepting insertions, or coordinate those
operations outside the filter.

These membership guarantees assume stable hash results and correct use of the
filter, including insertion of every key represented by the backing store.
Collisions can produce false positives. Without a `clear()` between
insertion and lookup, a successfully inserted value has no false negative.
There is no operation to remove one value or change the capacity.

If the backing store survives a process restart, restore a current filter
record or rebuild from its keys before treating `false` as permission to skip
a disk or database lookup. A new filter has zeroed bits, and a stale record
can omit keys added after its snapshot. See the
[persistence restart contract](bloom_persistence.md#restart-and-backing-store-state).

The default `jh::typed::monostate` stores the `jh::pod::array` directly inside
the filter and performs no dynamic allocation. Another allocator is rebound
to `std::uint64_t` and must support allocating, constructing, destroying, and
deallocating a fixed-length word sequence. Other types are rejected by the
template constraint. The constructor accepts an allocator instance when
needed. Both modes access words through the same internal `uint64_t*`.

The filter is neither copyable nor movable. Destroy it only after all calls
using it have ended. Concurrent insertions and clears take an exclusive lock.
Queries hold a shared lock throughout hashing and checking, so a query sees
the whole state before or after each insertion or clear. A hasher called by
`possibly_contains()` must not reenter the same filter.
Hashers must be safe for concurrent invocation when callers use the filter
concurrently; insertion computes hashes before taking the exclusive lock.

The public word operations also support caller-defined persistence. A custom
writer can stream word values from `visit_words()`; a custom reader can stage
and validate its own record before passing the resulting word sequence to
`replace_words()`. The visitor receives word values, so it does not expose an
internal word pointer. The supplied
[`bloom_persistence`](bloom_persistence.md) is one fixed-layout format built
on these operations. A custom format defines its own metadata and integrity
checks and retains the same stable-Hasher and backing-store coverage contracts.
The visitor runs while a shared lock is held; it should finish without waiting
for another operation on the same filter.
