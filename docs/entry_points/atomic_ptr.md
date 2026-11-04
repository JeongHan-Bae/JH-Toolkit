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

Use this header for common atomic smart-pointer slot operations across supported compiler and standard-library combinations. The public headers include `<memory>` and `<atomic>` before checking `__cpp_lib_atomic_shared_ptr`. When the macro indicates native support, calls forward to the C++20 specializations; otherwise the compatibility implementation is selected. The native path remains usable when a newer library removes the legacy atomic free functions.

See the shared and weak pointer references above for the available operations, implementation paths, and memory-order rules.
