# Dual-mode headers

The implementation lives in one header. Header-only users compile it with
their own code; static-library users link a precompiled form. Keeping the body
in one place removes the need to synchronize a header implementation with a
separate `.cpp` implementation. Precompilation lets the library build use
chosen flags, such as Release optimizations, and lets consumers reuse those
compiled objects. Runtime speed depends on the selected flags and workload.

`<jh/macros/header_begin.h>` and `<jh/macros/header_end.h>` select where a
header emits functions supplied by a static library. The guarded block can
contain ordinary functions and members of a concrete class-template
specialization such as `runtime_arr<bool>`. Primary templates and templates
with remaining type parameters stay visible in the header.

| Build setting | Guarded definitions | `JH_INLINE` |
|---|---|---|
| No mode macro | Emitted in the including translation unit | `inline` |
| `JH_HEADER_IMPL_BUILD` | Emitted in the library implementation translation unit | Empty |
| `JH_HEADER_NO_IMPL` | Supplied by the linked library | Unused for guarded definitions |

## Writing a dual-mode header

Declare ordinary functions and concrete specializations before the guarded
block. Put their definitions between the two helper headers. Keep generic
template definitions outside the block.

```cpp
#pragma once

int non_template_helper();

template<typename T>
struct sample_box { T value; };
template<>
struct sample_box<bool> { bool value() const; };

#include <jh/macros/header_begin.h>
#if JH_INTERNAL_SHOULD_DEFINE
JH_INLINE int non_template_helper() { return 42; }
JH_INLINE bool sample_box<bool>::value() const { return true; }
#endif
#include <jh/macros/header_end.h>
```

The begin header sets `JH_INTERNAL_SHOULD_DEFINE` and `JH_INLINE`. The end
header clears those temporary macros. Both helper headers are included each
time a dual-mode header needs them. The dual-mode header's `#pragma once` keeps
its selected mode consistent within a translation unit.

## Compilation units

For ordinary functions, the `.cpp` file selects implementation mode and
includes the header. `src/serio_tu.cpp` follows this pattern:

```cpp
#define JH_HEADER_IMPL_BUILD
#include "jh/serialize_io/base64.h"
```

For a class template, an instantiation file also names the specializations
to precompile. `src/immutable_str_inst.cpp` does this for both policies:

```cpp
#define JH_HEADER_IMPL_BUILD
#include "jh/core/immutable_str.h"

template class jh::immutable_str<true>;
template class jh::immutable_str<false>;
```

Both files use the definitions already present in their headers.

## Using JH-Toolkit

Link `jh::jh-toolkit` for header-only use or `jh::jh-toolkit-static` for the
static library. The static target supplies `JH_HEADER_NO_IMPL` to consumers.
Application code includes the normal entry header:

```cpp
#include <jh/immutable_str>
```

`immutable_str` keeps its template member definitions visible. Its two
non-template initialization helpers use the guarded block.

Use one build mode for a target. Configure `JH_IMMUTABLE_STR_AUTO_TRIM`
consistently across translation units that include `immutable_str`; that macro
sets the default policy and is separate from the dual-mode controls.
