# `<jh/atomic_ptr>`

## Include

```cpp
#include <jh/atomic_ptr>
```

## Entry Header

- [`jh/atomic_ptr`](../../include/jh/atomic_ptr)

## Including Modules

- [`jh::atomic::shared_ptr<T>`](../atomic/shared_ptr.md)
- [`jh::atomic::weak_ptr<T>`](../atomic/weak_ptr.md)

Use this header for common atomic smart-pointer slot operations across supported compiler and standard-library combinations. The compatibility path keeps them available on older libraries; the native path forwards the exposed operations to the C++20 specializations. A newer library can therefore use the same header after removing the legacy atomic free functions.

See the shared and weak pointer references above for the available operations, implementation paths, and memory-order rules.
