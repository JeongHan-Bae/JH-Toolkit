/**
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file view.h (View system umbrella header)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Aggregated interface for composable, allocation-free lazy views over `jh::sequence` types.
 *
 * This header provides unified access to the `jh::views` system:
 * - Lazy iteration over custom sequence types without copying
 * - Structured binding–friendly pairs with POD/ref-type dispatch
 * - Composable range-like views for functional-style loops
 *
 * ## Design Goals:
 * - Support pipeline-style iteration over `jh::sequence` types
 * - Preserve value semantics for PODs, reference semantics for non-PODs
 * - Avoid dependency on C++23 `std::views::zip` / `enumerate`
 * - Ensure safe element access via well-defined value/ref pairing
 *
 * ## Included Views:
 * | Component               | Description                                           |
 * |-------------------------|-------------------------------------------------------|
 * | `jh::views::enumerate`  | Lazily yields `(index, value)` pairs                  |
 * | `jh::views::zip`        | Lazily yields `(a[i], b[i])` pairs from two sequences |
 *
 * ## Header Usage:
 * | Include                  | Purpose                                         |
 * |--------------------------|-------------------------------------------------|
 * | `<jh/view>`              | ✅ Aggregated interface (recommended)           |
 * | `<jh/view.h>`            | ✅ Traditional umbrella include                 |
 * | `<jh/views/zip.h>`       | 🔧 Internal include for `zip` only              |
 * | `<jh/views/enumerate.h>` | 🔧 Internal include for `enumerate` only        |
 *
 * ## Notes:
 * - All views require the input types to satisfy `jh::sequence`
 * - All pair-like values are returned through `make_pair(...)`, which dynamically resolves to:
 *   - `jh::pod::pair<A, B>` for POD types (copyable trivially)
 *   - `jh::utils::ref_pair<A&, B&>` for non-POD types (reference-safe)
 *
 * @version 1.3.x
 * @date 2025
 */

#pragma once

#include "jh/views/zip.h"
#include "jh/views/enumerate.h"
