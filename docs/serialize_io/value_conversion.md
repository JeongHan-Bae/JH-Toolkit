# `jh::serio` value conversion

**Header:** `<jh/serialize_io/value_conversion.h>`

```cpp
template<jh::concepts::serializable T>
std::string serialize(const T& value);

template<jh::concepts::serializable T>
T deserialize(const std::string& text);

template<typename T>
void deserialize(const std::string& text, T& value);
```

`serialize(value)` returns a `std::string`, produced directly by the String
protocol or through an `std::ostringstream`. Both `deserialize` overloads take
a `const std::string&`. The String protocol constructs from that reference;
the Stream protocol attaches an `std::istream` to a read-only buffer borrowing
the string's characters. Thus a Stream-only round trip passes through
`ostream` &rarr; `std::string` &rarr; `istream`, with the string as the public
interchange value and no second payload copy for input. The input string
remains intact. Stream extraction
must succeed and consume the stream contents apart from trailing whitespace;
failure throws
`std::runtime_error`. A String protocol controls its own input validation.

For a Stream-only type that cannot be moved, use
`deserialize(text, existing_value)`. The destination is default constructed by
the caller and may be modified before a parse failure. The value-returning
`deserialize<T>(text)` is available when the String protocol applies or the
Stream-only `T` can be moved.

Bloom persistence constructs a String-protocol sentinel from the compile-time
signature text or extracts a Stream-protocol sentinel into a local object.
These conversion functions do not define a general binary object format.

## Borrowed results and lifetime

The `const std::string&` input keeps deserialization from moving or consuming
the caller's string. A String-protocol constructor can borrow its characters,
and the Stream adapter reads the same characters through a read-only buffer.
This is why a view-containing result can use the caller's string as its backing
storage without a second input string allocation.

When `T` contains `std::string_view` or another borrowed view, the caller keeps
the source string alive and its character storage stable for every use of the
result. The reference parameter does not extend that lifetime after the call.
`deserialize<std::string_view>` rejects a temporary `std::string` at compile
time; arbitrary user-defined view-containing types still need this caller
contract. Moving such a type may also affect its own borrowing invariants.

```cpp
const std::string text = "customer:42";
const auto key = jh::serio::deserialize<std::string_view>(text);
// Use key while text remains in scope.
```

Here `text` is created first, so `key` is destroyed first at scope exit.

Bloom persistence uses a stricter local scope: it creates the signature string
before the sentinel, constructs or extracts the sentinel in place, runs all
Hashers synchronously, and destroys the sentinel before the string. Its
[persistence reference](../core/bloom_persistence.md) describes that scope.
A Stream-protocol result must own any data needed after its local input stream
is destroyed.
