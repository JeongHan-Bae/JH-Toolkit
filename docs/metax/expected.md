# `jh::meta::expected`

`jh::meta::expected<T, E>` carries either a value or a scoped enum error. Its
layout stores `T value` and `jh::pod::optional<jh::concepts::enum_underlying_t<E>> ec`.
An empty `ec` means success.

```cpp
enum class parse_error : std::uint8_t {
    invalid_input,
    overflow
};

using parse_result = jh::meta::expected<double, parse_error>;

parse_result parse_number(bool valid) {
    if (!valid)
        return jh::meta::unexpected(parse_error::invalid_input);
    return 42.0;
}

auto result = parse_number(input_is_valid);
if (result) {
    use(result.value());
} else {
    handle(result.error());
}
```

`unexpected(E)` uses class template argument deduction, so callers do not need
to spell `unexpected<E>`. `expected<T, E>::rebind<U>` names
`expected<U, E>` while retaining the same error domain.

Use brace value-initialization for a default success result (`expected<T, E> r{};`),
consistent with the POD-like default-initialization rules of `jh::pod` types.

The class provides default, value, and `unexpected` construction, defaulted
destruction and assignment, `operator->`, `operator*`, checked `value()` and
`error()` accessors, and `value_or()` / `error_or()` fallbacks. Calling
`value()` on an error or `error()` on a value terminates; check the state first.
The observers are `noexcept`, and POD-like values support constant evaluation.

`T` may be a non-POD object type. When `T` satisfies
`jh::pod::cv_free_pod_like`, both `expected<T, E>` and `unexpected<E>` satisfy
`jh::pod::pod_like`. Constructing a failure also default-constructs `T`, so
that conversion is available when `T` is default-constructible.

Enum concepts and the `enum_underlying_t` alias are defined in
[`jh/conceptual/enum.h`](../conceptual/enum.md).
