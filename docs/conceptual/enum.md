# Enum Concepts

`jh/conceptual/enum.h` defines the shared enum abstractions in
`jh::concepts`:

| Name | Meaning |
|---|---|
| `enum_type<T>` | `T` is a scoped or unscoped enumeration. |
| `scoped_enum<T>` | `T` is an `enum class`. |
| `enum_underlying_t<E>` | The integer type underlying enum `E`. |

Both `jh::meta::enum_case` and `jh::meta::expected` use these shared
definitions.
