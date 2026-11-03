# `jh::bloom_persistence`

**Header:** `<jh/core/bloom_persistence.h>`  
**Aggregate header:** `<jh/bloom_filter>`

`bloom_persistence<Signature, BloomFilter>` stores a Bloom Filter in a fixed
binary record. `Signature` uses the existing `jh::meta::TStr` NTTP. The
filter's type determines bit capacity, word count, Hasher order, and total
hash count at compile time. `T` must satisfy
[`jh::concepts::serializable`](../conceptual/serializable.md), and the
signature must deserialize into a valid sentinel value of type `T`.
The template rejects types that are not an exact `jh::bloom_filter` instantiation.
This helper is optional: applications can use the filter's public
[`visit_words()` and `replace_words()`](bloom_filter.md#operations-and-guarantees)
operations to define another persistence format.

```cpp
using persistence = jh::bloom_persistence<"hello", decltype(filter)>;
static_assert(persistence::serialized_size ==
              8 * (3 + persistence::signature_words +
                   persistence::hash_count + persistence::word_count));

persistence::serialize(output, filter);
persistence::deserialize(input, filter);

// These functions deduce the full filter type from the argument.
jh::serialize_bloom<"hello">(output, filter);
jh::deserialize_bloom<"hello">(input, filter);
```

## Binary layout

All integer fields are unsigned 64-bit little-endian values. The record is:

| Field | Bytes |
|---|---:|
| Actual signature byte count | 8 |
| Signature bytes followed by zero padding | `8 * signature_words` |
| Raw hash value count | 8 |
| Every raw sentinel hash, in Hasher and tuple order | `8 * hash_count` |
| Bit capacity | 8 |
| Logical bit words, in index order | `8 * word_count` |

The implicit `TStr` terminator is excluded. Embedded zero bytes within the
signature count as valid data. `serialized_size` is exactly
`8 * (3 + signature_words + hash_count + word_count)`; all counts and offsets
are determined at compile time. The format requires `std::size_t` to fit in
`std::uint64_t`, checks every field count and the final size for overflow,
and never writes a C++ object layout, pointer, or allocator. Inline and
allocated filters have identical records.
The signature serves both as a record identifier and as the text used to
construct the sentinel whose raw hashes are stored and later compared.

The signature is first held as a `std::string`. The String protocol constructs
`T` from its `const std::string&`. Otherwise, the text initializes an input
stream borrowing its characters and `operator>>` extracts into a
default-constructed `T`; extraction
consumes the stream, while the signature string remains intact. This supports
immovable Stream-protocol sentinels. Each Hasher is called once for the
sentinel. Raw results are converted directly to `std::uint64_t`; the record
never stores reduced bit indices. The signature string stays alive until all
sentinel hashes have been read, so a String-protocol sentinel may borrow it
during those calls. The sentinel is constructed in place, never moved, and
destroyed before its signature string. This keeps `string_view` members in
view-like sentinel types valid throughout synchronous hasher evaluation when
those members borrow the signature string.

## Validation and failure

Deserialization checks the signature length and bytes, zero padding, hash
count, every recomputed sentinel hash, bit capacity, complete word data, and
unused high bits in the final word. It restores the bit array under one
exclusive lock only after every check succeeds. Any mismatch or short read
throws `std::runtime_error` and leaves the target filter unchanged. Reading
one record does not require EOF and leaves later stream data unread.

Serialization writes the word sequence while holding a shared lock, so the
record is coherent relative to concurrent filter changes. Deserialization
stages the compile-time fixed word count in a temporary allocated array before
the final commit; no capacity-sized stack array or variable-length record is
used. Stream write failures throw; a partially written output record may
remain.
The built-in helper uses the same public word operations as a custom
persistence implementation; the
[lock-scope argument](bloom_filter.md#concurrent-word-state-transitions)
explains why word snapshots and replacements cannot interleave.
Deserialization replaces the whole filter state. Insertions completed before
its final commit may be absent from the restored record, so services should
complete startup restoration before accepting new inserts or coordinate both
operations externally.

## Restart and backing-store state

When the backing service persists keys, the saved filter state must cover
every key still present after restart. Restore a current record before using
`possibly_contains()` to skip a disk or database lookup. If restoration fails
or the record predates durable insertions, rebuild the filter from the
authoritative keys. The service can also query that source directly while the
filter is being rebuilt.

A newly constructed filter has zeroed bits. For keys whose Hashers produce
bits, it reports `false` until those keys are inserted or restored, even if
they still exist in the backing store. Keeping the Bloom record synchronized
with durable insertions is part of the service's persistence contract.

**Hash stability is the caller's responsibility.** Successful serialization
does not prove that Hashers are stable across runs, processes, or platforms.
The sentinel detects changes only for that selected input; it cannot prove
stable results for every possible input. Choose stable Hashers before inserting
or writing persisted data. Do not use a Hasher whose persisted results depend
on unspecified `std::hash` behavior. A matching sentinel only means this
record passed the checks that the format performs.

The record stores semantic fields with fixed encoding. It has no CRC or other
payload integrity check. Callers that need CRC compute and verify it around
the record.
