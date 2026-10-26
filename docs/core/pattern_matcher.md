# `jh::pattern_matcher`

**Headers:** `<jh/pattern_matcher>` or `<jh/core/pattern_matcher.h>`  
**Namespace:** `jh`; rule and result types are in `jh::pattern`.

`jh::pattern_matcher<T, BacktrackLimit, Rules...>` matches complete runtime input against immutable patterns. Its builder, `jh::pattern_matcher_builder<T, BacktrackLimit, Rules...>`, collects patterns before building a compressed radix index. The implementation is in `jh::radix`; applications use the `jh` and `jh::pattern` names.

A pattern is a sequence of `(literal, rule)` pairs. `jh::pattern::none` ends a pattern after its literal. `jh::pattern::all` captures the entire remaining input, including an empty suffix, and must be last. `jh::pattern::rule_tag<Rule{}>` applies a custom rule after its literal. A rule has a compile-time `rank` and `match(std::string_view)` returning `std::pair<bool, std::size_t>`; the length must not exceed the supplied input. Distinct custom rules must have distinct ranks. An empty pattern, a duplicate pattern, or a terminal rule followed by another pair is rejected during `build()` with `std::invalid_argument`.

The matcher prefers a complete literal match, then custom rules in ascending `rank` order (smaller rank first), then catch-all. A failed descended branch can backtrack to another viable branch. `BacktrackLimit` counts those switches, not every failed prefix or rule check. Queries return `jh::pattern::match_error::unmatched` or `backtrack_limit_exceeded` when appropriate. The builder is mutable and not thread-safe; the built matcher has no rule modification API and supports repeated read-only queries.

## Captures and ownership

Both `match(input)` and `match_ref(input)` return a `jh::pattern::match_result<...>` whose success value is a pair: `.first` is `jh::runtime_arr<std::string_view>` and `.second` is the matched value or a `const T*`. The number of captures is decided at runtime; use `.first.size()`. Exact patterns return a valid empty array with size zero. Catch-all may produce one empty capture. The array is move-only, fixed in length after construction, and its elements remain assignable. The search uses a temporary vector; conversion moves its elements to a separately allocated `runtime_arr` buffer. It does not transfer the vector's buffer.

**Capture views never own input characters.** The `runtime_arr` owns only its `string_view` elements. Moving the result does not prolong the input lifetime. The original input data must remain alive, accessible, and free of modifications or reallocations that invalidate the views. If a capture must survive the input, copy it while the input is still valid:

```cpp
std::string owned_capture(result.value().first[0]);
```

The array may remain alive after input destruction, but its views may then dangle.

## Value access

`T` must satisfy `std::movable`. `match(input)` copies the value and is available only when `T` is copy constructible and default initializable. The latter constraint comes from `jh::meta::expected`'s error construction. `match_ref(input)` works for any legal `T`, including move-only values, and returns a pointer into the matcher's value storage. It neither copies nor transfers the value. Do not use the pointer after matcher destruction or move assignment. After moving a matcher, query its new owner to obtain fresh pointers.

See [the example](../../examples/example_pattern_matcher.cpp) for construction and matching.
