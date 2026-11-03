# `jh::concepts::serializable`

**Header:** `<jh/conceptual/serializable.h>`

`serializable<T>` accepts either of two complete, bidirectional protocols:

1. A value converts to `std::string`, and `T` can be constructed from
   `const std::string&`.
2. A value can be written to `std::ostream`, and a default constructed `T`
   can be extracted from `std::istream`.

One direction alone is insufficient. When both protocols are present,
[`jh::serio::serialize<T>` and `deserialize<T>`](../serialize_io/value_conversion.md)
use the String protocol. Deserialization always starts with a `std::string`;
the Stream path attaches an `std::istream` to a borrowed read-only view of
that text and consumes the stream.
A reconstructed value may borrow its input string;
callers must keep that string alive while using the value. A Stream-protocol
result must remain valid after the temporary input stream is destroyed. See
[borrowed results and lifetime](../serialize_io/value_conversion.md#borrowed-results-and-lifetime)
for the source-storage contract. The concept checks available operations; it
cannot infer whether a user-defined type borrows its input.
